//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _PromptCache_h_
#define _PromptCache_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "net/Url.h"
#include "os/OsFS.h"
#include "os/OsBSem.h"
#include "os/OsDefs.h"
#include "os/OsServerTask.h"
#include "os/OsStatus.h"
#include "utl/UtlHashMap.h"

// DEFINES
#define STD_PROMPTS_DIR                 "stdprompts"
#define FALLBACK_PROMPT                 "prompt_not_found.wav"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlString;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class PromptCache
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum SourceType
   {
      SourceUrl,
      SourceBuffer,
   } ;

/* ============================ CREATORS ================================== */

   PromptCache();


   virtual
   ~PromptCache();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   UtlString* lookup(Url& url, int flags = 0) ;
     //:Lookup the .WAV file and return a pointer to a memory buffer
     //
     //!param url - Url identifing the requested .WAV file
     //!param flags - Optional flags


/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   PromptCache(const PromptCache& rPromptCache);
     //:Copy constructor

   PromptCache& operator=(const PromptCache& rPromptCache);
     //:Assignment operator

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsStatus open(const UtlString preferredPath, const UtlString defaultPath);
     //:Opens the data source

   OsStatus close();
     //:Closes the data source

   OsStatus getLength(int& iLength);
     //:Gets the length of the stream (if available)

   OsStatus read(char *szBuffer, int iLength, int& iLengthRead);
     //:Reads iLength bytes of data from the data source and places the
     //:data into the passed szBuffer buffer.
     //
     //!param szBuffer - Buffer to place data
     //!param iLength - Max length to read
     //!param iLengthRead - The actual amount of data read.

   OsFile* mpFile;           // Actual File data source
   UtlString mFilePath;      // Actual File path
   OsMutex mCacheGuard;      // Guard closing/touch file from multiple threads
   UtlHashMap mPromptTable;  // Hash table for cached prompts
};

/* ============================ INLINE METHODS ============================ */

#endif  // _PromptCache_h_
