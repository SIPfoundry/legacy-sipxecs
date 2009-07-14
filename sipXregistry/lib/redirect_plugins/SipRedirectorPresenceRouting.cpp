// 
// 
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlRegex.h>
#include "net/XmlRpcRequest.h"
#include "net/XmlRpcResponse.h"
#include "os/OsDateTime.h"
#include "os/OsSysLog.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/CredentialDB.h"
#include "sipdb/ResultSet.h"
#include "SipRedirectorPresenceRouting.h"
#include "registry/SipRedirectServer.h"

// DEFINES
#define CONFIG_SETTING_REALM "REALM"
#define CONFIG_SETTING_VOICEMAIL_ON_BUSY "VOICEMAIL_ON_BUSY"
#define CONFIG_OPENFIRE_PRESENCE_SERVER_URL "OPENFIRE_PRESENCE_SERVER_URL"

#define VOICEMAIL_CONTACT_PREFIX ("~~vm~")

#define QUERY_PRESENCE_METHOD  "presenceServer.getUnifiedPresenceInfo"

// sipXopenfire presence plug-in  XML-RPC response member names
#define STATUS_CODE "status-code"   // string representing status of request "ERROR" or "OK"
#define STATUS_CODE_VALUE_OK    "OK"
#define STATUS_CODE_VALUE_ERROR "ERROR"
#define ERROR_CODE  "error-code"     // int providing a detailed error code.  Provided only when "status-code" is "ERROR"
#define ERROR_INFO  "error-string"  // string giving a plaintext explaination of the error. Provided only when "status-code" is "ERROR"
#define TELEPHONY_PRESENCE "sip-presence" // string representing telephony presence.  Can be "idle", "busy" or "undetermied"
#define XMPP_PRESENCE "xmpp-presence" // string representing XMPP presence.  Can be "available", "away", "xa", "dnd" or "offline"
#define UNIFIED_PRESENCE "unified-presence" // string representing unified XMPP presence which is a merge of the telephony and XMPP presences
#define CUSTOM_PRESENCE_MESSAGE "custom-presence-message" // string representing user-supplied cusomt message linked to presence state.  Can be an empty string
#define SIP_RESOURCE_ID "sipId" 
#define JABBER_ID "jabber-id" 

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


const RegEx TelUri( "tel:(.+@.+?)(\\s|\\Z)" );

// Static factory function.
extern "C" RedirectPlugin* getRedirectPlugin(const UtlString& instanceName)
{
   return new SipRedirectorPresenceRouting(instanceName);
}

// Constructor
SipRedirectorPresenceRouting::SipRedirectorPresenceRouting(const UtlString& instanceName) :
   RedirectPlugin(instanceName)
{
   mLogName.append("[");
   mLogName.append(instanceName);
   mLogName.append("] SipRedirectorPresenceRouting");
}

// Destructor
SipRedirectorPresenceRouting::~SipRedirectorPresenceRouting()
{
}

// Read config information.
void SipRedirectorPresenceRouting::readConfig(OsConfigDb& configDb)
{
   // extract the realm information from the config DB - we need this part
   // to do the credentials db look-up.
   if ((configDb.get(CONFIG_SETTING_REALM, mRealm) != OS_SUCCESS) ||
         mRealm.isNull())
   {
       OsSysLog::add(FAC_SIP, PRI_ERR,
                     "%s::readConfig No Realm specified in the configuration",
                     mLogName.data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "%s::readConfig mRealm = '%s'",
                    mLogName.data(), mRealm.data() );
   }
   
   mbForwardToVmOnBusy = configDb.getBoolean(CONFIG_SETTING_VOICEMAIL_ON_BUSY, TRUE);
   OsSysLog::add(FAC_SIP, PRI_INFO,
                 "%s::readConfig mbForwardToVmOnBusy = %d",
                 mLogName.data(), mbForwardToVmOnBusy);

   UtlString openFirePresenceServerUrlAsString;
   if ((configDb.get(CONFIG_OPENFIRE_PRESENCE_SERVER_URL, openFirePresenceServerUrlAsString) != OS_SUCCESS) ||
         openFirePresenceServerUrlAsString.isNull())
   {
       OsSysLog::add(FAC_SIP, PRI_ERR,
                     "%s::readConfig No URL specified for openfire presence server in the configuration",
                     mLogName.data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "%s::readConfig openFirePresenceServerUrlAsString = '%s'",
                    mLogName.data(), openFirePresenceServerUrlAsString.data() );
      mOpenFirePresenceServerUrl.fromString( openFirePresenceServerUrlAsString );
   }
}

