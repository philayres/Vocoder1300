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

static void kf_bfly2(
        COMP * Fout,
        const size_t fstride,
        const kiss_fft_cfg st,
        int m
        ) {
    COMP * Fout2;
    COMP * tw1 = st->twiddles;
    COMP t;
    Fout2 = Fout + m;
    do {
        C_MUL(t, *Fout2, *tw1);
        tw1 += fstride;
        C_SUB(*Fout2, *Fout, t);
        C_ADDTO(*Fout, t);
        ++Fout2;
        ++Fout;
    } while (--m);
}

static void kf_bfly4(
        COMP * Fout,
        const size_t fstride,
        const kiss_fft_cfg st,
        const size_t m
        ) {
    COMP *tw1, *tw2, *tw3;
    COMP scratch[6];
    size_t k = m;
    const size_t m2 = 2 * m;
    const size_t m3 = 3 * m;


    tw3 = tw2 = tw1 = st->twiddles;

    do {
        C_MUL(scratch[0], Fout[m], *tw1);
        C_MUL(scratch[1], Fout[m2], *tw2);
        C_MUL(scratch[2], Fout[m3], *tw3);

        C_SUB(scratch[5], *Fout, scratch[1]);
        C_ADDTO(*Fout, scratch[1]);
        C_ADD(scratch[3], scratch[0], scratch[2]);
        C_SUB(scratch[4], scratch[0], scratch[2]);
        C_SUB(Fout[m2], *Fout, scratch[3]);
        tw1 += fstride;
        tw2 += fstride * 2;
        tw3 += fstride * 3;
        C_ADDTO(*Fout, scratch[3]);

        if (st->inverse) {
            Fout[m].real = scratch[5].real - scratch[4].imag;
            Fout[m].imag = scratch[5].imag + scratch[4].real;
            Fout[m3].real = scratch[5].real + scratch[4].imag;
            Fout[m3].imag = scratch[5].imag - scratch[4].real;
        } else {
            Fout[m].real = scratch[5].real + scratch[4].imag;
            Fout[m].imag = scratch[5].imag - scratch[4].real;
            Fout[m3].real = scratch[5].real - scratch[4].imag;
            Fout[m3].imag = scratch[5].imag + scratch[4].real;
        }
        ++Fout;
    } while (--k);
}

static void kf_bfly3(
        COMP *Fout,
        const size_t fstride,
        const kiss_fft_cfg st,
        size_t m
        ) {
    size_t k = m;
    const size_t m2 = 2 * m;
    COMP *tw1, *tw2;
    COMP scratch[5];
    COMP epi3;
    epi3 = st->twiddles[fstride * m];

    tw1 = tw2 = st->twiddles;

    do {
        C_MUL(scratch[1], Fout[m], *tw1);
        C_MUL(scratch[2], Fout[m2], *tw2);

        C_ADD(scratch[3], scratch[1], scratch[2]);
        C_SUB(scratch[0], scratch[1], scratch[2]);
        tw1 += fstride;
        tw2 += fstride * 2;

        Fout[m].real = Fout->real - HALF_OF(scratch[3].real);
        Fout[m].imag = Fout->imag - HALF_OF(scratch[3].imag);

        C_MULBYSCALAR(scratch[0], epi3.imag);

        C_ADDTO(*Fout, scratch[3]);

        Fout[m2].real = Fout[m].real + scratch[0].imag;
        Fout[m2].imag = Fout[m].imag - scratch[0].real;

        Fout[m].real -= scratch[0].imag;
        Fout[m].imag += scratch[0].real;

        ++Fout;
    } while (--k);
}

static void kf_bfly5(
        COMP * Fout,
        const size_t fstride,
        const kiss_fft_cfg st,
        int m
        ) {
    COMP *Fout0, *Fout1, *Fout2, *Fout3, *Fout4;
    COMP scratch[13];
    COMP *twiddles = st->twiddles;
    COMP *tw;
    int u;

    COMP ya = twiddles[fstride * m];
    COMP yb = twiddles[fstride * 2 * m];

    Fout0 = Fout;
    Fout1 = Fout0 + m;
    Fout2 = Fout0 + 2 * m;
    Fout3 = Fout0 + 3 * m;
    Fout4 = Fout0 + 4 * m;

    tw = st->twiddles;
    for (u = 0; u < m; ++u) {
        scratch[0] = *Fout0;

        C_MUL(scratch[1], *Fout1, tw[u * fstride]);
        C_MUL(scratch[2], *Fout2, tw[2 * u * fstride]);
        C_MUL(scratch[3], *Fout3, tw[3 * u * fstride]);
        C_MUL(scratch[4], *Fout4, tw[4 * u * fstride]);

        C_ADD(scratch[7], scratch[1], scratch[4]);
        C_SUB(scratch[10], scratch[1], scratch[4]);
        C_ADD(scratch[8], scratch[2], scratch[3]);
        C_SUB(scratch[9], scratch[2], scratch[3]);

        Fout0->real += scratch[7].real + scratch[8].real;
        Fout0->imag += scratch[7].imag + scratch[8].imag;

        scratch[5].real = scratch[0].real + S_MUL(scratch[7].real, ya.real) + S_MUL(scratch[8].real, yb.real);
        scratch[5].imag = scratch[0].imag + S_MUL(scratch[7].imag, ya.real) + S_MUL(scratch[8].imag, yb.real);

        scratch[6].real = S_MUL(scratch[10].imag, ya.imag) + S_MUL(scratch[9].imag, yb.imag);
        scratch[6].imag = -S_MUL(scratch[10].real, ya.imag) - S_MUL(scratch[9].real, yb.imag);

        C_SUB(*Fout1, scratch[5], scratch[6]);
        C_ADD(*Fout4, scratch[5], scratch[6]);

        scratch[11].real = scratch[0].real + S_MUL(scratch[7].real, yb.real) + S_MUL(scratch[8].real, ya.real);
        scratch[11].imag = scratch[0].imag + S_MUL(scratch[7].imag, yb.real) + S_MUL(scratch[8].imag, ya.real);
        scratch[12].real = -S_MUL(scratch[10].imag, yb.imag) + S_MUL(scratch[9].imag, ya.imag);
        scratch[12].imag = S_MUL(scratch[10].real, yb.imag) - S_MUL(scratch[9].real, ya.imag);

        C_ADD(*Fout2, scratch[11], scratch[12]);
        C_SUB(*Fout3, scratch[11], scratch[12]);

        ++Fout0;
        ++Fout1;
        ++Fout2;
        ++Fout3;
        ++Fout4;
    }
}

