//
//
// Copyright (C) 2009 Nortel Networks, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREDIRECTORUSERPARAM_H
#define SIPREDIRECTORUSERPARAM_H

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
 * SipRedirectorUserParam is singleton class whose object
 * strips parameters from the userpart of a SIP URL
 */

class SipRedirectorUserParam : public RedirectPlugin
{
  public:

   explicit SipRedirectorUserParam(const UtlString& instanceName);

   ~SipRedirectorUserParam();

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

  private:

   /// Configuration flag for whether or not this plugin should remove embedded parameters
   bool      mStripAll;
   /// String to use in place of class name in log messages: "[instance] class".
   UtlString mLogName;

  protected:
};

#endif // SIPREDIRECTORUSERPARAM_H
