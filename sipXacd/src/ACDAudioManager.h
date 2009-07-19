//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ACDAudioManager_h_
#define _ACDAudioManager_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsFS.h>
#include <os/OsMutex.h>
#include <utl/UtlBool.h>
#include <utl/UtlHashMap.h>
#include <net/ProvisioningClass.h>
#include <net/ProvisioningAttrList.h>

// DEFINES
// Provisioning Tag Definitions
#define ACD_AUDIO_TAG                 "acd-audio"
#define AUDIO_NAME_TAG                "name"
#define AUDIO_LOCAL_STORE_TAG         "local-store"
#define AUDIO_URI_TAG                 "uri"
#define AUDIO_DESCRIPTION_TAG         "description"

#define SIPX_ACD_DIR                  "sipxacd"
#define SIPX_ACD_AUDIO_DIR            "audio"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class ACDServer;
class ACDAudio;

/**
 * ACDAgentManager
 */
class ACDAudioManager : public ProvisioningClass {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

   // Default constructor
   ACDAudioManager(ACDServer* pAcdServer);

   // Destructor
   ~ACDAudioManager();

/* ============================ MANIPULATORS ============================== */

   OsStatus initialize(void);

   OsStatus start(void);

   bool loadConfiguration(void);

   ACDAudio* createACDAudio(const char* pName,
                            bool        localStore,
                            const char* pAudioUriString,
                            const char* pDescription);

   void deleteACDAudio(const char* pName);

   ProvisioningAttrList* Create(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Delete(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Set(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Get(ProvisioningAttrList& rRequestAttributes);
//   ProvisioningAttrList* Action(ProvisioningAttrList& rRequestAttributes);


/* ============================ ACCESSORS ================================= */

   ACDAudio* getAcdAudioReference(UtlString& rName);

   OsPath&   getAudioStorePath(void);

   bool getAudio(UtlString& rName, char*& prAudio, unsigned long& rLength);

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
   private:
   OsMutex              mLock;                   // Lock used for atomic access
   ACDServer*           mpAcdServer;             // Reference to the parent ACDServer
   OsPath               mAudioStoreDirectory;    // Path the the ACD Audio Store directory
   UtlHashMap           mAcdAudioList;           // List of ACDAudio objects
};

#endif  // _ACDAudioManager_h_
