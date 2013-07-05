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
#include "net/SipXauthIdentity.h"
#include "net/SipXlocationInfo.h"
#include "net/NameValueTokenizer.h"
#include "net/NetMd5Codec.h"
#include "SipRedirectorFallback.h"
#include "sipXecsService/SipXecsService.h"
#include "sipXecsService/SharedSecret.h"
#include "sipdb/EntityDB.h"
#include "registry/SipRegistrar.h"

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
   return new SipRedirectorFallback(instanceName);
}

// Constructor
SipRedirectorFallback::SipRedirectorFallback(const UtlString& instanceName) :
   RedirectPlugin(instanceName),
   mMappingRulesLoaded(OS_FAILED)
{
   mLogName.append("[");
   mLogName.append(instanceName);
   mLogName.append("] SipRedirectorFallback");
   mAllowUnbound = FALSE;
}

// Destructor
SipRedirectorFallback::~SipRedirectorFallback()
{
}

// Read config information.
void SipRedirectorFallback::readConfig(OsConfigDb& configDb)
{
   configDb.get("MAPPING_RULES_FILENAME", mFileName);
   mAllowUnbound = configDb.getBoolean("ALLOW_UNBOUND", false);
}

// Initialize
OsStatus
SipRedirectorFallback::initialize(OsConfigDb& configDb,
                                 int redirectorNo,
                                 const UtlString& localDomainHost)
{
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "%s::SipRedirectorFallback Loading mapping rules from '%s'",
                 mLogName.data(), mFileName.data());

   mMappingRulesLoaded = mMap.loadMappings(mFileName);
   
#ifndef __USE_OLD_FALLBACKRULES_SCHEMA__
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "%s::SipRedirectorFallback using new, location-aware %s format",
                 mLogName.data(), mFileName.data());
#else
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "%s::SipRedirectorFallback using old, non-location-aware %s format",
                 mLogName.data(), mFileName.data());
#endif   
   
   // read the domain configuration
   OsConfigDb domainConfiguration;
   domainConfiguration.loadFromFile(SipXecsService::domainConfigPath());

   // get the shared secret for generating signatures
   SharedSecret secret(domainConfiguration);
   // Set secret for signing SipXauthIdentity
   SipXauthIdentity::setSecret(secret.data());   

   return mMappingRulesLoaded ? OS_SUCCESS : OS_FAILED;
}

// Finalize
void
SipRedirectorFallback::finalize()
{
}

RedirectPlugin::LookUpStatus
SipRedirectorFallback::lookUp(
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
   ResultSet urlMappingRegistrations;

   UtlString callTag = "UNK";

   if (mMappingRulesLoaded == OS_SUCCESS)
   {      
      UtlString callerLocation;
      determineCallerLocation( message, callerLocation );
    
#ifndef __USE_OLD_FALLBACKRULES_SCHEMA__      
      mMap.getContactList(
         requestUri,
         callerLocation,
         urlMappingRegistrations,
         callTag );
#else
      ResultSet dummyMappingPermissions;
      mMap.getContactList(
         requestUri,
         urlMappingRegistrations, 
         dummyMappingPermissions );
#endif      
      
      int numUrlMappingRegistrations = urlMappingRegistrations.getSize();
      Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                    "%s::lookUp got %d UrlMapping Contacts for %s @ location '%s'",
                    mLogName.data(), numUrlMappingRegistrations, requestString.data(), callerLocation.data() );

      if (numUrlMappingRegistrations > 0)
      {
         for (int i = 0; i < numUrlMappingRegistrations; i++)
         {
            UtlHashMap record;
            urlMappingRegistrations.getIndex(i, record);
            UtlString contactKey("contact");
            UtlString contact= *(dynamic_cast <UtlString*> (record.findValue(&contactKey)));
            UtlString callTagKey("callTag");
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "%s::lookUp contact = '%s'",
                          mLogName.data(), contact.data());
            Url contactUri(contact);
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "%s::lookUp contactUri = '%s'",
                          mLogName.data(), contactUri.toString().data());

            contactUri.setUrlParameter(SIP_SIPX_CALL_DEST_FIELD, callTag.data());

            // Add the contact.
            contactList.add( contactUri, *this );
         }
      }
   }
   return RedirectPlugin::SUCCESS;
}

