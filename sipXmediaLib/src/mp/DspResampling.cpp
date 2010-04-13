//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////



// In current implementation, microphone signal needs to be down-sampled from
// 32KHz to 8KHz within object MprFromMic. In the other direction, a 8KHz
// processed signal needs to be up-sampled to 32KHz within object MprToSpkr.
//
// To down-sample a 32kHz sequence to a 8kHz, the sequence will be
// passed through a 4kHz low-pass filter, following with a decimation by 4.
//
//   32k data   ----------------       ----------------   8k data
// ------------>|4KHz LP filter|------>|decimated by 4|------------>
//              ----------------       ----------------
// Conversely, to up-sample a 8kHz sequence, the sequence needs to be
// interpolated with 0's, followed by a 4kHz low-pass filtering.
//
//    8k data   -------------------      ----------------  32k data
// ------------>|interpolated by 4|----->|4KHz LP filter|------------->
//              -------------------      ----------------
//
// Since the two LP fitering are performed in 32K, they are virtually the
// same.  However, for practical implementation, there is a need to arrange
// the coefficients in different order for efficient calcalation. Therefere,
// we still maintain two sets of array to store those coefficients separately.
//
// The ORIGINAL coefficients in polyphase structure is as follows:
//        { 3,  -22,  115, 991,  -77, 13,    * group 1 in Q12 *
//         11,  -83,  431, 771, -125, 19,    * group 2 in Q12 *
//         19, -125,  771, 431,  -83, 11,    * group 3 in Q12 *
//         13,  -77,  991, 115,  -22,  3};   * group 4 in Q12 *
//
// SYSTEM INCLUDES
#include <assert.h>
#ifdef COMPARE_FILTERS /* [ */
#ifdef _VXWORKS
#include <taskLib.h>
#endif
#endif /* COMPARE_FILTERS ] */

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mp/DspResampling.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

#ifdef COMPARE_FILTERS /* [ */
         /* coefficients are arranged to accommodate polyphase structure */

const int DspResampling::saCoeffDownO[] = {
  15,   31,   44,   45,   28,   -5,  -43,  -72,  -75,  -46,   10,  71,
 111,  104,   44,  -53, -150, -197, -153,   -2,  237,  513,  754,  895
};
const int DspResampling::saCoeffUpO[] = {
  15,   28,  -75,  111, -150,  237,  895,   -2,  -53,   71,  -72,   45,
  31,   -5,  -46,  104, -197,  513,  754, -153,   44,   10,  -43,   44,
  44,  -43,   10,   44, -153,  754,  513, -197,  104,  -46,   -5,   31,
  45,  -72,   71,  -53,   -2,  895,  237, -150,  111,  -75,   28,   15
};
#endif /* COMPARE_FILTERS ] */

const int DspResampling::saCoeffDownN[] = {
     -2,   -6,  -12,  -19,  -23,  -22,  -14,   -1,   14,   25,   26,   16,
     -1,  -20,  -31,  -29,  -13,   11,   31,   39,   28,    2,  -28,  -46,
    -44,  -19,   18,   51,   61,   41,   -3,  -51,  -80,  -71,  -23,   45,
    102,  115,   68,  -28, -135, -199, -170,  -25,  219,  508,  765,  915
};
const int DspResampling::saCoeffUpN[] = {
  -2,  -23,   14,   -1,  -13,   28,  -44,   61,  -80,  102, -135,  219,
 915,  -25,  -28,   45,  -51,   51,  -46,   39,  -29,   16,   -1,  -19,

  -6,  -22,   25,  -20,   11,    2,  -19,   41,  -71,  115, -199,  508,
 765, -170,   68,  -23,   -3,   18,  -28,   31,  -31,   26,  -14,  -12,

 -12,  -14,   26,  -31,   31,  -28,   18,   -3,  -23,   68, -170,  765,
 508, -199,  115,  -71,   41,  -19,    2,   11,  -20,   25,  -22,   -6,

 -19,   -1,   16,  -29,   39,  -46,   51,  -51,   45,  -28,  -25,  915,
 219, -135,  102,  -80,   61,  -44,   28,  -13,   -1,   14,  -23,   -2
};

