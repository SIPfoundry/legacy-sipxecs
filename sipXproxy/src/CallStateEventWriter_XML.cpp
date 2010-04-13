//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "CallStateEventWriter_XML.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
static const char* ModuleName =
   "CallStateEventWriter_XML";

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/// Instantiate an event builder and set the observer name for its events
CallStateEventWriter_XML::CallStateEventWriter_XML(const char* logName)
                              : CallStateEventWriter(CallStateEventWriter::CseLogFile, logName),
                                mEventFile(NULL)
{
   OsSysLog::add(FAC_CDR, PRI_DEBUG,
                 "%s::constructor Log type file", ModuleName);
}

/// Destructor
CallStateEventWriter_XML::~CallStateEventWriter_XML()
{
   closeLog();
}

bool CallStateEventWriter_XML::openLog()
{
   bool bRet = false;

   if (!mbWriteable)
   {
      OsPath callStateLogPath(mLogName);
      mEventFile = new OsFile(callStateLogPath);

      OsStatus callStateLogStatus = mEventFile->open(OsFile::CREATE|OsFile::APPEND);
      if (OS_SUCCESS == callStateLogStatus)
      {
         OsSysLog::add(FAC_CDR, PRI_DEBUG,
                       "%s::openLog opened %s", ModuleName, mLogName.data());
         mbWriteable = true;
         bRet = true;
      }
      else
      {
         OsSysLog::add(FAC_CDR, PRI_ERR,
                       "%s::openLog failed (%d) to open Call State Event Log '%s'",
                       ModuleName, callStateLogStatus, mLogName.data());
         if (mEventFile)
         {
            delete mEventFile;
         }
         mEventFile = NULL;
      }
   }
   else
   {
     OsSysLog::add(FAC_CDR, PRI_ERR,
                   "%s::openLog log %s already open", ModuleName, mLogName.data());
   }
   return bRet;
}

bool CallStateEventWriter_XML::closeLog()
{
   bool bRet = false;

   if (mEventFile)
   {
      mEventFile->flush(); // try to get buffered records to the file...
      mEventFile->close();
      delete mEventFile;
      mEventFile = NULL;
   }
   mbWriteable = false;
   bRet = true;

   OsSysLog::add(FAC_CDR, PRI_DEBUG,
                 "%s::closeLog", ModuleName);
   return bRet;
}

bool CallStateEventWriter_XML::writeLog(const char* event)
{
   bool bRet = false;

   if (mbWriteable)
   {
      if (mEventFile)
      {
         // write it to the log file
         size_t written;
         mEventFile->write(event, strlen(event), written);
         OsSysLog::add(FAC_CDR, PRI_DEBUG,
                       "%s::writeLog", ModuleName);
         bRet = true;
      }
   }
   else
   {
      OsSysLog::add(FAC_CDR, PRI_ERR,
                    "%s::writeLog log %s not writeable", ModuleName, mLogName.data());
   }
   return bRet;
}

void CallStateEventWriter_XML::flush()
{
   if (mEventFile)
   {
      mEventFile->flush();
   }
}