// Initialize
OsStatus
SipRedirectorPresenceRouting::initialize(OsConfigDb& configDb,
                                 int redirectorNo,
                                 const UtlString& localDomainHost)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::SipRedirectorPresenceRouting::initialize", mLogName.data() );
   return OS_SUCCESS;
}

// Finalize
void
SipRedirectorPresenceRouting::finalize()
{
}

RedirectPlugin::LookUpStatus
SipRedirectorPresenceRouting::lookUp(
   const SipMessage& message,
   const UtlString& requestString,
   const Url& requestUri,
   const UtlString& method,
   ContactList& contactList,
   RequestSeqNo requestSeqNo,
   int redirectorNo,
   SipRedirectorPrivateStorage*& privateStorage,
   ErrorDescriptor& errorDescriptor)
{
   RedirectPlugin::LookUpStatus rc = RedirectPlugin::SUCCESS;
   // figure out if this is a new request or one that 
   // we have suspended on by chekcing with we already
   // have storoge for the request
   if( !privateStorage )
   {
      // this is a new request that we need to process.
      // If the called party is a local user, 
      // kick start a task that will perform the XML-RPC
      // query to find out the presence state of the called
      // party. 
      // Note: Spawning a task on every query is sub-optimal
      //       but it is good enough for demo purposes.
      // TODO: Improve scheme to remove multiple task spwans.
      
      Url toUrl;
      UtlString userId;
      UtlString authTypeDB;
      UtlString passTokenDB;
      message.getToUrl(toUrl);
      
      // If the identity portion of the To header can be found in the
      // identity column of the credentials database, then a request
      // is to a local user, find out its presence state...
      if(CredentialDB::getInstance()->getCredential(toUrl,
                                                    mRealm,
                                                    userId,
                                                    passTokenDB,
                                                    authTypeDB)) 
      {
         PresenceLookupTask* presenceLookupTask = 
                          new PresenceLookupTask( toUrl, 
                                                  requestSeqNo, 
                                                  redirectorNo,
                                                  mOpenFirePresenceServerUrl );    
         presenceLookupTask->start();
         privateStorage = presenceLookupTask;
         rc = RedirectPlugin::SEARCH_PENDING;
      }
   }
   else
   {
      // this is a request that we already processed and
      // suspended on...  Tweak the contacts based on the presence state
      PresenceLookupTask* presenceLookupTask = (PresenceLookupTask*) privateStorage;
      if( presenceLookupTask->isPresenceAvailable() )
      {
         UtlString userIdentity;
         UtlString telephonyPresence;
         UtlString xmppPresence;
	 UtlString customPresenceMessage;
         
         presenceLookupTask->getSipUserIdentity( userIdentity );
         presenceLookupTask->getTelephonyPresenceState( telephonyPresence );
         presenceLookupTask->getXmppPresenceState( xmppPresence );
         presenceLookupTask->getCustomPresenceMessage( customPresenceMessage );
         
         OsSysLog::add(FAC_SIP, PRI_INFO, "%s::LookUpStatus "
                       "Presence information for '%s':\r\n"
                       "    Telephony presence: '%s'"
                       "    XMPP presence: '%s'"
                       "    Custom presence message: '%s'",
                       mLogName.data(),
                       userIdentity.data(), telephonyPresence.data(),
                       xmppPresence.data(),  customPresenceMessage.data() );         

         // look for tel uri in the custom presence message
         RegEx telUri( TelUri );
         telUri.Search( customPresenceMessage.data() );
         UtlString targetUri;
         if( telUri.MatchString( &targetUri, 1 ) )
         {
            // prepend 'sip:' and add target as contact
            targetUri.insert( 0, "sip:" );
            contactList.add( targetUri, *this );
         }
         else
         {
            // If user is busy then call goes directly to voicemail.
            if( ( telephonyPresence.compareTo("busy", UtlString::ignoreCase ) == 0 && mbForwardToVmOnBusy ) ||
                  xmppPresence.compareTo("dnd", UtlString::ignoreCase ) == 0 )
            {
               // prune all non-voicemail contacts from the list
               removeNonVoicemailContacts( contactList );
            }
         }
      }      
   }
   return rc;
}

