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
#include "os/OsLogger.h"
#include "LogNotifier.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
LogNotifier::LogNotifier() :
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
   _alarmLogger.flush();
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
   std::ostringstream message;
   message << callingHost.data() << ":"
     << alarmData->getComponent().data() << ":"
     << alarmData->getCode().data() << ":"
     << alarmMsg;
   Os::Logger::instance().log_(FAC_ALARM, PRI_ALERT, message.str().c_str());
   _alarmLogger.log_(FAC_ALARM, PRI_ALERT, message.str().c_str());
   return OS_SUCCESS;
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
   if (alarmFile)
   {
      _alarmLogger.initialize<Os::LoggerHelper>(PRI_DEBUG, alarmFile.data(), Os::LoggerHelper::instance());
      Os::Logger::instance().log(FAC_ALARM, PRI_NOTICE,
            "Initializing alarm log file to %s", alarmFile.data());
   }
   return OS_SUCCESS;
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

