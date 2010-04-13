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


#ifndef _OsMsgQShared_h_
#define _OsMsgQShared_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsCSem.h"
#include "os/OsDefs.h"
#include "os/OsMsg.h"
#include "os/OsMsgQ.h"
#include "os/OsMutex.h"
#include "os/OsTime.h"
#include "utl/UtlDList.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
class UtlString;

// FORWARD DECLARATIONS

// #define OS_MSGQ_DEBUG
// #define OS_MSGQ_REPORTING

//:Message queue implementation for OS's with no native message queue support
class OsMsgQShared : public OsMsgQBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsMsgQShared(
      const int       maxMsgs=DEF_MAX_MSGS,      //:max number of messages
      const int       maxMsgLen=DEF_MAX_MSG_LEN, //:max msg length (bytes)
      const int       options=Q_PRIORITY, //:how to queue blocked tasks
      const UtlString& name=""             //:global name for this queue
      );
     //:Constructor
     // If name is specified but is already in use, throw an exception

   virtual
   ~OsMsgQShared();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus send(const OsMsg& rMsg,
                         const OsTime& rTimeout=OsTime::OS_INFINITY);
     //:Insert a message at the tail of the queue
     // Wait until there is either room on the queue or the timeout expires.

   virtual OsStatus sendP(OsMsg* pMsg,
                          const OsTime& rTimeout=OsTime::OS_INFINITY);
     //:Insert a message at the tail of the queue
     // Wait until there is either room on the queue or the timeout expires.
     // Takes ownership of *pMsg.

   virtual OsStatus sendUrgent(const OsMsg& rMsg,
                               const OsTime& rTimeout=OsTime::OS_INFINITY);
     //:Insert a message at the head of the queue
     // Wait until there is either room on the queue or the timeout expires.

   virtual OsStatus sendFromISR(const OsMsg& rMsg);
     //:Insert a message at the tail of the queue.
     // Sending from an ISR has a couple of implications.  Since we can't
     // allocate memory within an ISR, we don't create a copy of the message
     // before sending it and the sender and receiver need to agree on a
     // protocol (outside this class) for when the message can be freed.
     // The sentFromISR flag in the OsMsg object will be TRUE for messages
     // sent using this method.

   virtual OsStatus receive(OsMsg*& rpMsg,
                            const OsTime& rTimeout=OsTime::OS_INFINITY);
     //:Remove a message from the head of the queue
     // Wait until either a message arrives or the timeout expires.
     // Other than for messages sent from an ISR, the receiver is responsible
     // for freeing the received message.

/* ============================ ACCESSORS ================================= */

#ifdef OS_MSGQ_DEBUG
   int getFullCount(void) { return mFull.getValue();}
   int getEmptyCount(void) { return mEmpty.getValue();}
   UtlDList& getList() { return mDlist;}
#endif

   virtual int numMsgs(void);
     //:Return the number of messages in the queue

#ifdef MSGQ_IS_VALID_CHECK
   virtual void show(void);
     //:Print information on the message queue to the console
     // Output enabled via a compile-time #ifdef
#endif

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

#ifdef MSGQ_IS_VALID_CHECK
   virtual void testMessageQ();
#endif


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsMutex  mGuard;  // mutex used to synchronize access to the msg queue
   OsCSem   mEmpty;  // counting semaphore used to coordinate sending msgs to
                     //  the queue and blocking senders to keep the number
                     //  of messages less than or equal to maxMsgs.
   OsCSem   mFull;   // counting semaphore used to coordinate receiving msgs
                     //  from the queue and blocking receivers when there are
                     //  no messages to receive.
   UtlDList  mDlist;  // doubly-linked list used to store messages
   int      mOptions;// message queue options
   int      mHighCnt;// high water mark for the number of msgs in the queue

#ifdef OS_MSGQ_REPORTING
   int      mIncreaseLevel;   // emit a message to the log when the number
                              //  of messages reaches the mIncreaseLevel
   int      mDecreaseLevel;   // emit a message to the log when the number
                              //  of messages goes below the mDecreaseLevel
   int      mIncrementLevel;  // When the mIncreaseLevel or mDecreaseLevels
                              //  are reached, increment/decrement the level
                              //  by mIncrementLevel
#endif

   OsStatus doSend(const OsMsg& rMsg,
                   const OsTime& rTimeout,
                   const UtlBoolean isUrgent,
                   const UtlBoolean sendFromISR);
     //:Helper function for sending messages that may not need to be copied

   OsStatus doSendCore(OsMsg* pMsg,
                       const OsTime& rTimeout,
                       UtlBoolean isUrgent,
                       UtlBoolean deleteWhenDone
                       /**< If true, if the message is not sent, doSendCore
                        *   will delete it.  Set if the caller expects the
                        *   recipient to delete *pMsg.
                        */
      );
     //:Core helper function for sending messages.

   OsStatus doReceive(OsMsg*& rpMsg, const OsTime& rTimeout);
     //:Helper function for removing a message from the head of the queue

   OsMsgQShared(const OsMsgQShared& rOsMsgQShared);
     //:Copy constructor (not implemented for this class)

   OsMsgQShared& operator=(const OsMsgQShared& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsMsgQShared_h_
