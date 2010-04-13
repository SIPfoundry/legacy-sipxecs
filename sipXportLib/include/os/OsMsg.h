//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsMsg_h_
#define _OsMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "utl/UtlContainable.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Base class for message queue buffers

class OsMsg : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum MsgTypes
   {
      UNSPECIFIED = 0,
      OS_SHUTDOWN,           // Task shutdown request message
      OS_TIMER,              // Timer request messages
      OS_EVENT,              // Event notification messages
      PS_MSG,                // Phone set messages
      PHONE_APP,             // Phone application messages
      MP_TASK_MSG,           // Media processing task messages
      MP_FLOWGRAPH_MSG,      // Media processing flowgraph messages
      MP_BUFFER_MSG,         // Media processing buffer queue messages
      SIP_PROXY_STATE,       // SIP proxy call state message
      TAO_MSG,               // Tao messages
      TAO_EVENT,             // Tao listener event messages
      TAO_LISTENER_EVENT_MSG,// Tao listener event message (call backs)
      PINGER_MSG,            // Pinger messages
           REFRESH_MSG,
           LINE_MGR_MSG,
      OS_SYSLOG,             // OS SysLog events
      STREAMING_MSG,         // Streaming related messages
      USER_START  = 128
   };
   //!enumcode: OS_SHUTDOWN - Task shutdown request message
   //!enumcode: OS_TIMER - Timer request messages
   //!enumcode: OS_EVENT - Event notification messages
   //!enumcode: PS_MSG - Phone set messages
   //!enumcode: PHONE_APP - Phone application class of messages
   //!enumcode: MP_TASK_MSG - Media processing task class of messages
   //!enumcode: MP_FLOWGRAPH_MSG - Media processing flowgraph class of messages
   //!enumcode: MP_BUFFER_MSG - Media processing buffer queue messages
   //!enumcode: SIP_PROXY_STATE - SIP proxy call state message
   //!enumcode: OS_SYSLOG - OS SysLog Messages
   //!enumcode: USER_START - User defined message type categories start at USER_START

/* ============================ CREATORS ================================== */

   OsMsg(const unsigned char msgType, const unsigned char msgSubType);
     //:Constructor

   OsMsg(const OsMsg& rOsMsg);
     //:Copy constructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)
     // This virtual (ordinary) method is used because one cannot have a
     // virtual copy constructor.  Thus one writes
     // "OsMsg* copyp = originalp->createCopy();" to copy *originalp
     // according to its run-time type, rather than
     // "OsMsg* copyp = new OsMsg(*originalp);" because the latter
     // would invoke the OsMsg copy constructor regardless of the
     // run-time type of *originalp.

   virtual void releaseMsg(void);
     //:Done with message, delete it or mark it unused

   virtual
      ~OsMsg();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsMsg& operator=(const OsMsg& rhs);
     //:Assignment operator

   virtual void setMsgSubType(unsigned char subType);
     //:Set the message subtype

   virtual void setSentFromISR(UtlBoolean sentFromISR);
     //:Set the SentFromISR (interrupt service routine) flag

   virtual void setReusable(UtlBoolean isReusable);
     //:Set the Is Reusable (from permanent pool) flag

   virtual void setInUse(UtlBoolean isInUse);
     //:Set the Is In Use flag

/* ============================ ACCESSORS ================================= */

   virtual unsigned char getMsgType(void) const;
     //:Return the message type

   virtual unsigned char getMsgSubType(void) const;
     //:Return the message subtype

   virtual int getMsgSize(void) const;
     //:Return the size of the message in bytes
     // This is a virtual method so that it will return the accurate size for
     // the message object even if that object has been upcast to the type of
     // an ancestor class.

   virtual UtlBoolean getSentFromISR(void) const;
     //:Return TRUE if msg was sent from an interrupt svc routine, else FALSE

   virtual UtlBoolean isMsgReusable(void) const;
     //:Return TRUE if msg is from a permanent pool, else FALSE

   virtual UtlBoolean isMsgInUse(void) const;
     //:Return TRUE if msg is currently in use, else FALSE

   //! Implements the interface for a UtlContainable
   virtual unsigned hash() const;

   virtual UtlContainableType getContainableType() const;

   virtual int compareTo(UtlContainable const *) const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   unsigned char mMsgType;
   unsigned char mMsgSubType;
   UtlBoolean     mSentFromISR;
   UtlBoolean     mReusable;
   UtlBoolean     mInUse;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsMsg_h_
