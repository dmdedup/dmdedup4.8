/*
 * Copyright (C) 2012-2017 Vasily Tarasov
 * Copyright (C) 2012-2017 Erez Zadok
 * Copyright (c) 2016-2017 Vinothkumar Raja
 * Copyright (c) 2017-2017 Nidhi Panpalia
 * Copyright (c) 2012-2017 Stony Brook University
 * Copyright (c) 2012-2017 The Research Foundation for SUNY
 * This file is released under the GPL.
 */

#include "dm-dedup-target.h"
#include "dm-dedup-gc.h"
#include "dm-dedup-hash.h"
#include "dm-dedup-backend.h"
#include "dm-dedup-kvstore.h"

static int gc_process_cleanup_hash_pbn(void *key, int32_t ksize, void *value,
			    s32 vsize, void *data)
{
	int r = 0;
	u64 pbn_val = 0;
	struct hash_pbn_value hashpbn_value = *((struct hash_pbn_value *)value);
	struct dedup_config *dc = (struct dedup_config *)data;

	BUG_ON(!data);

	pbn_val = hashpbn_value.pbn;

	if (dc->mdops->get_refcount(dc->bmd, pbn_val) == 1) {
		r = dc->kvs_hash_pbn->kvs_delete(dc->kvs_hash_pbn,
							key, ksize);
		if (r < 0)
			goto out;
		r = dc->mdops->dec_refcount(dc->bmd, pbn_val);
		if (r < 0)
			goto out_dec_refcount;

		dc->physical_block_counter -= 1;
		dc->gc_blocks_cleaned++;
	}

	goto out;

out_dec_refcount:
	dc->kvs_hash_pbn->kvs_insert(dc->kvs_hash_pbn, key,
			ksize, (void *)&hashpbn_value,
			sizeof(hashpbn_value));
out:
	return r;
}

static int gc_process(struct dedup_config *dc)
{
	int err = 0;
	
	/* Set garbage collection status flag to true */
	dc->gc_status = true;

	BUG_ON(!dc);

	/* Cleanup hashes if the refcount of block == 1 */
	err = dc->kvs_hash_pbn->kvs_iterate(dc->kvs_hash_pbn,
			&gc_process_cleanup_hash_pbn, (void *)dc);

	/* Set garbage collection status flag to false */
	dc->gc_status = false;

	return err;
}

static void issue_work_gc(struct work_struct *ws)
{
        struct gc_work *data = container_of(ws, struct gc_work, worker);
        struct dedup_config *dc = (struct dedup_config *)data->dc;

        mempool_free(data, dc->gc_work_pool);

        gc_process(dc);
}

void dedup_defer_gc(struct dedup_config *dc)
{
        struct gc_work *data;

        data = mempool_alloc(dc->gc_work_pool, GFP_NOIO);
        if (!data) {
                /* XXX: Decide whether to pass error 
 *  		 * or silently pass if unable to perform
 *  		  		 * garbage collection 
 *  		  		  		 */ 	
                return;
        }

        data->dc = dc;

        INIT_WORK(&(data->worker), issue_work_gc);

        queue_work(dc->workqueue, &(data->worker));
}

static int gc_blocks_estimate_count(void *key, int32_t ksize, void *value,
                            int32_t vsize, void *data)
{
        struct dedup_config *dc = (struct dedup_config *)data;
        dc->gc_blocks_estimate++;
        return 0;
}

int gc_blocks_estimate(struct dedup_config *dc)
{
        int err = 0;

        BUG_ON(!dc);

        /* Count if the refcount of block == 1 */
        err = dc->kvs_hash_pbn->kvs_iterate(dc->kvs_hash_pbn,
                        &gc_blocks_estimate_count, (void *)dc);

        return err;
}
