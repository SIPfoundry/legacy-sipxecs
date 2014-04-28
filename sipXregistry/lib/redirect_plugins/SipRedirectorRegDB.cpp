//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#include "SipRedirectorRegDB.h"

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include <utl/UtlRegex.h>
#include "os/OsDateTime.h"
#include "os/OsLogger.h"
#include "sipdb/ResultSet.h"
#include "registry/SipRegistrar.h"
#include "sipdb/EntityDB.h"
#include "sipdb/RegDB.h"


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
   UtlString& requestString,
   Url& requestUri,
   const UtlString& method,
   ContactList& contactList,
   RequestSeqNo requestSeqNo,
   int redirectorNo,
   SipRedirectorPrivateStorage*& privateStorage,
   ErrorDescriptor& errorDescriptor)
{
   unsigned long timeNow = OsDateTime::getSecsSinceEpoch();
   
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
   if (Os::Logger::instance().willLog(FAC_SIP, PRI_DEBUG))
   {
      UtlString temp;
      requestUriCopy.getUri(temp);
      Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                    "%s::lookUp gridPresent = %d, gridParameter = '%s', "
                    "requestUriCopy after removing grid = '%s'",
                    mLogName.data(), gridPresent, gridParameter.data(),
                    temp.data());
   }

   RegDB::Bindings registrations;

   // Give the ~~in~ URIs separate processing.
   UtlString user;
   requestUriCopy.getUserId(user);
   RegDB* regDb = SipRegistrar::getInstance(NULL)->getRegDB();
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

         //regDB->
         //   getUnexpiredContactsUserInstrument(requestUriCopy, instrumentp, timeNow, registrations);
         UtlString identity;
         requestUriCopy.getIdentity(identity);
         regDb->getUnexpiredContactsUserInstrument(identity.str(), instrumentp, timeNow, registrations, true);
      }
      else
      {
         // This is a ~~in~[instrument] URI.
         const char* instrumentp = user.data() + sizeof (URI_IN_PREFIX) - 1;
         regDb->getUnexpiredContactsInstrument(instrumentp, timeNow, registrations, true);
      }         
   }
   else
   {
      // Note that getUnexpiredContactsUser will reduce the requestUri to its
      // identity (user/host/port) part before searching in the
      // database.  The requestUri identity is matched against the
      // "identity" column of the database, which is the identity part of
      // the "uri" column which is stored in registration.xml.

      UtlString identity;
     requestUriCopy.getIdentity(identity);
     regDb->getUnexpiredContactsUser(identity.str(), timeNow, registrations, true);

   }

   int numUnexpiredContacts = registrations.size();

   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "%s::lookUp got %d unexpired contacts",
                 mLogName.data(), numUnexpiredContacts);

   // Check for a per-user call forward timer.
   // Don't set timer if we're not going to forward to voicemail.
   std::ostringstream userCfwdTimer;
   bool foundUserCfwdTimer = false;

   if (method.compareTo(SIP_INVITE_METHOD) == 0)
   {
      UtlString userforwardParam;
      requestUriCopy.getUrlParameter("sipx-userforward", userforwardParam);
      
      UtlString expiresParam;
      requestUriCopy.getUrlParameter("sipx-expires", expiresParam);

      if ((!userforwardParam.isNull()) && (userforwardParam.compareTo("false", UtlString::ignoreCase) ) == 0)
      {
          // This is not a call scenerio controlled by this users "forward to voicemail" timer
      }
      else
      {
          UtlString identity;
          requestUriCopy.getIdentity(identity);
          EntityRecord entity;

          EntityDB* entityDb = SipRegistrar::getInstance(NULL)->getEntityDB();
          foundUserCfwdTimer = entityDb->findByIdentity(identity.str(), entity);
          if (foundUserCfwdTimer)
            userCfwdTimer << entity.callForwardTime();
      }
      
      if (userCfwdTimer.str().empty() && !expiresParam.isNull())
      {
        userCfwdTimer << expiresParam.data();
        foundUserCfwdTimer = true;
      }
   }

   for (RegDB::Bindings::const_iterator iter = registrations.begin(); iter != registrations.end(); iter++)
   {
      // Query the Registration DB for the contact, expires and qvalue columns.

      Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                    "%s::lookUp contact = '%s', qvalue = '%s', path = '%s'",
                    mLogName.data(), iter->getContact().c_str(), iter->getQvalue().c_str(), iter->getPath().c_str() );
      Url contactUri(iter->getContact().c_str());

      // If available set the per-user call forward timer.
      if (foundUserCfwdTimer)
      {
          contactUri.setHeaderParameter("expires", userCfwdTimer.str().c_str());
      }

      // If the contact URI is the same as the request URI, ignore it.
      if (!contactUri.isUserHostPortEqual(requestUriCopy))
      {
         // Check if the q-value from the database is valid, and if so,
         // add it into contactUri.
         if (!iter->getQvalue().empty())
         {
            // :TODO: (XPL-3) need a RegEx copy constructor here
            // Check if q value is numeric and between the range 0.0 and 1.0.
            static RegEx qValueValid("^(0(\\.\\d{0,3})?|1(\\.0{0,3})?)$");
            if (qValueValid.Search(iter->getQvalue().c_str()))
            {
               contactUri.setFieldParameter(SIP_Q_FIELD, iter->getQvalue().c_str());
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
         if (!iter->getPath().empty())
         {
            UtlString existingRouteValue;
            std::string pathVector = iter->getPath();
            if ( contactUri.getHeaderParameter(SIP_ROUTE_FIELD, existingRouteValue))
            {
               // there is already a Route header parameter in the contact; append it to the
               // Route derived from the Path vector.
                pathVector += SIP_MULTIFIELD_SEPARATOR;
                pathVector += existingRouteValue.str();
            }
            contactUri.setHeaderParameter(SIP_ROUTE_FIELD, pathVector.c_str());
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