#ifdef DETECT_OVERFLOW /* [ */
int DspResampling::smStatsReports = 0;
#endif /* DETECT_OVERFLOW ] */

// static int smUpGain = 10650;
static int smUpGain = 16000;
int setUpGain(int newGain) {
   int save = smUpGain;
   smUpGain = newGain;
   return save;
}

#ifdef COMPARE_FILTERS /* [ */
// $$$ change this to FILTER_Nn once we are happy with the new one...
int DspResampling::smFilterLen = FILTER_Nn; // use new (GIPS) filter by default
#else /* COMPARE_FILTERS ] [ */
int DspResampling::smFilterLen = FILTER_No;
#endif /* COMPARE_FILTERS ] */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor

DspResampling::DspResampling(int iResamplingFold, int iLowRateSamples,
   int iUpSamplingFlag):
   mpDown(0),
   mpUp(0),
   mpUpSampBuf(0),
#ifdef DETECT_OVERFLOW /* [ */
   mOverflowsU(0),
   mOverflowsD(0),
   mTotalSamplesU(0),
   mTotalSamplesD(0),
   mMaxD(0), mMinD(0),
   mMaxU(0), mMinU(0),
   mInMaxD(0), mInMinD(0),
   mInMaxU(0), mInMinU(0),
#endif /* DETECT_OVERFLOW ] */
   mpDownSampBuf(0)
{
   mFilterLen = DspResampling::smFilterLen;
   mHalfFilterLen = mFilterLen/2;

   mFold = iResamplingFold;
   mSubBandLength = mFilterLen/mFold;
   mFrameSizeInLowerRate = iLowRateSamples;
   mFrameSizeInHigherRate = mFrameSizeInLowerRate * mFold;
   if( mFold == 4) {
      if(iUpSamplingFlag) {
         //filter coefficients for up-sampling
#ifdef COMPARE_FILTERS /* [ */
         mpUp = (FILTER_Nn == mFilterLen) ? saCoeffUpN : saCoeffUpO;
#else /* COMPARE_FILTERS ] [ */
         mpUp = saCoeffUpN;
#endif /* COMPARE_FILTERS ] */
         mpUpSampBuf = new int[mSubBandLength+mFrameSizeInLowerRate-1];
         memset(mpUpSampBuf, 0, (sizeof(int) * (mSubBandLength - 1)));
      } else {
         //filter coefficients for down-sampling
#ifdef COMPARE_FILTERS /* [ */
         mpDown = (FILTER_Nn == mFilterLen) ? saCoeffDownN : saCoeffDownO;
#else /* COMPARE_FILTERS ] [ */
         mpDown = saCoeffDownN;
#endif /* COMPARE_FILTERS ] */
         mpDownSampBuf = new int[mFilterLen+mFrameSizeInHigherRate-1];
         memset(mpDownSampBuf, 0, (sizeof(int) * (mFilterLen - 1)));
      }
   }
#ifdef DETECT_OVERFLOW /* [ */
   mReport = smStatsReports;
#endif /* DETECT_OVERFLOW ] */

	int i;

	/* clear filters */
	for (i=0; i<8; i++) {
		state1[i]=0;
		state2[i]=0;
		state3[i]=0;
		state4[i]=0;
	}

}

// Destructor
DspResampling::~DspResampling()
{
    if(mpUpSampBuf)   delete[] mpUpSampBuf;
    if(mpDownSampBuf) delete[] mpDownSampBuf;
#ifdef DETECT_OVERFLOW /* [ */
    stats();
#endif /* DETECT_OVERFLOW ] */
}

/* ============================ MANIPULATORS ============================== */

#ifdef DETECT_OVERFLOW /* [ */
int DspResampling::resampleStats()
{
   return smStatsReports++; // trigger another report and reset
}
#endif /* DETECT_OVERFLOW ] */

