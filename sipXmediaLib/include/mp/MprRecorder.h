//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MprRecorder_h_
#define _MprRecorder_h_

// SYSTEM INCLUDES
#ifdef _VXWORKS /* [ */
  #include <ioLib.h>
#endif /* _VXWORKS ] */

#ifdef _WIN32 /* [ */
  #include <io.h>
#endif /* _WIN32 ] */

// APPLICATION INCLUDES
#include "os/OsMutex.h"
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpResource.h"
#include "os/OsProtectEvent.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS

struct MprRecorderStats
{
   double mTotalBytesWritten;
   double mTotalSamplesWritten;
   double mDuration;
   int mFinalStatus;
   int mDtmfTerm;
};

// TYPEDEFS

// FORWARD DECLARATIONS

//:The "Recorder" media processing resource
class MprRecorder : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum RecordFileFormat {
      RAW_PCM_16 = 0,
      WAV_PCM_16
   } ;

/* ============================ CREATORS ================================== */

   MprRecorder(const UtlString& rName, int samplesPerFrame, int samplesPerSec);
     //:Constructor

   virtual
   ~MprRecorder();
     //:Destructor

   enum Completion
   {
      RECORD_FINISHED,
      RECORD_STOPPED,
      RECORDING,
      WRITE_ERROR,
      RECORD_IDLE,
      INVALID_SETUP
   };

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus setup(int file, RecordFileFormat recFormat, int time = 0, int silenceLength = 5/*seconds*/, OsEvent* event = NULL);
   //: set parameters for next start; MUST BE disabled when this is called
   //! param: file - destination for record
   //! param: recFormat - output format type (e.g. WAV_PCM_16)
   //! param: time - max number of milliseconds to record, or 0 for no limit
   //! param: event - an optional OsEvent to signal on completion.

   virtual OsStatus begin(void);

   virtual OsStatus stop(void);

   virtual UtlBoolean disable(Completion code);

   virtual UtlBoolean enable(void);

   virtual UtlBoolean termDtmf(int currentToneKey);

   UtlBoolean closeRecorder();

/* ============================ ACCESSORS ================================= */

   void getRecorderStats(double& nBytes, double& nSamples, Completion& status);
   void getRecorderStats(struct MprRecorderStats* p);

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
  UtlBoolean updateWaveHeaderLengths(int handle);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   enum AddlMsgTypes
   {
      BEGIN = MpFlowGraphMsg::RESOURCE_SPECIFIC_START,
      STOP,
      SETUP
   };

   int mTermKey;
   int mFileDescriptor;
   RecordFileFormat mRecFormat;
   double mTotalBytesWritten;
   double mTotalSamplesWritten;
   int mConsecutiveInactive;
   int mSilenceLength;
   long mPrevEnergy;
   OsProtectedEvent* mpEvent;
   int mFramesToRecord;
   Completion mStatus;
   OsMutex  mMutex;


   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);
     //:Handle messages for this resource.

   MprRecorder(const MprRecorder& rMprRecorder);
     //:Copy constructor (not implemented for this class)

   MprRecorder& operator=(const MprRecorder& rhs);
     //:Assignment operator (not implemented for this class)

   UtlBoolean handleSetup(int file, int time, int silenceLength, OsProtectedEvent* event);

   UtlBoolean handleBegin(void);

   UtlBoolean handleStop(void);

   void progressReport(Completion code);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprRecorder_h_
