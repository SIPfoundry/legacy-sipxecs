//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _OsSysLog_h_
#define _OsSysLog_h_

// SYSTEM INCLUDES
#include <stdarg.h>
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "os/OsSysLogFacilities.h"
#include "os/OsSocket.h"
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsTime.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// ENUMS

// TYPEDEFS

/// The priority of log messages ordered from least to most severe
typedef enum tagOsSysLogPriority
{
   PRI_DEBUG,     /**< Developer message needed only when debugging code.
                   *   May include recording of such things as entry and exit from methods
                   *   or internal branch tracking.
                   *   Should never be required by end users.
                   */
   PRI_INFO,      /**< Informational message used to trace system inputs and actions.
                   *   Significant actions in the execution of some activity; for example, the
                   *   receipt of a message or an important decision in its disposition.
                   *   This level should be sufficient for an administrator or support person
                   *   to determine what occured in the system when debugging a configuration
                   *   or interoperability problem in the field.
                   */
   PRI_NOTICE,    /**< Normal but significant events.
                   *   Events that are expected but provide important context, such as service
                   *   restarts and reloading configuration files.
                   *   This is the default logging level, and generally logging should
                   *   always include at least these messages.
                   */
   PRI_WARNING,   /**< Conditions that imply that some failure is possible but not certain.
                   *   Generally, external inputs that are not as expected and possibly
                   *   invalid.  Especially useful in low level routines that are going to
                   *   return an error that may be recoverable by the caller.
                   */
   PRI_ERR,       /**< An unexpected condition likely to cause an end-user visibile failure.
                   *   This level should be used whenever an error response is being sent
                   *   outside the system to provide a record of the internal data that
                   *   are important to understanding it.
                   */
   PRI_CRIT,      /**< Endangers service operation beyond the current operation.
                   *   MUST be logged prior to any 'assert', or when exiting for any abnormal
                   *   reason.
                   */
   PRI_ALERT,     /**< Fault to be communicated to operations.
                   *   Should be replaced by usage of the new Alarm subsystem.
                   */
   PRI_EMERG,     /**< System is unusable.
                   */

   // NOTE: If adding/removing priorities, you MUST also adjust static name
   // initializer in OsSysLog.cpp

   SYSLOG_NUM_PRIORITIES ///< MUST BE LAST

} OsSysLogPriority;

// Signature for a callback function that takes three parameters: priority,
// source Id of the generating module, and the message itself.
typedef void (*OsSysLogCallback)(const char* szPriority,
                                 const char* szSource,
                                 const char* szMsg);


// FORWARD DECLARATIONS
class OsSysLogTask;

