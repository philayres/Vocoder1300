/*
 * Copyright (C) 1993-2016 David Rowe, danilo
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */

#include "codec2_fft.h"

codec2_fft_cfg codec2_fft_alloc(int nfft, int inverse_fft, void* mem, size_t* lenmem) {
    return kiss_fft_alloc(nfft, inverse_fft, mem, lenmem);
}

codec2_fftr_cfg codec2_fftr_alloc(int nfft, int inverse_fft, void* mem, size_t* lenmem) {
    return kiss_fftr_alloc(nfft, inverse_fft, mem, lenmem);
}

void codec2_fft_free(codec2_fft_cfg cfg) {
    free(cfg);
}

void codec2_fftr_free(codec2_fftr_cfg cfg) {
    free(cfg);
}

void codec2_fft_inplace(codec2_fft_cfg cfg, COMP *inout) {
    COMP in[FFT_SIZE];

    if (cfg->nfft <= FFT_SIZE) {
        memcpy(in, inout, cfg->nfft * sizeof (COMP));
        kiss_fft(cfg, in, (COMP *) inout);
    } else {
        kiss_fft(cfg, (COMP *) inout, (COMP *) inout);
    }
}
