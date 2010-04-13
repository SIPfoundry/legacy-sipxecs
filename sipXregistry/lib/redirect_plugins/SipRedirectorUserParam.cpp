//
//
// Copyright (C) 2009 Nortel Networks., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "registry/SipRedirectServer.h"
#include "SipRedirectorUserParam.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

const char* StripAllUserParam="STRIP_ALL";

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Static factory function.
extern "C" RedirectPlugin* getRedirectPlugin(const UtlString& instanceName)
{
   return new SipRedirectorUserParam(instanceName);
}

// Constructor
SipRedirectorUserParam::SipRedirectorUserParam(const UtlString& instanceName) :
   RedirectPlugin(instanceName),
   mStripAll(false)
{
   mLogName.append("[");
   mLogName.append(instanceName);
   mLogName.append("] SipRedirectorUserParam");
}

// Destructor
SipRedirectorUserParam::~SipRedirectorUserParam()
{
}

// Initializer
OsStatus
SipRedirectorUserParam::initialize(OsConfigDb& configDb,
                                 int redirectorNo,
                                 const UtlString& localDomainHost)
{
   return OS_SUCCESS;
}

// Finalizer
void
SipRedirectorUserParam::finalize()
{
}

// Read config information.
void SipRedirectorUserParam::readConfig(OsConfigDb& configDb)
{
   mStripAll = configDb.getBoolean( StripAllUserParam, false );
   OsSysLog::add(FAC_SIP, PRI_INFO,
                 "%s::readConfig '%s' = %s",
                 mLogName.data(), StripAllUserParam,
                 mStripAll ? "TRUE" : "FALSE");
}

RedirectPlugin::LookUpStatus
SipRedirectorUserParam::lookUp(
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
   if (mStripAll)
   {
      if (SipRedirectServer::getInstance()->sipUserAgent()->isMyHostAlias(requestUri))
      {
         UtlString userpart;
         requestUri.getUserId(userpart);

         ssize_t semiColonIndex;
         if ((semiColonIndex = userpart.index(';')) != UTL_NOT_FOUND)
         {
            UtlString strippedUser(userpart);
            strippedUser.remove(semiColonIndex);

            Url strippedUrl(requestUri);
            strippedUrl.setUserId(strippedUser);

            OsSysLog::add(FAC_SIP, PRI_INFO,
                          "%s::lookUp stripped parameters from '%s' -> '%s'",
                          mLogName.data(), userpart.data(), strippedUser.data());

            contactList.add(strippedUrl, *this);
         }
      }
      else
      {
         if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
         {
            UtlString logUri;
            requestUri.getUri(logUri);
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "%s::lookUp '%s' not in my domain - not modified",
                          mLogName.data(), logUri.data());
         }
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "%s::lookUp disabled by configuration",
                    mLogName.data());
   }

   return RedirectPlugin::SUCCESS;
}

const UtlString& SipRedirectorUserParam::name( void ) const
{
   return mLogName;
}