//:The OsSysLog provides a system wide logger and alternative to printf or
//:osPrintf for error/informational/debugging purposes.
//
// OsSysLog is designed to work on both the xpressa phone where fixed size
// circular buffer is needed along with server-type environments where a
// simple log is required.
//
// Daemon Setup
// ============
//
// Setting up a rolling log:
//
// To setup a rolling log, you need call the static initialize method
// followed by a call to setOutputFile.  For example:
//
//     OsSysLog::initialize(1024, "myXpressa3422") ;
//
// This tells OsSysLog to create log of up to 1024 entries and to use the
// string "myXpressa3422" to uniquely identify this process.
//
//     OsSysLog::setOutputFile(90, "/flash0/syslog.txt") ;
//
// This tell OsSysLog to save the logged data to disk every 90 seconds into
// the file "/flash0/syslog.txt".
//
// Setting up a server-style unbounded log
//
// Like setting up a rolling log, to setup a unbounded log, you will need to
// call the static initialize method followed by a call to setOutputFile.  For
// example:
//
//          OsSysLog::initialize(0, "ServerXYZ") ;
//
// This instructs OsSysLog not to create an in-memory buffer and names the
// process "ServerXYZ".  It is important that you specify "0" as the in-memory
// buffer size; otherwise, your log may limited in size.
//
//     OsSysLog::setOutputFile(0, "/usr/var/myserverlog") ;
//
// This last statement instructs the server to write data immediately to the
// log located at "/usr/var/myserverlog".
//
// Usage
// =====
//
// Ideally, you should use the syslog method build on top of OsTask.  This will
// make sure that that task id, and task name are included as part of the
// log entry.
//
//    syslog(FAC_HTTP, PRI_ERR, "The url '%s' is invalid", szUrl) ;
//
// Alternatively, you can also call static methods on OsSysLog.  I would
// recommend using a OsTask if possible, however, if you are unable to
// reference an OsTask:
//
//     OsSysLog::add(FAC_HTTP, PRI_ERR, "The url '%s' is invalid", szUrl) ;
//
// or
//
//     OsSysLog::add("MyTaskName", taskId, FAC_HTTP, PRI_ERR,
//             "The url '%s' is invalid", szUrl) ;
//
//
// The "FAC_HTTP" defines the facility or functional related to the log
// message.  This helps users and reviewer filter and categorize different
// log messages.  Please see OsSysLogFacilities.h for a list of available
// facilities.  Please also feel free to add additional facilities.
//
// The "PRI_ERR" defines the priority of the log message.  Please see the
// description above for the different priority levels.
//
class OsSysLog
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /// translate an OsSysLogPriority enum to a string constant
   static const char* priorityName(OsSysLogPriority priority);

   /// translate a string constant to an OsSysLogPriority enum
   static bool priority(const char* priorityName, OsSysLogPriority& priority);
   ///< @returns true iff the priorityName is a valid value (case insensitive)

   static const char* sFacilityNames[] ;
     //:List of Facility Names orders in the same order as OsSysLogFacility.

   enum OsSysLogOptions
   {
        OPT_NONE           = 0x00000000,     // No Options
        OPT_SHARED_LOGFILE = 0x00000001      // Assume a shared log file

     // NOTE: Options are designed to be used as bitmasks (and ORed together).
     //       Make sure new additions are defined as power of twos (0x01,
     //       0x02, 0x04, 0x08, 0x10, 0x10, 0x20, 0x40, ...)
   } ;
  //:Defines the set of possible options passed to the initialize method.
  //
  //!enumcode: OPT_NONE - No Extra Options
  //
  //!enumcode: OPT_SHARED_LOGFILE - Places the OsSysLog in a mode where
  //           multiple loggers (processes) to write to the same log file.
  //           This option should only be set if required to avoid
  //           performance hits.

/* ============================ CREATORS ================================== */


