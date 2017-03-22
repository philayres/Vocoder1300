/*
 * Copyright (C) 1993-2016 David Rowe
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */
#ifndef __DEFINES__
#define __DEFINES__

#ifdef __cplusplus
  extern "C" {
#endif
      
#ifndef M_PI
#define M_PI    3.14159265358979323846f
#endif

#define TAU     (M_PI * 2.0f)

/* General defines */


#define SAMPLE_RATE 8000

#define N          80  /* number of samples per frame          */
#define MAX_AMP    80  /* maximum number of harmonics          */
#define FS         8000  /* sample rate in Hz                    */
#define MAX_STR    256          /* maximum string size                  */

#define NW         279          /* analysis window size                 */
#define FFT_SIZE   512  /* size of FFT                          */
#define TW         40  /* Trapezoidal synthesis window overlap */
#define V_THRESH   6.0f         /* voicing threshold in dB              */
#define LPC_ORD    10  /* LPC order                            */

/* Pitch estimation defines */

#define M        320  /* pitch analysis frame size            */
#define P_MIN    20  /* minimum pitch                        */
#define P_MAX    160  /* maximum pitch                        */

/* Structure to hold model parameters for one frame */

typedef struct
{
    float Wo; /* fundamental frequency estimate in radians  */
    int L; /* number of harmonics                        */
    float A[MAX_AMP + 1]; /* amplitude of each harmonic                */
    float phi[MAX_AMP + 1]; /* phase of each harmonic                     */
    int voiced; /* non-zero if this frame is voiced           */
} MODEL;

/* describes each codebook  */

struct lsp_codebook
{
    int k; /* dimension of vector	*/
    int log2m; /* number of bits in m	*/
    int m; /* elements in codebook	*/
    const float *cb; /* The elements		*/
};

extern const struct lsp_codebook lsp_cb[];

#ifdef __cplusplus
}
#endif
#endif
