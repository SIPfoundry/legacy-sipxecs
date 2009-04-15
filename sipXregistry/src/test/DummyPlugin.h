// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef __DUMMY_PLUGIN__
#define __DUMMY_PLUGIN__

// APPLICATION INCLUDES
#include "registry/RedirectPlugin.h"
#include "net/Url.h"



// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class DummyPlugin : public RedirectPlugin
{
public:
   explicit DummyPlugin(const UtlString& instanceName) :
      RedirectPlugin(instanceName)
   {
      mName = instanceName;
   }
    
   virtual OsStatus initialize(OsConfigDb& configDb,
                               int redirectorNo,
                               const UtlString& localDomainHost)
   {
      return OS_SUCCESS;
   }
   
   virtual void finalize()
   {
      
   }
   
   virtual LookUpStatus lookUp(
      const SipMessage& message,      
      const UtlString& requestString, 
      const Url& requestUri,          
      const UtlString& method,
      ContactList& contactList,         
      RequestSeqNo requestSeqNo,      
      int redirectorNo,               
      class SipRedirectorPrivateStorage*& privateStorage, 
      ErrorDescriptor& errorDescriptor )
   {
      return RedirectPlugin::SUCCESS;
   }
   
   virtual const UtlString& name( void ) const
   {
      return mName;
   }
   
   UtlString mName;
};

#endif //__DUMMY_PLUGIN__
