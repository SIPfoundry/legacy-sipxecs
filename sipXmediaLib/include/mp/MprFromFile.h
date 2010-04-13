//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _MprFromFile_h_
#define _MprFromFile_h_

// SYSTEM INCLUDES
//#include <stdio.h>

// APPLICATION INCLUDES
#include "mp/dtmflib.h"
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpResource.h"
#include "os/OsProtectEvent.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:The "audio from file" media processing resource
class MprFromFile : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   MprFromFile(const UtlString& rName, int samplesPerFrame, int samplesPerSec);
     //:Constructor

   virtual
   ~MprFromFile();
     //:Destructor

   enum Completion // $$$ These need more thought and clarification...
   {
      PLAY_FINISHED,
      PLAY_STOPPED,
      PLAYING,
      READ_ERROR,
      PLAY_IDLE,
      INVALID_SETUP
   };

/* ============================ MANIPULATORS ============================== */

    //Plays buffer
    // type can be one of following:  (need a OsSoundType)
    // 0 = RAW
    // 1 = muLaw
    OsStatus playBuffer(const char* audioBuffer, unsigned long bufSize,
                                 int type, UtlBoolean repeat, OsProtectedEvent* notify);
     //: play sound from buffer w/ repeat option
     // TODO:
     //! param: repeat - TRUE/FALSE after the fromFile reaches the end of the
     //  file, go back to the beginning and continue to play.  Note this
     //  assumes that the file was opened for read.
     // Returns the result of attempting to queue the message to this
     // resource and/or opening the named file.


   OsStatus playFile(const char* fileName, UtlBoolean repeat,
      OsNotification* event = NULL);
     //: play from file w/ file name and repeat option
     // Opens file and calls <I>playFile</I> method taking FILE*
     // Note: if this resource is deleted before <I>stopFile</I> is called, it
     // will close the file.
     //! param: pFile - name of file from which to read raw audio data in exact
     //  format of the flowgraph (sample size, rate & number of channels).
     //! param: repeat - TRUE/FALSE after the fromFile reaches the end of the
     //  file, go back to the beginning and continue to play.  Note this
     //  assumes that the file was opened for read.
     // Returns the result of attempting to queue the message to this
     // resource and/or opening the named file.

   OsStatus stopFile(void);
     //: stop playing from file
     // Sends a STOP_FILE message to this resource to stop playing audio
     // from file
     // Returns the result of attempting to queue the message to this
     // resource.

   virtual UtlBoolean enable(void);
   virtual UtlBoolean disable(void);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   enum AddlMsgTypes
   {
      PLAY_FILE = MpFlowGraphMsg::RESOURCE_SPECIFIC_START,
      STOP_FILE
   };

   enum MessageAttributes
   {
       PLAY_ONCE,
       PLAY_REPEAT
   };

   UtlString* mpFileBuffer;
   int mFileBufferIndex;
   UtlBoolean mFileRepeat;
   OsNotification* mpNotify;

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

   virtual UtlBoolean handleSetup(MpFlowGraphMsg& rMsg);
   virtual UtlBoolean handleStop(void);
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);
     //:Handle messages for this resource.

   MprFromFile(const MprFromFile& rMprFromFile);
     //:Copy constructor (not implemented for this class)

   MprFromFile& operator=(const MprFromFile& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprFromFile_h_
