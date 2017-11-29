/*
 * Copyright (C) 2012-2017 Vasily Tarasov
 * Copyright (C) 2012-2017 Erez Zadok
 * Copyright (c) 2016-2017 Vinothkumar Raja
 * Copyright (c) 2017-2017 Nidhi Panpalia
 * Copyright (c) 2012-2017 Stony Brook University
 * Copyright (c) 2012-2017 The Research Foundation for SUNY
 * This file is released under the GPL.
 */

#ifndef DM_DEDUP_GC_H
#define DM_DEDUP_GC_H

extern int gc_process(struct dedup_config *dc);
extern int gc_blocks_estimate(struct dedup_config *dc);

#endif /* DM_DEDUP_GC_H */
