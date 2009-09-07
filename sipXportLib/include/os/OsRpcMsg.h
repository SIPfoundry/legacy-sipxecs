//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsRpcMsg_h_
#define _OsRpcMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsMsg.h"
#include "os/OsMsgQ.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class OsEvent;

//:Message object used for synchronous (RPC-style) inter-task communication
// RPC inter-task communication works as follows:
// <p>The client task sends an RPC message to the server task. The RPC message
// includes a pointer to an OsEvent object. After sending the message, the
// client either polls or blocks while waiting for the completion event to
// be signaled. <p>When the server finishes the requested operation, it uses
// the OsEvent to specify an integer result and to signal that the operation
// has completed.

class OsRpcMsg : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum OsRpcMsgType
   {
      REQUEST                  // message from client to server
   };
     //!enumcode: Request - Message from client to server

/* ============================ CREATORS ================================== */

   OsRpcMsg(const unsigned char msgType, const unsigned char msgSubType,
            const OsEvent& rEvent);
     //:Constructor

   OsRpcMsg(const OsRpcMsg& rOsRpcMsg);
     //:Copy constructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)

   virtual
      ~OsRpcMsg();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsRpcMsg& operator=(const OsRpcMsg& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

   virtual int getMsgSize(void) const;
     //:Return the size of the message in bytes
     // This is a virtual method so that it will return the accurate size for
     // the message object even if that object has been upcast to the type of
     // an ancestor class.

   virtual OsEvent* getEvent(void) const;
     //:Return the pointer to the OsEvent object used to signal completion

   virtual UtlContainableType getContainableType() const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsEvent*  mpEvent;   // pointer to the OsEvent used to signal completion

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsRpcMsg_h_