/* ============================ MANIPULATORS ============================== */

   static OsStatus initialize(const int maxInMemoryLogEntries,
                              const char* processId,
                              const int options = OPT_NONE);
     //:Initialize the sys log facility and the in-memory circular buffer
     //:size.
     // This must be called prior to invoking any of the set, add, or enable
     // manipulators defined in this class.
     //
     //!param maxInMemoryLogEntries - Defines the maximum number of in-memory
     //       log entries stored at any one point.  This setting may also
     //       impact file based output logging.  If the minFlushPeriod is
     //       greater than 0, the output file will never grow larger than
     //       this value.
     //
     //!param processId - This parameter represents an arbitrary identifier
     //       that uniquely differentiates this process from other processes.
     //       Examples could range from a phones mac address or server's name.
     //
     //!param options - This parameter defines instance specific options.  See
     //       the OsSysLogOptions enum defined above for valid settings.

   static OsStatus setOutputFile(const int minFlushPeriod,
                                 const char* logfile);
     //:Set an output file to collect logging results.
     //
     //!param minFlushPeriod - Defines the minimum amount of that that must
     //       pass before a log entries are flushed to the log file in
     //       seconds.  Specifying a value of less than or equal to zero will
     //       force immediate writes.  A positive value will limit only allow
     //       log writes periodically and will limit the log file size to the
     //       maxInMemoryLogEntries defined in the call to
     //       OsSysLog::initialize(maxInMemoryLogEntries).
     //!param logfile - The full qualified path the the target log file.

   static OsStatus setCallbackFunction(OsSysLogCallback pCallback);
     //:Set a callback function to collect logging results.
     //
     //!param pCallback - Pointer to a callback function that takes three
     //                   strings as parameters: Logging priority, source
     //                   of log entry, and the entry itself

   static OsStatus addOutputSocket(const char* remoteHost);
     //:Add an output socket to the list of output targets.
     // Log events are fired to output sockets on a low-priority thread and
     // are not subject to minFlushPeriods such as file output targets.

   static OsStatus enableConsoleOutput(const UtlBoolean enable);
     //:Enables or Disable console output for logging events.
     // Log events are displayed on a low-priority thread and are not subject
     // to minFlushPeriods as file output targets are.
     //
     //!param enable - Specify TRUE to enable console logging or FALSE to
     //       disable it.  The default is disabled.

   static OsStatus setLoggingPriority(const OsSysLogPriority priority);
     //:Sets the priority logging level.
     // All log events of lesser priority are disgarded immediately.  These
     // events are not delievered to sockets, file logs, or the console.
     //
     // Resetting the global logging priority will clear any facility
     // priorities.

     //!param priority - Defines the minimum priority level of log events that
     //       should be logged.

   static OsStatus setLoggingPriorityForFacility(const OsSysLogFacility facility,
                                                 const OsSysLogPriority priority);
     //:Set the priority logging level for a specific facility.
     // A facilities's logging level overrides the global logging level. This
     // allow developers to dynamically increase verbosity for a specific
     // facility.
     //
     // Resetting the global logging priority will clear any facility
     // priorities.
     //
     //!param facility - Defines the facility whose logging events should be
     //       either promoted or demoted.
     //!param priority - Defines the minimum priority level of log events that
     //       should be logged.

   static OsStatus add(const char*            taskName,
                       const pthread_t        taskId,
                       const OsSysLogFacility facility,
                       const OsSysLogPriority priority,
                       const char*            format,
                                              ...
                       )
#ifdef __GNUC__
       // with the -Wformat switch, this enables format string checking
       __attribute__ ((format (printf, 5, 6)))
#endif
       ;
     //:Adds an event to the sys log.  If the sys log has not been
     //:initialized, the message is printed to the console.
     //
     //!param: taskName - The name of the task if available.
     //!param: taskId - The TaskID of the task if available.
     //!param: facility - Defines the facility responsible for adding the
     //        event.  See the OsSysLogFacility for more information.
     //!param: priority - Defines the priority of the event.  See
     //        OsSysLogPriority for more information.

   static OsStatus add(const OsSysLogFacility facility,
                       const OsSysLogPriority priority,
                       const char*            format,
                                              ...)
#ifdef __GNUC__
       // with the -Wformat switch, this enables format string checking
       __attribute__ ((format (printf, 3, 4)))
#endif
       ;
     //:Adds an event to the sys log.  If the sys log has not been
     //:initialized, the message is printed to the console.
     //
     //!param: facility - Defines the facility responsible for adding the
     //        event.  See the OsSysLogFacility for more information.
     //!param: priority - Defines the priority of the event.  See
     //        OsSysLogPriority for more information.



   static OsStatus vadd(const char*            taskName,
                        const pthread_t        taskId,
                        const OsSysLogFacility facility,
                        const OsSysLogPriority priority,
                        const char*            format,
                        va_list&               ap)
#ifdef __GNUC__
       // with the -Wformat switch, this enables format string checking
       __attribute__ ((format (printf, 5, 0)))
