//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#include <stdlib.h>

// APPLICATION INCLUDES

#include "filereader/RefreshingFileReader.h"
#include <os/OsSysLog.h>
#include <os/OsFS.h>
#include <os/OsDateTime.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// Dummy time values.
const unsigned long RefreshingFileReader::sTimeCheckInvalid = 0;
const OsTime RefreshingFileReader::sModTimeInvalid(0, 0);
const OsTime RefreshingFileReader::sModTimeNoFileName(0, 1);
const OsTime RefreshingFileReader::sModTimeFileNotExist(0, 2);

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
RefreshingFileReader::RefreshingFileReader()
{
   // Set mFileName to "", meaning no file is set.
   mFileName.remove(0);
   // Set the state to force refresh() to set up the state and call
   // initialize(), when refresh() is next called.
   reinitialize_state();
   // However, do not call refresh() now, because this is a constructor of
   // a base class of the object the user is really creating, and
   // initialize() is a pure virtual function now.
   // When the user calls refresh() from his application, the state
   // variables will be brought into line with reality and initialize()
   // will be called.
}

// Set the file name, or clear it with NULL or "".
void RefreshingFileReader::setFileName(const UtlString* fileName)
{
   // Set the file name.
   mFileName = fileName ? *fileName : "";
   OsSysLog::add(FAC_DB, PRI_INFO, "RefreshingFileReader:: "
                 "File name is now '%s'", mFileName.data());

   // Set the state to force refresh() to set up the state and call
   // initialize().
   reinitialize_state();
   // Call refresh() to read the file and set the state variables.
   refresh();
}

// Reset the state variables to force the file to be re-read.
void RefreshingFileReader::reinitialize_state()
{
   // Force the modified-time check to run the next time refresh() is called.
   mFileLastModTimeCheck = sTimeCheckInvalid;
   // Force the file to be read and processed.
   mFileModTime = sModTimeInvalid;
}

// Destructor
RefreshingFileReader::~RefreshingFileReader()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Refresh the data structures if the contents of the file has changed.
// This function takes some care to avoid re-reading the file when it has
// not changed since the last call.
// The strategy is to check the modification time of the file,
// and only re-read the file if the modification time has changed since
// the last time we checked it.
// But checking the modification time is relatively slow, and we do not want
// to do it on all calls in a high-usage system.  So we check the clock time
// instead, and if it has been 1 second since the last time we checked
// the modification time of orbits.xml, we check it again.
// Checking the clock time is fast (about 1.6 microseconds on a 2GHz
// processor), and checking the modification time of the file once a
// second is acceptable.
// Any process which changes the file should wait 1 second before reporting
// that it has succeeded, and before doing any further changes to the file.
OsStatus RefreshingFileReader::refresh()
{
   OsStatus ret = OS_SUCCESS;

   // Check to see if 1 second has elapsed since the last time we checked
   // the modification time of the file.
   unsigned long current_time = OsDateTime::getSecsSinceEpoch();
   if (current_time != mFileLastModTimeCheck)
   {
      // It has been.
      mFileLastModTimeCheck = current_time;

      // Set mod_time to be the current modification time of the file,
      // or sModTimeNoFileName if there is no file name,
      // or sModTimeFileNotExist if the file does not exist.
      OsTime mod_time;
      if (mFileName.isNull())
      {
         mod_time = sModTimeNoFileName;
      }
      else
      {
         OsFile file(mFileName);
         OsFileInfo fileInfo;
         if (file.getFileInfo(fileInfo) == OS_SUCCESS) {
            // If the file exists, use its modification time.
            fileInfo.getModifiedTime(mod_time);
         }
         else
         {
            mod_time = sModTimeFileNotExist;
         }
      }

      // Check to see if the modification time of the file is different
      // than the last time we checked.
      if (mod_time != mFileModTime)
      {
         // It is different.
         mFileModTime = mod_time;

         // Read the file.
         ret = initialize();
      }
   }

   return ret;
}
