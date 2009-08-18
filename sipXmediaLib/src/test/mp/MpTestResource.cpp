//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/MpBuf.h"
#include "mp/MpTestResource.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
static const int RESOURCE_MSG_TYPE = MpFlowGraphMsg::RESOURCE_SPECIFIC_START;

// MpTestResource is a descendant of the MpResource class that we use for
// testing.

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   int numFramesProcessed(void);
     //:Returns the count of the number of frames processed by this resource.

   int numMsgsProcessed(void);
     //:Returns the count of the number of messages successfully processed by
     //:this resource.

/* ============================ INQUIRY =================================== */

   MpBufPtr getInputBuffer(int inPortIdx);
     //:Returns the input buffer for "inPortIdx" (or NULL if none available).

// Constructor
MpTestResource::MpTestResource(const UtlString& rName, int minInputs,
                               int maxInputs, int minOutputs,
                               int maxOutputs, int samplesPerFrame,
                               int samplesPerSec)
:  MpResource(rName, minInputs, maxInputs, minOutputs, maxOutputs,
              samplesPerFrame, samplesPerSec),
   mGenOutBufMask(0),
   mProcessInBufMask(0),
   mProcessedCnt(0),
   mMsgCnt(0),
   mLastMsg(0) //mLastMsg(NULL)
{
   // all of the work is done by the initializers
}

// Destructor
MpTestResource::~MpTestResource()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Sends a test message to this resource.
void MpTestResource::sendTestMessage(void* ptr1, void* ptr2,
                                     int int3, int int4)
{
   MpFlowGraphMsg msg(RESOURCE_MSG_TYPE, this, ptr1, ptr2, int3, int4);
   OsStatus       res;

   res = postMessage(msg);
}

// Specify the genOutBufMask.
// For each bit in the genOutBufMask that is set, if there is a
// resource connected to the corresponding output port, doProcessFrame()
// will create an output buffer on that output port.
void MpTestResource::setGenOutBufMask(int mask)
{
   mGenOutBufMask = mask;
}

// Specify the processInBufMask.
// For each bit in the processInBufMask that is set, doProcessFrame()
// will pass the input buffer from the corresponding input port,
// straight through to the corresponding output port.  If nothing is
// connected on the corresponding output port, the input buffer will
// be deleted.
void MpTestResource::setProcessInBufMask(int mask)
{
   mProcessInBufMask = mask;
}

/* ============================ ACCESSORS ================================= */

// Returns the count of the number of frames processed by this resource.
int MpTestResource::numFramesProcessed(void)
{
   return mProcessedCnt;
}

// Returns the count of the number of messages successfully processed by
// this resource.
int MpTestResource::numMsgsProcessed(void)
{
   return mMsgCnt;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Allocate a new buffer.
MpBufPtr MpTestResource::allocBuffer(void)
{
   return MpBuf_getBuf(MpMisc.UcbPool, 0, 0, MP_FMT_UNKNOWN);
}

// Free the buffer
void MpTestResource::freeBuffer(MpBufPtr pBuffer)
{
   if (pBuffer != NULL)
      MpBuf_delRef(pBuffer);
}

// Processes the next frame interval's worth of media.
UtlBoolean MpTestResource::doProcessFrame(MpBufPtr inBufs[],
                                         MpBufPtr outBufs[],
                                         int inBufsSize, int outBufsSize,
                                         UtlBoolean isEnabled,
                                         int samplesPerFrame,
                                         int samplesPerSecond)
{
   int i;

   // keep a copy of the arguments passed to this method
   mLastDoProcessArgs.inBufs           = inBufs;
   mLastDoProcessArgs.outBufs          = outBufs;
   mLastDoProcessArgs.inBufsSize       = inBufsSize;
   mLastDoProcessArgs.outBufsSize      = outBufsSize;
   mLastDoProcessArgs.isEnabled        = isEnabled;
   mLastDoProcessArgs.samplesPerFrame  = samplesPerFrame;
   mLastDoProcessArgs.samplesPerSecond = samplesPerSecond;

   for (i=0; i < outBufsSize; i++)
   {
      outBufs[i] = NULL;
      if (isOutputConnected(i))
      {
         if ((mProcessInBufMask & (1 << i)) &&
             (inBufsSize > i))
         {
            // if the corresponding bit in the mProcessInBufMask is set for
            // the input port then pass the input buffer straight thru
            outBufs[i] = inBufs[i];
            inBufs[i] = NULL;
         }

         if ((mGenOutBufMask & (1 << i)) &&
             (outBufs[i] == NULL))
         {
            // if the output buffer is presently NULL and the corresponding
            // bit in the mGenOutBufMask is set then allocate a new buffer
            // for the output port
            outBufs[i] = (MpBufPtr) allocBuffer();
         }
      }
   }

   for (i=0; i < inBufsSize; i++)
   {
      // if the corresponding bit in the mProcessInBufMask is set and we
      // haven't processed the input buffer then free it.
      if ((mProcessInBufMask & (1 << i)) &&
          (inBufs[i] != NULL))
      {
         freeBuffer(inBufs[i]);
         inBufs[i] = NULL;
      }
   }

   mProcessedCnt++;

   return TRUE;
}

// Handles messages for this resource.
UtlBoolean MpTestResource::handleMessage(MpFlowGraphMsg& rMsg)
{
   mLastMsg = rMsg;
   mMsgCnt++;

   if (rMsg.getMsg() == RESOURCE_MSG_TYPE)
      return TRUE;
   else
      return MpResource::handleMessage(rMsg);
}

/* ============================ FUNCTIONS ================================= */
