//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpStreamMsg_h_
#define _MpStreamMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlString.h>

#include "os/OsMsg.h"
#include "mp/StreamDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Message object used to communicate with the media processing task
class MpStreamMsg : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   // Phone set message types
   enum MpStreamMsgType
   {
      STREAM_REALIZE_URL,
      STREAM_REALIZE_BUFFER,
      STREAM_PREFETCH,
      STREAM_PLAY,
      STREAM_REWIND,
      STREAM_PAUSE,
      STREAM_STOP,
      STREAM_DESTROY
   };

/* ============================ CREATORS ================================== */

   MpStreamMsg(int msg, UtlString& target, StreamHandle handle, void* pPtr1=NULL, void* pPtr2=NULL,
         intptr_t int1=-1, intptr_t int2=-1);
     //:Constructor

   MpStreamMsg(const MpStreamMsg& rMpStreamMsg);
     //:Copy constructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)

   virtual
   ~MpStreamMsg();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   MpStreamMsg& operator=(const MpStreamMsg& rhs);
     //:Assignment operator

   void setTarget(UtlString& target);
     //:Sets the target id of the stream message

   void setHandle(StreamHandle handle);
     //:Sets the stream handle of the stream message

   void setPtr1(void* p);
     //:Sets pointer 1 (void*) of the stream message

   void setPtr2(void* p);
     //:Sets pointer 2 (void*) of the stream message

   void setInt1(intptr_t i);
     //:Sets integer 1 of the stream message

   void setInt2(intptr_t i);
     //:Sets integer 2 of the stream message

/* ============================ ACCESSORS ================================= */

   int getMsg(void) const;
     //:Returns the type of the stream  message

   UtlString getTarget(void) const;
     //:Return the target id of the stream message

   StreamHandle getHandle(void) const;
     //:Return stream handle of stream msg

   void* getPtr1(void) const;
     //:Return pointer 1 (void*) of the stream message

   void* getPtr2(void) const;
     //:Return pointer 2 (void*) of the stream message

   intptr_t getInt1(void) const;
     //:Return integer 1 of the media stream message

   intptr_t getInt2(void) const;
     //:Return integer 2 of the media stream message

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlString     mTarget;   // Target ID
   StreamHandle mHandle;   // Stream Handle
   void*        mpPtr1;    // Message pointer 1
   void*        mpPtr2;    // Message pointer 2
   intptr_t     mInt1;     // Message integer 1
   intptr_t     mInt2;     // Message integer 2

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpStreamMsg_h_
