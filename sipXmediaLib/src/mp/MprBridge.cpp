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
// #include "os/OsDefs.h"
// #include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MprBridge.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprBridge::MprBridge(const UtlString& rName,
                           int samplesPerFrame, int samplesPerSec)
:  MpResource(rName, 1, MAX_BRIDGE_PORTS, 1,
          MAX_BRIDGE_PORTS, samplesPerFrame, samplesPerSec),
   mPortLock(OsBSem::Q_FIFO, OsBSem::FULL)
{
   int i;

   mpConnectionIDs[0] = (1<<30);
   for (i=1; i<MAX_BRIDGE_PORTS; i++) {
      mpConnectionIDs[i] = -1;
   }
}

// Destructor
MprBridge::~MprBridge()
{
}

/* ============================ MANIPULATORS ============================== */

// Attach a connection container to an available port.
int MprBridge::connectPort(const MpConnectionID connID)
{
   int port = findFreePort();

   if (port > -1) {
      assert(-2 == mpConnectionIDs[port]);
      mpConnectionIDs[port] = connID;
   }
   return port;
}


// Disconnect a connection container from its port.
OsStatus MprBridge::disconnectPort(const MpConnectionID connID)
{
   int i;

   for (i=1; i<MAX_BRIDGE_PORTS; i++)
   {
      if (connID == mpConnectionIDs[i]) {
         // Found the port, mark it free
         mpConnectionIDs[i] = -1;
         return OS_SUCCESS;
      }
   }
   return OS_NOT_FOUND;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

//Find and return the index to an unused port pair
int MprBridge::findFreePort(void)
{
   int i;
   int found = -1;

   mPortLock.acquire();
   for (i=1; i<MAX_BRIDGE_PORTS; i++)
   {
      if (-1 == mpConnectionIDs[i]) {
         // Found a free port, we can put a new connection here.
         mpConnectionIDs[i] = -2;
         found = i;
         i = MAX_BRIDGE_PORTS;
      }
   }
   mPortLock.release();
   return found;
}

//Check whether this port is connected to both input and output
UtlBoolean MprBridge::isPortActive(int portIdx) const
{
   return (isInputConnected(portIdx) && isOutputConnected(portIdx));
}

UtlBoolean MprBridge::doProcessFrame(MpBufPtr inBufs[],
                                   MpBufPtr outBufs[],
                                   int inBufsSize,
                                   int outBufsSize,
                                   UtlBoolean isEnabled,
                                   int samplesPerFrame,
                                   int samplesPerSecond)
{
   int i;
   int inIdx;
   int outIdx;
   int n;
   int scale;
   int inputs;
   int N = samplesPerFrame;
   MpBufPtr in;
   MpBufPtr out;
   Sample* input;
   Sample* output;
   Sample* outstart;

   if (0 == outBufsSize) {
      Zprintf("MprBridge::doPF: outBufsSize = %d! (inBufsSize=%d)\n",
         outBufsSize, inBufsSize, 0,0,0,0);
      return FALSE;
   }
   if (inBufsSize != outBufsSize) {
      Zprintf("MprBridge::doPF: outBufsSize(%d) != inBufsSize(%d)\n",
         outBufsSize, inBufsSize, 0,0,0,0);
      return FALSE;
   }

   if (0 == inBufsSize)
      return FALSE;       // no input buffers, not allowed

   for (i=0; i < outBufsSize; i++) {
      outBufs[i] = NULL;
   }

   if (!isEnabled)
   { // Disabled.  Mix all remote inputs onto local speaker, and copy
     // our local microphone to all remote outputs.

      // First, count how many contributing inputs
      inputs = 0;
      for (inIdx=1; inIdx < inBufsSize; inIdx++) {
         if (isPortActive(inIdx)) {
       if((MpBuf_getSpeech(inBufs[inIdx]) != MP_SPEECH_SILENT) &&
                    (MpBuf_getSpeech(inBufs[inIdx]) != MP_SPEECH_COMFORT_NOISE))
                 inputs++;
         }
      }

      if (inputs > 0) {
         // Compute a scale factor to renormalize (approximately)
         scale = 0;
         while (inputs > 1) {
            scale++;
            inputs = inputs >> 1;
         }
         out = MpBuf_getBuf(MpMisc.UcbPool, N, 0, MP_FMT_T12);
         if (NULL == out) {
            Zprintf(
               "MprBridge::doPF(line #%d): MpBuf_getBuf() returned NULL!\n",
               __LINE__, 0,0,0,0,0);
            return FALSE;
         }

         outstart = MpBuf_getSamples(out);
         memset((char *) outstart, 0, N * sizeof(Sample));

         for (inIdx=1; inIdx < inBufsSize; inIdx++) {
            if (isPortActive(inIdx)) {
               output = outstart;
             //Mix only non-silent audio
               if((MpBuf_getSpeech(inBufs[inIdx]) != MP_SPEECH_COMFORT_NOISE) &&
                  (MpBuf_getSpeech(inBufs[inIdx]) != MP_SPEECH_SILENT) ) {
                  input = MpBuf_getSamples(inBufs[inIdx]);
                  n = min(MpBuf_getNumSamples(inBufs[inIdx]), samplesPerFrame);
                  for (i=0; i<n; i++) *output++ += (*input++) >> scale;
               }
            }
         }
      } else {
    //Local output==comfort noise if all remote inputs are disabled or silent
         out = MpMisc.comfortNoise;
         MpBuf_addRef(out);
      }
      outBufs[0] = out;

      in = inBufs[0];
      for (outIdx=1; outIdx < outBufsSize; outIdx++) {
         if (isPortActive(outIdx)) {
            outBufs[outIdx] = in;
            MpBuf_addRef(in);
         }
      }
      return TRUE;
   }

   // Enabled.  Mix together inputs onto outputs, with the requirement
   // that no output receive its own input.
   for (outIdx=0; outIdx < outBufsSize; outIdx++) {
      if (!isPortActive(outIdx)) continue; // if output unconnected

      // First, count how many contributing inputs
      inputs = 0;
      for (inIdx=0; inIdx < inBufsSize; inIdx++)
      {
         if ((inIdx != outIdx) && isPortActive(inIdx)) {
          /* Count in only non-silent and non-comfort-noise */
            if((inBufs[inIdx] != NULL) &&
               (MpBuf_getSpeech(inBufs[inIdx]) != MP_SPEECH_COMFORT_NOISE) &&
                   (MpBuf_getSpeech(inBufs[inIdx]) != MP_SPEECH_SILENT) ) {
                   inputs++;
            }
         }
      }

      if (inputs > 0) {
         // Compute a scale factor to renormalize (approximately)
         scale = 0;
         while (inputs > 1) {
            scale++;
            inputs = inputs >> 1;
         }
         out = MpBuf_getBuf(MpMisc.UcbPool, N, 0, MP_FMT_T12);
         if (NULL == out) {
            Zprintf(
               "MprBridge::doPF(line #%d): MpBuf_getBuf() returned NULL!\n",
               __LINE__, 0,0,0,0,0);
            return FALSE;
         }

         outstart = MpBuf_getSamples(out);
         memset((char *) outstart, 0, N * sizeof(Sample));

         for (inIdx=0; inIdx < inBufsSize; inIdx++)
         {
            if ((inIdx != outIdx) && isPortActive(inIdx))
            {
               output = outstart;
               /* Mix non-silent and non-comfort-noise inputs only */
               if((inBufs[inIdx] != NULL) &&
               (MpBuf_getSpeech(inBufs[inIdx]) != MP_SPEECH_COMFORT_NOISE) &&
                   (MpBuf_getSpeech(inBufs[inIdx]) != MP_SPEECH_SILENT) ) {
                   input = MpBuf_getSamples(inBufs[inIdx]);
                   n = min(MpBuf_getNumSamples(inBufs[inIdx]), samplesPerFrame);
                   for (i=0; i<n; i++) *output++ += (*input++) >> scale;
               }
            }
         }
      } else { // inputs==0 means only silence/comfort noise inputs present
          if(outIdx) {     //Remote output
             //Output silence to a remote if the input is silent
             out = inBufs[0];
             MpBuf_addRef(out);
          } else {            //Local output
             //Output comfort noise to local speaker if all remotes are silent
             //or comfort noise.
             out = MpMisc.comfortNoise;
             MpBuf_addRef(out);
         }
      }
      outBufs[outIdx] = out;
   }
   return TRUE;
}

/* ============================ FUNCTIONS ================================= */