#ifdef COMPARE_FILTERS /* [ */
int DspResampling::setFilter(int useNew)
{
   int save = DspResampling::smFilterLen;

   DspResampling::smFilterLen = useNew ? FILTER_Nn : FILTER_No;
/*
   osPrintf("setFilter: using %s filter.\n", useNew ? "new" : "old");
   osPrintf("     saCoeffUpN @ 0x%08X,   saCoeffUpO @ 0x%08X,\n"
            "   saCoeffDownN @ 0x%08X, saCoeffDownO @ 0x%08X\n",
          saCoeffUpN, saCoeffUpO, saCoeffDownN, saCoeffDownO);
   printf("now setting filter to %i\n",DspResampling::smFilterLen);
*/
   taskDelay(20);

   return save;
}
#endif /* COMPARE_FILTERS ] */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

#ifdef DETECT_OVERFLOW /* [ */
void DspResampling::stats()
{
   if ((mTotalSamplesD + mTotalSamplesU) > 0) {
      if (mOverflowsD > 0) {
         osPrintf(
          "DspResamp(0x%p): downsampling overflowed %d times in %d samples\n",
            this, mOverflowsD, mTotalSamplesD);
      }
      if (mOverflowsU > 0) {
         osPrintf(
          "DspResamp(0x%p): upsampling overflowed %d times in %d samples\n",
            this, mOverflowsU, mTotalSamplesU);
      }
      if (0 == (mOverflowsU + mOverflowsD)) {
         osPrintf("DspResamp(0x%p): no overflows in %d samples\n",
            this, mTotalSamplesD + mTotalSamplesU);
      }
      if (mpUpSampBuf) {
         osPrintf("DspResamp(0x%X): ranges: in(%d .. %d) out(%d .. %d) (UP)\n",
            this, mInMinU, mInMaxU, mMinU, mMaxU);
      }
      if (mpDownSampBuf) {
         osPrintf("DspResamp(0x%X): ranges: in(%d .. %d)"
            " out(%d .. %d) (DOWN)\n",
            this, mInMinD, mInMaxD, mMinD, mMaxD);
      }

   }
   mMinU = mMaxU = mMinD = mMaxD = mInMinU = mInMaxU = mInMinD = mInMaxD = 0;
   mOverflowsU = mOverflowsD = mTotalSamplesD = mTotalSamplesU = 0;
}
#endif /* DETECT_OVERFLOW ] */

