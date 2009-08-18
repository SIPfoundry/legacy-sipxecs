//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// Noise generation
// comfort noise genrator
//
// The confort noise is generated via passing a white nosie through a 500Hz
// low pass filter. The strength depends on the output a noise lvevl estimator
// residing in MprToSpkr.
//
// -------------         ------------       -----------
// |white noise|         | 500 Hz   |       |strength |
// |generator  |-------->| LP filter|------>|regulator|------> comfort noise
// -------------         ------------       -----------
//                                           ^
// -------------                             |
// |noise level|                             |
// |estimator  | ----------------------------|
// -------------
//

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

#include "mp/dsplib.h"

// 500Hz Lowpass filter for noise generation

#define FILTER_N 7   // MUST BE ODD

#define HALF_FILTER_N   (FILTER_N+1)/2

short shpLP500[HALF_FILTER_N] = {     //500Hz linear Lowpass filter parameters
      // 0.0328, 0.2396, 0.4553, 0.2396, 0.0328
      // 134, 981, 1865, 981, 134};   // in Q12
      // 0.0199, 0.0645,0.1664,0.2493,0.2493,0.1664,0.0645,0.0199
      // 82, 264, 682, 1021};      // in Q12 * 1.41
   115, 373, 961, 1440};      // in Q12 * 1.41

short shpFilterBuf[SAMPLES_PER_FRAME+FILTER_N];


void init_CNG() {
   int i;
   for (i = 0; i < SAMPLES_PER_FRAME+FILTER_N; i++)
      shpFilterBuf[i] = 0;
}

void white_noise_generator(Sample  *shpSamples,
                              int   iLength,
                           Word32   ulNoiseLevelAve)
{
   int     i;
   short   *shp;
   int     iBitShift = 0;

   iBitShift = 9 - ulNoiseLevelAve;

   // generate uniformly distributed random signal
   shp = shpFilterBuf + FILTER_N;
   for(i = 0; i < iLength; i++) {
      *shpSamples++ = (short) ((rand() - (RAND_MAX>>1) ) >> (iBitShift));
   }
}

void comfort_noise_generator(Sample  *shpSamples,
                                int   iLength,
                             Word32   ulNoiseLevelAve)
{
   int  i;
   int  j;
   long lS;
   short *shp0, *shp, *shp1, *shp2;
   int   iBitShift;

   if(ulNoiseLevelAve < 6000) {
      iBitShift = 13+12;
   }
   else if(ulNoiseLevelAve < 12000) {
      iBitShift = 12+12;
   }
   else  {
      iBitShift = 11+12;
   }

   // generate uniformly distributed randam signal
   shp = shpFilterBuf + FILTER_N;
   for(i = 0; i < iLength; i++) {
      *shp++ = (short) ((rand() - (RAND_MAX>>1)));
   }
   // Lowpass filtering, cut-off frequency 500 Hz
   shp = shpFilterBuf + FILTER_N;
   shp0 = shpFilterBuf;
   for(i = 0; i < iLength; i++) {
      shp1 = shp;
      shp2 = shp0;
      shp++;
      shp0++;
      lS = 0;
      for(j = 0; j < HALF_FILTER_N; j++) {
         lS += (long) shpLP500[j] * (long) (*shp1-- + *shp2++);
      }
      *shpSamples++ = (short) (lS>>(iBitShift));
   }
   // copy the previous data for filtering
   shp = shpFilterBuf;
   shp1 = shpFilterBuf + iLength;
   for( i = 0; i < FILTER_N; i++) {
      *shp++ = *shp1++;
   }

}

void background_noise_level_estimation(Word32&  ulNoiseLevel,
                                       Sample*  shpSamples,
                                       int      iLength)
{
   int i;
   Word32 ulStrength = 0;

   for (i = 0; i < iLength; i++) {
      ulStrength += (Word32) abs(*shpSamples++);
   }
   if( ulStrength <  (ulNoiseLevel<<1) ) { //updating if 6dB < existing average
      ulNoiseLevel = ulStrength  + (Word32) 31 * ulNoiseLevel;
      ulNoiseLevel >>= 5; //Divide by 32
      //Averaging the absolute sum of a frame to reduce calculation
      //fading factor 31/32 = 0.969
   }
}

#ifdef NEVER_GOT_USED /* [ */
void dspCopy32Sto16S(const int* src, short* dst, int count)
{
   int i;

   for (i=0; i<count; i++) {
      *dst++ = *src++;
   }
}

void dspCopy16Sto32S(const short* src, int* dst, int count)
{
   int i;

   for (i=0; i<count; i++) {
      *dst++ = *src++;
   }
}
#endif /* NEVER_GOT_USED ] */

#ifdef _VXWORKS /* [ */
#ifdef NOT_USING_ASM /* [ */

Word64S dspDotProd16x32(const short* v1, const int* v2, int count,
                        Word64S* res)
{
   int i;
   Word64S result = 0;
   Word64S a, b;

   for (i=0; i<count; i++) {
      a = *v1++;
      b = *v2++;
      result += a * b;
   }
   if (NULL != res) *res = result;
   return result;
}

