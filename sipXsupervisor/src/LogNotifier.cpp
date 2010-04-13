//
//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsDateTime.h"
#include "os/OsSysLogMsg.h"
#include "os/OsSysLog.h"
#include "LogNotifier.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
LogNotifier::LogNotifier() :
   mpOsSysLogTask(0),
   mEventCount(0)
{
}

// Copy constructor
LogNotifier::LogNotifier(const LogNotifier& rLogNotifier) :
   mEventCount(rLogNotifier.mEventCount)
{
}

// Destructor
LogNotifier::~LogNotifier()
{
   if (mpOsSysLogTask != 0)
   {
      delete mpOsSysLogTask;
      mpOsSysLogTask = 0;
   }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
LogNotifier& LogNotifier::operator=(const LogNotifier& rhs)
{
   if (this == &rhs) // handle the assignment to self case
   return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */

OsStatus LogNotifier::handleAlarm(
      const OsTime alarmTime,
      const UtlString& callingHost,
      const cAlarmData* alarmData,
      const UtlString& alarmMsg)
{
   OsStatus retval = OS_SUCCESS;
   OsSysLog::add(alarmData->getComponent().data(), 0, FAC_ALARM, alarmData->getSeverity(),
         "%s: %s", alarmData->getCode().data(), alarmMsg.data());

   OsDateTime logTime(alarmTime);
   UtlString   strTime ;
   logTime.getIsoTimeStringZus(strTime);

   char tempMsg[500 + alarmMsg.length()];
   snprintf(tempMsg, sizeof(tempMsg), "\"%s\":%zd:%s:%s:%s:%s::%s:\"%s\"",
             strTime.data(),
             ++mEventCount,
             OsSysLog::sFacilityNames[FAC_ALARM],
             OsSysLog::priorityName(alarmData->getSeverity()),
             callingHost.data(),
             alarmData->getComponent().data(),
             alarmData->getCode().data(),
             escape(alarmMsg).data());
   tempMsg[sizeof(tempMsg)-2]='"';
   tempMsg[sizeof(tempMsg)-1]=0;

   char* szPtr = strdup(tempMsg);
   OsSysLogMsg msg(OsSysLogMsg::LOG, szPtr);
   mpOsSysLogTask->postMessage(msg);

   return retval;
}

OsStatus LogNotifier::init(TiXmlElement* element, TiXmlElement* dummy)
{
   UtlString alarmFile;
   textContentShallow(alarmFile, element->FirstChildElement("file"));
   initLogfile(alarmFile);
   return OS_SUCCESS;
}

OsStatus LogNotifier::initLogfile(UtlString &alarmFile)
{
   OsStatus retval = OS_FAILED;

   if (mpOsSysLogTask == 0)
   {
      mpOsSysLogTask = new OsSysLogTask(0, OsSysLog::OPT_NONE, "alarmLog");
   }
   if (!mpOsSysLogTask)
   {
      OsSysLog::add(FAC_ALARM, PRI_CRIT, "Failed to create alarm log task");
      return retval;
   }

   retval = OS_SUCCESS;
   if (alarmFile)
   {
      OsSysLogMsg msgSetFile(OsSysLogMsg::SET_FILE, strdup(alarmFile.data()));
      mpOsSysLogTask->postMessage(msgSetFile);
      OsSysLog::add(FAC_ALARM, PRI_NOTICE,
            "Initializing alarm log file to %s", alarmFile.data());
   }

   return retval;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
UtlString LogNotifier::escape(const UtlString& source)
{
   UtlString    results;
   const char* pStart = source.data() ;
   const char* pTraverse = pStart ;
   const char* pLast = pStart ;

   results.capacity(source.length() + 100);
   while (*pTraverse)
   {
      switch (*pTraverse)
      {
         case '\\':
            // Copy old data
            if (pLast < pTraverse)
            {
               results.append(pLast, pTraverse-pLast);
            }
            pLast = pTraverse + 1 ;

            // Add escaped Text
            results.append("\\\\") ;
            break ;
         case '\r':
            // Copy old data
            if (pLast < pTraverse)
            {
               results.append(pLast, pTraverse-pLast);
            }
            pLast = pTraverse + 1 ;

            // Add escaped Text
            results.append("\\r") ;
            break ;
         case '\n':
            // Copy old data
            if (pLast < pTraverse)
            {
               results.append(pLast, pTraverse-pLast);
            }
            pLast = pTraverse + 1 ;

            // Add escaped Text
            results.append("\\n") ;
            break ;
         case '\"':
            // Copy old data
            if (pLast < pTraverse)
            {
               results.append(pLast, pTraverse-pLast);
            }
            pLast = pTraverse + 1 ;

            // Add escaped Text
            results.append("\\\"") ;
            break ;
         default:
            break ;
      }
      pTraverse++ ;
   }

   // if nothing to escape, short-circuit
   if (pLast == pStart)
   {
      return source ;
   }
   else if (pLast < pTraverse)
   {
      results.append(pLast, pTraverse-pLast);
   }

   return results ;
}

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

