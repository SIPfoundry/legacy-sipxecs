//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _CallStateEventWriter_h_
#define _CallStateEventWriter_h_

// SYSTEM INCLUDES
#include "utl/UtlString.h"
#include "os/OsSysLog.h"
#include "os/OsDateTime.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * This CallStateEventWriter writes CSE events out to either a file or a
 * database to the specification doc/cdr/call-state-events.html
 */
class CallStateEventWriter
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   enum CseLogTypes {
      CseLogFile,
      CseLogDatabase
   };
/* ============================ CREATORS ================================== */

   /// Instantiate an event builder and set the observer name for its events
   /*! \param logType - Type of log, either file or database
    * \param logName - file name or database name
    */
   CallStateEventWriter(CseLogTypes logType, const char* logName);

   /// Destructor
   virtual ~CallStateEventWriter();

   /// Write event to the CSE log (file or database)
   /*! \param event (in) - call state event
    * \returns  true if event could be written
    */
   virtual bool writeLog(const char* event) = 0;

   /// Open the log that was specified in the constructor
   virtual bool openLog() = 0;

   /// Close log that was specified in the constructor
   virtual bool closeLog() = 0;

   /// Get writeable state of log
   virtual bool isWriteable() {return mbWriteable;}

   /// Get type of log - file or database
   virtual bool getLogType() {return mLogType;}

   /// Flush log - only functional for file
   virtual void flush();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
   UtlString         mLogName;

   bool              mbWriteable;   // Indicates if openLog was successful

   CseLogTypes       mLogType;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
};

/* ============================ INLINE METHODS ============================ */

#endif    // _CallStateEventWriter_h_
