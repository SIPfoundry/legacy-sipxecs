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
#include "sipdb/RegistrationDB.h"
#include "sipdb/UserForwardDB.h"
#include "SipRedirectorRegDB.h"

// DEFINES

#define URI_IN_PREFIX "~~in~"

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
   return new SipRedirectorRegDB(instanceName);
}

// Constructor
SipRedirectorRegDB::SipRedirectorRegDB(const UtlString& instanceName) :
   RedirectPlugin(instanceName)
{
   mLogName.append("[");
   mLogName.append(instanceName);
   mLogName.append("] SipRedirectorRegDB");
}

// Destructor
SipRedirectorRegDB::~SipRedirectorRegDB()
{
}

// Initializer
OsStatus
SipRedirectorRegDB::initialize(OsConfigDb& configDb,
                               int redirectorNo,
                               const UtlString& localDomainHost)
{
   return OS_SUCCESS;
}

// Finalizer
void
SipRedirectorRegDB::finalize()
{
}

RedirectPlugin::LookUpStatus
SipRedirectorRegDB::lookUp(
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
   unsigned long timeNow = OsDateTime::getSecsSinceEpoch();
   ResultSet registrations;
   // Local copy of requestUri
   Url requestUriCopy = requestUri;

   // Look for any grid parameter and remove it.
   UtlString gridParameter;
   UtlBoolean gridPresent =
      requestUriCopy.getUrlParameter("grid", gridParameter, 0);
   if (gridPresent)
   {
      requestUriCopy.removeUrlParameter("grid");
   }
   if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
   {
      UtlString temp;
      requestUriCopy.getUri(temp);
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "%s::lookUp gridPresent = %d, gridParameter = '%s', "
                    "requestUriCopy after removing grid = '%s'",
                    mLogName.data(), gridPresent, gridParameter.data(),
                    temp.data());
   }

   RegistrationDB* regDB =
      RegistrationDB::getInstance(); // TODO - change to SipRegistrar::getRegistrationDB (see Scott Lawrence)

   // Give the ~~in~ URIs separate processing.
   UtlString user;
   requestUriCopy.getUserId(user);
   if (user.index(URI_IN_PREFIX) == 0)
   {
      // This is a ~~in~ URI.
      // Check for an '&' separator.
      ssize_t s = user.last('&');
      if (s != UTL_NOT_FOUND)
      {
         // This is a ~~in~[user]&[instrument] URI.
         const char* instrumentp = user.data() + s + 1;
         UtlString u;
         u.append(user,
                  sizeof (URI_IN_PREFIX) - 1,
                  s - (sizeof (URI_IN_PREFIX) - 1));
         requestUriCopy.setUserId(u);
         regDB->
            getUnexpiredContactsUserInstrument(requestUriCopy, instrumentp, timeNow, registrations);
      }
      else
      {
         // This is a ~~in~[instrument] URI.
         const char* instrumentp = user.data() + sizeof (URI_IN_PREFIX) - 1;
         regDB->
            getUnexpiredContactsInstrument(instrumentp, timeNow, registrations);
      }         
   }
   else
   {
      // Note that getUnexpiredContactsUser will reduce the requestUri to its
      // identity (user/host/port) part before searching in the
      // database.  The requestUri identity is matched against the
      // "identity" column of the database, which is the identity part of
      // the "uri" column which is stored in registration.xml.
      regDB->
         getUnexpiredContactsUser(requestUriCopy, timeNow, registrations);
   }

   int numUnexpiredContacts = registrations.getSize();

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::lookUp got %d unexpired contacts",
                 mLogName.data(), numUnexpiredContacts);

   // Check for a per-user call forward timer.
   // Don't set timer if we're not going to forward to voicemail.
   UtlString userCfwdTimer;
   bool foundUserCfwdTimer = false;

   if (method.compareTo(SIP_INVITE_METHOD) == 0)
   {
      UtlString noRoute;
      requestUriCopy.getUrlParameter("sipx-noroute", noRoute);

      if ((!noRoute.isNull()) && (noRoute.compareTo("Voicemail") == 0))
      {
          // This is not a call scenerio controlled by this users "forward to voicemail" timer
      }
      else
      {
          foundUserCfwdTimer = UserForwardDB::getInstance()->getCfwdTime(requestUriCopy, userCfwdTimer);
      }
   }

   for (int i = 0; i < numUnexpiredContacts; i++)
   {
      // Query the Registration DB for the contact, expires and qvalue columns.
      UtlHashMap record;
      registrations.getIndex(i, record);
      UtlString contactKey("contact");
      UtlString expiresKey("expires");
      UtlString qvalueKey("qvalue");
      UtlString pathKey("path");
      UtlString contact = *((UtlString*) record.findValue(&contactKey));
      UtlString qvalue = *((UtlString*) record.findValue(&qvalueKey));
      UtlString pathVector = *((UtlString*) record.findValue(&pathKey));
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "%s::lookUp contact = '%s', qvalue = '%s', path = '%s'",
                    mLogName.data(), contact.data(), qvalue.data(), pathVector.data() );
      Url contactUri(contact);

      // If available set the per-user call forward timer.
      if (foundUserCfwdTimer)
      {
          contactUri.setHeaderParameter("expires", userCfwdTimer);
      }

      // If the contact URI is the same as the request URI, ignore it.
      if (!contactUri.isUserHostPortEqual(requestUriCopy))
      {
         // Check if the q-value from the database is valid, and if so,
         // add it into contactUri.
         if (!qvalue.isNull() &&
             qvalue.compareTo(SPECIAL_IMDB_NULL_VALUE) != 0)
         {
            // :TODO: (XPL-3) need a RegEx copy constructor here
            // Check if q value is numeric and between the range 0.0 and 1.0.
            static RegEx qValueValid("^(0(\\.\\d{0,3})?|1(\\.0{0,3})?)$");
            if (qValueValid.Search(qvalue.data()))
            {
               contactUri.setFieldParameter(SIP_Q_FIELD, qvalue);
            }
         }

         // Re-apply any grid parameter.
         if (gridPresent)
         {
            contactUri.setUrlParameter("grid", gridParameter);
         }

         contactUri.setUrlParameter(SIP_SIPX_CALL_DEST_FIELD, "INT");
         // Check if database contained a Path value.  If so, add a Route
         // header parameter to the contact with the Path vector taken from
         // the registration data.
         if (!pathVector.isNull() &&
              pathVector.compareTo(SPECIAL_IMDB_NULL_VALUE) != 0)
         {
            UtlString existingRouteValue;
            if ( contactUri.getHeaderParameter(SIP_ROUTE_FIELD, existingRouteValue))
            {
               // there is already a Route header parameter in the contact; append it to the
               // Route derived from the Path vector.
               pathVector.append(SIP_MULTIFIELD_SEPARATOR);
               pathVector.append(existingRouteValue);
            }
            contactUri.setHeaderParameter(SIP_ROUTE_FIELD, pathVector);
         }

         // Add the contact.
         contactList.add( contactUri, *this );
      }
   }

   return RedirectPlugin::SUCCESS;
}

const UtlString& SipRedirectorRegDB::name( void ) const
{
   return mLogName;
}
