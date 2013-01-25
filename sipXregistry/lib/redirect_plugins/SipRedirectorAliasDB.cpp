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
   RedirectPlugin(instanceName),
   _enableDiversionHeader(FALSE),
   _enableEarlyAliasResolution(FALSE)
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

   _enableDiversionHeader =  configDb.getBoolean("SIP_REGISTRAR_ADD_DIVERSION", FALSE);
   _enableEarlyAliasResolution =  configDb.getBoolean("SIP_REGISTRAR_EARLY_ALIAS_RESOLUTION", FALSE);
}

bool
SipRedirectorAliasDB::resolveAlias(
   const SipMessage& message,
   UtlString& requestString,
   Url& requestUri
)
{
  bool isDomainAlias = false;
  bool isUserAlias = false;

  UtlString domain;
  UtlString hostAlias;
  UtlString userAlias;

  requestUri.getHostAddress(domain);
  UtlBoolean isMyHostAlias = mpSipUserAgent->isMyHostAlias(requestUri);
  if (mpSipUserAgent && domain != _localDomain && isMyHostAlias)
  {
    isDomainAlias = true;
    hostAlias = domain;
  }

  UtlString requestIdentity;
  requestUri.getIdentity(requestIdentity);

  EntityDB::Aliases aliases;
  bool isUserIdentity = false;
  EntityDB* entityDb = SipRegistrar::getInstance(NULL)->getEntityDB();
  entityDb->getAliasContacts(requestUri, aliases, isUserIdentity);
  int numAliasContacts = aliases.size();
  EntityDB::Aliases::iterator iter = aliases.begin();

  if (numAliasContacts == 1 && iter != aliases.end())
  {
    // If disableForwarding and the relation value is "userforward",
    // do not record this contact.
    if (isUserIdentity && iter->relation != ALIASDB_RELATION_USERFORWARD && iter->relation != "callgroup")
    {
      UtlString contact = iter->contact.c_str();
      Url contactUri(contact);
      isUserAlias = true;
      contactUri.getUserId(userAlias);
    }
  }


  if (isUserAlias)
  {
    requestUri.setUserId(userAlias.data());
    requestUri.getUri(requestString);
  }

  if (isDomainAlias)
  {
    requestUri.setHostAddress(hostAlias);
    requestUri.getUri(requestString);
  }

  if (isUserAlias || isDomainAlias)
  {
    OS_LOG_NOTICE(FAC_SIP, "SipRedirectorAliasDB::lookUp normalized request-uri to " << requestString.data());
  }


  return isUserAlias || isDomainAlias;
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

   if (_enableEarlyAliasResolution)
   {
     resolveAlias(message, requestString, requestUri);
   }


   UtlString requestIdentity;
   requestUri.getIdentity(requestIdentity);


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
               contactList.add( contactUri, *this );

                if (_enableDiversionHeader && contactList.getDiversionHeader().empty())
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
                  strm << ">;reason=unconditional;sipxfwd=" << iter->relation;
                  UtlString diversion = strm.str().c_str();
                  OS_LOG_INFO(FAC_SIP, "SipRedirectorAliasDB::lookUp inserting diversion from " << diversion.data());
                  contactList.setDiversionHeader(diversion.data());
                }
            }
      }
   }
   
   return RedirectPlugin::SUCCESS;
}

const UtlString& SipRedirectorAliasDB::name( void ) const
{
   return mLogName;
}
