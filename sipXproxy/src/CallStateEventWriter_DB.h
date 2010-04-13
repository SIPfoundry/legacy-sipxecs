//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _CallStateEventWriter_DB_h_
#define _CallStateEventWriter_DB_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "odbc/OdbcWrapper.h"
#include "CallStateEventWriter.h"

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
class CallStateEventWriter_DB : public CallStateEventWriter
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:
/* ============================ CREATORS ================================== */

   /// Instantiate an event builder and set the observer name for its events
   /*
    * \param logName - database name
    * \param logLocation - database host
    * \param logUserName - database user
    * \param logDriver - database driver name
    * \param logPassword - database password
    * \returns  pointer to an event writer object
    */
    CallStateEventWriter_DB(const char* logName,
                            const char* logLocation=NULL,
                            const char* logUserName=NULL,
                            const char* logDriver=NULL,
                            const char* logPassword=NULL);

   /// Destructor
   virtual ~CallStateEventWriter_DB();

   /// Write event to the CSE log (file or database)
   /*! \param event (in) - call state event
    * \returns  true if event could be written
    */
   bool writeLog(const char* event);

   /// Open the log that was specified in the constructor
   bool openLog();

   /// Close log that was specified in the constructor
   bool closeLog();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
   UtlString         mLogLocation;
   UtlString         mLogUserName;
   UtlString         mLogDriver;
   UtlString         mLogPassword;

   OdbcHandle        mHandle;

   /// no copy constructor or assignment operator
   CallStateEventWriter_DB(const CallStateEventWriter_DB& rCallStateEventWriter_DB);
   CallStateEventWriter_DB operator=(const CallStateEventWriter_DB& rCallStateEventWriter_DB);
};

/* ============================ INLINE METHODS ============================ */

#endif    // _CallStateEventWriter_DB_h_
