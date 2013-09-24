//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREDIRECTORISN_H
#define SIPREDIRECTORISN_H

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
 * SipRedirectorISN is singleton class whose object adds contacts by looking
 * up the NAPTR records for ISN dialing strings ("digits*digits").
 */

class SipRedirectorISN : public RedirectPlugin
{
  public:

   explicit SipRedirectorISN(const UtlString& instanceName);

   ~SipRedirectorISN();

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

   UtlString mPrefix;
   UtlString mBaseDomain;
};

#endif // SIPREDIRECTORISN_H
