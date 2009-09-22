//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ProvisioningAgent_h_
#define _ProvisioningAgent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsFS.h>
#include <os/OsStatus.h>
#include <utl/UtlString.h>
#include <utl/UtlHashMap.h>
#include "net/ProvisioningAttrList.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TiXmlDocument;
class ProvisioningClass;

/**
 *
 */
class ProvisioningAgent {
public:
/* //////////////////////////// PUBLIC //////////////////////////////////// */
   enum ReturnCodes {
      UNDEFINED      = 0,
      SUCCESS        = 1,
      FAILURE        = 2,
      READONLY       = 3,
      UNKNOWN_OBJECT = 4,
      UNKNOWN_CLASS  = 5,
      INVALID_ATTR   = 6,
      MISSING_ATTR   = 7,
      DUPLICATE      = 8,
      CREATE_FAILURE = 9
   };

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   ProvisioningAgent(const char* pServerClass = NULL, bool persistentStore = FALSE);

   /**
    * Destructor
    */
   ~ProvisioningAgent();

/* ============================ MANIPULATORS ============================== */
   OsStatus registerClass(ProvisioningClass* pProvisioningClass);
   OsStatus unregisterClass(ProvisioningClass* pProvisioningClass);

   ProvisioningAttrList* Create(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Delete(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Set(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Get(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Action(ProvisioningAttrList& rRequestAttributes);


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   ProvisioningClass* lookupProvisioningClass(ProvisioningAttrList& rRequestAttributes);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlString       mServerClass;       /* The name of the server class that
                                        * this agent is provisioning. */
   OsPath*         mpConfigFile;       /* The path and name of the xml config
                                        * file for this Provisioning Agent. */
   TiXmlDocument*  mpXmlConfigDoc;     /* The TinyXml persistant store for
                                        * this Provisioning Agent. */
   UtlHashMap      mRegisteredClasses; /* Provisioning Classes that are retistered
                                        * with this Provisioning Agent. */
};

#endif  // _ProvisioningAgent_h_
