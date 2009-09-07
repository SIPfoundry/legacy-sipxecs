//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsSysLogMsg_h_
#define _OsSysLogMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "os/OsMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Base class for message queue buffers

class OsSysLogMsg : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum SubMsgTypes
   {
      LOG = 0,          // Log Message
      ENABLE_CONSOLE,   // Enable console output
      DISABLE_CONSOLE,  // Disable console output
      HEAD,             // Head the log
      TAIL,             // Tail the log
      SET_FILE,         // Set the target output file
      ADD_SOCKET,       // Add a target output socket
      SET_FLUSH_PERIOD, // Set the flush period
      FLUSH_LOG,        // Flush the log (write to disk)
      SET_CALLBACK,     // Set the callback function
   } ;
  //: Defines the various SysLog Msg Subtypes
  //
  //
  //!enumcode: LOG - Log Message
  //!enumcode: ENABLE_CONSOLE - Enable console output
  //!enumcode: DISABLE_CONSOLE - Disable console output
  //!enumcode: HEAD - Head the log
  //!enumcode: TAIL - Tail the log
  //!enumcode: SET_FILE - Set the target output file
  //!enumcode: ADD_SOCKET - Add a target output socket
  //!enumcode: SET_FLUSH_PERIOD - Set the flush period
  //!enumcode: FLUSH_LOG - Flush the log (write to disk)


/* ============================ CREATORS ================================== */

   OsSysLogMsg(const unsigned char msgSubType, const void* pData = NULL) ;
     //:Constructor

   OsSysLogMsg(const OsSysLogMsg& rOsSysLogMsg);
     //:Copy constructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)

   virtual
      ~OsSysLogMsg();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsSysLogMsg& operator=(const OsSysLogMsg& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

   const void* getData() const ;
     //:Get data associated with this message

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   const void* mpData;

};

/* ============================ INLINE METHODS ============================ */

#endif  /* _OsSysLogMsg_h_ */
