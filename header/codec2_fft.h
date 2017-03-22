/*
 * Copyright (C) 1993-2016 David Rowe, danilo
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */

#ifndef CODEC2_FFT_H
#define CODEC2_FFT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "defines.h"
#include "comp.h"
#include "kiss_fft.h"
#include "kiss_fftr.h"

typedef kiss_fftr_cfg codec2_fftr_cfg;
typedef kiss_fft_cfg codec2_fft_cfg;

static inline void codec2_fftr(codec2_fftr_cfg cfg, float *in, COMP *out)
{
    kiss_fftr(cfg, in, (COMP *) out);
}

static inline void codec2_fftri(codec2_fftr_cfg cfg, COMP *in, float *out)
{
    kiss_fftri(cfg, in, out);
}

codec2_fft_cfg codec2_fft_alloc(int, int, void *, size_t *);
codec2_fftr_cfg codec2_fftr_alloc(int, int, void *, size_t *);
void codec2_fft_free(codec2_fft_cfg);
void codec2_fftr_free(codec2_fftr_cfg);

static inline void codec2_fft(codec2_fft_cfg cfg, COMP *in, COMP *out)
{
    kiss_fft(cfg, in, out);
}

void codec2_fft_inplace(codec2_fft_cfg, COMP *);

#ifdef __cplusplus
}
#endif
#endif

