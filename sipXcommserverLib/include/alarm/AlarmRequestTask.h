// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _ALARM_REQUEST_TASK_H_
#define _ALARM_REQUEST_TASK_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "utl/UtlSList.h"
#include "net/Url.h"
#include "os/OsServerTask.h"
#include "os/OsMsg.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS


/** 
 * Class used to communicate with the Alarm Server in an asynchronous fashion.
 * This class ensures that XML-RPC requests to the Alarm Server are non-blocking. 
 */
class AlarmRequestTask : public OsServerTask
{
public:
   virtual ~AlarmRequestTask();

   /// Singleton Accessor
   static AlarmRequestTask* getInstance();
   
   /// Send asynchynchronous alarm request to AlarmRequestTask and thence to Alarm Server
   void raiseAlarm(const UtlString& alarmId,           ///< internal alarm id
                   const UtlSList& alarmParams         ///< list of runtime parameters
                   );

   
private:
   UtlString mLocalHostname;                 ///< sending host for authentication
   Url       mAlarmServerUrl;                ///< address of alarm server
   
   static AlarmRequestTask* spAlarmRequestTask;  ///< Singleton instance
   static OsMutex           sLockMutex;          ///< Exclusive binary lock

   /// Constructor
   AlarmRequestTask();

   /// Process asynchronous request from application code
   virtual UtlBoolean handleMessage(OsMsg& rMsg); 
   
   /// Perform one-time init (read Alarm Server url from config file)
   OsStatus initXMLRPCsettings();
};

/** 
 * Message sent to the Alarm Request Task to prevent application code from blocking
 * while an xmlrpc request is sent.
 */
class AsynchAlarmMsg : public OsMsg
{
public:
   
   enum EventSubType
   {
      RAISE_ALARM    = 59
   };   

   /// Constructor to use for RAISE_ALARM.  Strings (not pointers) are copied into the message.
   AsynchAlarmMsg(EventSubType eventSubType,
                  const UtlString& controllerHandle, ///< sending host for authentication
                  const UtlString& alarmId,          ///< internal alarm ID
                  const UtlSList&  alarmParams       ///< list of runtime parameters
                  );

   /// Destructor
   virtual ~AsynchAlarmMsg();

   // Component accessors.
   const UtlString& getLocalHostname( void )    const;
   const UtlString& getAlarmId( void )          const;
   const UtlSList&  getAlarmParams( void )      const;
 
protected:
   static const UtlContainableType TYPE;   ///< Class type used for runtime checking

private:
   UtlString mLocalHostname;               ///< sending host for authentication
   UtlString mAlarmId;                     ///< internal alarm ID
   UtlSList  mAlarmParams;                 ///< list of runtime parameters

   /// Copy constructor
   AsynchAlarmMsg( const AsynchAlarmMsg& rhs);

   virtual OsMsg* createCopy(void) const;
};

#endif
