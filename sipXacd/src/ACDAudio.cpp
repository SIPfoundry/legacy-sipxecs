//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpRawAudioBuffer.h>
#include <net/HttpMessage.h>
#include "ACDAudioManager.h"
#include "ACDAudio.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern OsSysLogPriority gACD_DEBUG;

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType ACDAudio::TYPE = "ACDAudio";


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudio::ACDAudio
//
//  SYNOPSIS:    ACDAudio(
//                 ACDAudioManager* pAcdAudioManager, Pointer to parent ACDAudioManager instance
//                 const char*      pName,            The name of this audio object
//                 bool             localStore,       Is this audio local or streamed
//                 const char*      pAudioUriString,  The URI of this audio data
//                 const char*      pDescription)     Description of this audio object
//
//  DESCRIPTION: The default constructor for the ACDAudio class.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDAudio::ACDAudio(ACDAudioManager* pAcdAudioManager,
                   const char*      pName,
                   bool             localStore,
                   const char*      pAudioUriString,
                   const char*      pDescription)
: OsTask("ACDAudio-%d"), mLock(OsMutex::Q_FIFO)
{
   mpAcdAudioManager = pAcdAudioManager;
   mName             = pName;
   mLocalStore       = localStore;
   mUri              = pAudioUriString;
   mUriString        = pAudioUriString;
   mDescription      = pDescription;

   mpAudioBuffer     = NULL;

   if (mLocalStore) {
      // This audio is flagged as local-store
      // Check to see if it has been fetched and stored locally
      mAudioPath = mpAcdAudioManager->getAudioStorePath();
      if (mAudioPath != NULL) {
         // Valid audio store path, append the file
         OsPath uriPath;

         mUri.getPath(uriPath);
         uriPath.Split();
         mAudioPath += OsPath::separator;
         mAudioPath += uriPath.getFilename();
         mAudioPath += uriPath.getExt();

         // Check to see if the file is already there
         if (!OsFileSystem::exists(mAudioPath)) {
            // No, attempt to download it
            OsSysLog::add(FAC_ACD, PRI_INFO, "ACDAudio::ACDAudio - starting download of: %s", mUriString.data());
            start();
         }
      }
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudio::~ACDAudio
//
//  SYNOPSIS:    None.
//
//  DESCRIPTION: The destructor for the ACDAudio class.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDAudio::~ACDAudio()
{
   // If there is an MpRawAudioBuffer instance, free it up
   if (mpAudioBuffer) {
      delete mpAudioBuffer;
   }
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudio::setAttributes
//
//  SYNOPSIS:    void setAttributes(
//                 ProvisioningAttrList& rRequestAttributes) ProvisioningAgent attribute list
//                                                           containing one or more ACDAgent
//                                                           attributes to be updated.
//
//  DESCRIPTION: Used by the ACDAudioManager::set() function to update on or more attributes
//               in response to recieving a provisioning set request.  There are side effects
//               to setting monitor-presence and always-available if there is a change in value.
//
//  RETURNS:     None.
//
//  ERRORS:      If an error is encountered, an exception will be thrown, with the description
//               of the error being specified in "UtlString error".
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDAudio::setAttributes(ProvisioningAttrList& rRequestAttributes)
{
   mLock.acquire();

   // Set the individual attributes

   // name
   rRequestAttributes.getAttribute(AUDIO_NAME_TAG, mName);

   // local-store
   rRequestAttributes.getAttribute(AUDIO_LOCAL_STORE_TAG, mLocalStore);

   // uri
   rRequestAttributes.getAttribute(AUDIO_URI_TAG, mUriString);

   // description
   rRequestAttributes.getAttribute(AUDIO_DESCRIPTION_TAG, mDescription);

   mLock.release();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudio::deleteLocalStore
//
//  SYNOPSIS:    void deleteLocalStore(void)
//
//  DESCRIPTION: Deletes the local file store, if it exists.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDAudio::deleteLocalStore(void)
{
   // Check for the existance of the local store file
   if (mLocalStore && OsFileSystem::exists(mAudioPath)) {
      // Found it, now remove it with force
      OsFileSystem::remove(mAudioPath, false, true);
   }
}

/* ============================ ACCESSORS ================================= */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudio::getUriString
//
//  SYNOPSIS:    UtlString* getUriString(void)
//
//  DESCRIPTION: Returns a pointer to a UtlString object representing the URI which has been
//               configured as the source for the ACDAudio data
//
//  RETURNS:     Pointer to a UtlString representing the configured URI for the ACDAudio data
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

UtlString* ACDAudio::getUriString(void)
{
   return &mUriString;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudio::getAudioName
//
//  SYNOPSIS:    UtlString* getAudioName(void)
//
//  DESCRIPTION: Returns the provisioned name, which is also the index attribute, of this
//               ACDAudio object.
//
//  RETURNS:     Pointer to a UtlString containing the provisioned name of this ACDAudio object
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

UtlString* ACDAudio::getAudioName(void)
{
   return &mName;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudio::getAudio
//
//  SYNOPSIS:    bool getAudio(
//                 char*& prAudio,        Pointer to the audio data
//                 unsigned ling& length) Length of the audio data
//
//  DESCRIPTION: Return the audio objects actual audio data and length
//
//  RETURNS:     bool representing the success or failure of the request.
//
//  ERRORS:      If no audio data is available, returns false.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ACDAudio::getAudio(char*& prAudio, unsigned long& rLength)
{
   if (!mLocalStore) {
      // Audio is not local
      return false;
   }

   // See if the audio is loaded into memory yet
   if (mpAudioBuffer == NULL) {
      // No, try to load it
      mpAudioBuffer = new MpRawAudioBuffer(mAudioPath);
   }

   if (mpAudioBuffer->getAudio(prAudio, rLength) == NULL) {
      // Loading failed, clean up
      delete mpAudioBuffer;
      mpAudioBuffer = false;
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDAudio::getAudio - failed to load audio: %s", mAudioPath.data());
      return false;
   }

   // All is good
   return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudio::hash
//
//  SYNOPSIS:    unsigned hash(void) const
//
//  DESCRIPTION: Calculate a unique hash code for this object.  If the equals operator returns
//               true for another object, then both of those objects must return the same hashcode.
//               The name String is used as the basis of the hash.
//
//  RETURNS:     unsigned int representing the calculated hash code
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned ACDAudio::hash(void) const
{
   return mName.hash();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudio::getContainableType
//
//  SYNOPSIS:    UtlContainableType getContainableType(void) const
//
//  DESCRIPTION: Get the ContainableType for a UtlContainable derived class.
//
//  RETURNS:     UtlContainableType identifying this container type.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

UtlContainableType ACDAudio::getContainableType(void) const
{
   return ACDAudio::TYPE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudio::getAttributes
//
//  SYNOPSIS:    bool getAttributes(
//                 ProvisioningAttrList&  rRequestAttributes, ProvisioningAgent attribute list
//                                                            containing one or more ACDAgent
//                                                            attributes to be retrieved.
//                 ProvisioningAttrList*& prResponse)         ProvisioningAgent attribute list
//                                                            containing the requested attributes
//                                                            and their corresponding values.
//
//  DESCRIPTION: Used by the ACDAudioManager::get() function to request the value of one or more
//               attributes in response to recieving a provisioning get request.
//
//  ERRORS:      If an error is encountered, an exception will be thrown, with the description
//               of the error being specified in "UtlString error".
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDAudio::getAttributes(ProvisioningAttrList& rRequestAttributes, ProvisioningAttrList*& prResponse)
{
   // See if there are any specific attributes listed in the request
   if (rRequestAttributes.attributePresent(AUDIO_LOCAL_STORE_TAG) ||
       rRequestAttributes.attributePresent(AUDIO_URI_TAG) ||
       rRequestAttributes.attributePresent(AUDIO_DESCRIPTION_TAG)) {
      // At least one attribute has been requested, go through list and retrieve
      // name
      if (rRequestAttributes.attributePresent(AUDIO_NAME_TAG)) {
         prResponse->setAttribute(AUDIO_NAME_TAG, mName);
      }

      // local-store
      if (rRequestAttributes.attributePresent(AUDIO_LOCAL_STORE_TAG)) {
         prResponse->setAttribute(AUDIO_LOCAL_STORE_TAG, mLocalStore);
      }

      // uri
      if (rRequestAttributes.attributePresent(AUDIO_URI_TAG)) {
         prResponse->setAttribute(AUDIO_URI_TAG, mUriString);
      }

      // description
      if (rRequestAttributes.attributePresent(AUDIO_DESCRIPTION_TAG)) {
         prResponse->setAttribute(AUDIO_DESCRIPTION_TAG, mDescription);
      }
   }
   else {
      // No specific attributes were requested, send them all back
      // name
      prResponse->setAttribute(AUDIO_NAME_TAG, mName);

      // local-store
      prResponse->setAttribute(AUDIO_LOCAL_STORE_TAG, mLocalStore);

      // uri
      prResponse->setAttribute(AUDIO_URI_TAG, mUriString);

      // description
      prResponse->setAttribute(AUDIO_DESCRIPTION_TAG, mDescription);
   }
}

/* ============================ INQUIRY =================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudio::compareTo
//
//  SYNOPSIS:    int compareTo(
//                 UtlContainable const* pInVal) Pointer to a UtlContainable type to perform
//                                               the comparison against.
//
//  DESCRIPTION: Compare the this object to another UtlContainable derived object of the
//               same type.
//
//  RETURNS:     int value indicating result. zero if equal, non-zero if not.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

int ACDAudio::compareTo(UtlContainable const* pInVal) const
{
   int result;

   if (pInVal->isInstanceOf(ACDAudio::TYPE)) {
      result = mName.compareTo(((ACDAudio*)pInVal)->getAudioName());
   }
   else {
      result = -1;
   }

   return result;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudio::run
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

int ACDAudio::run(void* pArg)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAudio::run - starting get from: %s", mUriString.data());

   HttpMessage *pGetRequest = new HttpMessage;

   pGetRequest->get(mUri, HTTP_GET_TIMEOUT);

   UtlString status;

   pGetRequest->getResponseStatusText(&status);

   if (status == "OK") {
      UtlString audioData;
      ssize_t audioLength;

      const HttpBody* pResponseBody = pGetRequest->getBody();
      pResponseBody->getBytes(&audioData, &audioLength);

      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAudio::run - received %zd bytes from: %s\n",
                    audioLength, mUriString.data());

      // Now save the downloaded audio to a local file
      size_t writeLength;

      OsFile audioFile(mAudioPath);
      if (audioFile.open(OsFile::CREATE) != OS_SUCCESS) {
         OsSysLog::add(FAC_ACD, PRI_ERR, "ACDAudio::run - "
                       "Unable to create audio file: %s", mAudioPath.data());
      }
      else {
         audioFile.write(audioData, audioLength, writeLength);
         audioFile.close();
      }
   }
   else {
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDAudio::run - failed get from: %s", mUriString.data());
   }

   delete pGetRequest;

   return 0;
}

/* ============================ FUNCTIONS ================================= */
