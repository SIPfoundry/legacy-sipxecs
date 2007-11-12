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
   UtlString urlType;
   UtlString urlPath, defaultPath, preferredPath, usedPath;
   size_t index;
   UtlBoolean cacheEntry;
   UtlContainable* pKey = NULL;
   UtlContainable* pCachedKey;
   UtlString* pBuffer = NULL;
   UtlString* pCachedBuffer;

   url.getUrlType(urlType);
   if (urlType.compareTo("file", UtlString::ignoreCase) == 0)
   {
      UtlString preferredLanguage;
      
      // The path from the URL does not contain a language ID. Use the
      // specified path as the default path and insert the language ID
      // from the "prefer-language" parameter (if provided) to get the
      // preferred path      
      url.getHeaderParameter("prefer-language", preferredLanguage, 0) ;
      url.removeHeaderParameter("prefer-language") ;
       
      url.getPath(urlPath, TRUE);
      defaultPath = urlPath;
      index = urlPath.index(STD_PROMPTS_DIR, UtlString::ignoreCase);
      if (index == UTL_NOT_FOUND)
      {
         cacheEntry = FALSE;
      }
      else
      {
         cacheEntry = TRUE;
         index += strlen(STD_PROMPTS_DIR);
         if (urlPath(index) == '_')
         {
            // The path from the URL already contains a language ID. Use the
            // specified path as the preferred path and remove the language
            // ID to get the default path
            preferredPath = urlPath;
            defaultPath.remove(index, defaultPath.index(OsPath::separator, index) - index);
         }
         else
         {
            if (preferredLanguage.length())
            {
                preferredPath = urlPath;
                preferredPath.insert(index, "_" + preferredLanguage);
            }
         }
         
         // First try to find the preferred and then the default prompt
         if (preferredPath.length())
         {
            usedPath = preferredPath;
            pKey = mPromptTable.find(&usedPath);
         }
         else
         {
            usedPath = defaultPath;
            pKey = mPromptTable.find(&usedPath);
         }
      }

      if (pKey == NULL)
      {
         if (cacheEntry == TRUE)
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                          "PromptCache::lookup - Did not find key: %s in prompt cache", usedPath.data());
         else
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                          "PromptCache::lookup - Buffering non-cached prompt: %s", usedPath.data());

         mpFile = NULL;

         status = open(preferredPath, defaultPath);
         if (status == OS_SUCCESS)
         {
            usedPath = mFilePath;
         }
         else
         {
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                          "PromptCache::lookup - Unable to open prompt file: preferred=%s, default=%s",
                          preferredPath.data() ? preferredPath.data() : "NULL",
                          defaultPath.data()? defaultPath.data() : "NULL");
            if (index == UTL_NOT_FOUND)
            {
               return pBuffer;
            }
            
            // This is a standard prompt, try to substitute the requested file
            // name with the error prompt
            defaultPath.remove(index + 1);
            defaultPath.append(FALLBACK_PROMPT);
            status = open("", defaultPath);
            if (status != OS_SUCCESS)
            {                
               return pBuffer;
            }
            usedPath = defaultPath;
         }
         status = getLength(length);
         if (status != OS_SUCCESS)
         {
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                          "PromptCache::lookup - Unable to determine lenght of prompt file: %s", usedPath.data());
            return pBuffer;
         }
         szSource = new char[length];
         status = read(szSource, length, readLength);
         if ((status != OS_SUCCESS) || (length != readLength))
         {
            delete[] szSource;
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                          "PromptCache::lookup - Failure on read of prompt file: %s", usedPath.data());
            return pBuffer;
         }
         close();

         if (cacheEntry == TRUE)
         {
            // Now add the prompt to the table
            pCachedKey = new UtlString(usedPath);
            pCachedBuffer = new UtlString(szSource, readLength);
            delete[] szSource;
            pKey = mPromptTable.insertKeyAndValue(pCachedKey, pCachedBuffer);
            if (pKey == NULL)
            {
               delete pCachedKey;
               delete pCachedBuffer;
               OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                             "PromptCache::lookup - Insertion of key: %s failed!!", usedPath.data());
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
                          "PromptCache::lookup - Retrieval of cached buffer for key: %s failed!!", usedPath.data());
         }
         else
         {
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                          "PromptCache::lookup - Found cached buffer for key: %s", usedPath.data());
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
OsStatus PromptCache::open(const UtlString preferredPath, const UtlString defaultPath)
{
   OsStatus status = OS_FAILED;
   // First try to open the prompt file from the preferred path (i.e., for the
   // preferred or specifically requested language)
   mFilePath.remove(0);
   if (preferredPath.length())
   {
      mpFile = new OsFile(preferredPath);
      status = mpFile->open(OsFile::READ_ONLY);
      if (status == OS_SUCCESS)
      {
         mFilePath = preferredPath;
         return status;
      }
      delete mpFile;
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                    "PromptCache::open - %s not found - trying default language", preferredPath.data());
   }
   // Next try to open the prompt file from the default path (i.e., from the
   // stdprompts directory (that matches the default language) or from the URL
   // specified location if this is not a standard prompt
   if (status != OS_SUCCESS)
   {      
      mpFile = new OsFile(defaultPath);
      status = mpFile->open(OsFile::READ_ONLY);
      if (status == OS_SUCCESS)
      {
         mFilePath = defaultPath;
         return status;
      }
      delete mpFile;
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                    "PromptCache::open - %s not found - trying English", defaultPath.data());
   }
   // Finally, if this is a standard prompt, try to open the prompt file from
   // the English path (which is always included in the build).
   if (status != OS_SUCCESS)
   {
      size_t index = defaultPath.index(STD_PROMPTS_DIR, UtlString::ignoreCase);
      if (index != UTL_NOT_FOUND)
      {
         index += strlen(STD_PROMPTS_DIR);
         UtlString path = defaultPath;
         path.insert(index, "_en");
         mpFile = new OsFile(path);
         status = mpFile->open(OsFile::READ_ONLY);
         if (status == OS_SUCCESS)
         {
            mFilePath = defaultPath;
            return status;
         }
         delete mpFile;
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                       "PromptCache::open - %s not found", path.data());
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
