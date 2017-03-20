/*
Copyright (c) 2003-2010, Mark Borgerding

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the author nor the names of any contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef KISS_FFT_H
#define KISS_FFT_H

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "defines.h"
#include "comp.h"

#ifdef __cplusplus
extern "C" {
#endif

struct kiss_fft_state {
    int nfft;
    int inverse;
    int factors[64];
    COMP twiddles[1];
};

typedef struct kiss_fft_state* kiss_fft_cfg;

#define S_MUL(a,b) ( (a)*(b) )

#define C_MUL(m,a,b) \
    do{ (m).real = (a).real*(b).real - (a).imag*(b).imag;\
        (m).imag = (a).real*(b).imag + (a).imag*(b).real; }while(0)

#define C_MULBYSCALAR( c, s ) \
    do{ (c).real *= (s);\
        (c).imag *= (s); }while(0)

#define  C_ADD( res, a,b)\
    do { \
	    (res).real=(a).real+(b).real;  (res).imag=(a).imag+(b).imag; \
    }while(0)

#define  C_SUB( res, a,b)\
    do { \
	    (res).real=(a).real-(b).real;  (res).imag=(a).imag-(b).imag; \
    }while(0)

#define C_ADDTO( res , a)\
    do { \
	    (res).real += (a).real;  (res).imag += (a).imag;\
    }while(0)

#define C_SUBFROM( res , a)\
    do {\
	    (res).real -= (a).real;  (res).imag -= (a).imag; \
    }while(0)

#define HALF_OF(x) ((x)*.5)

#define  kf_cexp(x,phase) \
	do{ \
		(x)->real = cosf(phase);\
		(x)->imag = sinf(phase);\
	}while(0)

kiss_fft_cfg kiss_fft_alloc(int, int, void *, size_t *);
void kiss_fft(kiss_fft_cfg, const COMP *, COMP *);
void kiss_fft_stride(kiss_fft_cfg, const COMP *, COMP *, int);
int kiss_fft_next_fast_size(int);

#ifdef __cplusplus
}
#endif

#endif