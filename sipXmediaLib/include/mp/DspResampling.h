//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//////////////////////////////////////////////////////////////////////////////
//
//       NOTE                      NOTE                    NOTE
//
//          This IS NOT a general purpose resampling object.
//
// It has the following very specific behaviors:
//
// 1.  It only converts between these two formats:
//  1A) 8000 samples per second, 1 channel of 16 bits per sample
//  1B) 32000 samples per second, 2 channels of 16 bits per sample
//
// 2.  It does the conversion between mono and stereo in support of our
//     particular use of the left and right channels as the speakerphone
//     and handset/headset connections, respectively.
//
// 3.  Each instance created EITHER does upsampling or downsampling,
//     BUT NOT BOTH
//
// 4.  It may only support 10 millisecond blocks, but I think that has
//     been generalized since the comment to that effect was inserted.
//
//////////////////////////////////////////////////////////////////////////////
// Outdated?
// This program assumes that the processing frame size is 80 samples.
// If some other frame size is to be used, the frame size setting needs to
// to be changed.
//////////////////////////////////////////////////////////////////////////////
#ifndef _DspResampling_  /* [ */
#define _DspResampling_

#undef DETECT_OVERFLOW
#undef COMPARE_FILTERS

#ifdef _VXWORKS /* [ */
// We should turn these off before release
#undef DETECT_OVERFLOW
#undef COMPARE_FILTERS

#define DETECT_OVERFLOW
#define COMPARE_FILTERS
#endif /* _VXWORKS ] */

#include "mp/DSP_type.h"
#include "mp/MpTypes.h"
#include "mp/resamplingby2.h"

//signal resampling.

// SYSTEM INCLUDES

// APPLICATION INCLUDES

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class DspResampling
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   DspResampling(int iResamplingFold, int iLowRateSamples, int iUpSamplingFlag);
     //:Constructor

   virtual
   ~DspResampling();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   void up(Sample* shpOut, Sample* shpIn, int lGain, int iSpeakerFlag);
   void down(Sample* shpOut, Sample* shpIn, int UseLeft);
void upfrom16k(Sample *dest, Sample *data, int lGain, int iSpkrPhoneFlag);
void downto16k(Sample* output, Sample* input, int UseLeft);
   static int setLimits(int limit);

#define GIPS_FRAME_LENGTH 80
#ifdef DETECT_OVERFLOW /* [ */
   static int resampleStats();
#endif /* DETECT_OVERFLOW ] */

#ifdef COMPARE_FILTERS /* [ */
   static int setFilter(int which);
#endif /* COMPARE_FILTERS ] */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   enum{FILTER_No = 48,         // # of filter coefficients (old)
        FILTER_Nn = 96};        // # of filter coefficients (new)

#ifdef COMPARE_FILTERS /* [ */
   static const int saCoeffDownO[FILTER_No/2]; // Downsampling coefficients
   static const int saCoeffUpO[FILTER_No];     // Upsampling coefficients
#endif /* COMPARE_FILTERS ] */

   static const int saCoeffDownN[FILTER_Nn/2]; // Downsampling coefficients
   static const int saCoeffUpN[FILTER_Nn];     // Upsampling coefficients

#ifdef DETECT_OVERFLOW /* [ */
   static int smStatsReports;
#endif /* DETECT_OVERFLOW ] */

   enum{MAX_AMPLITUDE = 27000};
   static int smPosSampleLimit;
   static int smNegSampleLimit;

   static int smFilterLen;

   const int *mpDown;      // Downsampling coefficients
   const int *mpUp;        // Upsampling coefficients

   int        *mpUpSampBuf;    // Upsampling work space
   int        *mpDownSampBuf;  // Downsampling work space

   int        mFrameSizeInLowerRate;   // # of samples
   int        mFrameSizeInHigherRate;  // # of samples (per channel!)

   int        mFilterLen;
   int        mHalfFilterLen;

   int        mFold;           // essentially, rate ratio ( == 4)
   int        mSubBandLength;  // # coefficients / iFold  ( == 12)

// GIPS variables
GIPS_Word32 state1[8];
GIPS_Word32 state2[8];
GIPS_Word32 state3[8];
GIPS_Word32 state4[8];

GIPS_Word16 speech16[160];
GIPS_Word16 speech32[320];

#ifdef DETECT_OVERFLOW /* [ */
   int        mOverflowsD;
   int        mTotalSamplesD;
   int        mOverflowsU;
   int        mTotalSamplesU;
   int        mMaxD;
   int        mMinD;
   int        mMaxU;
   int        mMinU;
   int        mReport;
   int        mInMaxD;
   int        mInMinD;
   int        mInMaxU;
   int        mInMinU;

   void stats(void);
#endif /* DETECT_OVERFLOW ] */

   DspResampling(const DspResampling& rDspResampling);
   //:Copy constructor (not implemented for this class)

   DspResampling& operator=(const DspResampling& rhs);
   //:Assignment operator (not implemented for this class)
};

/* ============================ INLINE METHODS ============================ */

#endif  /* _DspResampling_   ] */
