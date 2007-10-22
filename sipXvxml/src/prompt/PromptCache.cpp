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
#include "PromptCache.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Contructor
PromptCache::PromptCache()
   : mCacheGuard(OsMutex::Q_FIFO)
   , mPromptTable()
{
}


// Destructor
PromptCache::~PromptCache()
{
   mPromptTable.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Attempt to lookup the .WAV file and return a pointer to a memory buffer
// containing the .WAV data.  If the file data is not cached, then also add
// it to the data cache.

UtlString* PromptCache::lookup(Url& url, int flags)
{
   OsLock lock(mCacheGuard);

   OsStatus status;
   char* szSource;
   int length, readLength;
   UtlString  urlType;
   UtlString  pathName;
   UtlBoolean cacheEntry;
   UtlContainable* pKey;
   UtlContainable* pCachedKey;
   UtlString* pBuffer = NULL;
   UtlString* pCachedBuffer;


   url.getUrlType(urlType);
   if (urlType.compareTo("file", UtlString::ignoreCase) == 0)
   {
      url.getPath(pathName, TRUE);
      if (pathName.index(STD_PROMPTS_DIR, UtlString::ignoreCase) == UTL_NOT_FOUND)
      {
         cacheEntry = FALSE;
         pKey = NULL;
      }
      else
      {
         cacheEntry = TRUE;
         pKey = mPromptTable.find(&pathName);
      }

      if (pKey == NULL)
      {
         if (cacheEntry == TRUE)
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                          "PromptCache::lookup - Did not find key: %s in prompt cache", pathName.data());
         else
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                          "PromptCache::lookup - Buffering non-cached prompt: %s", pathName.data());

         mUrl = url;
         mpFile = NULL;

         status = open();
         if (status != OS_SUCCESS)
         {
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                          "PromptCache::lookup - Unable to open prompt file: %s", pathName.data());
            return pBuffer;
         }
         status = getLength(length);
         if (status != OS_SUCCESS)
         {
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                          "PromptCache::lookup - Unable to determine lenght of prompt file: %s", pathName.data());
            return pBuffer;
         }
         szSource = new char[length];
         status = read(szSource, length, readLength);
         if ((status != OS_SUCCESS) || (length != readLength))
         {
            delete[] szSource;
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                          "PromptCache::lookup - Failure on read of prompt file: %s", pathName.data());
            return pBuffer;
         }
         close();

         if (cacheEntry == TRUE)
         {
            // Now add the prompt to the table
            pCachedKey = new UtlString(pathName);
            pCachedBuffer = new UtlString(szSource, readLength);
            delete[] szSource;
            pKey = mPromptTable.insertKeyAndValue(pCachedKey, pCachedBuffer);
            if (pKey == NULL)
            {
               delete pCachedKey;
               delete pCachedBuffer;
               OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                             "PromptCache::lookup - Insertion of key: %s failed!!", pathName.data());
               return pBuffer;
            }
            else
            {
               pBuffer = new UtlString(pCachedBuffer->data(), pCachedBuffer->length());
            }
         }
         else
         {
            pBuffer = new UtlString(szSource, readLength);
            delete[] szSource;
         }
      }
      else
      {
         pCachedBuffer = (UtlString*)mPromptTable.findValue(pKey);
         if (pCachedBuffer == NULL)
         {
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                          "PromptCache::lookup - Retrieval of cached buffer for key: %s failed!!", pathName.data());
         }
         else
         {
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                          "PromptCache::lookup - Found cached buffer for key: %s", pathName.data());
            pBuffer = new UtlString(pCachedBuffer->data(), pCachedBuffer->length());
         }
      }
   }

   return pBuffer;
}


/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


// Copy constructor
PromptCache::PromptCache(const PromptCache& rPromptCache)
   : mCacheGuard(OsMutex::Q_FIFO)
   , mPromptTable()
{
}

// Assignment operator
PromptCache& PromptCache::operator=(const PromptCache& rPromptCache)
{
   if (this == &rPromptCache)            // handle the assignment to self case
      return *this;

   return *this;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Opens the data source
OsStatus PromptCache::open()
{
   UtlString pathName;
   OsStatus status = OS_FAILED;

   mUrl.getPath(pathName);
   
   UtlString language_pref;
      
   //Try the localized version first if 'prefer-language' is supplied
   if (mUrl.getHeaderParameter("prefer-language", language_pref, 0)) {
      //Assume that original file path == 'en'
      if (language_pref.compareTo("en", UtlString::ignoreCase) == 0) {
         language_pref = ""; 
      } else {
         size_t urlExtensionDot = pathName.last('.');
         if (urlExtensionDot != UTL_NOT_FOUND) {
            pathName.insert(urlExtensionDot + 1, language_pref + ".");
         }
      }
   }
   
   mpFile = new OsFile(pathName);

   if (mpFile == NULL)
      return status;

   status = mpFile->open(OsFile::READ_ONLY);
   if (status != OS_SUCCESS)
   {
      delete mpFile;
      
      if (language_pref.length()) {
      
         //Fallback to the original file path
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_WARNING,
                         "PromptCache::open - Unable to open localized prompt file: %s, trying the original one.", pathName.data());
         mUrl.getPath(pathName);
         mpFile = new OsFile(pathName);

         if (mpFile == NULL)
            return status;

         status = mpFile->open(OsFile::READ_ONLY);
      }
      if (status != OS_SUCCESS)
      {
         delete mpFile;

          // If this is a standard prompt file that we could not find, substitute with error prompt.
         unsigned int index = pathName.index(STD_PROMPTS_DIR, UtlString::ignoreCase);
         if (index != UTL_NOT_FOUND)
         {
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                           "PromptCache::open - Unable to open prompt file: %s, substituting with default.", pathName.data());

            // Strip off old .wav file and replace with fallback .wav file
            pathName.remove(index + strlen(STD_PROMPTS_DIR) + 1);
            pathName.append(FALLBACK_PROMPT);
            mpFile = new OsFile(pathName);
            status = mpFile->open(OsFile::READ_ONLY);
            if (status == OS_FAILED)
               delete mpFile;
         }
      }
   }

   return status;
}


// Closes the data source
OsStatus PromptCache::close()
{
   OsStatus status = OS_FAILED;

   if (mpFile != NULL)
   {
      mpFile->close();
      delete mpFile;
      mpFile = NULL;
      status = OS_SUCCESS;
   }
   return status;
}


// Gets the length of the stream (if available)
OsStatus PromptCache::getLength(int& iLength)
{
   OsStatus rc = OS_FAILED;

   unsigned long lLength = 0;
   if (mpFile != NULL)
      rc = mpFile->getLength(lLength);

   iLength = lLength;
   return rc;
}


// Reads iLength bytes of data from the data source and places the data into
// the passed szBuffer buffer.
OsStatus PromptCache::read(char *szBuffer, int iLength, int& iLengthRead)
{
   OsStatus rc = OS_FAILED;

   if (mpFile != NULL)
   {
      rc = mpFile->read(szBuffer, iLength, (unsigned long&) iLengthRead);
   }

   return rc;
}


/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */
