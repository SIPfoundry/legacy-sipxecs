//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsTimeLog_h_
#define _OsTimeLog_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "os/OsTime.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlString;

/// Record a series of named timestamps - useful for measuring performance
/**
 * Use this class to record times for a named sequence of events, and then
 * get a formatted report of the times for those events.
 * @code
 * OsTimeLog log;
 * UtlString results;
 *
 *  results.append("Entries");
 *
 *  log.addEvent("event 1");
 *  log.addEvent("event 2");
 *  log.addEvent("event 3");
 *  log.addEvent("event 4");
 *  log.addEvent("event 5");
 *
 *  log.getLogString(results);
 * @endcode
 * Produces a log result string like:
 * <pre>
 * Entries
 *     Time            Increment  Name
 *     0.000000                   event 1
 *     0.000005         0.000005  event 2
 *     0.000008         0.000003  event 3
 *     0.000015         0.000007  event 4
 *     0.000018         0.000003  event 5
 * </pre>
 *
 * To minimize the performance impact of the recording itself, make sure
 * that you initialize the maximum number of events you expect to record
 * and keep the average label length <= EVENT_LABEL_MEAN_LENGTH (20).
 * If the labels exceed this, the UtlString where they are recorded may
 * need to be reallocated and copied.
 */
class OsTimeLog
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /// Constructor
   OsTimeLog(size_t maxEventCount = 20 ///< set conservatively to prevent malloc during addEvent
             );

   virtual ~OsTimeLog();

/* ============================ MANIPULATORS ============================== */

   /// Adds an event to the log for the current time.
   void addEvent(const char* eventName);

   /// Dumps the log using osPrintf.
   void dumpLog() const;

/* ============================ ACCESSORS ================================= */

   /// Get formatted rows of named events with elapsed and incremental times
   void getLogString(UtlString& logString) const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    size_t    mMaxEventCount;
    size_t    mNumEvents;

    OsTime*   mpEventTimes;
    size_t*   mpEventLabelOffsets;
    UtlString mEventLabels;

    // disable assignment and copy operators
    OsTimeLog& operator=(const OsTimeLog& rhs);
    OsTimeLog(const OsTimeLog& rOsTimeLog);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsTimeLog_h_
