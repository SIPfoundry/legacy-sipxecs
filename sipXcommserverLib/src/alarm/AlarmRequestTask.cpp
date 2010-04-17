// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "alarm/AlarmRequestTask.h"
#include "net/XmlRpcRequest.h"
#include "os/OsSysLog.h"
#include "utl/UtlSListIterator.h"
#include "utl/XmlContent.h"
#include "sipXecsService/SipXecsService.h"

// STATIC INITIALIZATIONS
const UtlContainableType AsynchAlarmMsg::TYPE = "AsynchAlarmMsg";
AlarmRequestTask* AlarmRequestTask::spAlarmRequestTask = NULL;
OsMutex           AlarmRequestTask::sLockMutex (OsMutex::Q_FIFO);

// DEFINES
#define RAISE_ALARM_METHOD             "Alarm.raiseAlarm"

// Misc
// CONSTANTS
const char* SupervisorConfigName = "sipxsupervisor-config";
const char* SUPERVISOR_HOST = "SUPERVISOR_HOST";
const int   DEFAULT_SUPERVISOR_PORT = 8092;  // MUST match sipXsupervisor/src/sipXsupervisor.cpp !

// TYPEDEFS
// FORWARD DECLARATIONS


AlarmRequestTask::AlarmRequestTask()
   : OsServerTask("AlarmRequestTask-%d")
{
   // pull Alarm Server address out of config file
   initXMLRPCsettings();
   
   // start the task which will listen for messages and send xmlrpc requests
   start();
}

AlarmRequestTask::~AlarmRequestTask()
{
   // Critical Section here
   OsLock lock( sLockMutex );
   
   // reset the static instance pointer to NULL
   spAlarmRequestTask = NULL;
}

AlarmRequestTask* AlarmRequestTask::getInstance()
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // See if this is the first time through for this process
    if ( spAlarmRequestTask == NULL )
    {   // Create the singleton class for clients to use
       spAlarmRequestTask = new AlarmRequestTask();
    }

    return spAlarmRequestTask;
}


void AlarmRequestTask::raiseAlarm(const UtlString& alarmId, const UtlSList& alarmParams )
{
   OsSysLog::add(FAC_ALARM, PRI_INFO,
                 "AlarmRequestTask::raiseAlarm( %s, %s )", mLocalHostname.data(), alarmId.data() );
   AsynchAlarmMsg message( AsynchAlarmMsg::RAISE_ALARM, mLocalHostname, alarmId, alarmParams );
   postMessage( message );
}


// Parses the config files and formats the Alarm Server URL and localhost.
// Current design is that the Alarm Server runs in the supervisor on the local host.
// Returns OS_SUCCESS only if a valid URL was found 
OsStatus AlarmRequestTask::initXMLRPCsettings()
{
   OsStatus retval = OS_FAILED;

   int supervisorPort;

   OsConfigDb domainConfiguration;
   OsPath     domainConfigPath = SipXecsService::domainConfigPath();

   if (OS_SUCCESS == domainConfiguration.loadFromFile(domainConfigPath.data()))
   {
      supervisorPort = domainConfiguration.getPort(SipXecsService::DomainDbKey::SUPERVISOR_PORT);
      if ( PORT_DEFAULT == supervisorPort )
      {
         supervisorPort = DEFAULT_SUPERVISOR_PORT;
      }
      else if ( PORT_NONE == supervisorPort )
      {
         supervisorPort = DEFAULT_SUPERVISOR_PORT;
         OsSysLog::add(FAC_ALARM, PRI_NOTICE,
                       "Alarm::initXMLRPCsettings: '%s' not configured, defaulting to %d",
                       SipXecsService::DomainDbKey::SUPERVISOR_PORT, supervisorPort);
      }
   }
   else
   {
      OsSysLog::add(FAC_ALARM, PRI_WARNING,
            "Alarm::initXMLRPCsettings: failed to load domain configuration from '%s'",
            domainConfigPath.data() );
   }

   OsConfigDb supervisorConfiguration;
   OsPath     supervisorConfigPath =
      SipXecsService::Path(SipXecsService::ConfigurationDirType, SupervisorConfigName);

   if (OS_SUCCESS == supervisorConfiguration.loadFromFile(supervisorConfigPath.data()))
   {
      supervisorConfiguration.get(SUPERVISOR_HOST, mLocalHostname);
   }
   else
   {
      OsSysLog::add(FAC_ALARM, PRI_WARNING,
            "Alarm::initXMLRPCsettings: failed to load supervisor configuration from '%s'",
            supervisorConfigPath.data() );
   }

   if (mLocalHostname.isNull())
   {
      // getHostName does not always return the proper fully qualified hostname,
      // but it is worth trying rather than giving up entirely
      OsSocket::getHostName(&mLocalHostname);
      OsSysLog::add(FAC_ALARM, PRI_WARNING,
            "Alarm::initXMLRPCsettings: failed to find Supervisor host in '%s'; "
                    "defaulting to local host '%s'",
            supervisorConfigPath.data(), mLocalHostname.data() );
   }

   // Construct the URL to the Alarm Server's XMLRPC server.
   mAlarmServerUrl.fromString(mLocalHostname, Url::AddrSpec);
   if (mAlarmServerUrl.getScheme() == Url::UnknownUrlScheme)
   {
      OsSysLog::add(FAC_ALARM, PRI_ERR,
            "Alarm::initXMLRPCsettings: badly formed Supervisor host '%s' in '%s'",
            mLocalHostname.data(), supervisorConfigPath.data() );
   }
   else
   {
      mAlarmServerUrl.setScheme(Url::HttpsUrlScheme);
      mAlarmServerUrl.setHostPort(supervisorPort);
      mAlarmServerUrl.setPath("/RPC2");

      UtlString logUrl;
      mAlarmServerUrl.toString(logUrl);
      OsSysLog::add(FAC_ALARM, PRI_NOTICE,
                    "Alarm::initXMLRPCsettings: Alarm Server URL: '%s'",
                    logUrl.data());

      retval = OS_SUCCESS;
   }

   return retval;
}


void replaceEach(UtlString& value, const UtlString& replaceWhat, const UtlString& replaceWith)
{
   UtlString modifiedString;
   size_t lastIndex;
   ssize_t index;
   for (index = 0, lastIndex = 0;
        (index = value.index(replaceWhat, lastIndex, UtlString::ignoreCase)) != UTL_NOT_FOUND;
        lastIndex = index+replaceWhat.length() )
   {
      modifiedString.append(value, lastIndex, index-lastIndex);
      modifiedString.append(replaceWith);
   }

   if (lastIndex < value.length())
   {
      modifiedString.append(value, lastIndex, UtlString::UTLSTRING_TO_END);
   }

   value = modifiedString;
}


