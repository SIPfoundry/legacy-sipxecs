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

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsConfigDb.h"
#include "os/OsUtil.h"
/* #include "pinger/Pinger.h" */
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MpCallFlowGraph.h"
#include "mp/MprToneGen.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS
static const int NO_WAIT = 0;
const int MprToneGen::MIN_SAMPLE_RATE = 8000;
const int MprToneGen::MAX_SAMPLE_RATE = 48000;

// STATIC VARIABLE INITIALIZATIONS
// Note: Both of the following variables are used only in this module.  They
//       are declared static and do not appear in the interface (.h) file for
//       the module.
static UtlBoolean sNeedsStaticInit = TRUE;
static char      sCallProgressTonesLocale[3] = { 0, 0, 0 };

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprToneGen::MprToneGen(const UtlString& rName,
                              int samplesPerFrame, int samplesPerSec,
                              const char* locale)
:  MpResource(rName, 0, 1, 1, 1, samplesPerFrame, samplesPerSec),
   mpToneGenState(NULL)
{
   // If we haven't yet initialized our static variables, do so now
   if (sNeedsStaticInit)
   {
      if (locale != NULL)
      {
         // get the call progress tones locale setting
         // (represented using an ISO-3166 two letter country code)
         strncpy(sCallProgressTonesLocale, locale, 2);
         sCallProgressTonesLocale[2] = '\0';
      }

      sNeedsStaticInit = FALSE;
   }

   mpToneGenState = MpToneGen_MpToneGen(samplesPerSec,
                                        sCallProgressTonesLocale);
}

// Destructor
MprToneGen::~MprToneGen()
{
   MpToneGen_delete(mpToneGenState);
}

/* ============================ MANIPULATORS ============================== */

#ifdef LATER
Later (soon) this will be incorporated, but this is not quite the right
implementation.  At least these changes are needed:
(1) this should be an overriding virtual function, named
    handleSetSamplesPerSec.
(2) MpResource (the base class) needs to be enhanced so that the base
    virtual function exists to be overridden.
// Sets the number of samples expected per second.
// Returns FALSE if the specified rate is not supported, TRUE otherwise.
UtlBoolean MprToneGen::setSamplesPerSec(int samplesPerSec)
{
   if (MprToneGen::MIN_SAMPLE_RATE > samplesPerSec)
      return FALSE;
   if (samplesPerSec > MprToneGen::MAX_SAMPLE_RATE)
      return FALSE;
   MpToneGen_delete(mpToneGenState);
   mpToneGenState = MpToneGen_MpToneGen(samplesPerSec);
   return TRUE;
}
#endif

// Sends a START_TONE message to this resource to begin generating
// an audio tone.
// Returns the result of attempting to queue the message to this
// resource.
OsStatus MprToneGen::startTone(int toneId)
{
   MpFlowGraphMsg msg(START_TONE, this, NULL, NULL, toneId);
   OsStatus       res;

   res = postMessage(msg);
   return res;
}

// Sends a STOP_TONE message to this resource to stop generating
// an audio tone.
// Returns the result of attempting to queue the message to this
// resource.
OsStatus MprToneGen::stopTone(void)
{
   MpFlowGraphMsg msg(STOP_TONE, this);
   OsStatus       res;

   res = postMessage(msg);
   return res;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprToneGen::doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame,
                                    int samplesPerSecond)
{
   MpBufPtr out = NULL;
   int16_t *outbuf;
   int count;
   OsStatus ret;

   if (0 == outBufsSize) return FALSE;
   *outBufs = NULL;
   if (0 == samplesPerFrame) return FALSE;
   if (isEnabled) {
      out = MpBuf_getBuf(MpMisc.UcbPool, samplesPerFrame, 0, MP_FMT_T12);
      assert(NULL != out);
      count = min(samplesPerFrame, MpBuf_getNumSamples(out));
      MpBuf_setNumSamples(out, count);
      outbuf = (int16_t*)MpBuf_getSamples(out);
      ret = MpToneGen_getNextBuff(mpToneGenState, outbuf, count);
      switch (ret) {
      case OS_WAIT_TIMEOUT: /* one-shot tone completed */
         ((MpCallFlowGraph*)getFlowGraph())->stopTone();
         MpBuf_setSpeech(out, MP_SPEECH_TONE);
         break;
      case OS_NO_MORE_DATA: /* silent */
         MpBuf_delRef(out);
         out = NULL;      // Will replace with silence before returning...
         break;
      case OS_SUCCESS:
      default:
         MpBuf_setSpeech(out, MP_SPEECH_TONE);
         break;
      }
   } else {
      if (0 < inBufsSize) out = *inBufs;
      *inBufs = NULL;
   }

   if (NULL == out) {
      out = MpBuf_getFgSilence();
   }
   *outBufs = out;

   return (NULL != mpToneGenState);
}

// Handle messages for this resource.
UtlBoolean MprToneGen::handleMessage(MpFlowGraphMsg& rMsg)
{
   int msgType;

   msgType = rMsg.getMsg();
   switch (msgType)
   {
   case START_TONE:
      MpToneGen_startTone(mpToneGenState, rMsg.getInt1());
      enable();
      break;
   case STOP_TONE:
      MpToneGen_stopTone(mpToneGenState);
      disable();
      break;
   default:
      return MpResource::handleMessage(rMsg);
      break;
   }
   return TRUE;
}

/* ============================ FUNCTIONS ================================= */
