//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////



// SYSTEM INCLUDES
#include <assert.h>
#include "os/OsDefs.h"

// APPLICATION INCLUDES

#include <os/OsMsg.h>
#include <os/OsMsgPool.h>

#include "mp/MpBuf.h"
#include "mp/MprToSpkr.h"
#include "mp/MpBufferMsg.h"
#include "mp/dsplib.h"
#include "mp/MpMediaTask.h"

int iTrainingNoiseFlag = 0;
static int iComfortNoiseFlag = 1;
/*
int comfortNoise(int Flag) {
   int save = iComfortNoiseFlag;
   iComfortNoiseFlag = Flag;
   return (save);
}
*/

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS
static const int NO_WAIT = 0;

// STATIC VARIABLE INITIALIZATIONS
TOSPEAKERHOOK MprToSpkr::s_fnToSpeakerHook = NULL ;

static const int DEF_INIT_VOL = 363188;
static const int DEF_VOL_STEP = 27500; // approx. 2dB

int MprToSpkr::slInitAtten = 32768;
int MprToSpkr::slInitVol   = DEF_INIT_VOL;
int MprToSpkr::slVolStep   = DEF_VOL_STEP;

#define DEFAULT_RAMP_STEPS 256
#ifndef TUNING_AUDIO_POP_DELAY /* [ */
const
#endif /* TUNING_AUDIO_POP_DELAY ] */
   int MprToSpkr::sNRampSteps = DEFAULT_RAMP_STEPS;

#ifdef TUNING_AUDIO_POP_DELAY /* [ */
extern "C" {extern int RampSteps(int num);};

int RampSteps(int num) {
   return MprToSpkr::setRampSteps(num);
}
#endif /* TUNING_AUDIO_POP_DELAY ] */

#ifdef DETECT_SPKR_OVERFLOW /* [ */
int MprToSpkr::smStatsReports = 0;
#endif /* DETECT_SPKR_OVERFLOW ] */

#ifdef REIMPLEMENT_CLARISIS_EQ /* [ */
#if defined(_WIN32) || defined(__pingtel_on_posix__) /* [ */
int MprToSpkr::smClarisisHandsetSpeakerEq[EqFilterLen_ix] =  {
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 74,
      231, -6883, 32767, -6883, 231, 74
   };
#endif /* WIN32 ] */
#endif /* REIMPLEMENT_CLARISIS_EQ ] */


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprToSpkr::MprToSpkr(const UtlString& rName,
                           int samplesPerFrame, int samplesPerSec)
:  MpResource(rName, 1, 1, 0, 2, samplesPerFrame, samplesPerSec),
   mCurAttenDb(ATTEN_LOUDEST),
   mMaxAttenDb(ATTEN_LOUDEST),
   mulNoiseLevel(1000L),
#ifdef DETECT_SPKR_OVERFLOW /* [ */
   mOverflowsIn(0),
   mUnderflowsIn(0),
   mOverflowsOut(0),
   mUnderflowsOut(0),
   mTotalSamples(0),
   mMaxIn(0),
   mMinIn(0),
   mMaxOut(0),
   mMinOut(0),
#endif /* DETECT_SPKR_OVERFLOW ] */

#ifdef REIMPLEMENT_CLARISIS_EQ /* [ */
#if defined(_WIN32) || defined(__pingtel_on_posix__) /* [ */
   mpEqSave_ix(NULL),
   mpCurEq_ix(NULL),
   mLastSpkr_ix(0),
