/*
 * Copyright (C) 2010 Perens LLC
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */
#include "defines.h"
#include "quantise.h"

/* Compile-time constants */
/* Size of unsigned char in bits. Assumes 8 bits-per-char. */
static const unsigned int WordSize = 8;

/* Mask to pick the bit component out of bitIndex. */
static const unsigned int IndexMask = 0x7;

/* Used to pick the word component out of bitIndex. */
static const unsigned int ShiftRight = 3;

/** Pack a bit field into a bit string, encoding the field in Gray code.
 *
 * The output is an array of unsigned char data. The fields are efficiently
 * packed into the bit string. The Gray coding is a naive attempt to reduce
 * the effect of single-bit errors, we expect to do a better job as the
 * codec develops.
 *
 * This code would be simpler if it just set one bit at a time in the string,
 * but would hit the same cache line more often. I'm not sure the complexity
 * gains us anything here.
 *
 * Although field is currently of int type rather than unsigned for
 * compatibility with the rest of the code, indices are always expected to
 * be >= 0.
 */
void pack_charbits(
  unsigned char *bitArray,
  int field
){
  char charstr[10];

  sprintf(charstr, "%c", field);
  strcat(bitArray, charstr);
}
 
 
 
void pack_natural_or_gray(
        unsigned char *bitArray, /* The output bit string. */
        unsigned int *bitIndex, /* Index into the string in BITS, not bytes.*/
        int field, /* The bit field to be packed. */
        unsigned int fieldWidth, /* Width of the field in BITS, not bytes. */
        unsigned int gray /* non-zero for gray coding */
        ) {
    if (gray) {
        /* Convert the field to Gray code */
        field = (field >> 1) ^ field;
    }

    do {
        unsigned int bI = *bitIndex;
        unsigned int bitsLeft = WordSize - (bI & IndexMask);
        unsigned int sliceWidth = bitsLeft < fieldWidth ? bitsLeft : fieldWidth;
        unsigned int wordIndex = bI >> ShiftRight;

        bitArray[wordIndex] |=
                ((unsigned char) ((field >> (fieldWidth - sliceWidth)) <<
                (bitsLeft - sliceWidth)));

        *bitIndex = bI + sliceWidth;
        fieldWidth -= sliceWidth;
    } while (fieldWidth != 0);
}

/** Unpack a field from a bit string, to binary, optionally using
 * natural or Gray code.
 *
 */
int unpack_natural_or_gray(
        const unsigned char *bitArray, /* The input bit string. */
        unsigned int *bitIndex, /* Index into the string in BITS, not bytes.*/
        unsigned int fieldWidth, /* Width of the field in BITS, not bytes. */
        unsigned int gray /* non-zero for Gray coding */
        ) {
    unsigned int field = 0;
    unsigned int t;

    do {
        unsigned int bI = *bitIndex;
        unsigned int bitsLeft = WordSize - (bI & IndexMask);
        unsigned int sliceWidth = bitsLeft < fieldWidth ? bitsLeft : fieldWidth;

        field |= (((bitArray[bI >> ShiftRight] >>
                (bitsLeft - sliceWidth)) & ((1 << sliceWidth) - 1)) <<
                (fieldWidth - sliceWidth));

        *bitIndex = bI + sliceWidth;
        fieldWidth -= sliceWidth;
    } while (fieldWidth != 0);

    if (gray) {
        /* Convert from Gray code to binary. Works for maximum 8-bit fields. */
        t = field ^ (field >> 8);
        t ^= (t >> 4);
        t ^= (t >> 2);
        t ^= (t >> 1);
    } else {
        t = field;
    }

    return t;
}
