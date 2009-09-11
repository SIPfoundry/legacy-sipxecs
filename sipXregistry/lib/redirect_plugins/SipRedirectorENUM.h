//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREDIRECTORENUM_H
#define SIPREDIRECTORENUM_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "registry/RedirectPlugin.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * SipRedirectorENUM is singleton class whose object adds contacts by looking
 * up the NAPTR records for ENUM dialing.
 * Currently, it is experimental, and only knows how to look up numbers
 * in one ENUM tree.
 */

class SipRedirectorENUM : public RedirectPlugin
{
  public:

   explicit SipRedirectorENUM(const UtlString& instanceName);

   ~SipRedirectorENUM();

   /**
    * Uses the following parameters:
    *
    * PREFIX - dialing prefix (optional)
    * BASE_DOMAIN - base well-known domain for doing NAPTR lookups (required)
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

   // Dialing prefix.
   UtlString mDialPrefix;
   // Whether to include "+" in the application-specific-string.
   UtlBoolean mPrefixPlus;
   // Digits to prefix to dialed digits to product E.164 number.
   UtlString mE164Prefix;
   // Base domain for DNS lookup.
   UtlString mBaseDomain;
};

#endif // SIPREDIRECTORENUM_H
