//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "net/Url.h"
#include "net/SipMessage.h"
#include "sipdb/ResultSet.h"
#include "SipRedirectorSubscribe.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Static factory function.
extern "C" RedirectPlugin* getRedirectPlugin(const UtlString& instanceName)
{
   return new SipRedirectorSubscribe(instanceName);
}

// Constructor
SipRedirectorSubscribe::SipRedirectorSubscribe(const UtlString& instanceName) :
   RedirectPlugin(instanceName)
{
   mLogName.append("[");
   mLogName.append(instanceName);
   mLogName.append("] SipRedirectorSubscribe");
}

// Destructor
SipRedirectorSubscribe::~SipRedirectorSubscribe()
{
}

// Initializer

OsStatus
SipRedirectorSubscribe::initialize(OsConfigDb& configDb,
                                   int redirectorNo,
                                   const UtlString& localDomainHost)
{
   return OS_SUCCESS;
}

// Finalizer
void
SipRedirectorSubscribe::finalize()
{
}

RedirectPlugin::LookUpStatus
SipRedirectorSubscribe::lookUp(
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
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "%s::lookUp uri '%s'",
                 mLogName.data(), requestUri.toString().data());

   // This redirector operates only on SUBSCRIBE requests.
   if (method.compareTo(SIP_SUBSCRIBE_METHOD, UtlString::ignoreCase)==0)
   {
      // Remove q values from all contacts so that SUBSCRIBE is parallel-forked
      // (otherwise the low value subscriptions will never be found because the
      // proxy will CANCEL after the high values return final status).
      UtlString thisContact;
      for (int contactNum = 0;
           contactList.get( contactNum, thisContact );
           contactNum++)
      {
         Url contactUri(thisContact);
         UtlString qValue;

         // Check if the contact has a q value.
         if (contactUri.getFieldParameter(SIP_Q_FIELD, qValue))
         {
            // If so, remove it.
            contactUri.removeFieldParameter(SIP_Q_FIELD);
            UtlString contactUriString(contactUri.toString());
            contactList.set( contactNum, contactUriString, *this );

            OsSysLog::add(FAC_SIP, PRI_NOTICE,
                          "%s::lookUp Remove q value '%s' from '%s'",
                          mLogName.data(), qValue.data(), contactUriString.data());
         }
      } // for all contacts
   }

   return RedirectPlugin::SUCCESS;
}

const UtlString& SipRedirectorSubscribe::name( void ) const
{
   return mLogName;
}
