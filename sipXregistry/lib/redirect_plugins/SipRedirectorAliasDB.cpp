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
   UtlString& requestString,
   Url& requestUri,
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

   bool isDomainAlias = false;
   UtlString domain;
   UtlString hostAlias;
   requestUri.getHostAddress(domain);
   UtlBoolean isMyHostAlias = mpSipUserAgent->isMyHostAlias(requestUri);
   if (mpSipUserAgent && domain != _localDomain && isMyHostAlias)
   {
     isDomainAlias = true;
     hostAlias = domain;
     requestUri.setHostAddress(_localDomain);
   }

   UtlString requestIdentity;
   requestUri.getIdentity(requestIdentity);

   OS_LOG_DEBUG(FAC_SIP, mLogName.data() << "::lookUp identity: " << requestIdentity.data()
           << " domain: " << domain.data()
           << " local-domain: " << _localDomain.data()
           << " isHostAlias: " << isMyHostAlias);

   
   //ResultSet aliases;
   //AliasDB::getInstance()->getContacts(requestUri, aliases);
   //int numAliasContacts = aliases.getSize();

   EntityDB::Aliases aliases;
   bool isUserIdentity = false;
   EntityDB* entityDb = SipRegistrar::getInstance(NULL)->getEntityDB();
   entityDb->getAliasContacts(requestUri, aliases, isUserIdentity);
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
               
               
               if (numAliasContacts == 1 && isDomainAlias && isUserIdentity)
               {

                 UtlString userId;
                 contactUri.getUserId(userId);
                 requestUri.setUserId(userId.data());
                 requestUri.getUri(requestString);
                 OS_LOG_NOTICE(FAC_SIP, "SipRedirectorAliasDB::lookUp normalized request-uri to " << requestString.data());
               }
               else
               {
                 // Add the contact.
                 contactList.add( contactUri, *this );
               }


                if (contactList.getDiversionHeader().empty())
                {
                  //
                  // Add a Diversion header for all deflections
                  //
                  UtlString stringUri;
                  message.getRequestUri(&stringUri);
                  // The requestUri is an addr-spec, not a name-addr.
                  Url diversionUri(stringUri, TRUE);
                  UtlString userId;
                  diversionUri.getUserId(userId);
                  UtlString host;
                  diversionUri.getHostWithPort(host);


                  std::ostringstream strm;
                  strm << "<sip:";
                  if (!userId.isNull())
                    strm << userId.data() << "@";
                  strm << host.data();
                  strm << ">;reason=unconditional;relation=" << iter->relation;
                  UtlString diversion = strm.str().c_str();
                  OS_LOG_INFO(FAC_SIP, "SipRedirectorAliasDB::lookUp inserting diversion from " << diversion.data());
                  contactList.setDiversionHeader(diversion.data());
                }

            }
      }
   }
   else if (isDomainAlias)
   {
     //
     // No alias found.  If this is was towards a domain alias, make sure to reset it back to
     // the old value prior to feeding it to the rest of the redirectors.
     //
     requestUri.setHostAddress(hostAlias);
     requestUri.getUri(requestString);
   }

   return RedirectPlugin::SUCCESS;
}

const UtlString& SipRedirectorAliasDB::name( void ) const
{
   return mLogName;
}
