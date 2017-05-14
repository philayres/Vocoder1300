/*---------------------------------------------------------------------------*\

  FILE........: c2dec.c
  AUTHOR......: David Rowe
  DATE CREATED: 23/8/2010

  Decodes a file of bits to a file of raw speech samples using codec2.

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
    int            ret;
    unsigned int  charbits = 0;

    if (argc < 2) {
        fprintf(stderr, "Usage: c2dec filename.c2 filename.raw\n\n");
        exit(-1);
    }

    if (strcmp(argv[1], "-")  == 0) fin = stdin;
    else if ( (fin = fopen(argv[1],"rb")) == NULL ) {
	fprintf(stderr, "Error opening input bit file: %s: %s.\n",
         argv[1], strerror(errno));
	exit(1);
    }

    if (strcmp(argv[2], "-") == 0) fout = stdout;
    else if ( (fout = fopen(argv[2],"wb")) == NULL ) {
	fprintf(stderr, "Error opening output speech file: %s: %s.\n",
         argv[2], strerror(errno));
	exit(1);
    }

    if (argv[3] && strcmp(argv[3], "charbits") == 0)
      charbits=1;

    codec2_create();
    nsam = codec2_samples_per_frame();
    nbit = codec2_bits_per_frame();
    buf = (short*)malloc(nsam*sizeof(short));
    
    if(charbits)
      nbyte = NUM_CHARBITS;
    else
      nbyte = (nbit + 7) / 8;
    
    bits = (unsigned char*)malloc(nbyte*sizeof(char));

    ret = (fread(bits, sizeof(char), nbyte, fin) == (size_t)nbyte);

    while(ret) {
	codec2_decode(buf, bits, charbits);
 	fwrite(buf, sizeof(short), nsam, fout);

	//if this is in a pipeline, we probably don't want the usual
        //buffering to occur

        if (fout == stdout) fflush(stdout);
        if (fin == stdin) fflush(stdin);

        ret = (fread(bits, sizeof(char), nbyte, fin) == (size_t)nbyte);
    }

    codec2_destroy();

    free(buf);
    free(bits);
    fclose(fin);
    fclose(fout);

    return 0;
}

