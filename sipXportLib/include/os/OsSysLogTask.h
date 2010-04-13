//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsSysLogTask_h_
#define _OsSysLogTask_h_

// SYSTEM INCLUDES
#include <stdarg.h>
// APPLICATION INCLUDES
#include "os/OsSocket.h"
#include "os/OsSysLog.h"
#include "os/OsSysLogFacilities.h"
#include "os/OsServerTask.h"
#include "os/OsRWMutex.h"

// DEFINES
#define MAX_SOCKET_TARGETS          4       // Max number of output sockets
#define MAX_REOPEN_LOG_DELAY_SEC    15      // Close/Reopen log after 15
                                            // seconds.  This is actually
                                            // performed on the next message,
                                            // so it will likely be larger-
                                            // perhaps much.
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// ENUMS

// TYPEDEFS
// FORWARD DECLARATIONS
class OsTimer;
class OsSocket;
class OsEvent;

// The OsSysLogTask handles all of the syslog processing
class OsSysLogTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsSysLogTask(const int maxInMemoryLogEntries = 0,
                const int options = OsSysLog::OPT_NONE,
                const char *taskName = "syslog");
     //:Default constructor

   virtual ~OsSysLogTask();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean handleMessage(OsMsg& eventMessage);
     //:Handles all incoming requests

   OsStatus clear();
     //:Clear all of the log entries

   OsStatus flush(const OsTime& rTimeout = OsTime::OS_INFINITY);
     //:Stores all of the in-memory log entries to storage

/* ============================ ACCESSORS ================================= */

   OsStatus getMaxEntries(int& maxEntries);
     //:Obtains the maximum number of in-memory log entries.
     //!param maxEntries - The maximum number of in-memory log entries

   OsStatus getLogEntries(  const int maxEntries,
                            char* entries[],
                            int& actualEntries) ;
     //:Gets the last <maxEntries> log entries ordered with the most recent
     //:entry first.
     //!param: maxEntries - The maximum number of entries to fetch.
     //!param: entries - Array of char* large enough to accommodate
     //        maxEntries entries.  It is the caller responsibility to free
     //        all of the char* pointers.
     //!param: actualEntries - The actual number of entries returned.  This
     //        will always be less than or equal to maxEntries.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

protected:
   UtlBoolean mConsoleEnabled;    // Is console output enabled?

   char**    mpRingBuffer;       // In memory ring buffer
   int       mLogCount;          // Number of entries in ring buffer
   int       mRingBufferLength;  // Length of ring buffer
   int       mRingBufferNext;    // Next available ring buffer

   FILE*     mpUnboundedLog;     // Unbounded Log file (if used)
   UtlString  mUnboundedLogFile;  // File/Path of Unbounded log file

   UtlString  mBoundedLogFile;    // Name/Path of bounded log file
   int       mFlushPeriod;       // How often the log file should be flushed
   UtlBoolean mLogChanged;        // Has the log changed/need flushing?
   OsTimer*  mpTimer;            // Timer responsible for flushing log
   OsSocket* mpSockets[MAX_SOCKET_TARGETS] ; // Output sockets
   OsSysLogCallback mpCallback;  // Callback function
   OsRWMutex mRWMutex;           // Guards log data
   OsTime    mpLastReopen ;      // Time of last reopen (unbounded only)
   int       mOptions ;          // Instance-specific options


   OsStatus processAdd(char* pEntry);
     //:Handlers adding a new log entry
   OsStatus processAddTail(char* pEntry);
     //:Handlers adding a log entry to the "tail" of the list
   OsStatus processConsoleEnable(const UtlBoolean enable);
     //:Handles enabling/disabling console output
   OsStatus processHeadCommand(const int iEntries);
     //:Handles displaying log contents starting from "head" of log
   OsStatus processTailCommand(const int iEntries);
     //:Handles displaying log contents starting from "tail" of log
   OsStatus processSetFile(const char* szFile);
     //:Process setting the target output file
   OsStatus processAddSocket(const char* remoteHost);
     //:Process adding a target output socket
   OsStatus processSetFlushPeriod(const int iPeriod);
     //:Process setting the flush period
   OsStatus processFlushLog(OsEvent* pEvent);
     //:Process flushing the actual log.
   OsStatus processSetCallback(OsSysLogCallback pCallback);
     //:Process setting a callback function

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsSysLogTask(const OsSysLogTask& rOsSysLogTask);
     //:Copy constructor

   OsSysLogTask& operator=(const OsSysLogTask& rhs);
     //:Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  /* _OsSysLogTask_h_ */