#endif /* WIN32 ] */
#endif /* REIMPLEMENT_CLARISIS_EQ ] */

   mCurRampStep(0),
   mCurVolumeFactor(0),
   mOldVolumeFactor(0),
   mTargetVolumeFactor(0),
   mLastVolume(-1)
{
   int i;

   init_CNG();

#ifdef REIMPLEMENT_CLARISIS_EQ /* [ */
#if defined(_WIN32) || defined(__pingtel_on_posix__) /* [ */
   mpCurEq_ix = &MprToSpkr::smClarisisHandsetSpeakerEq[0];
   mLastSpkr_ix = CODEC_DISABLE_SPKR;

   mpEqSave_ix = new int[samplesPerFrame + EqFilterLen_ix - 1];
   memset(mpEqSave_ix, 0, (samplesPerFrame + EqFilterLen_ix - 1) * sizeof(int));
#endif /* WIN32 ] */
#endif /* REIMPLEMENT_CLARISIS_EQ ] */

   /*** Speaker volume control ***/
   /* Originally, it is set to control by the codec. It is moved to here for
      acoustic echo cancellation purpose */

   for (i = 0; i < (VOLUME_CONTROL_TABLE_SIZE); i++) {
      mlpVolTable[i] = 0;
   }

#ifdef DETECT_SPKR_OVERFLOW /* [ */
   mReport = smStatsReports;
#endif /* DETECT_SPKR_OVERFLOW ] */

}

// Destructor
MprToSpkr::~MprToSpkr()
{
#ifdef DETECT_SPKR_OVERFLOW /* [ */
   stats();
#endif /* DETECT_SPKR_OVERFLOW ] */
}

/* ============================ MANIPULATORS ============================== */

#ifdef DETECT_SPKR_OVERFLOW /* [ */
int MprToSpkr::spkrStats()
{
   return smStatsReports++; // trigger another report and reset
}
#endif /* DETECT_SPKR_OVERFLOW ] */

#if defined(__pingtel_on_posix__) /* [ */
int MpCodec_getVolume() { return 0;}
int MpCodec_isBaseSpeakerOn() { return 0;}
#endif /* __pingtel_on_posix__ ] */

void MprToSpkr::setAttenuation(int finalDb, int framesPerStep)
{
   assert((finalDb <= 0) && (finalDb >= ATTEN_QUIETEST));
   mMaxAttenDb = finalDb;

}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

#ifdef DETECT_SPKR_OVERFLOW /* [ */
void MprToSpkr::stats()
{
   if ((mOverflowsOut+mUnderflowsOut+mOverflowsIn+mUnderflowsIn) > 0) {
      osPrintf(
         "MprToSpkr(0x%p): volume control handled %d total samples\n"
         "  input under/overflows %d+%d, output %d+%d\n",
         this, mTotalSamples,
         mUnderflowsIn, mOverflowsIn, mUnderflowsOut, mOverflowsOut);
   } else {
      osPrintf("MprToSpkr(0x%p): no under/overflows in %d samples\n",
         this, mTotalSamples);
   }
/*
   osPrintf(" ranges: input(%d .. %d), output(%d .. %d)\n",
      mMinIn, mMaxIn, mMinOut, mMaxOut);
*/

   mMinIn = mMaxIn = mMinOut = mMaxOut = mOverflowsOut =
   mUnderflowsOut = mOverflowsIn = mUnderflowsIn = mTotalSamples = 0;
}
#endif /* DETECT_SPKR_OVERFLOW ] */

void MprToSpkr::initVolTable()
{

   __int64 tempGain;

   int i;

   mlpVolTable[0] = (int) (tempGain = MprToSpkr::slInitVol);
   mlpVolTable[1] = (int) tempGain;
   mlpVolTable[VOLUME_CONTROL_TABLE_SIZE-2] = 0;
   mlpVolTable[VOLUME_CONTROL_TABLE_SIZE-1] = 0;
   for (i = 2; i < (VOLUME_CONTROL_TABLE_SIZE-2); i++) {
      mlpVolTable[i] = (int) (tempGain = (tempGain * slVolStep)>>15);
   }

   osPrintf("MprToSpkr::gains:");
   for( i = 0; i < VOLUME_CONTROL_TABLE_SIZE; i++) {
      osPrintf("%c%d", ((7 == (i&7)) ? '\n' : ' '), mlpVolTable[i]);
   }
   osPrintf("\n");

}


