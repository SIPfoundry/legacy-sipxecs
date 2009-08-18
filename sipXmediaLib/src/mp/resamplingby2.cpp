//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef __pingtel_on_posix__ /* [ */

#include "mp/resamplingby2.h"

/* C + the 32 most significant bits of A * B */
#define GIPS_SCALEDIFF32(A, B, C) \
      ((C) + ((B)>>16)*(A) + (((GIPS_UWord32)(0x0000FFFF & (B))*(A)) >> 16))

#if 0 /* [ */
#undef GIPS_SCALEDIFF32

static
GIPS_Word32 GIPS_SCALEDIFF32(GIPS_Word32 A, GIPS_Word32 B, GIPS_Word32 C)
{
   return (C + (B>>16)*A + ( ((GIPS_UWord32)(0x0000FFFF & B)*A) >> 16 ));
}

#undef GIPS_SCALEDIFF32
#define GIPS_SCALEDIFF32(A, B, C) GIPS_SCALEDIFF32X((A), (B), (C))

static
GIPS_Word32 GIPS_SCALEDIFF32X(GIPS_Word16 a, GIPS_Word32 b, GIPS_Word32 c)
{
   long long A, B, C;
   GIPS_Word32 D;

   A = a;
   B = b;
   C = (A * B);
   C = (C >> 16) & 0xFFFFFFFF;
   D = C;
   return (c + D);
}
#undef GIPS_SCALEDIFF32
#endif /* 0 ] */

/* allpass filter coefficients. */
static const GIPS_Word32 allpassfilter1[3] = {3284, 24441, 49528};
static const GIPS_Word32 allpassfilter2[3] = {12199, 37471, 60255};


/* decimator */
void GIPS_downsampling2(
   const GIPS_Word16* in,               /* input array */
   int                len,              /* length of input array */
   GIPS_Word16*       out,              /* output array (of length len/2) */
   GIPS_Word32*       filtState         /* filter state array; length = 8 */
)
{
   const GIPS_Word16 *inptr;
   GIPS_Word16       *outptr;
   GIPS_Word32       *state;
   GIPS_Word32       tmp1, tmp2, diff, in32, out32;

   /* local versions of pointers to input and output arrays */
   inptr  = in;
   outptr = out;
   state  = filtState;

   for( ; len > 0; len-=2 )
   {
      /* lower allpass filter */
      in32     = (GIPS_Word32)(*inptr++) << 10;
      diff     = in32 - state[1];
      tmp1     = GIPS_SCALEDIFF32(allpassfilter2[0], diff, state[0]);
      state[0] = in32;
      diff     = tmp1 - state[2];
      tmp2     = GIPS_SCALEDIFF32(allpassfilter2[1], diff, state[1]);
      state[1] = tmp1;
      diff     = tmp2 - state[3];
      state[3] = GIPS_SCALEDIFF32(allpassfilter2[2], diff, state[2]);
      state[2] = tmp2;

      /* upper allpass filter */
      in32     = (GIPS_Word32)(*inptr++) << 10;
      diff     = in32 - state[5];
      tmp1     = GIPS_SCALEDIFF32(allpassfilter1[0], diff, state[4]);
      state[4] = in32;
      diff     = tmp1 - state[6];
      tmp2     = GIPS_SCALEDIFF32(allpassfilter1[1], diff, state[5]);
      state[5] = tmp1;
      diff     = tmp2 - state[7];
      state[7] = GIPS_SCALEDIFF32(allpassfilter1[2], diff, state[6]);
      state[6] = tmp2;

      /* add two allpass outputs, divide by two and round */
      out32    = (state[3] + state[7] + 1024) >> 11;

      /* limit amplitude to prevent wrap-around, and write to output array */
      if (out32 > 32767 )
         out32 = 32767;
      else if (out32 < -32768 )
         out32 = -32768;

      *outptr++ = (GIPS_Word16) out32;
   }
}



void GIPS_upsampling2(
   const GIPS_Word16* in,         /* input array */
   int                len,        /* length of input array */
   GIPS_Word16*       out,        /* output array (of length len*2) */
   GIPS_Word32*       filtState   /* filter state array; length = 8 */
)
{
   const GIPS_Word16 *inptr;
   GIPS_Word16       *outptr;
   GIPS_Word32       *state;
   GIPS_Word32       tmp1, tmp2, diff, in32, out32;

   /* local versions of pointers to input and output arrays */
   inptr  = in;
   outptr = out;
   state  = filtState;

   for( ; len > 0; len-- )
   {
      /* lower allpass filter */
      in32     = (GIPS_Word32)(*inptr++) << 10;
      diff     = in32 - state[1];
      tmp1     = GIPS_SCALEDIFF32(allpassfilter1[0], diff, state[0]);
      state[0] = in32;
      diff     = tmp1 - state[2];
      tmp2     = GIPS_SCALEDIFF32(allpassfilter1[1], diff, state[1]);
      state[1] = tmp1;
      diff     = tmp2 - state[3];
      state[3] = GIPS_SCALEDIFF32(allpassfilter1[2], diff, state[2]);
      state[2] = tmp2;

      /* round; limit amplitude to prevent wrap-around; write to output array */
      out32    = (state[3] + 512) >> 10;
      if (out32 > 32767)
         out32 = 32767;
      else if (out32 < -32768)
         out32 = -32768;

      *outptr++ = (GIPS_Word16) out32;


      /* upper allpass filter */
      diff     = in32 - state[5];
      tmp1     = GIPS_SCALEDIFF32(allpassfilter2[0], diff, state[4]);
      state[4] = in32;
      diff     = tmp1 - state[6];
      tmp2     = GIPS_SCALEDIFF32(allpassfilter2[1], diff, state[5]);
      state[5] = tmp1;
      diff     = tmp2 - state[7];
      state[7] = GIPS_SCALEDIFF32(allpassfilter2[2], diff, state[6]);
      state[6] = tmp2;

      /* round; limit amplitude to prevent wrap-around; write to output array */
      out32    = (state[7] + 512) >> 10;
      if (out32 > 32767)
         out32 = 32767;
      else if (out32 < -32768)
         out32 = -32768;

      *outptr++ = (GIPS_Word16) out32;
   }
}

#endif /* __pingtel_on_posix__ ] */
