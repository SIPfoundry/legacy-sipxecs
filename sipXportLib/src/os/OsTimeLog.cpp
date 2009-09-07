//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsTimeLog.h"
#include "os/OsDateTime.h"
#include "utl/UtlString.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define EVENT_LABEL_MEAN_LENGTH 20

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsTimeLog::OsTimeLog(size_t maxEventCount) :
   mMaxEventCount(maxEventCount),
   mNumEvents(0),
   mpEventTimes(new OsTime[maxEventCount]),
   mpEventLabelOffsets(new size_t[maxEventCount])
{
   assert(maxEventCount > 0);
   mEventLabels.capacity(maxEventCount * EVENT_LABEL_MEAN_LENGTH);
}

// Destructor
OsTimeLog::~OsTimeLog()
{
  delete[] mpEventLabelOffsets;
  delete[] mpEventTimes;
}

/* ============================ MANIPULATORS ============================== */


//: Adds an event to the log for the current time
void OsTimeLog::addEvent(const char* eventName)
{
   if(mNumEvents < mMaxEventCount)
   {
      OsDateTime::getCurTime(mpEventTimes[mNumEvents]);
      mpEventLabelOffsets[mNumEvents] = mEventLabels.length();
      mEventLabels.append(eventName);
   }

   // count this even if it is > mMaxEventCount to detect overflow
   mNumEvents++;
}

//: Dumps the log out using osPrintf
void OsTimeLog::dumpLog() const
{
    UtlString log;

    getLogString(log);
    osPrintf("%s\n", log.data());
}

void OsTimeLog::getLogString(UtlString& log) const
{
   if ( mNumEvents > 0 )
   {
      size_t event;
      OsTime time;
      OsTime deltaTime;
      OsTime tZero;
      OsTime previousTime;
      char   formatted[40];

      log.append("\n           Time          Increment  Name");

      tZero = mpEventTimes[0];

      size_t numRecordedEvents = (mNumEvents < mMaxEventCount ? mNumEvents : mMaxEventCount);
      for(event = 0; event < numRecordedEvents; event++)
      {
         time = mpEventTimes[event] - tZero;;
         sprintf(formatted, "\n  %8d.%.06d", time.seconds(), time.usecs());
         log.append(formatted);

         if(event > 0)
         {
            deltaTime = time - previousTime;
            sprintf(formatted, "  %8d.%.06d  ", deltaTime.seconds(), deltaTime.usecs());
            log.append(formatted);
         }
         else
         {
            log.append("                   ");
         }
         previousTime = time;

         size_t labelEnd = (  event < numRecordedEvents-1  // last event?
                            ? mpEventLabelOffsets[event+1] // no  - use start of next event
                            : mEventLabels.length()        // yes - use end of string
                            );
         log.append(mEventLabels,mpEventLabelOffsets[event],labelEnd-mpEventLabelOffsets[event]);
      }

      if ( mNumEvents > mMaxEventCount )
      {
         sprintf(formatted, "\n  !!! Overflow - %ld events lost !!!", long(mNumEvents - mMaxEventCount));
         log.append(formatted);
      }
   }
   else
   {
      log.append("\n No Events Logged.");
   }
}
