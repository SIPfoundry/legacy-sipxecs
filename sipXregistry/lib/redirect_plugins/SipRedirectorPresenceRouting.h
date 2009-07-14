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
#include "registry/RedirectPlugin.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * SipRedirectorPresenceRouting is a class whose object adds or removes contacts
 * based on the presence state of the called users.
 */

class SipRedirectorPresenceRouting : public RedirectPlugin
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
   
   virtual const UtlString& name( void ) const;

  protected:

   // String to use in place of class name in log messages:
   // "[instance] class".
   UtlString mLogName;
   UtlString mRealm;
   UtlBoolean mbForwardToVmOnBusy;
   Url mOpenFirePresenceServerUrl;

  private:
   void removeNonVoicemailContacts( ContactList& contactList );   
};

/**
 * Private storage for presence state look-ups.
 */
class PresenceLookupTask : public SipRedirectorPrivateStorage, public OsTask
{
   friend class SipRedirectorPresenceRouting;
   friend class SipRedirectorPresenceRoutingTest;

public:
   
   PresenceLookupTask( const Url& resourceUrl, RedirectPlugin::RequestSeqNo requestSeqNo, int redirectorNo, const Url& mOpenFirePresenceServerUrl);

   virtual ~PresenceLookupTask(){};
   /// Start running the PresenceLookupTask task.
   virtual int run( void* runArg );

   virtual UtlContainableType getContainableType() const;

   bool isPresenceAvailable( void ) const ;
   void getSipUserIdentity( UtlString& sipUserIdentity ) const;
   void getTelephonyPresenceState( UtlString& telephonyPresence ) const;
   void getXmppPresenceState( UtlString& xmppPresence ) const;
   void getUnifiedPresenceState( UtlString& unifiedPresence ) const;
   void getCustomPresenceMessage( UtlString& customPresenceMessage ) const;
   
protected:

   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

private:
   Url mResourceUrl;
   RedirectPlugin::RequestSeqNo mRequestSeqNo;
   int mRedirectorNo;
   Url mOpenFirePresenceServerUrl;
   
   bool mbPresenceInfoAvailable;
   UtlString mSipUserIdentity;
   UtlString mTelephonyPresence;
   UtlString mXmppPresence;  
   UtlString mUnifiedPresence;
   UtlString mCustomPresenceMessage;
   
   bool getStringValueFromMap( const UtlHashMap* map, UtlString keyName, UtlString& returnedValue );               

};

#endif // SIPREDIRECTORPRESENCEROUTING_H