void SipRedirectorPresenceRouting::removeNonVoicemailContacts( ContactList& contactList )
{
   // walk the list to find the contact entry for voicemail
   size_t index;
   Url contactUrl;
   bool bVoicemailContactFound = false;
   for( index = 0; index < contactList.entries(); index++ )
   {
      if( contactList.get( index, contactUrl ) )
      {
         UtlString userPart;
         contactUrl.getUserId( userPart );
         if( userPart.index( VOICEMAIL_CONTACT_PREFIX ) == 0 )
         {
            bVoicemailContactFound = true;
            break;
         }
      }
   }

   // if vm contact found, remove all and put vm contact back in.
   if( bVoicemailContactFound )
   {
      contactList.removeAll( *this );
      contactList.add( contactUrl, *this );
   }
}

const UtlString& SipRedirectorPresenceRouting::name( void ) const
{
   return mLogName;
}

const UtlContainableType PresenceLookupTask::TYPE =
    "PresenceLookupTask";

PresenceLookupTask::PresenceLookupTask( const Url& resourceUrl, 
                                        RedirectPlugin::RequestSeqNo requestSeqNo,
                                        int redirectorNo,
                                        const Url& openFirePresenceServerUrl ) :
   mResourceUrl( resourceUrl ),
   mRequestSeqNo( requestSeqNo ),
   mRedirectorNo( redirectorNo ),
   mOpenFirePresenceServerUrl( openFirePresenceServerUrl ),
   mbPresenceInfoAvailable( false )
{
}
                                        

const char* const
PresenceLookupTask::getContainableType() const
{
   return PresenceLookupTask::TYPE;
}

