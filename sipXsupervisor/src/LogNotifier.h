//
//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _LogNotifier_h_
#define _LogNotifier_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsSysLogTask.h"
#include "NotifierBase.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Logs the alarm in a dedicated alarm file
class LogNotifier : public NotifierBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   ///Default constructor
   LogNotifier();

   ///Destructor
   virtual ~LogNotifier();

/* ============================ MANIPULATORS ============================== */

   ///Assignment operator
   LogNotifier& operator=(const LogNotifier& rhs);

   /// Log the alarm in the dedicated file.  Non-blocking (logging done in another task).
   virtual OsStatus handleAlarm(
         const OsTime alarmTime,           ///< time alarm was reported
         const UtlString& callingHost,     ///< host on which event occurred
         const cAlarmData* alarmData,      ///< pointer to alarmData structure
         const UtlString& alarmParameters  ///< formatted message with parameters
         );

   /// Initialize notifier (including loading parameters from the provided xml element).
   virtual OsStatus init(
         TiXmlElement* element,            ///< pointer to xml config element for this notifier
         TiXmlElement* dummy               ///< currently unused
         );

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsSysLogTask* mpOsSysLogTask;           ///< pointer to the log task
   size_t        mEventCount;              ///< sequential alarm count

   /// Create a task for logging and initialize the alarm log file
   OsStatus initLogfile(UtlString& logFile);

   /// no copy constructor
   LogNotifier(const LogNotifier& nocopy);

   /// Escape newlines, tabs, quotes, etc for saving in log format
   UtlString escape(const UtlString& source);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _LogNotifier_h_

