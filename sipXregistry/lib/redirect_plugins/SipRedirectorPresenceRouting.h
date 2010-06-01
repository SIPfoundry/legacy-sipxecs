//
//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREDIRECTORPRESENCEROUTING_H
#define SIPREDIRECTORPRESENCEROUTING_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "filereader/RefreshingFileReader.h"
#include "registry/RedirectPlugin.h"
#include "registry/RedirectPlugin.h"
#include "os/OsTimer.h"
#include "os/OsMutex.h"
#include "net/XmlRpcMethod.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class PresenceRoutingUserPreferences : public RefreshingFileReader
{
public:
   PresenceRoutingUserPreferences();
   bool forwardToVoicemailOnDnd(const UtlString& sipUsername );
   virtual ~PresenceRoutingUserPreferences(){};

protected:
   virtual OsStatus initialize();

private:
   UtlHashMap mUserVmOnDndPreferences;
   OsMutex    mMutex;
   OsStatus parseDocument( TiXmlDocument* pDoc );
};

class UnifiedPresence : public UtlString
{
public:
   UnifiedPresence( UtlString aor );
   const UtlString& getXmppPresence( void ) const;
   void setXmppPresence( const UtlString& xmppPresence );
   const UtlString& getXmppStatusMessage( void ) const;
   void setXmppStatusMessage( const UtlString& xmppStatusMessage );
   const UtlString& getSipState( void ) const;
   void setSipState( const UtlString& sipState );
   const UtlString& getUnifiedPresence( void ) const;
   void setUnifiedPresence( const UtlString& unifiedPresence );
private:
   UtlString mXmppPresence;
   UtlString mXmppStatusMessage;
   UtlString mSipState;
   UtlString mUnifiedPresence;
};

class UnifiedPresenceContainer
{
public:
   static UnifiedPresenceContainer* pInstance;
   static UnifiedPresenceContainer* getInstance( void );
   void insert( UtlString* pAor, UnifiedPresence* pUnifiedPresence ); // if entry already exists, it is updated
   const UnifiedPresence* lookup( UtlString* pAor );
   void reset( void );

private:
   UnifiedPresenceContainer();
   UtlHashMap mUnifiedPresences;
   OsMutex    mMutex;
};

/**
 * SipRedirectorPresenceRouting is a class whose object adds or removes contacts
 * based on the presence state of the called users.
 */
class SipRedirectorPresenceRouting : public RedirectPlugin, OsNotification
{
  public:

   explicit SipRedirectorPresenceRouting(const UtlString& instanceName);

   ~SipRedirectorPresenceRouting();

   virtual void readConfig(OsConfigDb& configDb);

   virtual OsStatus initialize(OsConfigDb& configDb,
                               int redirectorNo,
                               const UtlString& localDomainHost);

   virtual void finalize();

   virtual RedirectPlugin::LookUpStatus lookUp(
      const SipMessage& message,
      const UtlString& requestString,
      const Url& requestUri,
      const UtlString& method,
      ContactList& contactList,
      RequestSeqNo requestSeqNo,
      int redirectorNo,
      SipRedirectorPrivateStorage*& privateStorage,
      ErrorDescriptor& errorDescriptor);

   // created for unit testability
   RedirectPlugin::LookUpStatus
   doLookUp( const Url& requestUri,
             const SipMessage& message,
             ContactList& contactList );

   virtual const UtlString& name( void ) const;

   // OsNotification virtual method implementation
   virtual OsStatus signal(intptr_t eventData);

   static const UtlString& getLocalDomainName( void );
  protected:

   // String to use in place of class name in log messages:
   // "[instance] class".
   UtlString mLogName;
   UtlString mRealm;
   UtlString mSipDomain;
   UtlBoolean mbForwardToVmOnBusy;
   Url mOpenFirePresenceServerUrl;
   Url mLocalPresenceMonitorServerUrl;
   UtlHashMap mUnifiedPresences;
   OsTimer mPingTimer;
   bool mbRegisteredWithOpenfire;

  private:
   void addContactToContactList( const UtlString& target, const Url& requestUri, const SipMessage& message, ContactList& contactList );
   void removeNonVoicemailContacts( ContactList& contactList );
   OsStatus startPresenceMonitorXmlRpcServer( void );
   OsStatus registerPresenceMonitorServerWithOpenfire(void );
   OsStatus pingOpenfire( void );
   static UtlString  sLocalDomain;
   UtlString mOpenfireInstanceHandle;
   PresenceRoutingUserPreferences mUserPrefs;   
};

class UnifiedPresenceChangedMethod : public XmlRpcMethod
{
public:
      static XmlRpcMethod* get();
      virtual bool execute(const HttpRequestContext& requestContext,
                           UtlSList& params,
                           void* userData,
                           XmlRpcResponse& response,
                           ExecutionStatus& status);
};

#endif // SIPREDIRECTORPRESENCEROUTING_H

