//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _CallStateEventWriter_XML_h_
#define _CallStateEventWriter_XML_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsFS.h"
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
class CallStateEventWriter_XML : public CallStateEventWriter
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:
/* ============================ CREATORS ================================== */

   /// Instantiate an event builder and set the observer name for its events
   /*! \param logName - file name
    * \returns  pointer to an event writer object
    */
   CallStateEventWriter_XML(const char* logName);

   /// Destructor
   virtual ~CallStateEventWriter_XML();

   /// Write event to the CSE log (file or database)
   /*! \param event (in) - call state event
    * \returns  true if event could be written
    */
   bool writeLog(const char* event);

   /// Open the log that was specified in the constructor
   bool openLog();

   /// Close log that was specified in the constructor
   bool closeLog();

   /// Flush log - only functional for file
   void flush();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
   OsFile*           mEventFile;

   /// no copy constructor or assignment operator
   CallStateEventWriter_XML(const CallStateEventWriter_XML& rCallStateEventWriter_XML);
   CallStateEventWriter_XML operator=(const CallStateEventWriter_XML& rCallStateEventWriter_XML);
};

/* ============================ INLINE METHODS ============================ */

#endif    // _CallStateEventWriter_MXL_h_
