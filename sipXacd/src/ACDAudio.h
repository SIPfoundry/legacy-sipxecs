//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ACDAudio_h_
#define _ACDAudio_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsFS.h>
#include <os/OsTask.h>
#include <net/Url.h>
#include <utl/UtlString.h>
#include <utl/UtlContainable.h>

// DEFINES
#define HTTP_GET_TIMEOUT 180000

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpRawAudioBuffer;
class ACDAudioManager;


class ACDAudio : public UtlContainable, public OsTask {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

   // Default constructor
   ACDAudio(ACDAudioManager* pAcdAudioManager,
            const char*     pName,
            bool            localStore,
            const char*     pAudioUriString,
            const char*     pDescription);

   // Destructor
   ~ACDAudio();

/* ============================ MANIPULATORS ============================== */

   // Set the requested ACDAudio attributes.
   void setAttributes(ProvisioningAttrList& rRequestAttributes);

   // Delete the local file store if it exists
   void deleteLocalStore(void);

/* ============================ ACCESSORS ================================= */

   // Return the name for this audio object.
   UtlString* getAudioName(void);

   // Return the URI associated with this audio object.
   UtlString* getUriString(void);

   // Return the audio objects actual audio data and length
   bool getAudio(char*& prAudio, unsigned long& rLength);

   // Retrieve the requested attribute values.
   void getAttributes(ProvisioningAttrList& rRequestAttributes, ProvisioningAttrList*& prResponse);

   // Calculate a unique hash code for this object.
   virtual unsigned hash() const;

   // Get the ContainableType for a UtlContainable derived class.
   virtual UtlContainableType getContainableType() const;

/* ============================ INQUIRY =================================== */

   // Compare the this object to another like-object.
   virtual int compareTo(UtlContainable const *) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   static const UtlContainableType TYPE; // Class type used for runtime checking

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsMutex           mLock;                  // Lock used for atomic access
   UtlString         mName;                  // The name for this audio object
   bool              mLocalStore;            // Is this audio source local or streamed
   Url               mUri;                   // The uri to the audio source
   UtlString         mUriString;             // The uri to the audio source
   UtlString         mDescription;           // The description of this audio object
   ACDAudioManager*  mpAcdAudioManager;      // Reference to the parent ACDAudioManager object
   OsPath            mAudioPath;             // Path to the local file store of this audio data
   MpRawAudioBuffer* mpAudioBuffer;          // Pointer to memory resident raw audio data

   // Underlying thread body
   int run(void* pArg);
};

#endif  // _ACDAudio_h_
