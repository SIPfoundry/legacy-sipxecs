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
#include "os/OsSysLog.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/ResultSet.h"
#include "sipdb/AliasDB.h"
#include "SipRedirectorAliasDB.h"

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
                                 SipUserAgent* pSipUserAgent,
                                 int redirectorNo,
                                 const UtlString& localDomainHost)
{
   return OS_SUCCESS;
}

// Finalizer
void
SipRedirectorAliasDB::finalize()
{
}

RedirectPlugin::LookUpStatus
SipRedirectorAliasDB::lookUp(
   const SipMessage& message,
   const UtlString& requestString,
   const Url& requestUri,
   const UtlString& method,
   SipMessage& response,
   RequestSeqNo requestSeqNo,
   int redirectorNo,
   SipRedirectorPrivateStorage*& privateStorage)
{
   UtlString requestIdentity;
   requestUri.getIdentity(requestIdentity);

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "%s::lookUp identity '%s'",
                 mLogName.data(), requestIdentity.data());

   ResultSet aliases;
   AliasDB::getInstance()->getContacts(requestUri, aliases);
   int numAliasContacts = aliases.getSize();
   if (numAliasContacts > 0)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "%s::lookUp "
                    "got %d AliasDB contacts", mLogName.data(),
                    numAliasContacts);

      for (int i = 0; i < numAliasContacts; i++)
      {
         static UtlString contactKey("contact");

         UtlHashMap record;
         if (aliases.getIndex(i, record))
         {
            UtlString contact = *((UtlString*)record.findValue(&contactKey));
            Url contactUri(contact);

            // Add the contact.
            addContact(response, requestString, contactUri,
                       mLogName.data());
         }
      }
   }

   return RedirectPlugin::LOOKUP_SUCCESS;
}
