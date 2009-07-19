//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ACDLineManager_h_
#define _ACDLineManager_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsMutex.h>
#include <utl/UtlHashMap.h>
#include <net/ProvisioningClass.h>
#include <net/ProvisioningAttrList.h>
#include "ACDLine.h"

// DEFINES
// Provisioning Tag Definitions
#define ACD_LINE_TAG                  "acd-line"
#define LINE_URI_TAG                  "uri"
#define LINE_NAME_TAG                 "name"
#define LINE_EXTENSION_TAG            "extension"
#define LINE_TRUNK_MODE_TAG           "trunk-mode"
#define LINE_PUBLISH_PRESENCE_TAG     "publish-line-presence"
#define LINE_ACD_QUEUE_TAG            "acd-queue"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class CredentialDB;
class ACDServer;
class ACDCallManager;
class ACDQueueManager;


// ACDLineManager object is a centralized manager of ACDLine objects.
// ACDLine object register and deregister with the LineManager
// in response to construction and destruction.  The LineManager is then
// responsible for registering and deregistering with the UA via the
// associated ACDCallManager.

class ACDLineManager : public ProvisioningClass {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   // Default constructor
   ACDLineManager(ACDServer* pAcdServer);

   // Destructor
   ~ACDLineManager();

/* ============================ MANIPULATORS ============================== */

   OsStatus initialize(void);

   OsStatus start(void);

   bool loadConfiguration(void);

   ACDLine* createACDLine(const char* pLineUriString,
                          const char* pName,
                          const char* pExtension,
                          bool        trunkMode,
                          bool        publishLinePresence,
                          const char* pAcdQueue);

   void deleteACDLine(const char* pLineUriString, const char* pName, const char* pExtension);

   ProvisioningAttrList* Create(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Delete(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Set(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Get(ProvisioningAttrList& rRequestAttributes);
//   ProvisioningAttrList* Action(ProvisioningAttrList& rRequestAttributes);

/* ============================ ACCESSORS ================================= */

   SIPX_INST getAcdCallManagerHandle(void);

   ACDCallManager* getAcdCallManager(void);

   ACDQueueManager* getAcdQueueManager(void);

   ACDLine* getAcdLineReference(SIPX_LINE hLineHandle);

   ACDLine* getAcdLineReference(UtlString& rLineUriString);

   ACDServer* getAcdServer(void);

   ACDLine* getAcdLineReferenceByName(UtlString& rLineNameString);

   ACDLine* getAcdLineReferenceByExtension(UtlString& rLineExtensionString);

   /// Get a handle to the database where we look up line credentials.
   CredentialDB* getCredentialDb(void);

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
   private:
   OsMutex          mLock;                   ///< Lock used for atomic access
   ACDServer*       mpAcdServer;             ///< Refernece to the parent ACDServer
   ACDCallManager*  mpAcdCallManager;        ///< Reference to the associated UA
   SIPX_INST        mhAcdCallManagerHandle;  ///< The sipXtapi handle for the UA
   ACDQueueManager* mpAcdQueueManager;       ///< Reference to the associated UA
   UtlHashMap       mLineHandleMap;          ///< Handle Map for ACDLine objects
   UtlHashMap       mAcdLineList;            ///< List of ACDLine objects
   UtlHashMap       mAcdLineNameList;        ///< List of ACDLine Name objects
   UtlHashMap       mAcdLineExtensionList;   ///< List of ACDLine Extension objects
   CredentialDB*    mCredentialDb;           ///< Handle for database to find line credentials

};

#endif  // _ACDLineManager_h_