int PresenceLookupTask::run( void* runArg )
{  
   OsSysLog::add(FAC_NAT, PRI_DEBUG, "PresenceLookupTask::run");
   XmlRpcRequest getPresenceInfoRequest( mOpenFirePresenceServerUrl, QUERY_PRESENCE_METHOD );
   UtlString resourceIdentity;
   mResourceUrl.getIdentity( resourceIdentity );
   getPresenceInfoRequest.addParam( &resourceIdentity );
   XmlRpcResponse presenceInfoResponse;
   if( getPresenceInfoRequest.execute( presenceInfoResponse ) == true )
   {
      UtlContainable* pValue = NULL;
      if ( !presenceInfoResponse.getResponse( pValue ) || !pValue )
      {
         OsSysLog::add(FAC_NAT, PRI_CRIT, "PresenceLookupTask::run response had no result.");
      }
      else
      {
         UtlString keyName;
         UtlHashMap* pMap = dynamic_cast<UtlHashMap*>( pValue );
         if ( !pMap )
         {
            OsSysLog::add(FAC_NAT, PRI_CRIT, 
                          "PresenceLookupTask::run response result had unexpected type: %s",
                          pValue->getContainableType() ); 
         }
         else
         {
            // extract status code and check it.
            keyName = STATUS_CODE;
            UtlString* pStatusCode = dynamic_cast<UtlString*>( pMap->findValue( &keyName ) );
            if( pStatusCode->compareTo( STATUS_CODE_VALUE_OK, UtlString::ignoreCase ) != 0 )
            {
               // status-code is not "OK", some error happened - extract error information.
               keyName = ERROR_CODE;
               UtlString* pErrorCode = dynamic_cast<UtlString*>( pMap->findValue( &keyName ) );         
               keyName = ERROR_INFO;
               UtlString* pErrorInfo = dynamic_cast<UtlString*>( pMap->findValue( &keyName ) );
               if( pErrorCode && pErrorInfo )
               {
                  OsSysLog::add(FAC_NAT, PRI_CRIT, "PresenceLookupTask::run request failed: %s:'%s'", pErrorCode->data(), pErrorInfo->data() );
               }
            }
            else
            {
               mbPresenceInfoAvailable = true;
               if( getStringValueFromMap( pMap, SIP_RESOURCE_ID, mSipUserIdentity ) == false )
               {
                  OsSysLog::add(FAC_NAT, PRI_CRIT, "PresenceLookupTask::run failed to get SIP_RESOURCE_ID" );
                  mbPresenceInfoAvailable = false;
               }
               
               if( getStringValueFromMap( pMap, TELEPHONY_PRESENCE, mTelephonyPresence ) == false )
               {
                  OsSysLog::add(FAC_NAT, PRI_CRIT, "PresenceLookupTask::run failed to get TELEPHONY_PRESENCE" );
                  mbPresenceInfoAvailable = false;
               }
               
               if( getStringValueFromMap( pMap, XMPP_PRESENCE, mXmppPresence ) == false )
               {
                  OsSysLog::add(FAC_NAT, PRI_CRIT, "PresenceLookupTask::run failed to get XMPP_PRESENCE" );
                  mbPresenceInfoAvailable = false;
               }

               if( getStringValueFromMap( pMap, CUSTOM_PRESENCE_MESSAGE, mCustomPresenceMessage ) == false )
               {
                  OsSysLog::add(FAC_NAT, PRI_CRIT, "PresenceLookupTask::run failed to get CUSTOM_UNIFIED_PRESENCE" );
                  mbPresenceInfoAvailable = false;
               }
               
            }
         }
      }
   }
   else
   {
      // Check if the request failed because of a failed connection.
      // That error can sometimes happen when the server closed the TCP
      // connection we were using to communicate to it and can usually
      // be recovered by sending the request again.  Try to send the 
      // request once more to see if it will fly this time.
      int faultCode;
      UtlString faultString;      
      presenceInfoResponse.getFault( &faultCode, faultString );
      OsSysLog::add( FAC_NAT, PRI_CRIT,
                     "PresenceLookupTask::run failed to execute() request: %d : %s",
                     faultCode, faultString.data() );
   }   
   SipRedirectServer::getInstance()->resumeRequest(mRequestSeqNo, mRedirectorNo);
   return 0;
}

bool PresenceLookupTask::isPresenceAvailable( void ) const
{
   return mbPresenceInfoAvailable;
}

void PresenceLookupTask::getSipUserIdentity( UtlString& sipUserIdentity ) const
{
   sipUserIdentity = mSipUserIdentity;
}

void PresenceLookupTask::getTelephonyPresenceState( UtlString& telephonyPresence ) const
{
   telephonyPresence = mTelephonyPresence;

}

void PresenceLookupTask::getXmppPresenceState( UtlString& xmppPresence ) const
{
   xmppPresence = mXmppPresence;
}

void PresenceLookupTask::getUnifiedPresenceState( UtlString& unifiedPresence ) const
{
   unifiedPresence = mUnifiedPresence;
}

void PresenceLookupTask::getCustomPresenceMessage( UtlString& customPresenceMessage ) const
{
   customPresenceMessage = mCustomPresenceMessage;
}

bool PresenceLookupTask::getStringValueFromMap( const UtlHashMap* pMap, UtlString keyName, UtlString& returnedValue )
{
   bool bKeyFound = false;
   UtlString* pValue = dynamic_cast<UtlString*>( pMap->findValue( &keyName ) );
   if( pValue )
   {
      returnedValue = *pValue; 
      bKeyFound = true;
   }
   return bKeyFound;
}
               
 