Word64S dspDotProd16skip32(const short* v1, const int* v2, int count,
                           Word64S* res)
{
   int i;
   Word64S result = 0;
   Word64S a, b;

   for (i=0; i<count; i++) {
      a = *v1++; v1++; /* skip every other item in v1 */
      b = *v2++;
      result += a * b;
   }
   if (NULL != res) *res = result;
   return result;
}

#ifdef NEVER_GOT_USED /* [ */
Word64S dspDotProd32x32(const int* v1, const int* v2, int count,
                        Word64S* res)
{
   int i;
   Word64S result = 0;
   Word64S a, b;

   for (i=0; i<count; i++) {
      a = *v1++;
      b = *v2++;
      result += a * b;
   }
   if (NULL != res) *res = result;
   return result;
}
#endif /* NEVER_GOT_USED ] */

/* Coefficient update routines */
/* adjust each coefficient by (factor * v1[j]) */
void dspCoeffUpdate16x32(const short* v1, int* v2, int count, int factor)
{
   int i;

   for (i=0; i<count; i++) {
      v2[i] += (factor * ((int) (v1[i]))) >> 7;
   }
}

/* adjust each coefficient by (factor * v1[2*j]) - every other v1 */
void dspCoeffUpdate16skip32(const short* v1, int* v2, int count, int factor)
{
   int i;

   for (i=0; i<count; i++) {
      v2[i] += (factor * ((int) (v1[2*i]))) >> 7;
   }
}


#define DspDotProd16x32 dspDotProd16x32
#define DspDotProd32x32 dspDotProd32x32

#else /* NOT_USING_ASM ] [ */

Word64S DspDotProd16x32(const short* v1, const int* v2, int count,
                        Word64S* res)
{
   int i;
   Word64S result = 0;
   Word64S a, b;

   for (i=0; i<count; i++) {
      a = *v1++;
      b = *v2++;
      result += a * b;
   }
   if (NULL != res) *res = result;
   return result;
}

Word64S DspDotProd16skip32(const short* v1, const int* v2, int count,
                           Word64S* res)
{
   int i;
   Word64S result = 0;
   Word64S a, b;

   for (i=0; i<count; i++) {
      a = *v1++; v1++; /* skip every other item in v1 */
      b = *v2++;
      result += a * b;
   }
   if (NULL != res) *res = result;
   return result;
}

Word64S DspDotProd32x32(const int* v1, const int* v2, int count,
                        Word64S* res)
{
   int i;
   Word64S result = 0;
   Word64S a, b;

   for (i=0; i<count; i++) {
      a = *v1++;
      b = *v2++;
      result += a * b;
   }
   if (NULL != res) *res = result;
   return result;
}

/* Coefficient update routines */
/* adjust each coefficient by (factor * v1[j]) */
void DspCoeffUpdate16x32(const short* v1, int* v2, int count, int factor)
{
   int i;

   for (i=0; i<count; i++) {
      v2[i] += (factor * ((int) (v1[i]))) >> 7;
   }
}

/* adjust each coefficient by (factor * v1[2*j]) - every other v1 */
void DspCoeffUpdate16skip32(const short* v1, int* v2, int count, int factor)
{
   int i;

   for (i=0; i<count; i++) {
      v2[i] += (factor * ((int) (v1[2*i]))) >> 7;
   }
}

#endif /* NOT_USING_ASM ] */

#ifdef TESTING /* [ */
int testDotProd1()
{
   int ints[10];
   int int2[10];
   short shorts[10] = {1, -2, 3, -4, 5, -6, 7, -8, 9, -10};
   short outs[10];
   Word64S sum;
   int hi, lo;
   int i;

   dspCopy16Sto32S(shorts, ints, 10);

   for (i=0; i<10; ) {
      int2[i++] = -1000000000;
      int2[i++] =  1000000000;
   }

   sum = dspDotProd32S(ints, int2, 10);
   lo = sum & 0xffffffff;
   sum = (sum >> 32);
   hi = sum & 0xffffffff;

   printf("0x%08X%08X\n", hi, lo);
   dspCopy32Sto16S(ints, outs, 10);
   printf("%d, %d, %d, %d, %d,", outs[0], outs[1], outs[2], outs[3], outs[4]);
   printf("%d, %d, %d, %d, %d\n", outs[5], outs[6], outs[7], outs[8], outs[9]);
}
#endif /* TESTING ] */

