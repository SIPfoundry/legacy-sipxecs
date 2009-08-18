//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifdef WIN32
#define INSERT_SAWTOOTH
#undef INSERT_SAWTOOTH
#endif

// SYSTEM INCLUDES
#include <assert.h>
#include "os/OsDefs.h"

// APPLICATION INCLUDES
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MprFromMic.h"
#include "mp/MpBufferMsg.h"
#include "mp/dmaTask.h"
#include "mp/DSP_type.h"


// function prototype
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
MICDATAHOOK MprFromMic::s_fnMicDataHook = 0 ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprFromMic::MprFromMic(const UtlString& rName,
                           int samplesPerFrame, int samplesPerSec)
#ifdef  FLOWGRAPH_DOES_RESAMPLING /* [ */
:  MpResource(rName, 0, 0, 2, 2, samplesPerFrame, samplesPerSec),
#else /* FLOWGRAPH_DOES_RESAMPLING ] [ */
:  MpResource(rName, 0, 0, 1, 2, samplesPerFrame, samplesPerSec),
#endif /* FLOWGRAPH_DOES_RESAMPLING ] */
   mpDspResamp(0),
   mNumEmpties(0),
   mNumFrames(0)
{
   Init_highpass_filter800();
}

// Destructor
MprFromMic::~MprFromMic()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


UtlBoolean MprFromMic::doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond)
{
	MpBufPtr        out = NULL ;
	MpBufferMsg*    pMsg;

	if (0 == outBufsSize)
	{
		return FALSE;
	}


	// Clear the the number of empty frames every 512 frames
	mNumFrames++;
	if (0 == (mNumFrames & 0x1ff))
	{
		mNumEmpties = 0;
	}

	if (isEnabled)
	{
		// If the microphone queue (holds unprocessed mic data) has more then
		// the max_mic_buffers threshold, drain the queue until in range)
		OsMsgQ* pMicOutQ;
		pMicOutQ = MpMisc.pMicQ;
		while (pMicOutQ && MpMisc.max_mic_buffers < pMicOutQ->numMsgs())
		{
	        if (OS_SUCCESS == pMicOutQ->receive((OsMsg*&) pMsg,
					OsTime::NO_WAIT))
			{
				MpBuf_delRef(pMsg->getTag());
				MpBuf_delRef(pMsg->getTag(1));
				pMsg->releaseMsg();
			}
		}

		if (pMicOutQ && pMicOutQ->numMsgs() <= 0)
		{
//			osPrintf("MprFromMic: No data available (total frames=%d, starved frames=%d)\n",
//					mNumFrames, mNumEmpties);
		}
		else
		{
			if (pMicOutQ && OS_SUCCESS == pMicOutQ->receive((OsMsg*&) pMsg,
					OsTime::NO_WAIT))
			{
				out = pMsg->getTag();
				pMsg->releaseMsg();

				if (NULL != out)
				{
#ifdef REAL_SILENCE_DETECTION /* [ */
					Sample* shpTmpFrame;
					MpBufPtr tpBuf;
					int n;
#endif /* REAL_SILENCE_DETECTION ] */

					switch(MpBuf_getSpeech(out))
					{
						case MP_SPEECH_TONE:
							break;
						case MP_SPEECH_MUTED:
							MpBuf_setSpeech(out, MP_SPEECH_SILENT);
							break;
						default:
#ifdef REAL_SILENCE_DETECTION /* [ */
							Sample *shpSamples;
							n = MpBuf_getNumSamples(out);
							shpSamples = MpBuf_getSamples(out);

							tpBuf = MpBuf_getBuf(MpMisc.UcbPool, n, 0, MP_FMT_T12);
							assert(NULL != tpBuf);
							shpTmpFrame = MpBuf_getSamples(tpBuf);
							highpass_filter800(shpSamples, shpTmpFrame, n);

							if(0 == speech_detected(shpTmpFrame,n))
							{
								MpBuf_setSpeech(out, MP_SPEECH_SILENT);
							}
							else
							{
								MpBuf_setSpeech(out, MP_SPEECH_ACTIVE);
							}
							MpBuf_delRef(tpBuf);
#else /* REAL_SILENCE_DETECTION ] [ */
							// 24 April 2001 (HZM)  I am disabling this because it takes
							// too long to recognize the beginning of a talk spurt, and
							// causes the bridge mixer to drop the start of each word.
							MpBuf_isActiveAudio(out);
#endif /* REAL_SILENCE_DETECTION ] */
							break;
					}
				}
			}
		}

#ifdef INSERT_SAWTOOTH /* [ */
		if (NULL == out)
		{
			out = MpBuf_getBuf(MpMisc.UcbPool, MpMisc.frameSamples, 0, MP_FMT_T12);
		}
		MpBuf_insertSawTooth(out);
		MpBuf_setSpeech(out, MP_SPEECH_ACTIVE);
#endif /* INSERT_SAWTOOTH ] */

		if (s_fnMicDataHook)
		{
			//
			// Allow an external identity to source microphone data.  Ideally,
            // this should probably become a different resource, but abstracting
            // a new CallFlowGraph is a lot of work.
            //

			if (NULL == out)
			{
				out = MpBuf_getBuf(MpMisc.UcbPool, MpMisc.frameSamples, 0, MP_FMT_T12);
			}

			if (NULL != out)
			{
	            int n = 0;
				Sample* s = NULL;

				s = MpBuf_getSamples(out);
				n = MpBuf_getNumSamples(out);

				s_fnMicDataHook(n, s) ;

				MpBuf_setSpeech(out, MP_SPEECH_UNKNOWN);
				MpBuf_isActiveAudio(out);
			}
		}

		if (NULL == out)
		{
			out = MpBuf_getFgSilence();
		}
	}

	*outBufs = out;

	return TRUE;
}