UtlBoolean AlarmRequestTask::handleMessage( OsMsg& rMsg )
{
   UtlBoolean handled = FALSE;
   XmlRpcRequest* pXmlRpcRequestToSend = 0;
   AsynchAlarmMsg* pAlarmMsg = dynamic_cast <AsynchAlarmMsg*> ( &rMsg );
   
   UtlString localHostname       = pAlarmMsg->getLocalHostname();
   UtlString alarmId             = pAlarmMsg->getAlarmId();
   UtlSListIterator iterator(pAlarmMsg->getAlarmParams());
   UtlString* pObject;
   UtlSList  alarmParams;
   UtlString newline="\n";
   UtlString tab="\t";
   while ( (pObject = dynamic_cast<UtlString*>(iterator())))
   {
      UtlString tempStr;
      XmlEscape(tempStr, *pObject);
      // newlines and tabs are not normally preserved in xml,
      // but we want them to survive - so manually escape them.
      replaceEach(tempStr, newline, "&#xA;");
      replaceEach(tempStr, tab, "&#x9;");
      alarmParams.append(new UtlString(tempStr));
   }
   
   switch ( rMsg.getMsgType() )
   {
   case OsMsg::OS_EVENT:
      switch( rMsg.getMsgSubType() )
      {
      case AsynchAlarmMsg::RAISE_ALARM:
         pXmlRpcRequestToSend = new XmlRpcRequest( mAlarmServerUrl, RAISE_ALARM_METHOD );
         pXmlRpcRequestToSend->addParam( &localHostname );
         pXmlRpcRequestToSend->addParam( &alarmId );
         pXmlRpcRequestToSend->addParam( &alarmParams );
         break;
         
      default:
         OsSysLog::add(FAC_ALARM, PRI_CRIT,
                       "AlarmRequestTask::handleMessage: received unknown sub-type: %d",
                       rMsg.getMsgSubType() );
         break;
      }
      handled = TRUE;
      break;
      
   default:
      OsSysLog::add(FAC_ALARM, PRI_CRIT,
                    "AlarmRequestTask::handleMessage: '%s' unhandled message type %d.%d",
                    mName.data(), rMsg.getMsgType(), rMsg.getMsgSubType());
      break;
   }

   if( pXmlRpcRequestToSend )
   {
      XmlRpcResponse response1;
      if (!pXmlRpcRequestToSend->execute(response1)) // blocks; returns false for any fault
      {
         // The XMLRPC request failed.
         int faultCode;
         UtlString faultString;
         response1.getFault(&faultCode, faultString);         
         OsSysLog::add(FAC_ALARM, PRI_CRIT, "Alarm.raiseAlarm failed, fault %d : %s",
                       faultCode, faultString.data());
         // since we could not send the alarm, log it here
         faultString = "Failed to report alarm ";
         faultString.append(alarmId.data());
         UtlSListIterator iterator(alarmParams);
         while ( (pObject = dynamic_cast<UtlString*>(iterator())))
         {
            faultString.append(" ");
            faultString.append(*pObject);
         }
         OsSysLog::add(FAC_ALARM, PRI_CRIT, faultString);
      }
   }
   delete pXmlRpcRequestToSend;
   alarmParams.destroyAll();
   return handled;
}

//////////////////////////////////////////////////////////////////////////////
AsynchAlarmMsg::AsynchAlarmMsg(EventSubType eventSubType,
                               const UtlString& localHostname,
                               const UtlString& alarmId,
                               const UtlSList& alarmParams) :
   OsMsg( OS_EVENT, eventSubType ),
   mLocalHostname( localHostname ),
   mAlarmId( alarmId )
{
   UtlSListIterator iterator(alarmParams);
   UtlString* pObject;
   mAlarmParams.removeAll();
   while ( (pObject = dynamic_cast<UtlString*>(iterator())))
   {
      mAlarmParams.append(new UtlString(*pObject));
   }
}
         
// deep copy of alarm and parameters
AsynchAlarmMsg::AsynchAlarmMsg( const AsynchAlarmMsg& rhs) :
   OsMsg( OS_EVENT, rhs.getMsgSubType() ),
   mLocalHostname( rhs.getLocalHostname() ),
   mAlarmId( rhs.getAlarmId() )
{
   UtlSListIterator iterator(rhs.getAlarmParams());
   UtlString* pObject;
   mAlarmParams.removeAll();
   while ( (pObject = dynamic_cast<UtlString*>(iterator())))
   {
      mAlarmParams.append(new UtlString(*pObject));
   }
}

AsynchAlarmMsg::~AsynchAlarmMsg()
{
   mAlarmParams.destroyAll();
}

OsMsg* AsynchAlarmMsg::createCopy( void ) const
{
   return new AsynchAlarmMsg( *this );
}

const UtlString& AsynchAlarmMsg::getLocalHostname( void ) const
{
   return mLocalHostname;
}

const UtlString& AsynchAlarmMsg::getAlarmId( void ) const
{
   return mAlarmId;
}

const UtlSList& AsynchAlarmMsg::getAlarmParams( void ) const
{
   return mAlarmParams;
}

