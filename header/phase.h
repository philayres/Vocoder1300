/*
 * Copyright (C) 1993-2016 David Rowe
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */
#ifndef __PHASE__
#define __PHASE__

#ifdef __cplusplus
extern "C"
{
#endif

#define CODEC2_RAND_MAX 32767

int codec2_rand(void);
void phase_synth_zero_order(MODEL *, float *, COMP []);

#ifdef __cplusplus
}
#endif
#endif
