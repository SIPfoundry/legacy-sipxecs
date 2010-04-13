//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpBufferMsg_h_
#define _MpBufferMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "mp/MpBuf.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Message object used to communicate with the media processing task
class MpBufferMsg : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   // Phone set message types
   enum MpBufferMsgType
   {
      AUD_RECORDED,
      AUD_PLAYED,
      AUD_RTP_RECV,
      AUD_RTCP_RECV,
      AUD_PLAY,
      ACK_EOSTREAM
   };

/* ============================ CREATORS ================================== */

   MpBufferMsg(int msg, int linenum=-1, MpBufPtr pTag=NULL,
                        Sample* pBuf=NULL, int int1=-1);
     //:Constructor

   MpBufferMsg(const MpBufferMsg& rMpBufferMsg);
     //:Copy constructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)

   virtual
   ~MpBufferMsg();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   MpBufferMsg& operator=(const MpBufferMsg& rhs);
     //:Assignment operator

   void setTag(MpBufPtr p, int index=0);
     //:Set buffer object pointer of the buffer message

   void setBuf(Sample* p, int index=0);
     //:Set data pointer of the buffer message

   void setLen(int i, int index=0);
     //:Set length of buffer data in the buffer message

   // the mpFrom is only set during construction

/* ============================ ACCESSORS ================================= */

   int getMsg(void) const;
     //:Return the type of the buffer message

   MpBufPtr getTag(int index=0) const;
     //:Return buffer object pointer from the buffer message

   Sample* getBuf(int index=0) const;
     //:Return data pointer from the buffer message

   int getLen(int index=0) const;
     //:Return length of buffer data from the buffer message

   int getFrom(void) const;
     //:Return creation line number from the buffer message

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   enum {MAX_BUFFERS_IN_MSG = 4};      // size of arrays

     // the descriptors of the buffers
   MpBufPtr mpTag[MpBufferMsg::MAX_BUFFERS_IN_MSG];
     // pointers to the data in the buffers
   Sample*  mpBuf[MpBufferMsg::MAX_BUFFERS_IN_MSG];
     // lengths of the data in the buffers
   int      mLen[MpBufferMsg::MAX_BUFFERS_IN_MSG];
   int      mFrom;      // the line number of the sender.

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpBufferMsg_h_
