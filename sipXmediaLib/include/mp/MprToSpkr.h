//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MprToSpkr_h_
#define _MprToSpkr_h_

#define DETECT_SPKR_OVERFLOW
#undef DETECT_SPKR_OVERFLOW
// #define DETECT_SPKR_OVERFLOW  //MG 10-3-01, trying to understand where the signal clips

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "os/OsNotification.h"
#include "mp/MpMisc.h"
#include "mp/MpResource.h"
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpCodec.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef void (*TOSPEAKERHOOK)(const int nLength, Sample* samples) ;
// FORWARD DECLARATIONS

class DspResampling;

//:The "To Speaker" media processing resource
class MprToSpkr : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   enum{GTABLE_SIZE = 7};  /*$$$ */
   enum{VOLUME_CONTROL_TABLE_SIZE=64};
   enum{MIN_SPKR_DtoA=-32767};
   enum{MAX_SPKR_DtoA= 32767};

/* ============================ CREATORS ================================== */

   MprToSpkr(const UtlString& rName, int samplesPerFrame, int samplesPerSec);
     //:Constructor

   virtual
   ~MprToSpkr();
     //:Destructor

#if defined(_WIN32) || defined(__pingtel_on_posix__) /* [ */
   // I had to increase this on Win/NT because of the bursty nature of
   // the completion callbacks:  the waveOut operations send completion
   // acknowledgements in bursts covering 60 to 100 msecs at once.  At
   // 10 msec per buffer, this resulted in frequent starvation.
   enum { MAX_SPKR_BUFFERS = 12 };
   enum { MIN_SPKR_BUFFERS = 1 };
   enum { SKIP_SPKR_BUFFERS = 2 };
#ifdef REIMPLEMENT_CLARISIS_EQ /* [ */
   enum {EqFilterLen_ix = 24};

   static       int  smClarisisHandsetSpeakerEq[EqFilterLen_ix];
   int*              mpEqSave_ix;
   int               mLastSpkr_ix;
   int*              mpCurEq_ix;

   void SpeakerEqualization_ix(Sample* samples, int iLength);
#endif /* REIMPLEMENT_CLARISIS_EQ ] */

#endif /* WIN32 ] */

#ifdef ORIGINAL
   enum AttenValues {
      ATTEN_LOUDEST = 0,    // 0 dB, no attenuation
      ATTEN_QUIETEST = -30, // Please do not make this lower than -48
      ATTEN_RAMP_DELTA = 3, // in dB
   };
#else
   enum AttenValues {
      ATTEN_LOUDEST = 0,                // 0 dB, no attenuation
      ATTEN_QUIETEST = 1-GTABLE_SIZE,   // -6.00 dB
      ATTEN_QUIET = 1-GTABLE_SIZE,      // -6.00 dB
      ATTEN_MIDDLE = 1-GTABLE_SIZE,     // -6.00 dB
      ATTEN_RAMP_DELTA = 1,             //  1.00 dB
   };
#endif
/* ============================ MANIPULATORS ============================== */

void setAttenuation(int finalDb=ATTEN_LOUDEST, int framesPerStep=1);

static int setInitAtten(int gain);
static int setInitVol(int gain, int volStep);

#ifdef TUNING_AUDIO_POP_DELAY /* [ */
static int setRampSteps(int nSteps);
#endif /* TUNING_AUDIO_POP_DELAY ] */

#ifdef DETECT_SPKR_OVERFLOW /* [ */
   static int spkrStats();
#endif /* DETECT_SPKR_OVERFLOW ] */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   enum AddlMsgTypes
   {
      PLAY_FILE = MpFlowGraphMsg::RESOURCE_SPECIFIC_START
   };

   enum{EqFilterLen = 24};


   static int      slInitAtten;
   static int      slInitVol;
   static int      slVolStep;

#ifdef TUNING_AUDIO_POP_DELAY /* [ */
   static       int sNRampSteps;
#else /* TUNING_AUDIO_POP_DELAY ] [ */
   static const int sNRampSteps;
#endif /* TUNING_AUDIO_POP_DELAY ] */

   int             mCurAttenDb;
   int             mMaxAttenDb;
   unsigned int    mulNoiseLevel;
   int             mlpAttenTable[GTABLE_SIZE];

   enum{MAX_SUPPRESSION = 7};
   int             mlpVolTable[VOLUME_CONTROL_TABLE_SIZE];
                                     /* volume levels + a mute level */

#ifdef DETECT_SPKR_OVERFLOW /* [ */

   static int smStatsReports;

   int        mOverflowsIn;
   int        mUnderflowsIn;
   int        mOverflowsOut;
   int        mUnderflowsOut;
   int        mTotalSamples;
   int        mMaxIn;
   int        mMinIn;
   int        mMaxOut;
   int        mMinOut;
   int        mReport;

   void stats(void);

#endif /* DETECT_SPKR_OVERFLOW ] */

   int        mCurRampStep;
   int        mCurVolumeFactor;
   int        mOldVolumeFactor;
   int        mTotalRampFactor;
   int        mTargetVolumeFactor;
   int        mLastVolume;

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

   void initAttenTable(void);

   void initVolTable(void);

   MprToSpkr(const MprToSpkr& rMprToSpkr);
     //:Copy constructor (not implemented for this class)

   MprToSpkr& operator=(const MprToSpkr& rhs);
     //:Assignment operator (not implemented for this class)

public:
    static TOSPEAKERHOOK s_fnToSpeakerHook ;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprToSpkr_h_