#ifdef REIMPLEMENT_CLARISIS_EQ /* [ */
#if defined(_WIN32) || defined(__pingtel_on_posix__) /* [ */
void MprToSpkr::SpeakerEqualization_ix(Sample* shpSamples, int iLength)
{

   int         ShiftGain_ix = 13;
   int         i;
   int         c;
   int         iSample;
   int*        ip = mpEqSave_ix;
   const int*  pCurCoeff;
   Sample*     sp = shpSamples;

   const int* pCoeffSet = &MprToSpkr::smClarisisHandsetSpeakerEq[0];


   for (c = 1; c < EqFilterLen_ix; c++) {  // copy last few down to beginning
      *ip = ip[iLength];
      ip++;
   }

   for (i = 0; i < iLength; i++) { // convert new set to 32 bits
      *ip++ = *sp++;
   }


   for (i = 0; i < iLength; i++) {
      iSample = 0;
      ip = &mpEqSave_ix[i];
      pCurCoeff = pCoeffSet;
      for (c = 0; c < EqFilterLen_ix; c++) {
         iSample += *pCurCoeff++ * *ip++;
      }

      // rescale, then down 3 dB for Handset, +3dB for Headset
      iSample = ((iSample >> 15) * 13300) >> ShiftGain_ix;
      if (iSample < MIN_SPKR_DtoA) iSample = MIN_SPKR_DtoA;
      if (iSample > MAX_SPKR_DtoA) iSample = MAX_SPKR_DtoA;
      shpSamples[i] = iSample;
   }
}
#endif /* WIN32 ] */
#endif /* REIMPLEMENT_CLARISIS_EQ ] */

