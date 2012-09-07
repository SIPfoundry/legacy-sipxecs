//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include <utl/UtlRegex.h>
#include "os/OsDateTime.h"
#include "os/OsLogger.h"
#include "sipdb/ResultSet.h"
#include "SipRedirectorAliasDB.h"
#include "net/NetBase64Codec.h"
#include "net/SipXauthIdentity.h"
#include "sipXecsService/SipXecsService.h"
#include "sipXecsService/SharedSecret.h"
#include "registry/SipRegistrar.h"
#include "sipdb/EntityDB.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Static factory function.
extern "C" RedirectPlugin* getRedirectPlugin(const UtlString& instanceName)
{
   return new SipRedirectorAliasDB(instanceName);
}

static UtlString _localDomain;

// Constructor
SipRedirectorAliasDB::SipRedirectorAliasDB(const UtlString& instanceName) :
   RedirectPlugin(instanceName)
{
   mLogName.append("[");
   mLogName.append(instanceName);
   mLogName.append("] SipRedirectorAliasDB");
}

// Destructor
SipRedirectorAliasDB::~SipRedirectorAliasDB()
{
}

// Initializer
OsStatus
SipRedirectorAliasDB::initialize(OsConfigDb& configDb,
                                 int redirectorNo,
                                 const UtlString& localDomainHost)
{
   _localDomain = localDomainHost;
   return OS_SUCCESS;
}

// Finalizer
void
SipRedirectorAliasDB::finalize()
{
}

// Read config information.
void SipRedirectorAliasDB::readConfig(OsConfigDb& configDb)
{
   // read the domain configuration
   OsConfigDb domainConfiguration;
   domainConfiguration.loadFromFile(SipXecsService::domainConfigPath());

   // get the shared secret for generating signatures
   SharedSecret secret(domainConfiguration);
   // Set secret for signing SipXauthIdentity
   SipXauthIdentity::setSecret(secret.data());

   UtlString base64;
   NetBase64Codec::encode(secret, base64);
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "SipRedirectorAliasDB::readConfig"
                 "%s::readConfig "
                 "set SipXauthIdentity secret",
                 mLogName.data()
                 );
}

RedirectPlugin::LookUpStatus
SipRedirectorAliasDB::lookUp(
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
   // If url param sipx-userforward = false, do not redirect to user-forward
   // aliases.
   UtlString userforwardParam;
   requestUri.getUrlParameter("sipx-userforward", userforwardParam);
   bool disableForwarding =
      userforwardParam.compareTo("false", UtlString::ignoreCase) == 0;
   if (disableForwarding)
   {
      Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "%s::lookUp user forwarding disabled by parameter",
                    mLogName.data());
   }

   Url identityUrl = requestUri;
   UtlString domain;
   identityUrl.getHostAddress(domain);
   if (mpSipUserAgent && domain != _localDomain && mpSipUserAgent->isMyHostAlias(identityUrl))
     identityUrl.setHostAddress(_localDomain);

   UtlString requestIdentity;
   identityUrl.getIdentity(requestIdentity);

   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "%s::lookUp identity '%s'",
                 mLogName.data(), requestIdentity.data());

   //ResultSet aliases;
   //AliasDB::getInstance()->getContacts(requestUri, aliases);
   //int numAliasContacts = aliases.getSize();

   EntityDB::Aliases aliases;
   bool isUserIdentity = false;
   EntityDB* entityDb = SipRegistrar::getInstance(NULL)->getEntityDB();
   entityDb->getAliasContacts(identityUrl, aliases, isUserIdentity);
   int numAliasContacts = aliases.size();

   if (numAliasContacts > 0)
   {
      Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "%s::lookUp "
                    "got %d AliasDB contacts", mLogName.data(),
                    numAliasContacts);

      // Check if the request identity is a real user/extension
      UtlString realm;
      UtlString authType;
      

      SipXauthIdentity authIdentity;
      authIdentity.setIdentity(requestIdentity);

      for (EntityDB::Aliases::iterator iter = aliases.begin(); iter != aliases.end(); iter++)
      {

            // If disableForwarding and the relation value is "userforward",
            // do not record this contact.
            if (!(disableForwarding && iter->relation == ALIASDB_RELATION_USERFORWARD))
            {
               UtlString contact = iter->contact.c_str();
               Url contactUri(contact);

               // if the request identity is a real user
               if (isUserIdentity)
               {
                  // Encode AuthIdentity into the URI
                  authIdentity.encodeUri(contactUri, message);
               }

               contactUri.setUrlParameter(SIP_SIPX_CALL_DEST_FIELD, "AL");
               // Add the contact.
               contactList.add( contactUri, *this );
            }
      }
   }

   return RedirectPlugin::SUCCESS;
}

const UtlString& SipRedirectorAliasDB::name( void ) const
{
   return mLogName;
}