#if /*!defined(_WIN32) &&*/ !defined(__pingtel_on_posix__) /* [ */
void DspResampling::up(Sample *dest, Sample *data, int lGain, int iSpkrPhoneFlag)
{
   int       i;
   int       j;
   int       l;
   int       *lp;
   const int *lp0;
   int       *lp1;
   int       *lp2;
   int       lSam;
   int       lSamL;   // Loudspeaker channel
   int       lSamH;   // handset speaker channel


   	//Hack to insert Global IP Sound Resamling filters
   mFilterLen = DspResampling::smFilterLen;

	if(mFilterLen==96){

      //osPrintf("G_up ");
		//From 80 to 160 samples
		GIPS_upsampling2(data,GIPS_FRAME_LENGTH,speech16,state1);
		// From 160 to 320 samples
		GIPS_upsampling2(speech16,GIPS_FRAME_LENGTH<<1, speech32,state2);

   		for (i = 0; i < mFrameSizeInHigherRate; i++) {
			 *dest++ =((int)speech32[i]*lGain)>>15;
			 *dest++ =iSpkrPhoneFlag? 0:speech32[i]; 	// too slow, re-write
   		};

    		return;
    	}



#ifdef DETECT_OVERFLOW /* [ */
   if (mReport < smStatsReports) {
      mReport = smStatsReports;
      stats();
   }
#endif /* DETECT_OVERFLOW ] */

   lp = mpUpSampBuf + mSubBandLength-1;

   //Copy and convert data into processing buffer.
   for (i = 0; i < mFrameSizeInLowerRate; i++) {
      lSam = *data++;
#ifdef DETECT_OVERFLOW /* [ */
      if (mInMaxU < lSam) mInMaxU = lSam;
      if (mInMinU > lSam) mInMinU = lSam;
#endif /* DETECT_OVERFLOW ] */
      *lp++ = lSam;
   }

   lp2 = mpUpSampBuf + mSubBandLength-1;
   // Lowpass filtering, cut-off frequency 4000 Hz
   for (l = 0; l < mFrameSizeInLowerRate; l++) {
      lp0 = mpUp;
      lp = lp2++;
      for (i = 0; i < mFold; i++) {
         lp1  = lp;
         lSam = 0L;
         for (j = 0; j < mSubBandLength; j++) {
            lSam += *lp0++ * *lp1--;
         }
         lSam >>= (13-2);  // 13: Q12, plus 1 for good luck to prevent overflow
                           // -2: must boost by factor of 4 (inserted 0).

         // lSam = (lSam * 10650) >> 13;  // scale up by 1.3 ( = 10650/8192)
         lSam = (lSam * smUpGain) >> 13;  // Experimenting...
// lGain reflects the echo suppression in the application of speakerphone,
// when the handset output is set to zero to avoid possible audio loop back.

         lSamL = (lSam * lGain) >> 15;    // Loudspeaker channel
         lSamH = iSpkrPhoneFlag? 0:lSam;  // handset speaker channel

#ifdef DETECT_OVERFLOW /* [ */
         if (mMaxU < lSamL) mMaxU = lSamL;
         if (mMinU > lSamL) mMinU = lSamL;
         if (mMaxU < lSamH) mMaxU = lSamH;
         if (mMinU > lSamH) mMinU = lSamH;

         if (lSamL > 32767) {
            lSamL = 32767;
            mOverflowsU++;
         }
         if (lSamL < -32768) {
            lSamL = -32768;
            mOverflowsU++;
         }
         if (lSamH > 32767) {
            lSamH = 32767;
            mOverflowsU++;
         }
         if (lSamH < -32768) {
            lSamH = -32768;
            mOverflowsU++;
         }
#else /* DETECT_OVERFLOW ] [ */
         if (lSamL > 32767) {
            lSamL = 32767;
         }
         if (lSamL < -32768) {
            lSamL = -32768;
         }
         if (lSamH > 32767) {
            lSamH = 32767;
         }
         if (lSamH < -32768) {
            lSamH = -32768;
         }
#endif /* DETECT_OVERFLOW ] */

         *dest++ = lSamL;    // Loudspeaker channel
         *dest++ = lSamH;    // handset speaker channel
      }
   }
#ifdef DETECT_OVERFLOW /* [ */
   mTotalSamplesU += mFrameSizeInLowerRate;
#endif /* DETECT_OVERFLOW ] */

   // save the last few data points, for beginning of next frame
   memcpy(mpUpSampBuf, mpUpSampBuf + mFrameSizeInLowerRate,
      sizeof(int)*(mSubBandLength-1));
}