static void kf_bfly_generic(
        COMP * Fout,
        const size_t fstride,
        const kiss_fft_cfg st,
        int m,
        int p
        ) {
    COMP *twiddles = st->twiddles;
    COMP t;
    int Norig = st->nfft;
    int u, k, q1, q;

    COMP *scratch = (COMP*) malloc(sizeof (COMP) * p);

    for (u = 0; u < m; ++u) {
        k = u;
        for (q1 = 0; q1 < p; ++q1) {
            scratch[q1] = Fout[ k ];
            k += m;
        }

        k = u;
        for (q1 = 0; q1 < p; ++q1) {
            int twidx = 0;
            Fout[ k ] = scratch[0];
            for (q = 1; q < p; ++q) {
                twidx += fstride * k;
                if (twidx >= Norig)
                    twidx -= Norig;
                C_MUL(t, scratch[q], twiddles[twidx]);
                C_ADDTO(Fout[ k ], t);
            }
            k += m;
        }
    }

    free(scratch);
}

static void kf_work(
        COMP * Fout,
        const COMP * f,
        const size_t fstride,
        int in_stride,
        int * factors,
        const kiss_fft_cfg st
        ) {
    COMP * Fout_beg = Fout;
    const int p = *factors++; /* the radix  */
    const int m = *factors++; /* stage's fft length/p */
    const COMP * Fout_end = Fout + p*m;

    if (m == 1) {
        do {
            *Fout = *f;
            f += fstride*in_stride;
        } while (++Fout != Fout_end);
    } else {
        do {
            kf_work(Fout, f, fstride*p, in_stride, factors, st);
            f += fstride*in_stride;
        } while ((Fout += m) != Fout_end);
    }

    Fout = Fout_beg;

    // recombine the p smaller DFTs
    switch (p) {
        case 2: kf_bfly2(Fout, fstride, st, m);
            break;
        case 3: kf_bfly3(Fout, fstride, st, m);
            break;
        case 4: kf_bfly4(Fout, fstride, st, m);
            break;
        case 5: kf_bfly5(Fout, fstride, st, m);
            break;
        default: kf_bfly_generic(Fout, fstride, st, m, p);
            break;
    }
}

static void kf_factor(int n, int * facbuf) {
    int p = 4;
    float floor_sqrt = floorf(sqrtf((float) n));

    /*factor out powers of 4, powers of 2, then any remaining primes */
    do {
        while (n % p) {
            switch (p) {
                case 4: p = 2;
                    break;
                case 2: p = 3;
                    break;
                default: p += 2;
                    break;
            }
            if (p > floor_sqrt)
                p = n; /* no more factors, skip to end */
        }
        n /= p;
        *facbuf++ = p;
        *facbuf++ = n;
    } while (n > 1);
}

kiss_fft_cfg kiss_fft_alloc(int nfft, int inverse_fft, void * mem, size_t * lenmem) {
    kiss_fft_cfg st = NULL;
    size_t memneeded = sizeof (struct kiss_fft_state) + sizeof (COMP) * (nfft - 1); /* twiddle factors*/

    if (lenmem == NULL) {
        st = (kiss_fft_cfg) malloc(memneeded);
    } else {
        if (mem != NULL && *lenmem >= memneeded)
            st = (kiss_fft_cfg) mem;
        *lenmem = memneeded;
    }
    if (st) {
        int i;
        st->nfft = nfft;
        st->inverse = inverse_fft;

        for (i = 0; i < nfft; ++i) {
            float phase = -2.0f * M_PI * i / nfft;
            if (st->inverse)
                phase *= -1;
            kf_cexp(st->twiddles + i, phase);
        }

        kf_factor(nfft, st->factors);
    }
    return st;
}

void kiss_fft_stride(kiss_fft_cfg st, const COMP *fin, COMP *fout, int in_stride) {
    if (fin == fout) {
        COMP *tmpbuf = (COMP*) malloc(sizeof (COMP) * st->nfft);
        kf_work(tmpbuf, fin, 1, in_stride, st->factors, st);
        memcpy(fout, tmpbuf, sizeof (COMP) * st->nfft);
        free(tmpbuf);
    } else {
        kf_work(fout, fin, 1, in_stride, st->factors, st);
    }
}

void kiss_fft(kiss_fft_cfg cfg, const COMP *fin, COMP *fout) {
    kiss_fft_stride(cfg, fin, fout, 1);
}

int kiss_fft_next_fast_size(int n) {
    while (1) {
        int m = n;
        while ((m % 2) == 0) m /= 2;
        while ((m % 3) == 0) m /= 3;
        while ((m % 5) == 0) m /= 5;
        if (m <= 1)
            break; /* n is completely factorable by twos, threes, and fives */
        n++;
    }

    return n;
}
