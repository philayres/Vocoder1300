/*
Copyright (c) 2003-2010, Mark Borgerding

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the author nor the names of any contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "kiss_fft.h"
#include "kiss_fftr.h"

kiss_fftr_cfg kiss_fftr_alloc(int nfft, int inverse_fft, void * mem, size_t * lenmem) {
    int i;
    kiss_fftr_cfg st = NULL;
    size_t subsize, memneeded;

    if (nfft & 1) {
        return NULL;
    }
    
    nfft >>= 1;

    kiss_fft_alloc(nfft, inverse_fft, NULL, &subsize);
    memneeded = sizeof (struct kiss_fftr_state) + subsize + sizeof(COMP) * (nfft * 3 / 2);

    if (lenmem == NULL) {
        st = (kiss_fftr_cfg) malloc(memneeded);
    } else {
        if (*lenmem >= memneeded) {
            st = (kiss_fftr_cfg) mem;
        }
        
        *lenmem = memneeded;
    }
    
    if (!st) {
        return NULL;
    }

    st->substate = (kiss_fft_cfg) (st + 1);
    st->tmpbuf = (COMP *) (((char *) st->substate) + subsize);
    st->super_twiddles = st->tmpbuf + nfft;
    kiss_fft_alloc(nfft, inverse_fft, st->substate, &subsize);

    for (i = 0; i < nfft / 2; ++i) {
        float phase = -M_PI * ((float) (i + 1) / nfft + .5f);
        
        if (inverse_fft) {
            phase *= -1.0f;
        }
        
        kf_cexp(st->super_twiddles + i, phase);
    }
    
    return st;
}

void kiss_fftr(kiss_fftr_cfg st, const float *timedata, COMP *freqdata) {
    int k, ncfft;
    COMP fpnk, fpk, f1k, f2k, tw, tdc;

    ncfft = st->substate->nfft;
    kiss_fft(st->substate, (const COMP*) timedata, st->tmpbuf);

    tdc.real = st->tmpbuf[0].real;
    tdc.imag = st->tmpbuf[0].imag;

    freqdata[0].real = tdc.real + tdc.imag;
    freqdata[ncfft].real = tdc.real - tdc.imag;
    freqdata[ncfft].imag = freqdata[0].imag = 0.0f;

    for (k = 1; k <= ncfft / 2; ++k) {
        fpk = st->tmpbuf[k];
        fpnk.real = st->tmpbuf[ncfft - k].real;
        fpnk.imag = -st->tmpbuf[ncfft - k].imag;

        C_ADD(f1k, fpk, fpnk);
        C_SUB(f2k, fpk, fpnk);
        C_MUL(tw, f2k, st->super_twiddles[k - 1]);

        freqdata[k].real = HALF_OF(f1k.real + tw.real);
        freqdata[k].imag = HALF_OF(f1k.imag + tw.imag);
        freqdata[ncfft - k].real = HALF_OF(f1k.real - tw.real);
        freqdata[ncfft - k].imag = HALF_OF(tw.imag - f1k.imag);
    }
}

void kiss_fftri(kiss_fftr_cfg st, const COMP *freqdata, float *timedata) {
    int k, ncfft;

    ncfft = st->substate->nfft;

    st->tmpbuf[0].real = freqdata[0].real + freqdata[ncfft].real;
    st->tmpbuf[0].imag = freqdata[0].real - freqdata[ncfft].real;

    for (k = 1; k <= ncfft / 2; ++k) {
        COMP fk, fnkc, fek, fok, tmp;
        fk = freqdata[k];
        fnkc.real = freqdata[ncfft - k].real;
        fnkc.imag = -freqdata[ncfft - k].imag;

        C_ADD(fek, fk, fnkc);
        C_SUB(tmp, fk, fnkc);
        C_MUL(fok, tmp, st->super_twiddles[k - 1]);
        C_ADD(st->tmpbuf[k], fek, fok);
        C_SUB(st->tmpbuf[ncfft - k], fek, fok);

        st->tmpbuf[ncfft - k].imag *= -1.0f;
    }

    kiss_fft(st->substate, st->tmpbuf, (COMP *) timedata);
}
