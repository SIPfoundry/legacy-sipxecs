//
// Copyright (C) 2009 Nortel Networks, certain elements licensed under a Contributor Agreement.
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _OsMsgQ_h_
#define _OsMsgQ_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsTime.h"
#include "utl/UtlString.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS

// FORWARD DECLARATIONS
class OsMsg;

// TYPEDEFS
typedef UtlBoolean (*OsMsgQSendHookPtr) (const OsMsg& rMsg);
typedef void      (*OsMsgQFlushHookPtr) (const OsMsg& rMsg);

//:Message queue for inter-task communication

class OsMsgQBase
{
   friend class OsMsgQShared;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static const int DEF_MAX_MSGS;     // Default maximum number of messages
   static const int DEF_MAX_MSG_LEN;  // Default maximum msg length (in bytes)
   static const UtlString MSGQ_PREFIX; // Prefix for OsMsgQ names stored in
                                      //  the name database

   enum Options
   {
      Q_FIFO     = 0x0, // queue blocked tasks on a first-in, first-out basis
      Q_PRIORITY = 0x1  // queue blocked tasks based on their priority
   };
     //!enumcode: Q_FIFO - queues blocked tasks on a first-in, first-out basis
     //!enumcode: Q_PRIORITY - queues blocked tasks based on their priority


/* ============================ CREATORS ================================== */

   OsMsgQBase(const UtlString& name);
     //:Constructor
     // If the name is specified but is already in use, throw an exception.

   virtual
   ~OsMsgQBase();
     //:Destructor

   static OsMsgQBase* getMsgQByName(const UtlString& name);
     //:Return a pointer to the named queue, or NULL if not found

/* ============================ MANIPULATORS ============================== */


   virtual OsStatus send(const OsMsg& rMsg,
                         const OsTime& rTimeout=OsTime::OS_INFINITY) = 0;
     //:Insert a message at the tail of the queue and wait for a response
     // Wait until there is either room on the queue or the timeout expires.

   virtual OsStatus sendP(OsMsg* pMsg,
                          const OsTime& rTimeout=OsTime::OS_INFINITY) = 0;
     //:Insert a message at the tail of the queue
     // Wait until there is either room on the queue or the timeout expires.
     // Takes ownership of *pMsg.

   virtual OsStatus sendUrgent(const OsMsg& rMsg,
                               const OsTime& rTimeout=OsTime::OS_INFINITY) = 0;
     //:Insert a message at the head of the queue
     // Wait until there is either room on the queue or the timeout expires.

   virtual OsStatus sendFromISR(const OsMsg& rMsg) = 0;
     //:Insert a message at the tail of the queue
     // Sending from an ISR has a couple of implications.  Since we can't
     // allocate memory within an ISR, we don't create a copy of the message
     // before sending it and the sender and receiver need to agree on a
     // protocol (outside this class) for when the message can be freed.
     // The sentFromISR flag in the OsMsg object will be TRUE for messages
     // sent using this method.

   virtual OsStatus receive(OsMsg*& rpMsg,
                            const OsTime& rTimeout=OsTime::OS_INFINITY) = 0;
     //:Remove a message from the head of the queue
     // Wait until either a message arrives or the timeout expires.
     // Other than for messages sent from an ISR, the receiver is responsible
     // for freeing the received message.

   virtual void flush(void);
     //:Delete all messages currently in the queue

   virtual void setSendHook(OsMsgQSendHookPtr func);
     //:Set the function that is invoked whenever a msg is sent to the queue
     // The function takes the message to be sent as an argument and returns a
     // boolean value indicating whether the SendHook method has handled the
     // message. If TRUE, the message is not inserted into the queue (since it
     // has already been handled. If FALSE, the (possibly modified) message is
     // inserted into the queue.

   virtual void setFlushHook(OsMsgQFlushHookPtr func);
     //:Set the function that is invoked whenever a msg is flushed from the
     //:queue.  Messages get flushed when the OsMsgQ is deleted while there
     //:are messages still queued.
     // The function takes an OsMsg reference as an argument.

/* ============================ ACCESSORS ================================= */

   virtual int numMsgs(void) = 0;
     //:Return the number of messages in the queue

   int maxMsgs() const;
     //: Returns the maximum number of messages that can be queued

   virtual OsMsgQSendHookPtr getSendHook(void) const;
     //:Return a pointer to the current send hook function

   /** Return the global name for the message queue.
    *  The return pointer is valid only as long as the queue exists.
    */
   const UtlString* getName();

/* ============================ INQUIRY =================================== */

   virtual UtlBoolean isEmpty(void);
     //:Return TRUE if the message queue is empty, FALSE otherwise

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsMsgQSendHookPtr  mSendHookFunc;
     //:Method that is invoked whenever a message is sent to the queue

   OsMsgQFlushHookPtr mFlushHookFunc;
     //:Method that is invoked whenever a message is flushed from the queue

/* ---------------------------- DEBUG SCAFFOLDING ------------------------- */
protected:

   int      mMaxMsgs;// maximum number of messages the queue can hold

#if MSGQ_IS_VALID_CHECK
   virtual void testMessageQ() = 0;

   unsigned int mNumInsertEntry;
   unsigned int mNumInsertExitOk;
   unsigned int mNumInsertExitFail;

   unsigned int mNumRemoveEntry;
   unsigned int mNumRemoveExitOk;
   unsigned int mNumRemoveExitFail;

   unsigned int mLastSuccessTest;
#endif

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   UtlString mName;   // global name associated with the msg queue

   OsMsgQBase(const OsMsgQBase& rOsMsgQBase);
     //:Copy constructor (not implemented for this class)

   OsMsgQBase& operator=(const OsMsgQBase& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

// Depending on the native OS that we are running on, we include the class
// declaration for the appropriate lower level implementation and use a
// "typedef" statement to associate the OS-independent class name (OsMsgQ)
// with the OS-dependent realization of that type (e.g., OsMsgQWnt).
#if defined(_WIN32)
#  include "os/shared/OsMsgQShared.h"
   typedef class OsMsgQShared OsMsgQ;
#elif defined(_VXWORKS)
#  include "os/Vxw/OsMsgQVxw.h"
   typedef class OsMsgQVxw OsMsgQ;
#elif defined(__pingtel_on_posix__)
#  include "os/shared/OsMsgQShared.h"
   typedef class OsMsgQShared OsMsgQ;
#else
#  error Unsupported target platform.
#endif

#endif  // _OsMsgQ_h_
