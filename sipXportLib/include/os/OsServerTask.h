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

#ifndef _OsServerTask_h_
#define _OsServerTask_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsTask.h"
#include "os/OsTime.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class OsMsg;


/// Abstract base class for tasks that process incoming msgs from an OsMsgQ.
/**
 * If the task only waits on its input queue, then it can be implemented
 * by just overriding the handleMessage method of this class.  See the method
 * for details on how it indicates that it has handled any given message from
 * the queue.
 *
 * @note When the destructor is invoked to shut down the task, the special
 * message OsMsg::OS_SHUTDOWN is sent to the task message queue.  The task may
 * choose to do any cleanup from within the thread when this message is received,
 * or it may choose to just acknowledge the message by calling OsTask::requestShutdown,
 * and then do its cleanup (see OsTask).
 *
 * The real inheritence of this class is:
 *
 * * OsServerTask isa OsTask<platform> isa OsTaskBase
 *
 * but doxygen doesn't understand the cute typedef trick that is used to interpose
 * the correct platform-specific class, so @see OsTaskLinux and @see OsTaskBase.
 * The OsTaskBase class is the more useful, actually.
 */

class OsServerTask : public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static const int DEF_MAX_MSGS;

/* ============================ CREATORS ================================== */

   OsServerTask(const char* name,
                void* pArg = NULL,
                const int maxRequestQMsgs = DEF_MAX_MSGS,
                const int priority = DEF_PRIO,
                const int options = DEF_OPTIONS,
                const int stackSize = DEF_STACKSIZE);

   virtual
   ~OsServerTask();
   /**<
    * As part of destroying the task, flush all messages from the incoming
    * OsMsgQ.
    */

/* ============================ MANIPULATORS ============================== */

   /// Handles an incoming message.
   virtual UtlBoolean handleMessage(OsMsg& rMsg);
   /**<
    * If the message is not one that the object is prepared to process,
    * the handleMessage() method in the derived class should return FALSE
    * which will cause the default OsServerTask handler method to be
    * invoked on the message.
    */

   /// Posts a message to this task.
   virtual OsStatus postMessage(const OsMsg& rMsg,
                                ///< rMsg is usually copied before being used.
                                //   Caller retains ownership of rMsg.
                                const OsTime& rTimeout=OsTime::OS_INFINITY,
                                UtlBoolean sentFromISR=FALSE);
   ///< Return the result of the message send operation.

   /// Posts a message to this task.
   virtual OsStatus postMessageP(OsMsg* pMsg,
                                 ///< *pMsg becomes owned by the task.
                                 const OsTime& rTimeout=OsTime::OS_INFINITY);
   ///< Return the result of the message send operation.

   ///< Send a message to the task requesting that it shut down.
   virtual void requestShutdown(void);
   /**<
    * Calls OsTask::requestShutdown() and then posts an OS_SHUTDOWN message
    * to the incoming message queue to unblock the task.  This is called
    * automatically from the destructor when another task is attempting to
    * delete the task - it need not be called explicitly before invoking
    * the 'delete' operator on the task.
    */

/* ============================ ACCESSORS ================================= */

   /// Get the pointer to the incoming message queue.
   OsMsgQ* getMessageQueue();

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsMsgQ mIncomingQ;                 ///< Queue for incoming messages.

   /// Waits for a message to arrive on the task's incoming message queue.
   virtual OsStatus receiveMessage(OsMsg*& rpMsg);

   /// Waits for a message to arrive on the task's incoming message queue, with timeout.
   virtual OsStatus receiveMessage(OsMsg*& rpMsg,
                                   const OsTime& rTimeout);

   /// The entry point for the task.
   virtual int run(void* pArg);
   /**<
    * This method executes a message processing loop until either
    * requestShutdown(), deleteForce(), or the destructor for this object
    * is called.
    */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Copy constructor (not implemented for this class)
   OsServerTask(const OsServerTask& rOsServerTask);

   /// Assignment operator (not implemented for this task)
   OsServerTask& operator=(const OsServerTask& rhs);

};

#endif  // _OsServerTask_h_
