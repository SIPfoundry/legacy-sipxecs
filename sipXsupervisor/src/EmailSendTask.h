//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _EMAIL_SEND_TASK_H_
#define _EMAIL_SEND_TASK_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "utl/UtlSList.h"
#include "net/MailMessage.h"
#include "os/OsServerTask.h"
#include "os/OsMsg.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
 * Class used to send a MailMessage in an asynchronous fashion.
 * This class ensures that the Alarm Server does not block.
 */
class EmailSendTask : public OsServerTask
{
public:
   virtual ~EmailSendTask();

   /// Singleton Accessor
   static EmailSendTask* getInstance();

   /// Send asynchynchronous email message
   void sendMessage(const MailMessage& msg    ///< msg to send
                   );


private:
   MailMessage              mMessage;         ///< msg to send

   static EmailSendTask*    spEmailSendTask;  ///< Singleton instance
   static OsMutex           sLockMutex;       ///< Exclusive binary lock

   /// Constructor
   EmailSendTask(UtlString& fromStr,
         UtlString& replyTo,
         UtlString& smtpServer);

   /// Process asynchronous request from application code
   virtual UtlBoolean handleMessage(OsMsg& rMsg);
};

/**
 * Message sent to the Email Send Task to prevent Alarm Server code from blocking
 * while an email message is sent.
 */
class AsynchEmailMsg : public OsMsg
{
public:

   enum EventSubType
   {
      SEND_MESSAGE    = 52
   };

   /// Constructor.
   AsynchEmailMsg(//EventSubType eventSubType,
                  const MailMessage& msg            ///< msg to send
                  //const UtlString& alarmId,          ///< internal alarm ID
                  //const UtlSList&  alarmParams       ///< list of runtime parameters
                  );

   /// Destructor
   virtual ~AsynchEmailMsg();

   // Component accessors.
   const MailMessage& getMessage( void )    const;
   //const UtlString& getAlarmId( void )          const;
   //const UtlSList&  getAlarmParams( void )      const;

protected:
   static const UtlContainableType TYPE;   ///< Class type used for runtime checking

private:
   MailMessage mMessage;               ///< sending host for authentication
   //UtlString mAlarmId;                     ///< internal alarm ID
   //UtlSList  mAlarmParams;                 ///< list of runtime parameters

   /// Copy constructor
   AsynchEmailMsg( const AsynchEmailMsg& rhs);

   virtual OsMsg* createCopy(void) const;
};

#endif
