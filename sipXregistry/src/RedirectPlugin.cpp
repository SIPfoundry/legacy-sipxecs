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
#include "os/OsFS.h"
#include "sipdb/ResultSet.h"
#include "registry/RedirectPlugin.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

const char* RedirectPlugin::Prefix  = "SIP_REDIRECT";
const char* RedirectPlugin::Factory = "getRedirectPlugin";

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Null default destructor.
RedirectPlugin::~RedirectPlugin()
{
}

// Null default cancel() implementation.
void RedirectPlugin::cancel(RequestSeqNo request)
{
}

// Null default readConfig() implementation
void 
RedirectPlugin::readConfig(OsConfigDb& configDb)
{
}

void
RedirectPlugin::addContact(SipMessage& response,
                          const UtlString& requestString,
                          const Url& contact,
                          const char* label)
{
   // Get the number of contacts already present.
   int numContactsInHeader =
      response.getCountHeaderFields(SIP_CONTACT_FIELD);

   // Add the contact field to the response at the end.
   // Need to keep this UtlString allocated till the end of this function.
   // The semantics of the Contact: header have the additional restriction
   // that if the URI contains a '?', it must be enclosed in <...> (sec. 20).
   // But beware that the BNF in sec. 25.1 does not require this.
   // Scott has changed Url::toString to always add <...> if there are header
   // parameters in the URI.
   UtlString contactUtlString = contact.toString();
   const char* contactString = contactUtlString.data();
   response.setContactField(contactString, numContactsInHeader);

   OsSysLog::add(FAC_SIP, PRI_INFO,
                 "RedirectPlugin::addContact Redirector '%s' maps '%s' to '%s'",
                 label, requestString.data(), contactString);
}

void
RedirectPlugin::removeAllContacts(SipMessage& response)
{
   // Get the number of contacts already present.
   int numContactsInHeader =
      response.getCountHeaderFields(SIP_CONTACT_FIELD);

   OsSysLog::add(FAC_SIP, PRI_INFO,
                 "RedirectPlugin::removeAllContacts Removing %d contacts",
                 numContactsInHeader);

   for (int i = numContactsInHeader - 1; i >= 0; i--)
   {
      response.removeHeader(SIP_CONTACT_FIELD, i);
   }
}

SipRedirectorPrivateStorage::~SipRedirectorPrivateStorage()
{
}
