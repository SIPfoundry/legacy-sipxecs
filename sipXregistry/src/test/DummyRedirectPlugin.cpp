// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

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

extern std::vector<UtlString> globalList;

class DummyRedirectPlugin: public RedirectPlugin
{
  public:

   explicit DummyRedirectPlugin(const UtlString& instanceName) : 
      RedirectPlugin( instanceName ),
      mLogName( instanceName ){}

   ~DummyRedirectPlugin(){};

   virtual void readConfig(OsConfigDb& configDb)
   {
      configDb.get("BEHAVIOR", mBehavior );
   }

   virtual OsStatus initialize(OsConfigDb& configDb,
                               int redirectorNo,
                               const UtlString& localDomainHost)
   {
      return OS_SUCCESS;
   }

   virtual void finalize()
   {
   }

   virtual RedirectPlugin::LookUpStatus lookUp(
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
      char diagMessage[100];
      sprintf( diagMessage, "%s::lookUp: contactList Size=%zu", mLogName.data(), contactList.entries() );
      globalList.push_back( diagMessage );
      if( mBehavior.compareTo("ADD_SELF_AS_CONTACT") == 0 )
      {
         contactList.add( mLogName, *this );
         return RedirectPlugin::SUCCESS;
      }
      else if( mBehavior.compareTo("DONT_ADD_CONTACT") == 0 )
      {
         return RedirectPlugin::SUCCESS;
      }
      else if( mBehavior.compareTo("RETURN_ERROR") == 0 )
      {
         return RedirectPlugin::ERROR;
      }
      return RedirectPlugin::SUCCESS;
   }
   
   virtual void observe(
      const SipMessage& message,      ///< the incoming SIP message
      const UtlString& requestString, /**< the request URI from the SIP message as a UtlString
                                       *   ONLY for use in debugging messages; all comparisons
                                       *   should be with requestUri */
      const Url& requestUri,          ///< the request URI from the SIP message as a Uri,
      const UtlString& method,        ///< Method of the request to observe
      const ContactList& contactList, ///< Read-only list of contacts to use for redirection  
      RequestSeqNo requestSeqNo,      ///< the request sequence number
      int redirectorNo                ///< the identifier for this redirector
                               )
   {
      char diagMessage[100];
      sprintf( diagMessage, "%s::observe: contactList Size=%zu", mLogName.data(), contactList.entries() );
      globalList.push_back( diagMessage );
   }
   
   virtual const UtlString& name( void ) const
   {
      return mLogName;
   }

  private:
   UtlString mLogName;
   UtlString mBehavior;
};

// Static factory function.
extern "C" RedirectPlugin* getRedirectPlugin(const UtlString& instanceName)
{
   return new DummyRedirectPlugin(instanceName);
}