#define TESTING2
#undef TESTING2
#ifdef TESTING2 /* [ */
int testDotProd2()
{
   int ints[10];
   int int2[10];
   short shorts[10] = {1, -2, 3, -4, 5, -6, 7, -8, 9, -10};
   short outs[10];
   Word64S sum, ret1, ret2;
   int hi, lo;
   int i;

   dspCopy16Sto32S(shorts, ints, 10);

   for (i=0; i<10; ) {
      int2[i++] = -1000000000;
      int2[i++] =  1000000000;
   }

   sum = dspDotProd16x32(shorts, int2, 10, &ret2);
   lo = sum & 0xffffffff;
   sum = (sum >> 32);
   hi = sum & 0xffffffff;

   printf("16x32: 0x%08X%08X\n", hi, lo);

   sum = dspDotProd32x32(ints, int2, 10, &ret1);
   lo = sum & 0xffffffff;
   sum = (sum >> 32);
   hi = sum & 0xffffffff;

   printf("32x32: 0x%08X%08X\n", hi, lo);

   dspCopy32Sto16S(ints, outs, 10);
   printf("%d, %d, %d, %d, %d,", outs[0], outs[1], outs[2], outs[3], outs[4]);
   printf("%d, %d, %d, %d, %d\n", outs[5], outs[6], outs[7], outs[8], outs[9]);
   return 0;
}

#ifdef _VXWORKS /* [ */
#include "mp/sa1100.h"
#endif /* _VXWORKS ] */

int testDotProd3(int times, int count)
{
#ifdef _VXWORKS /* [ */
   volatile int* pOsTC = (int*) SA1100_OSTIMER_COUNTER;
#else /* _VXWORKS ] [ */
   int  foo = 0;
   int* pOsTC = &foo;
#endif /* _VXWORKS ] */
   int* ints = (int*) malloc(count * sizeof(int));
   int* int2 = (int*) malloc(count * sizeof(int));
   short* shorts = (short*) malloc(count * sizeof(short));
   Word64S sum, ret1, ret2;
   int hi, lo;
   int i, x;
   int before, after;

   x = -1000000000;
   for (i=0; i<count; i++) {
      shorts[i] = (i&1) ? -(i+1) : (i+1);
      int2[i] = x;
      x = -x;
   }

   dspCopy16Sto32S(shorts, ints, count);

   before = *pOsTC;
   for (i=0; i<times; i++) {
      sum = DspDotProd16x32(shorts, int2, count, &ret1);
   }
   after = *pOsTC;
   lo = sum & 0xffffffff;
   sum = (sum >> 32);
   hi = sum & 0xffffffff;

   printf("C++: 0x%08X%08X, in %d ticks\n", hi, lo, after-before);

   before = *pOsTC;
   for (i=0; i<times; i++) {
      sum = dspDotProd16x32(shorts, int2, count, &ret2);
   }
   after = *pOsTC;
   lo = sum & 0xffffffff;
   sum = (sum >> 32);
   hi = sum & 0xffffffff;

   printf("ASM: 0x%08X%08X, in %d ticks\n", hi, lo, after-before);

   free(shorts);
   free(int2);
   free(ints);
   return 0;
}

/* Some code fragments to see if the GNU compiler has any clever
 * tricks for handling 64 bit values.
 */
extern "C" {
extern long long combine(int a, int b);
extern int compilerCheck(int i, int j);
}

long long combine(int a, int b)
{
   long long t1, t2, t3;

   t1 = a << 11;
   t2 = ((long long) b) << 19;
   t3 = t1 + t2;
   return t3;
}

int compilerCheck(int i, int j)
{
   long long a, b;
   int c;

   a = combine(i,j);
   b = combine(j, i) >> 7;
   a = a - (b >> 5);
   c = a >> 19;
   return c;
}
#endif /* TESTING2 ] */
#else /* _VXWORKS ] [ */

Word64S dspDotProd16x32(const short* v1, const int* v2, int count,
                        Word64S* res)
{
   int i;
   Word64S result = 0;
   Word64S a, b;

   for (i=0; i<count; i++) {
      a = *v1++;
      b = *v2++;
      result += a * b;
   }
   if (NULL != res) *res = result;
   return result;
}

Word64S dspDotProd16skip32(const short* v1, const int* v2, int count,
                           Word64S* res)
{
   int i;
   Word64S result = 0;
   Word64S a, b;

   for (i=0; i<count; i++) {
      a = *v1++; v1++; /* skip every other item in v1 */
      b = *v2++;
      result += a * b;
   }
   if (NULL != res) *res = result;
   return result;
}

#ifdef NEVER_GOT_USED /* [ */
Word64S dspDotProd32x32(const int* v1, const int* v2, int count,
                        Word64S* res)
{
   int i;
   Word64S result = 0;
   Word64S a, b;

   for (i=0; i<count; i++) {
      a = *v1++;
      b = *v2++;
      result += a * b;
   }
   if (NULL != res) *res = result;
   return result;
}
#endif /* NEVER_GOT_USED ] */

/* Coefficient update routines */
/* adjust each coefficient by (factor * v1[j]) */
void dspCoeffUpdate16x32(const short* v1, int* v2, int count, int factor)
{
   int i;

   for (i=0; i<count; i++) {
      v2[i] += (factor * ((int) (v1[i]))) >> 7;
   }
}

/* adjust each coefficient by (factor * v1[2*j]) - every other v1 */
void dspCoeffUpdate16skip32(const short* v1, int* v2, int count, int factor)
{
   int i;

   for (i=0; i<count; i++) {
      v2[i] += (factor * ((int) (v1[2*i]))) >> 7;
   }
}

#endif /* _VXWORKS ] */
