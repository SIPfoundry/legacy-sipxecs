//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <assert.h>
#ifdef __pingtel_on_posix__
#  include <pthread.h>
#endif

#include "utl/UtlRscTrace.h"

// APPLICATION INCLUDES
#include "os/OsExcept.h"
#include "os/OsMsg.h"
#include "os/OsMsgQ.h"
#include "os/OsUtil.h"
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
const int OsMsgQBase::DEF_MAX_MSGS    = 100;
const int OsMsgQBase::DEF_MAX_MSG_LEN = 32;
const UtlString OsMsgQBase::MSGQ_PREFIX("MsgQ.");

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
// If the name is specified but is already in use, throw an exception
OsMsgQBase::OsMsgQBase(const char* name)
:  mSendHookFunc(NULL),
   mFlushHookFunc(NULL),
   mName(name)
{
   if (mName != "")
   {
      // Duplicate names are OK.
      OsUtil::insertKeyValue(MSGQ_PREFIX, mName, this, FALSE);
   }
}

// Destructor
OsMsgQBase::~OsMsgQBase()
{
   if (mName != "")
   {
      OsUtil::deleteKeyValue(MSGQ_PREFIX, mName);
      // If the name was a duplicate, the first deletion of it
      // will succeed and later ones will fail.
   }
}

// Return a pointer to the named queue, or NULL if not found
OsMsgQBase* OsMsgQBase::getMsgQByName(const UtlString& name)
{
   OsStatus res;
   void*    val;

   if (name == "")
      return NULL;
   else
   {
      res = OsUtil::lookupKeyValue(MSGQ_PREFIX, name, &val);
      if (res == OS_SUCCESS)
         return ((OsMsgQBase*) val);
      else
         return NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

// Delete all messages currently in the queue
void OsMsgQBase::flush(void)
{
   OsMsg* pMsg;

   while (receive(pMsg, OsTime::NO_WAIT) != OS_WAIT_TIMEOUT)
   {
      if (mFlushHookFunc != NULL)
         mFlushHookFunc(*pMsg);
      pMsg->releaseMsg();
   }
}

// Set the function that is invoked whenever a msg is sent to the queue
// The function takes the message to be sent as an argument and returns a
// boolean value indicating whether the SendHook method has handled the
// message. If TRUE, the message is not inserted into the queue (since it
// has already been handled. If FALSE, the (possibly modified) message is
// inserted into the queue.
void OsMsgQBase::setSendHook(OsMsgQSendHookPtr func)
{
   mSendHookFunc = func;
}

// Set the function that is invoked whenever a msg is flushed from the
// queue.  Messages get flushed when the OsMsgQ is deleted while there
// are messages still queued.
// The function takes an OsMsg reference as an argument.
void OsMsgQBase::setFlushHook(OsMsgQFlushHookPtr func)
{
   mFlushHookFunc = func;
}

/* ============================ ACCESSORS ================================= */

// Return a pointer to the current send hook function
OsMsgQSendHookPtr OsMsgQBase::getSendHook(void) const
{
   return mSendHookFunc;
}

// Return the queue size
int OsMsgQBase::maxMsgs() const
{
    return(mMaxMsgs);
}

// Return the global name of the queue.
const UtlString* OsMsgQBase::getName()
{
   return &mName;
}

/* ============================ INQUIRY =================================== */

// Return TRUE if the message queue is empty, FALSE otherwise
UtlBoolean OsMsgQBase::isEmpty(void)
{
   return (numMsgs() == 0);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
