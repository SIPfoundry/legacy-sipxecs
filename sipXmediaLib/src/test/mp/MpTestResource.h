//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _MpTestResource_h_
#define _MpTestResource_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpResource.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class mpFlowGraphMsg;

//:Descendant of the MpResource class used for testing.
class MpTestResource : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   //:Structure holding a snapshot of the args passed to the most recent
   //:call to doProcessFrame()
   struct DoProcessArgs
   {
      MpBufPtr* inBufs;
      MpBufPtr* outBufs;
      int       inBufsSize;
      int       outBufsSize;
      UtlBoolean isEnabled;
      int       samplesPerFrame;
      int       samplesPerSecond;
   };

   //:For now we use the following as a surrogate for real buffers
   struct Buffer
   {
      int value;
   };

   int            mGenOutBufMask;
   int            mProcessInBufMask;
   int            mProcessedCnt;
   int            mMsgCnt;
   MpFlowGraphMsg mLastMsg;
   DoProcessArgs  mLastDoProcessArgs;

/* ============================ CREATORS ================================== */

   MpTestResource(const UtlString& rName, int minInputs,
                  int maxInputs, int minOutputs,
                  int maxOutputs, int samplesPerFrame=80,
                  int samplesPerSec=8000);
     //:Constructor

   virtual
   ~MpTestResource();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   void sendTestMessage(void* ptr1, void* ptr2, int int3, int int4);
     //:Sends a test message to this resource.

   void setGenOutBufMask(int mask);
     // Specify the genOutBufMask.
     // For each bit in the genOutBufMask that is set, if there is a
     // resource connected to the corresponding output port, doProcessFrame()
     // will create an output buffer on that output port.

   void setProcessInBufMask(int mask);
     // Specify the processInBufMask.
     // For each bit in the processInBufMask that is set, doProcessFrame()
     // will pass the input buffer from the corresponding input port,
     // straight through to the corresponding output port.  If nothing is
     // connected on the corresponding output port, the input buffer will
     // be deleted.

/* ============================ ACCESSORS ================================= */

   int numFramesProcessed(void);
     //:Returns the count of the number of frames processed by this resource.

   int numMsgsProcessed(void);
     //:Returns the count of the number of messages successfully processed by
     //:this resource.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   MpBufPtr allocBuffer(void);
     //:Allocate a new buffer.

   void freeBuffer(MpBufPtr pBuffer);
     //:Free the buffer

   UtlBoolean doProcessFrame(MpBufPtr inBufs[], MpBufPtr outBufs[],
                            int inBufsSize, int outBufsSize,
                            UtlBoolean isEnabled, int samplesPerFrame=80,
                            int samplesPerSecond=8000);
     //:Processes the next frame interval's worth of media.

   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);
     //:Handles messages for this resource.

   MpTestResource(const MpTestResource& rMpTestResource);
     //:Copy constructor (not implemented for this class)

   MpTestResource& operator=(const MpTestResource& rhs);
     //:Assignment operator (not implemented for this class)
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpTestResource_h_