#endif
       ;

     //:Adds an event to the sys log.  If the sys log has not been
     //:initialized, the message is printed to the console.
     //
     //!param: taskName - The name of the task if available.
     //!param: taskId - The TaskID of the task if available.
     //!param: facility - Defines the facility responsible for adding the
     //        event.  See the OsSysLogFacility for more information.
     //!param: priority - Defines the priority of the event.  See
     //        OsSysLogPriority for more information.

   static OsStatus clearInMemoryLog() ;
     //:Purges all entries from the in memory log.
     // If configured to using a flush period (setOutputFile), then entries
     // entries will also be flushed from the file on next flush.

   static OsStatus flush(const OsTime& rTimeout = OsTime::OS_INFINITY) ;
     //:Flushes the in-memory circular buffer log to disk or an unbounded
     //:log file.

   static void initSysLog(const OsSysLogFacility facility,
           const char* processID,
           const char* logname,
           const char* loglevel);
   // Initialize the OsSysLog priority

/* ============================ ACCESSORS ================================= */

   static OsStatus getMaxInMemoryLogEntries(int& maxEntries) ;
     //:Obtains the maximum number of in-memory log entries.
     // This value is specified as part of initialize() process and cannot
     // be changed at runtime.
     //!param maxEntries - The maximum number of in-memory log entries

   static OsStatus tailMemoryLog(const int numEntries) ;
     //:Displays the last numEntries log entries available within the
     //:in-memory log buffer.
     //
     //!param: numEntries - The number of log entries display starting from
     //        the end (or tail) of the log.  Specify a value of <= 0 to
     //        display the entire in-memory log contents.

   static OsStatus headMemoryLog(const int numEntries) ;
     //:Displays the first numEntries log entries available within the
     //:in-memory log buffer.
     //
     //!param: numEntries - The number of log entries display starting from
     //        the beginning (or head) of the log. Specify a value of <= 0
     //        to display the entire in-memory log contents.

   static OsStatus getLogEntries(  const int maxEntries,
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

   static OsStatus parseLogString(const char *szSource,
                                  UtlString& date,
                                  UtlString& eventCount,
                                  UtlString& facility,
                                  UtlString& priority,
                                  UtlString& hostname,
                                  UtlString& taskname,
                                  UtlString& taskId,
                                  UtlString& processId,
                                  UtlString& content) ;
   //:Parses a log string into its parts.

/* ============================ INQUIRY =================================== */

   static OsSysLogPriority getLoggingPriority() ;
     //:Return the logging level priority.
     // All log events added with a lower priority are discarded.

   static OsSysLogPriority getLoggingPriorityForFacility(const OsSysLogFacility facility) ;
     //:Return the logging level for a specific facility.
     // A facilities's logging level overrides the global logging level. This
     // allow developers to dynamically increase verbosity for a specific
     // facility.

   static UtlBoolean willLog(OsSysLogFacility facility, OsSysLogPriority priority) ;
     //:Determine if a message of a given facility/priority will be logged or
     //:discarded

   static int getNumFacilities();
     //:Return the number of available facilities.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   static const char* sPriorityNames[] ;
   //:List of Priority Names orders in the same order as OsSysLogPriority.

   static OsSysLogTask* spOsSysLogTask;

   static unsigned long sEventCount;
   static OsSysLogPriority* spPriorities;
   static OsSysLogPriority sLoggingPriority;
   static UtlString sProcessId;
   static UtlString sHostname;
   static UtlBoolean bPrioritiesInitialized;

   OsSysLog(const OsSysLog& rOsSysLog);
     //:Copy constructor

   OsSysLog& operator=(const OsSysLog& rhs);
     //:Assignment operator

   OsSysLog();
     //:Default constructor

   virtual ~OsSysLog();
     //:Destructor

   static UtlString escape(const UtlString& source) ;
     //:Escapes Quotes and CrLfs within a string

   static UtlString unescape(const UtlString& source) ;
     //:Unescapes previously escaped Quotes and CrLfs

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /** Protect against simple apps that do not call setLoggingPriority */
   static void initializePriorities() ;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsSysLog_h_
