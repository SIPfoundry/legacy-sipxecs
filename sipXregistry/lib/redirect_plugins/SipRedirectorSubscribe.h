//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREDIRECTORSUBSCRIBE_H
#define SIPREDIRECTORSUBSCRIBE_H

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
class SipMessage;

class SipRedirectorSubscribe : public RedirectPlugin
{
public:

   explicit SipRedirectorSubscribe(const UtlString& instanceName);

   ~SipRedirectorSubscribe();

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

private:

};

#endif // SIPREDIRECTORSUBSCRIBE_H