UtlBoolean MprToSpkr::doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame,
                                    int samplesPerSecond)
{
	MpBufferMsg* pMsg;
	MpBufferMsg* pFlush;
	OsMsgPool*   spkrPool;
	MpBufPtr     ob;
	MpBufPtr     out;
	Sample*      shpSamples;
	int          iLength;

	if (0 == inBufsSize)
	{
		return FALSE;
	}

#ifdef DETECT_SPKR_OVERFLOW /* [ */
	if (mReport < smStatsReports)
	{
		mReport = smStatsReports;
		stats();
	}
#endif /* DETECT_SPKR_OVERFLOW ] */

	out = *inBufs;
	if ((NULL != out) && isEnabled)
	{
		shpSamples = MpBuf_getSamples(out);
		iLength = MpBuf_getNumSamples(out);

		if (s_fnToSpeakerHook)
		{
			/*
			 * Allow an external identity to consume speaker data.  Ideally,
			 * this should probably become a different resource, but
			 * abstracting a new CallFlowGraph is a lot of work.
			 */

			s_fnToSpeakerHook(iLength, shpSamples) ;
		}

		/////////////////////////////////////////////////
        // samples ready for EQ processing //////////////
        /////////////////////////////////////////////////

#ifdef REIMPLEMENT_CLARISIS_EQ /* [ */
#if defined(_WIN32) || defined(__pingtel_on_posix__) /* [ */

		SpeakerEqualization_ix(shpSamples, iLength);

#endif /* WIN32 ] */
#endif /* REIMPLEMENT_CLARISIS_EQ ] */

		if(iTrainingNoiseFlag > 0)
		{
			/* generate white noise to test the performance if AEC only.
             * This is for parameter tweaking only. The original speaker
             * signal will be dropped.
             */
			MpBuf_delRef(out);
			out = MpBuf_getBuf(MpMisc.UcbPool, samplesPerFrame, 0, MP_FMT_T12);
			assert(NULL != out);
			shpSamples = MpBuf_getSamples(out);
            white_noise_generator(shpSamples, iLength, iTrainingNoiseFlag);
		}
        else
		{
			if(out == MpMisc.comfortNoise)
			{
				MpBuf_delRef(out);
				out = MpBuf_getBuf(MpMisc.UcbPool, samplesPerFrame, 0,
						MP_FMT_T12);
				assert(NULL != out);
				shpSamples = MpBuf_getSamples(out);
				if(iComfortNoiseFlag > 0)
				{
					comfort_noise_generator(shpSamples, samplesPerFrame,
							mulNoiseLevel);
				}
				else
				{
					memset((char *)shpSamples, 0 , iLength*2);
				}
			}
            else
			{
				background_noise_level_estimation(mulNoiseLevel, shpSamples,
						iLength);
			}
		}

#ifdef FLOWGRAPH_DOES_RESAMPLING /* [ */
		ob = MpBuf_getBuf(MpMisc.DMAPool, 640, 0, MP_FMT_T12);
        assert(NULL != ob);
        Sample* dest = MpBuf_getSamples(ob);

		mpDspResamp->up(dest, shpSamples, MpCodec_isBaseSpeakerOn());

		MpBuf_delRef(out);
        if (isOutputConnected(1))
		{
			MpBufPtr ob2 = MpBuf_getBuf(MpMisc.DMAPool, 640, 0, MP_FMT_T12);
            assert(NULL != ob2);
            short* dest2 = MpBuf_getSamples(ob2);
            memcpy(dest2, dest, MpBuf_getByteLen(ob));
            outBufs[1] = ob2;
		}
#else /* FLOWGRAPH_DOES_RESAMPLING ] [ */
		ob = out;
#endif /* FLOWGRAPH_DOES_RESAMPLING ] */

        while (MpMisc.pSpkQ && MpMisc.max_spkr_buffers < MpMisc.pSpkQ->numMsgs())
		{
			OsStatus  res;
            res = MpMisc.pSpkQ->receive((OsMsg*&) pFlush, OsTime::NO_WAIT);
            if (OS_SUCCESS == res)
			{
				MpBuf_delRef(pFlush->getTag());
				MpBuf_delRef(pFlush->getTag(1));
				pFlush->releaseMsg();
            }
			else
			{
				osPrintf("MprToSpkr: queue was full, now empty (res=%d)\n",
						res);
            }
		}

		if (isOutputConnected(0))
		{
			outBufs[0] = ob;
            MpBuf_addRef(ob);
		}

		MpBuf_setAtten(ob, 0);

		spkrPool = MpMediaTask::getMediaTask(0)->getBufferMsgPool();
        assert(NULL != spkrPool);
        pMsg = spkrPool ? (MpBufferMsg*) spkrPool->findFreeMsg() : NULL;
        if (NULL == pMsg)
		{
			pMsg = new MpBufferMsg(MpBufferMsg::AUD_PLAY, __LINE__);
		}
		else
		{
			pMsg->setTag(NULL);
            pMsg->setTag(NULL, 1);
            pMsg->setTag(NULL, 2);
            pMsg->setTag(NULL, 3);
		}

        pMsg->setMsgSubType(MpBufferMsg::AUD_PLAY);
        pMsg->setTag(ob);
        pMsg->setBuf(MpBuf_getSamples(ob));
        pMsg->setLen(MpBuf_getNumSamples(ob));


        if (MpMisc.pSpkQ && OS_SUCCESS == MpMisc.pSpkQ->send(*pMsg, OsTime::NO_WAIT))
		{
			*inBufs = NULL;

		 // Post a copy of this message to the MpMisc.pEchoQ so that it
		 // can be used in AEC calculations.
		 MpBufferMsg* pAECMsg = new MpBufferMsg(MpBufferMsg::ACK_EOSTREAM) ;

		 // TODO: We should pre-allocate a bunch of messages for
		 //       this purpose (see DmaMsgPool as an example).

		 MpBuf_addRef(ob) ;
		 pAECMsg->setTag(ob) ;
		 if (MpMisc.pEchoQ->numMsgs() >= MpMisc.pEchoQ->maxMsgs() ||  MpMisc.pEchoQ->send(*pAECMsg, OsTime::NO_WAIT) != OS_SUCCESS)
		 {
			 pAECMsg->releaseMsg() ;
			 MpBuf_delRef(ob) ;
		 }

		}
		else
		{
			if (pMsg->isMsgReusable())
			{
				pMsg->releaseMsg();
			}
            MpBuf_delRef(ob);
		}

		if (!pMsg->isMsgReusable())
		{
			delete pMsg;
		}
	}
	else
	{
		mCurAttenDb = mMaxAttenDb;
		mCurRampStep = mCurVolumeFactor = mOldVolumeFactor = 0;
		mTotalRampFactor = mTargetVolumeFactor = mLastVolume = 0;
	}

   return TRUE;
}

/* ============================ FUNCTIONS ================================= */
