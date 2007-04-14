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
#include "SipRedirectorAuthRouter.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define CONFIG_SETTING_AUTH_PROXY "AUTH_PROXY"

// STATIC VARIABLE INITIALIZATIONS

// Static factory function.
extern "C" RedirectPlugin* getRedirectPlugin(const UtlString& instanceName)
{
   return new SipRedirectorAuthRouter(instanceName);
}

// Constructor
SipRedirectorAuthRouter::SipRedirectorAuthRouter(const UtlString& instanceName) :
   RedirectPlugin(instanceName)
{
   mLogName.append("[");
   mLogName.append(instanceName);
   mLogName.append("] SipRedirectorAuthRouter");
}

// Destructor
SipRedirectorAuthRouter::~SipRedirectorAuthRouter()
{
}

// Read config information.
void SipRedirectorAuthRouter::readConfig(OsConfigDb& configDb)
{
   UtlString authProxyConfig;
   if (   (OS_SUCCESS == configDb.get(CONFIG_SETTING_AUTH_PROXY, authProxyConfig) )
       && ! authProxyConfig.isNull()
       )
   {
      Url authUrl(authProxyConfig);
      if ( Url::SipUrlScheme == authUrl.getScheme() )
      {
         authUrl.setUserId(NULL);
         authUrl.setDisplayName(NULL);
         authUrl.removeFieldParameters();
         authUrl.removeHeaderParameters();
         authUrl.setUrlParameter("lr",NULL);

         authUrl.toString(mAuthUrl);
         
         OsSysLog::add(FAC_SIP,
                       (authProxyConfig.compareTo(mAuthUrl, UtlString::ignoreCase)
                        ? PRI_INFO : PRI_NOTICE),
                       "%s::readConfig authorization proxy route '%s'",
                       mLogName.data(), mAuthUrl.data()
                       );
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR, "%s::readConfig "
                       "invalid route '%s'", mLogName.data(), authProxyConfig.data());
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_INFO, "%s::readConfig "
                    "No authorization proxy specified", mLogName.data());
   }
}

// Initializer
OsStatus
SipRedirectorAuthRouter::initialize(OsConfigDb& configDb,
                                    SipUserAgent* pSipUserAgent,
                                    int redirectorNo,
                                    const UtlString& localDomainHost)
{
   return OS_SUCCESS;
}

// Finalizer
void
SipRedirectorAuthRouter::finalize()
{
}

RedirectPlugin::LookUpStatus SipRedirectorAuthRouter::lookUp(
   const SipMessage& message,
   const UtlString& requestString,
   const Url& requestUri,
   const UtlString& method,
   SipMessage& response,
   RequestSeqNo requestSeqNo,
   int redirectorNo,
   SipRedirectorPrivateStorage*& privateStorage)
{
   RedirectPlugin::LookUpStatus lookupStatus = RedirectPlugin::LOOKUP_SUCCESS; // always, so far
   
   // Do the cheap global tests first
   //   Is there an authorization proxy route?
   //   Is the request method INVITE? (This operates only on initial invites)
   //   Does the response have any Contacts? (if not, there's nothing to do)
   if (!mAuthUrl.isNull())
   {
      int contacts = response.getCountHeaderFields(SIP_CONTACT_FIELD);
      if (   (method.compareTo(SIP_INVITE_METHOD, UtlString::ignoreCase) == 0)
          && (contacts)
          )
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "%s::lookUp checking for To tag",
                       mLogName.data()
                       );
         
         /*
          * Loop through each contact in the response,
          *   checking to see if the contact includes a route set
          */
         UtlString contact;
         for (int contactNumber = 0;
              response.getContactEntry(contactNumber, &contact);
              contactNumber++
              )
         {
            Url contactUri(contact);
            
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "%s::lookUp contact %d '%s'",
                          mLogName.data(), contactNumber, contact.data()
                          );

            // is there a route header parameter in the contact?
            UtlString routeValue;
            if (contactUri.getHeaderParameter(SIP_ROUTE_FIELD, routeValue))
            {
               // there is a route in the contact
               // prepend the authproxy route to ensure it is checked
               UtlString checkedRoute(mAuthUrl);
               checkedRoute.append(SIP_MULTIFIELD_SEPARATOR);

               checkedRoute.append(routeValue);

               contactUri.setHeaderParameter(SIP_ROUTE_FIELD, checkedRoute);

               // and put the modified contact back into the message
               UtlString modifiedContact;
               contactUri.toString(modifiedContact);
               response.setContactField(modifiedContact, contactNumber);

               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "%s::lookUp modified:\n"
                             "   '%s'\n"
                             "in '%s'\n"
                             "to '%s'\n"
                             "in '%s'\n",
                             mLogName.data(),
                             routeValue.data(), contact.data(),
                             checkedRoute.data(), modifiedContact.data()
                             );
            }
         } // loop over all contacts
      }
      else
      {
         // request is not an INVITE, or no Contact headers in the response
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "%s::lookUp "
                       "'%s' request is not an INVITE or has no response contacts (%d) - ignored.",
                       mLogName.data(), method.data(), contacts
                       );
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "%s::lookup No authproxy configured",
                    mLogName.data());
   }

   return lookupStatus;
}
