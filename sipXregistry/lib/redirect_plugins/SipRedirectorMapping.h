//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREDIRECTORMAPPING_H
#define SIPREDIRECTORMAPPING_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "registry/RedirectPlugin.h"
#include "digitmaps/MappingRulesUrlMapping.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * SipRedirectorMapping is a class whose object adds contacts that are
 * listed a mapping rules file.
 *
 * Currently, we instantiate two objects within the class, one for
 * mappingrules.xml and one for fallbackrules.xml.
 */

class SipRedirectorMapping : public RedirectPlugin
{
  public:

   explicit SipRedirectorMapping(const UtlString& instanceName);

   ~SipRedirectorMapping();

   /**
    * Requires the following config parameters:
    *
    * MEDIA_SERVER - the URI of the Media Server.
    *
    * VOICEMAIL_SERVER - the URI of the voicemail server.
    *
    * MAPPING_RULES_FILENAME - full filename of the mapping rules file
    * to load (e.g., "mappingrules.xml")
    *
    */
   virtual void readConfig(OsConfigDb& configDb);

   virtual OsStatus initialize(OsConfigDb& configDb,
                               int redirectorNo,
                               const UtlString& localDomainHost);

   virtual void finalize();

   virtual RedirectPlugin::LookUpStatus lookUp(
      const SipMessage& message,
      const UtlString& requestString,
      const Url& requestUri,
      const UtlString& method,
      ContactList& contactList,
      RequestSeqNo requestSeqNo,
      int redirectorNo,
      SipRedirectorPrivateStorage*& privateStorage,
      ErrorDescriptor& errorDescriptor);

   virtual const UtlString& name( void ) const;

  protected:

   // String to use in place of class name in log messages:
   // "[instance] class".
   UtlString mLogName;

   /**
    * Set to OS_SUCCESS once the file of mapping rules is loaded into memory.
    */
   OsStatus mMappingRulesLoaded;

   /**
    * The mapping rules parsed from the file.
    */
   MappingRulesUrlMapping mMap;

   /**
    * SIP URI to access the Media Server.
    */
   UtlString mMediaServer;

   /**
    * HTTP URL to access the Voicemail Server.
    */
   UtlString mVoicemailServer;

   /**
    * Full name of file containing the mapping rules.
    */
   UtlString mFileName;
};

#endif // SIPREDIRECTORMAPPING_H