const UtlString& SipRedirectorFallback::name( void ) const
{
   return mLogName;
}

OsStatus 
SipRedirectorFallback::determineCallerLocation(
   const SipMessage& message,
   UtlString& callerLocation )
{
   // First, try to determine the location of the caller based on its public IP address.  If that 
   // does not yield any result revert to looking up the location provisioned against the 
   // caller's identity
   OsStatus result = OS_SUCCESS;   
   if( determineCallerLocationFromCallerIpAddress( message, callerLocation ) != OS_SUCCESS )
   {
      result = determineCallerLocationFromProvisionedUserLocation( message, callerLocation );
   }
   if (OS_SUCCESS != result)
   {
     result = determineCallerLocationFromLocationInfoHeader(message,callerLocation );
   }

   return result;
}

OsStatus 
SipRedirectorFallback::determineCallerLocationFromCallerIpAddress(
   const SipMessage& message,
   UtlString& callerLocation )
{
   OsStatus result = OS_FAILED;
   return result;
}

OsStatus 
SipRedirectorFallback::determineCallerLocationFromProvisionedUserLocation(
   const SipMessage& message,
   UtlString& callerLocation )
{
   OsStatus result = OS_FAILED;
   callerLocation.remove( 0 );


  // First, determine the identity of the caller.  This is done by looking for
  // a properly signed P-Asserted identity in the request message.
  // If the request contains a P-Asserted-Identity header and is not signed,
  // we will not trust it the returned location will be blank.
  UtlString matchedIdentityHeader;
  SipXauthIdentity* pSipxIdentity = 0;
  Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipRedirectorFallback:: unbound entities allowing: %s", mAllowUnbound ? "TRUE" : "FALSE");
  if (!mAllowUnbound) {
	  pSipxIdentity = new SipXauthIdentity( message, matchedIdentityHeader, false );
  } else {
	  pSipxIdentity = new SipXauthIdentity( message, matchedIdentityHeader, false, SipXauthIdentity::allowUnbound);
  }

  if( pSipxIdentity && !matchedIdentityHeader.isNull() )
  {

     UtlString authenticatedUserIdentity;
     bool bRequestIsAuthenticated;
     bRequestIsAuthenticated = pSipxIdentity->getIdentity( authenticatedUserIdentity );
     if( bRequestIsAuthenticated )
     {
        // we now have the autheticated identity of the caller.  Look up the user location
        // database to find out the location that is mapped to it.
        //ResultSet userLocationsResult;

        // Check in User Location database if user has locations
        //mpUserLocationDbInstance->getLocations( authenticatedUserIdentity, userLocationsResult );

        // Get the caller's site location. Only the first returned location is used.
        // This is not a problem given that a user should only belong to one location.

         EntityRecord entity;
         EntityDB* entityDb = SipRegistrar::getInstance(NULL)->getEntityDB();
         if (entityDb->findByIdentity(authenticatedUserIdentity.str(), entity))
        {

              callerLocation = entity.location().c_str();
              result = OS_SUCCESS;
              Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                            "%s::determineCallerLocationFromProvisionedUserLocation mapped user '%s' taken from header '%s' to location '%s' based on its provisioned location",
                            mLogName.data(), authenticatedUserIdentity.data(),
                            authenticatedUserIdentity.data(),
                            entity.location().c_str() );
        }
     }
  }
   delete pSipxIdentity;
   return result;
}

OsStatus
SipRedirectorFallback::determineCallerLocationFromLocationInfoHeader(
   const SipMessage& message,
   UtlString& callerLocation )
{
   OsStatus result = OS_FAILED;
   callerLocation.remove(0);

   SipXlocationInfo locationInfo(message);
   if (locationInfo.getLocation(callerLocation))
   {
     result = OS_SUCCESS;

     Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                   "%s::determineCallerLocationFromLocationInfoHeader got from header 'X-sipX-Location-Branch' location '%s'",
                   mLogName.data(),
                   callerLocation.data());
   }

   return result;
}