void DspResampling::down(Sample* output, Sample* input, int UseLeft)
{
   //Low-pass filtering and decimation.
   int       i;
   int       l;
   int*  lp0;
   int*  lp1;
   int*  lp2;
   const int*  lp;
   short*    shp = input;
   int   lSam;

#ifdef DETECT_OVERFLOW /* [ */
   if (mReport < smStatsReports) {
      mReport = smStatsReports;
      stats();
   }
#endif /* DETECT_OVERFLOW ] */

   	//Hack to insert Global IP Sound Resamling filters
      mFilterLen = DspResampling::smFilterLen;

   	if(mFilterLen==96){
	   //osPrintf("G_dn ");
		if (UseLeft == 0) shp++;
   		for (i = 0; i < mFrameSizeInHigherRate; i++) {
      			speech32[i]=*shp++;
              // speech32[i] >>= 1;  // this will bring the gain down by 6dB to
                                   // match the gain of an existing (non-GIPS)
                                   // resampling filter
      			shp++;   // skip unused channel
   		}
		GIPS_downsampling2(speech32, mFrameSizeInHigherRate,speech16,state3);
		GIPS_downsampling2(speech16, mFrameSizeInHigherRate>>1,output, state4);
    		return;
    	}


   // Copy the input samples into the processing buffer.  We convert them
   // from 16 bit to 32 bit in the copy.

   // But first, we might adjust the starting value of the source pointer
   // to select the correct channel.  The two input channels are interleaved
   // starting with right channel signal, so we increment the pointer if the
   // left channel is what we are supposed to be extracting.

   if (UseLeft == 0) shp++;

   // Copy the input buffer into processing buffer.
   lp0 = mpDownSampBuf + mFilterLen-1;
   for (i = 0; i < mFrameSizeInHigherRate; i++) {
      lSam = *shp++;
#ifdef DETECT_OVERFLOW /* [ */
      if (mInMaxD < lSam) mInMaxD = lSam;
      if (mInMinD > lSam) mInMinD = lSam;
#endif /* DETECT_OVERFLOW ] */
      *lp0++ = lSam;
      shp++;   // skip unused channel
   }

   lp0 = mpDownSampBuf + mFilterLen-1;

   //Linear filtering & decimation
   for (l = 0; l < mFrameSizeInLowerRate; l++) {
      lp   = mpDown;
      lp1  = lp0;
      lp2  = lp1 - mFilterLen + 1;
      lp0 += 4;
      lSam = 0L;
      for (i = 0; i < mHalfFilterLen; i++) {
         //Exploit coefficients' symmetry
         lSam += *lp++ * (*lp1-- + *lp2++);
      }

      lSam >>= 13;    //rescale:  coefficients in Q12, plus 1 for good luck
#ifdef DETECT_OVERFLOW /* [ */
      if (mMaxD < lSam) mMaxD = lSam;
      if (mMinD > lSam) mMinD = lSam;
#endif /* DETECT_OVERFLOW ] */
      if (lSam > 32767) {
         lSam = 32767;
#ifdef DETECT_OVERFLOW /* [ */
         mOverflowsD++;
#endif /* DETECT_OVERFLOW ] */
      }
      if (lSam < -32768) {
         lSam = -32767;
#ifdef DETECT_OVERFLOW /* [ */
         mOverflowsD++;
#endif /* DETECT_OVERFLOW ] */
      }
      output[l] = lSam;
   }
#ifdef DETECT_OVERFLOW /* [ */
   mTotalSamplesD += mFrameSizeInLowerRate;
#endif /* DETECT_OVERFLOW ] */

   //save last few data samples for beginning of next frame.
   memcpy(mpDownSampBuf, (mpDownSampBuf + mFrameSizeInHigherRate),
      sizeof(int)*(mFilterLen-1));
}

void DspResampling::upfrom16k(Sample *dest, Sample *data, int lGain, int iSpkrPhoneFlag)
{
   int       i;

	GIPS_upsampling2(data,GIPS_FRAME_LENGTH<<1,speech32,state1);

   	for (i = 0; i < mFrameSizeInHigherRate; i++) {
		*dest++ =((int)speech32[i]*lGain)>>15;
		*dest++ =iSpkrPhoneFlag? 0:speech32[i];

   	}
}

void DspResampling::downto16k(Sample* output, Sample* input, int UseLeft)
{
   //Low-pass filtering and decimation.
   int       i;
   short*    shp = input;

	if (UseLeft == 0) shp++;
   	for (i = 0; i < mFrameSizeInHigherRate; i++) {
     		speech32[i]=*shp++;
      		shp++;   // skip unused channel
   	}
	GIPS_downsampling2(speech32, GIPS_FRAME_LENGTH<<2,output,state3);
}
#endif /* ] _WIN32?, __pingtel_on_posix__ */
