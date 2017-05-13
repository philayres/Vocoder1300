/*---------------------------------------------------------------------------*\

  FILE........: c2enc.c
  AUTHOR......: David Rowe
  DATE CREATED: 23/8/2010

  Encodes a file of raw speech samples using codec2 and outputs a file
  of bits.

\*---------------------------------------------------------------------------*/

/*
  Copyright (C) 2010 David Rowe

  All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 2.1, as
  published by the Free Software Foundation.  This program is
  distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include "codec2.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    FILE          *fin;
    FILE          *fout;
    short         *buf;
    unsigned char *bits;
    int            nsam, nbit, nbyte;
    float         *unpacked_bits;

    unsigned int  charbits = 1;

    if (argc < 3) {
	printf("usage: c2enc InputRawspeechFile OutputBitFile\n");
	printf("e.g    c2enc ../raw/hts1a.raw hts1a.c2\n");
	exit(1);
    }

    if (strcmp(argv[1], "-")  == 0) fin = stdin;
    else if ( (fin = fopen(argv[1],"rb")) == NULL ) {
	fprintf(stderr, "Error opening input speech file: %s: %s.\n",
         argv[1], strerror(errno));
	exit(1);
    }

    if (strcmp(argv[2], "-") == 0) fout = stdout;
    else if ( (fout = fopen(argv[2],"wb")) == NULL ) {
	fprintf(stderr, "Error opening output compressed bit file: %s: %s.\n",
         argv[2], strerror(errno));
	exit(1);
    }

    codec2_create();
    nsam = codec2_samples_per_frame();
    nbit = codec2_bits_per_frame();
    buf = (short *) malloc(nsam * sizeof(short));
    
    if(charbits)
      nbyte = 13;
    else
      nbyte = (nbit + 7) / 8;

    bits = (unsigned char *) malloc(nbyte * sizeof(char));
    unpacked_bits = (float *) malloc(nbit * sizeof(float));

    if(charbits)
      bits[0]=0;

    while(fread(buf, sizeof(short), nsam, fin) == (size_t)nsam) {

	codec2_encode(bits, buf, charbits);

        fwrite(bits, sizeof(char), nbyte, fout);

	// if this is in a pipeline, we probably don't want the usual
        // buffering to occur

        if (fout == stdout) fflush(stdout);
        if (fin == stdin) fflush(stdin);
    }

    codec2_destroy();

    free(buf);
    free(bits);
    free(unpacked_bits);
    fclose(fin);
    fclose(fout);

    return 0;
}