/* ============================ FUNCTIONS ================================= */

static const short         shLambda = 32765;        // 0.9999 in Q15
static const short         shLambdaC =    3;        // 0.0001 in Q15

static const short         shLambdaSr = 30147;      // 0.92 in Q15
static const short         shLambdaCSr = 2621;      // 0.08 in Q15

static const short         shLambdaSf = 32702;      // 0.998 in Q15
static const short         shLambdaCSf =   67;      // 0.002 in Q15

int FromMicThresh = 3;
short MprFromMic::speech_detected(Sample* shpSample, int iLength)
{
   int i;
   static Word64S  llLTPower = 8000L;
   static Word64S  llSTPower = 80000L;

   static int      iSpeechHangOver = 0;
   static short    shThreshold = 21799;     // 2.661 in Q13

   Word32  ulSample;
   Word32  ulSampleH;
   int     iSpeechCounter = 0;
   Word32  tmp32;

   for( i = 0; i < iLength; i++) {
      ulSample = (Word32) abs(*shpSample++);
      ulSampleH = ulSample << 8;

      if( ulSampleH > (Word32) llSTPower) {
         tmp32 = (Word32) shLambdaCSr * ulSample;
         llSTPower *= (Word64S) shLambdaSr;
         llSTPower += (((Word64S) tmp32) << 8);
         llSTPower >>= 15;
      }
      else {
         tmp32 = (Word32) shLambdaCSf * ulSample;
         llSTPower *= (Word64S) shLambdaSf;
         llSTPower += (((Word64S) tmp32) << 8);
         llSTPower >>= 15;
      }

      //If STPower > 2.661 LTPower, then speech activity
      //Note 2.661 == 8.5 dB

      Word64S tmp64 = llLTPower * (Word64S) shThreshold;
      tmp64 >>= 13;

      if(iSpeechHangOver > 0) FromMicThresh = 2;
      else FromMicThresh = 3;
      if (llSTPower > (llLTPower*FromMicThresh) ) {
         iSpeechCounter++;
      }
      else if((2*llSTPower) <= (llLTPower*3) ) {
         /* long term */
         tmp32 = (Word32) shLambdaC * ulSample;
         llLTPower *= (Word64S) shLambda;
         llLTPower += (((Word64S) tmp32) << 8);
         llLTPower >>= 15;
      }

   }
   if(  (llSTPower>>4) > llLTPower )
      llLTPower = llSTPower >> 4;

   if(iSpeechCounter > 1)  {
      iSpeechHangOver = 30;
   }
   if(iSpeechHangOver) {
      iSpeechHangOver--;
      return (1);
   }
   else  {
      return (0);     // speech detected
   }
}

static const int             HP800_N = 10;
static const int             HP800_N_HALF = HP800_N/2 + 1;

static const short           shpB800[] = {15, 0, -123, -446, -844, 1542};
/*
 * shpB800[0] =   15;   // 0.0036158;     in Q12
 * shpB800[1] =    0;   // 0.0;           in Q12
 * shpB800[2] = -123;   //-0.0299701;     in Q12
 * shpB800[3] = -446;   //-0.1090062;     in Q12
 * shpB800[4] = -844;   //-0.2061356;     in Q12
 * shpB800[5] = 1542;   // 0.3765164;     in Q12, original value 0.7530327. Here we
 *                      // divide it by 2 to make the following arthmetic process simpler.
 */
void MprFromMic::Init_highpass_filter800(void)
{
  int i;
  for(i = 0; i < 80+HP800_N; i++) {
      shpFilterBuf[i] = 0;
  }
}

void MprFromMic::highpass_filter800(
                short *signal,    /* input signal */
                short *pOutput,   /* output signal */
                short lg)         /* length of signal    */
{
   short   i, j;
   Word32S lS;           //32bit temp storage
   short*  shp1;
   short*  shp2;
   short*  shp0;

   shp1 = shpFilterBuf;
   shp2 = shpFilterBuf + lg;
   for (i = 0; i < HP800_N; i++) {
      *shp1++ = *shp2++;
   }
   shp2 = signal;
   for (i = 0; i < lg; i++) {
      *shp1++ = *shp2++;
   }

   shp0 = shpFilterBuf;
   for(i = 0; i < lg; i++)
   {
      lS = 0L;
      shp1 = shp0++;
      shp2 = shp1 + HP800_N;
      for(j = 0; j < HP800_N_HALF; j++) {
         lS += (Word32S) (*shp1++ + *shp2--) * (Word32S) shpB800[j];
      }
      pOutput[i] = (lS>>12);
   }
   return;
}

#ifdef HF_ANALYSIS
   static unsigned long ulStrength = 1000L;
   static unsigned long ulStrength1 = 1000L;
   HF_HF(src, (samples<<3), &ulStrength, &ulStrength1);

   int kkk = ulStrength1/ulStrength;
#if 0
   if (ulStrength > 3000L && ulStrength < 6000L) {
      if(kkk < 16) osPrintf("NE ");
   }
#endif
   if (ulStrength >= 6000L && ulStrength < 9000L) {
      if(kkk < 12) osPrintf("NE1 ");
   }
   else if (ulStrength >= 9000L) {
      if( kkk < 9) osPrintf("NE2 ");
   }
   if(iShowHFFlag > 0) {
      osPrintf("%6d %6d %4d\n", ulStrength,ulStrength1,samples);
      iShowHFFlag--;
   }
#endif /* HF_ANALYSIS */
