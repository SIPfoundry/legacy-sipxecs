//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef HAVE_GIPS /* [ */

#include "assert.h"

#include "mp/JB/JB_API.h"
#include "mp/MpJitterBuffer.h"
#include "mp/MpSipxDecoders.h"

#define LOCAL static
#undef LOCAL
#define LOCAL

/* ============================ CREATORS ================================== */

class MpJbG711a : public MpSipxDecoder
{
};


/* ============================ MANIPULATORS ============================== */

#define SIGN_BIT        (0x80)          /* Sign bit for a A-law byte. */
#define QUANT_MASK      (0xf)           /* Quantization field mask. */
#define SEG_SHIFT       (4)             /* Left shift for segment number. */
#define SEG_MASK        (0x70)          /* Segment field mask. */

#define BIAS            (0x84)          /* Bias for linear code. */

LOCAL short hzm_ULaw2linear(unsigned char u)
{
        int L;
        int seg;

        u = ~u;
        seg = (u & 0x70) >> 4;
        L = ((0x0f & u) << 3) + BIAS;
        L = (L << seg);
        if (0x80 & u) {
                L = BIAS - L;
        } else {
                L = L - BIAS;
        }
        return L;
}

LOCAL int ULawToLinear(Sample *Dest, unsigned char *Source, int samples)
{
        int i;
        unsigned char  *src;
        short *dst;

        src = Source;
        dst = (short *) Dest;

        for (i=0; i<samples; i++) {
            *dst = hzm_ULaw2linear(*src);
            dst++; src++;
        }
        return samples;
}


/*
 * ALaw2Linear() - Convert an A-law value to 16-bit linear PCM
 *
 */
LOCAL int ALaw2Linear(unsigned char a_val)
{
        int             t;
        int             seg;

        a_val ^= 0x55;

        t = (a_val & QUANT_MASK) << 4;
        seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
        switch (seg) {
        case 0:
                t += 8;
                break;
        case 1:
                t += 0x108;
                break;
        default:
                t += 0x108;
                t <<= seg - 1;
        }
        return ((a_val & SIGN_BIT) ? t : -t);
}

LOCAL int ALawToLinear(Sample *Dest, unsigned char *src, int samples)
{
        int i;
        short *dst;

        dst = (short *) Dest;

        for (i=0; i<samples; i++) {
            *dst = ALaw2Linear(*src);
            dst++; src++;
        }
        return samples;
}

LOCAL short seg_end[8] = {0xFF, 0x1FF, 0x3FF, 0x7FF,
                            0xFFF, 0x1FFF, 0x3FFF, 0x7FFF};

LOCAL int search(int val, short *table, int size)
{
        int             i;

        for (i = 0; i < size; i++) {
                if (val <= *table++)
                        return (i);
        }
        return (size);
}

JB_ret G711A_Decoder(JB_size noOfSamples,
                     JB_uchar* codBuff,
                     Sample* outBuff)
{
   ALawToLinear(outBuff, codBuff, noOfSamples);
   return 0;
}

JB_ret G711U_Decoder(JB_size noOfSamples,
                     JB_uchar* codBuff,
                     Sample* outBuff)
{
   ULawToLinear(outBuff, codBuff, noOfSamples);
   return 0;
}

/*
 * Linear2ALaw() - Convert a 16-bit linear PCM value to 8-bit A-law
 *
 * Linear2ALaw() accepts an 16-bit integer and encodes it as A-law data.
 *
 *              Linear Input Code       Compressed Code
 *      ------------------------        ---------------
 *      0000000wxyza                    000wxyz
 *      0000001wxyza                    001wxyz
 *      000001wxyzab                    010wxyz
 *      00001wxyzabc                    011wxyz
 *      0001wxyzabcd                    100wxyz
 *      001wxyzabcde                    101wxyz
 *      01wxyzabcdef                    110wxyz
 *      1wxyzabcdefg                    111wxyz
 *
 * For further information see John C. Bellamy's Digital Telephony, 1982,
 * John Wiley & Sons, pps 98-111 and 472-476.
 */
LOCAL unsigned char Linear2ALaw(
        int             pcm_val)        /* 2's complement (16-bit range) */
{
        int             mask;
        int             seg;
        unsigned char   aval;

        if (pcm_val >= 0) {
                mask = 0xD5;            /* sign (7th) bit = 1 */
        } else {
                mask = 0x55;            /* sign bit = 0 */
                pcm_val = -pcm_val - 8;
        }

        /* Convert the scaled magnitude to segment number. */
        seg = search(pcm_val, seg_end, 8);

        /* Combine the sign, segment, and quantization bits. */

        if (seg >= 8)           /* out of range, return maximum value. */
                return (0x7F ^ mask);
        else {
                aval = seg << SEG_SHIFT;
                if (seg < 2)
                        aval |= (pcm_val >> 4) & QUANT_MASK;
                else
                        aval |= (pcm_val >> (seg + 3)) & QUANT_MASK;
                return (aval ^ mask);
        }
}

LOCAL int LinearToALaw(unsigned char *Dest, Sample *src, int samples)
{
        int i;

        for (i=0; i<samples; i++) {
            *Dest = Linear2ALaw(*src);
            Dest++; src++;
        }
        return samples;
}

JB_ret G711A_Encoder(JB_size noOfSamples,
                     Sample* inBuff,
                     JB_uchar* codBuff,
                     JB_size *size_in_bytes)
{
   LinearToALaw(codBuff, inBuff, noOfSamples);
   *size_in_bytes = noOfSamples;
   return 0;
}

LOCAL unsigned char hzm_Linear2ULaw(int L)
{
   int seg;
   unsigned char signmask;

   if (0 > L) {
      L = BIAS - L;
      signmask = 0x7f;
   } else {
      signmask = 0xff;
      L = BIAS + L;
   }
   if (L > 32767) L = 32767;
   if (0x7800 & L) {
      seg = (4<<4);
   } else {
      seg = 0;
      L = L << 4;
   }
   if (0x6000 & L) {
      seg += (2<<4);
   } else {
      L = L << 2;
   }
   if (0x4000 & L) {
      seg += (1<<4);
   } else {
      L = L << 1;
   }
   return ((seg | ((0x3C00 & L) >> 10)) ^ signmask);
}

LOCAL int LinearToULaw(unsigned char *Dest, Sample *src, int samples)
{
   int i;

   for (i=0; i<samples; i++) {
      *Dest = hzm_Linear2ULaw(*src);
      Dest++; src++;
   }
   return samples;
}

JB_ret G711U_Encoder(JB_size noOfSamples,
                     Sample* inBuff,
                     JB_uchar* codBuff,
                     JB_size *size_in_bytes)
{
   LinearToULaw(codBuff, inBuff, noOfSamples);
   *size_in_bytes = noOfSamples;
   return 0;
}
#endif /* NOT(HAVE_GIPS) ] */
