/*
 * Copyright (C) 1993-2016 David Rowe
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */
#ifndef __NLP__
#define __NLP__

#ifdef __cplusplus
extern "C"
{
#endif

#include "codec2_fft.h"

void nlp_create(int);
float nlp(codec2_fft_cfg, float [], float *, float *);

#define PMAX_M      600		/* maximum NLP analysis window size     */
#define COEFF       0.95	/* notch filter parameter               */
#define DEC         5		/* decimation factor                    */
#define CNLP        0.3	        /* post processor constant              */
#define NLP_NTAP    48	        /* Decimation LPF order */

#ifdef __cplusplus
}
#endif
#endif
