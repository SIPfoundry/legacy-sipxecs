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
#include "sipdb/PermissionDB.h"
#include "sipdb/ResultSet.h"
#include "SipRedirectorMapping.h"

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
   return new SipRedirectorMapping(instanceName);
}

// Constructor
SipRedirectorMapping::SipRedirectorMapping(const UtlString& instanceName) :
   RedirectPlugin(instanceName),
   mMappingRulesLoaded(OS_FAILED)
{
   mLogName.append("[");
   mLogName.append(instanceName);
   mLogName.append("] SipRedirectorMapping");
}

// Destructor
SipRedirectorMapping::~SipRedirectorMapping()
{
}

// Read config information.
void SipRedirectorMapping::readConfig(OsConfigDb& configDb)
{
   configDb.get("MEDIA_SERVER", mMediaServer);
   configDb.get("VOICEMAIL_SERVER", mVoicemailServer);
   configDb.get("MAPPING_RULES_FILENAME", mFileName);
}

// Initialize
OsStatus
SipRedirectorMapping::initialize(OsConfigDb& configDb,
                                 int redirectorNo,
                                 const UtlString& localDomainHost)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::SipRedirectorMapping Loading mapping rules from '%s'",
                 mLogName.data(), mFileName.data());

   mMappingRulesLoaded = mMap.loadMappings(mFileName,
                                           mMediaServer,
                                           mVoicemailServer,
                                           localDomainHost);

   return mMappingRulesLoaded ? OS_SUCCESS : OS_FAILED;
}

// Finalize
void
SipRedirectorMapping::finalize()
{
}

RedirectPlugin::LookUpStatus
SipRedirectorMapping::lookUp(
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
   UtlString callTag = "UNK";
   UtlString permissionName;
   ResultSet urlMappingRegistrations;
   ResultSet urlMappingPermissions;

   // @JC This variable is strangely overloaded
   // If we have no permissions then add any encountered
   // contacts. If we do have permissions then the
   // permission must match
   UtlBoolean permissionFound = TRUE;

   if (mMappingRulesLoaded == OS_SUCCESS)
   {
      mMap.getContactList(
         requestUri,
         urlMappingRegistrations,
         urlMappingPermissions,
         callTag);
   }

   int numUrlMappingPermissions = urlMappingPermissions.getSize();

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "%s::lookUp "
                 "got %d UrlMapping Permission requirements for %d contacts",
                 mLogName.data(), numUrlMappingPermissions,
                 urlMappingRegistrations.getSize());

   if (numUrlMappingPermissions > 0)
   {
      // check if we have field parameter that indicates that some permissions should be ignored
      UtlString ignorePermissionStr;
      // :KLUDGE: remove const_cast and uri declaration after XSL-88 gets fixed
      Url& uri = const_cast<Url&>(requestUri);
      uri.getUrlParameter("sipx-noroute", ignorePermissionStr);

      for (int i = 0; i<numUrlMappingPermissions; i++)
      {
         UtlHashMap record;
         urlMappingPermissions.getIndex(i, record);
         UtlString permissionKey("permission");
         UtlString urlMappingPermissionStr =
            *((UtlString*) record.findValue(&permissionKey));
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "%s::lookUp checking permissions DB for "
                       "urlMappingPermissions[%d] = '%s'",
                       mLogName.data(), i,
                       urlMappingPermissionStr.data());

         // Try to match the permission
         // so assume it cannot be found unless we
         // see a match in the IMDB
         permissionFound = FALSE;

         // if the permission we are looking for is the one the we are supposed to ignore,
         // than assume that permission is not found
         if (urlMappingPermissionStr.compareTo(ignorePermissionStr, UtlString::ignoreCase) == 0)
         {
             OsSysLog::add(FAC_SIP, PRI_DEBUG,
                           "%s::lookUp ignoring permission '%s'",
                           mLogName.data(),
                           ignorePermissionStr.data());
             continue;
         }

         // See if we can find a matching permission in the IMDB
         ResultSet dbPermissions;

         // check in permission database is user has permisssion for voicemail
         PermissionDB::getInstance()->
            getPermissions(requestUri, dbPermissions);

         int numDBPermissions = dbPermissions.getSize();

         UtlString permissionsFound;
         for (int j=0; j<numDBPermissions; j++)
         {
            UtlHashMap record;
            dbPermissions.getIndex(j, record);
            UtlString dbPermissionStr =
               *((UtlString*)record.
                 findValue(&permissionKey));

            bool equal = dbPermissionStr.compareTo(urlMappingPermissionStr, UtlString::ignoreCase) == 0;
            if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
            {
               permissionsFound.append(" ");
               permissionsFound.append(dbPermissionStr);
               if (equal)
               {
                  permissionsFound.append("[matches]");
               }
            }
            if (equal)
            {
               // matching permission found in IMDB
               permissionFound = TRUE;
               break;
            }
            dbPermissionStr.remove(0);
         }
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "%s::lookUp %d permissions configured for request URI '%s'.  Checking: %s",
                       mLogName.data(), numDBPermissions,
                       requestUri.toString().data(),
                       permissionsFound.data());

         if (permissionFound)
         {
            break;
         }
         urlMappingPermissionStr.remove(0);
      }
   }

   // either there were no requirements to match against voicemail
   // or there were and we found a match in the IMDB for the URI
   // so now add the contacts to the SIP message
   if (permissionFound)
   {
      int numUrlMappingRegistrations = urlMappingRegistrations.getSize();

      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "%s::lookUp got %d UrlMapping Contacts",
                    mLogName.data(), numUrlMappingRegistrations);

      if (numUrlMappingRegistrations > 0)
      {
         for (int i = 0; i < numUrlMappingRegistrations; i++)
         {
            UtlHashMap record;
            urlMappingRegistrations.getIndex(i, record);
            UtlString contactKey("contact");
            UtlString contact= *(dynamic_cast <UtlString*> (record.findValue(&contactKey)));

            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "%s::lookUp contact = '%s'",
                          mLogName.data(), contact.data());
            Url contactUri(contact);
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "%s::lookUp contactUri = '%s'",
                          mLogName.data(), contactUri.toString().data());
            // We no longer check for recursive loops here because we
            // have comprehensive loop detection in the proxy.
            UtlString recordRoute;
            UtlString curCallDest;
            if (message.getRecordRouteField(0,&recordRoute)) {
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "%s::lookUp RecordRouteField = '%s'",
                          mLogName.data(), recordRoute.data());
            }
            contactUri.setUrlParameter(SIP_SIPX_CALL_DEST_FIELD, callTag.data());

            // Add the contact.
            contactList.add( contactUri, *this );
         }
      }
   }

   return RedirectPlugin::SUCCESS;
}

const UtlString& SipRedirectorMapping::name( void ) const
{
   return mLogName;
}
