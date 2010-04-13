//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "EmailSendTask.h"
#include "os/OsSysLog.h"
#include "utl/UtlSListIterator.h"
#include "sipXecsService/SipXecsService.h"

// STATIC INITIALIZATIONS
const UtlContainableType AsynchEmailMsg::TYPE = "AsynchEmailMsg";
EmailSendTask*    EmailSendTask::spEmailSendTask = NULL;
OsMutex           EmailSendTask::sLockMutex (OsMutex::Q_FIFO);

// DEFINES
// Misc
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS


EmailSendTask::EmailSendTask(UtlString& fromStr,
                             UtlString& replyTo,
                             UtlString& smtpServer
                             ) :
   mMessage(fromStr, replyTo, smtpServer) // garbage initialization
{
   // start the task which will listen for messages and send email notifications
   start();
}

EmailSendTask::~EmailSendTask()
{
   // Critical Section here
   OsLock lock( sLockMutex );

   // reset the static instance pointer to NULL
   spEmailSendTask = NULL;
}

EmailSendTask* EmailSendTask::getInstance()
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // See if this is the first time through for this process
    if ( spEmailSendTask == NULL )
    {   // Create the singleton class for clients to use
       UtlString dummyStr("");
       spEmailSendTask = new EmailSendTask(dummyStr, dummyStr, dummyStr);
    }

    return spEmailSendTask;
}


void EmailSendTask::sendMessage(const MailMessage& msg)
{
   AsynchEmailMsg message( msg );
   postMessage( message );
}


UtlBoolean EmailSendTask::handleMessage( OsMsg& rMsg )
{
   UtlBoolean handled = FALSE;
   AsynchEmailMsg* pEmailMsg = dynamic_cast <AsynchEmailMsg*> ( &rMsg );

   MailMessage msg       = pEmailMsg->getMessage();

   switch ( rMsg.getMsgType() )
   {
   case OsMsg::OS_EVENT:
   {
      UtlString response = msg.Send();
      if (!response.isNull())
      {
         if (response.length() > 0)
         {
            OsSysLog::add(FAC_ALARM, PRI_ERR, "EmailSendTask: "
                  " Error sending e-mail: response %s", response.data());
         }
      }
      handled = TRUE;
      break;
   }

   default:
      OsSysLog::add(FAC_ALARM, PRI_CRIT,
                    "EmailSendTask::handleMessage: '%s' unhandled message type %d.%d",
                    mName.data(), rMsg.getMsgType(), rMsg.getMsgSubType());
      break;
   }

   return handled;
}

//////////////////////////////////////////////////////////////////////////////
AsynchEmailMsg::AsynchEmailMsg(//EventSubType eventSubType,
                               const MailMessage& msg
                               ) :
   OsMsg( OS_EVENT, AsynchEmailMsg::SEND_MESSAGE ),
   mMessage( msg )
{
}

// deep copy of alarm and parameters
AsynchEmailMsg::AsynchEmailMsg( const AsynchEmailMsg& rhs) :
   OsMsg( OS_EVENT, rhs.getMsgSubType() ),
   mMessage( rhs.getMessage() )
{
}

AsynchEmailMsg::~AsynchEmailMsg()
{
}

OsMsg* AsynchEmailMsg::createCopy( void ) const
{
   return new AsynchEmailMsg( *this );
}

const MailMessage& AsynchEmailMsg::getMessage( void ) const
{
   return mMessage;
}

