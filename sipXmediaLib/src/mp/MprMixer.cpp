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
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MprMixer.h"

#ifndef ABS
#define ABS(x) (max((x), -(x)))
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS
static const int NO_WAIT = 0;

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprMixer::MprMixer(const UtlString& rName, int numWeights,
                           int samplesPerFrame, int samplesPerSec)
:  MpResource(rName, 1, numWeights, 1, 1, samplesPerFrame, samplesPerSec),
   mScale(0)
{
   int i;

   mNumWeights = max(0, min(numWeights, MAX_MIXER_INPUTS));
   for (i=0; i<numWeights; i++)
      mWeights[i] = 0;
}

// Destructor
MprMixer::~MprMixer()
{
   // no work to do
}

/* ============================ MANIPULATORS ============================== */

// Sets the weighting factors for the first "numWeights" inputs.
// For now, this method always returns TRUE.
UtlBoolean MprMixer::setWeights(int *newWeights, int numWeights)
{
   int            i;
   MpFlowGraphMsg msg(SET_WEIGHTS, this, NULL, NULL, numWeights);
   OsStatus       res;
   int*           weights;

   weights = new int[numWeights];  // allocate storage for the weights here.
   for (i=0; i < numWeights; i++)  //  the storage will be freed by
      weights[i] = newWeights[i];  //  handleMessage()

   msg.setPtr1(weights);

   res = postMessage(msg);
   return res;
}

// Sets the weighting factor for the "weightIndex" input.
// For now, this method always returns TRUE.
UtlBoolean MprMixer::setWeight(int newWeight, int weightIndex)
{
   MpFlowGraphMsg msg(SET_WEIGHT, this, NULL, NULL,
                      newWeight, weightIndex);
   OsStatus       res;

   res = postMessage(msg);
   return res;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprMixer::doProcessFrame(MpBufPtr inBufs[],
                                   MpBufPtr outBufs[],
                                   int inBufsSize,
                                   int outBufsSize,
                                   UtlBoolean isEnabled,
                                   int samplesPerFrame,
                                   int samplesPerSecond)
{
   int i;
   int in;
   int wgt;
   int n;
   int N = samplesPerFrame;
   MpBufPtr out;
   Sample* input;
   Sample* output;
   Sample* outstart;

   if (0 == outBufsSize) return FALSE;

   *outBufs = NULL;

   if ((0 == mScale) || (0 == inBufsSize)) {
      out = MpBuf_getFgSilence();
      *outBufs = out;
      return TRUE;       // scale factors are all zero, or
                         // no input buffers, return silence
   }

   if (!isEnabled)
   { // Disabled, return first input
      out = *inBufs;
      *inBufs = NULL;
      if (NULL == out) {
         out = MpBuf_getFgSilence();
      }
      *outBufs = out;
      return TRUE;
   }

   if (1 == mScale) { // must be only one weight != 0, and it is == 1
      out = NULL;
      for (i=0; i < inBufsSize; i++)
      {
         if (0 != mWeights[i])
         {
            out = inBufs[i];
            inBufs[i] = NULL;
            i = inBufsSize;   // all done, exit loop
         }
      }
      if (NULL == out) {
         out = MpBuf_getFgSilence();
      }
      *outBufs = out;
      return TRUE; // even if we did not find it (mNWeights > inBufsSize)
   }

   out = MpBuf_getBuf(MpMisc.UcbPool, N, 0, MP_FMT_T12);
   assert(NULL != out);
   *outBufs = out;

   outstart = MpBuf_getSamples(out);
   memset((char *) outstart, 0, N * sizeof(Sample));

   for (in=0; in < inBufsSize; in++)
   {
      wgt = mWeights[in];
      if ((NULL != inBufs[in]) && (0 != wgt))
      {
         output = outstart;
         input = MpBuf_getSamples(inBufs[in]);
         n = min(MpBuf_getNumSamples(inBufs[in]), samplesPerFrame);
         if (1 == wgt)
         {
            for (i=0; i<n; i++) *output++ += (*input++) / mScale;
         }
         else
         {
            for (i=0; i<n; i++) *output++ += (*input++ * wgt) / mScale;
         }
      }
   }
   return TRUE;
}

// Handle messages for this resource.
UtlBoolean MprMixer::handleMessage(MpFlowGraphMsg& rMsg)
{
   UtlBoolean boolRes;
   int       msgType;
   int*      weights;

   msgType = rMsg.getMsg();
   switch (msgType)
   {
   case SET_WEIGHT:
      return handleSetWeight(rMsg.getInt1(), rMsg.getInt2());
      break;
   case SET_WEIGHTS:
      weights = (int*) rMsg.getPtr1();
      boolRes = handleSetWeights(weights, rMsg.getInt1());
      delete[] weights; // delete storage allocated in the setWeights() method
      return boolRes;
      break;
   default:
      return MpResource::handleMessage(rMsg);
      break;
   }
}

// Handle the SET_WEIGHT message.
UtlBoolean MprMixer::handleSetWeight(int newWeight, int weightIndex)
{
   if (weightIndex < mNumWeights)
   {
      mScale -= ABS(mWeights[weightIndex]);
      mScale += ABS(newWeight);
      mWeights[weightIndex] = newWeight;
   }

   return TRUE;
}

// Handle the SET_WEIGHTS message.
UtlBoolean MprMixer::handleSetWeights(int *newWeights, int numWeights)
{
   int i;
   int wgt;

   mNumWeights = max(0, min(numWeights, MAX_MIXER_INPUTS));
   mScale = 0;
   for (i=0; i < numWeights; i++)
   {
      mWeights[i] = wgt = newWeights[i];
      mScale += ABS(wgt);
   }

   return TRUE;
}

/* ============================ FUNCTIONS ================================= */
