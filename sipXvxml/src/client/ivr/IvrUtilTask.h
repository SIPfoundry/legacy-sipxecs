// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _IvrUtilTask_h_
#define _IvrUtilTask_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "os/OsRWMutex.h"
#include <VXIlog.h>

// DEFINES
#define DEF_LOG_FLUSH_PERIOD 300
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// ENUMS

// TYPEDEFS
// FORWARD DECLARATIONS
class OsTimer;
class OsEvent;

// The IvrUtilTask handles all of the syslog processing
class IvrUtilTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   virtual ~IvrUtilTask();
   //:Destructor

/* ============================ MANIPULATORS ============================== */
     
   static IvrUtilTask* getIvrUtilTask();

   virtual UtlBoolean handleMessage(OsMsg& eventMessage);
   //:Handles all incoming requests

   virtual OsStatus setLogHandle(VXIlogInterface *pLog, char* nfname, int logToStdout);

   virtual OsStatus flushLog();
   //:Stores all of the in-memory log entries to storage

   virtual OsStatus setFlushPeriod(int seconds);


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

  protected:
   int       mLogFlushPeriod;    // How often the log file should be flushed     
   OsTimer*  mpTimer;            // Timer responsible for flushing log
   OsRWMutex mRWMutex;           // Guards log data

   VXIlogInterface *mpLog;
   char            *mpLogFileName;  // File/Path log file
   int              mLogToStdout;

   IvrUtilTask(const UtlString& name = "IvrUtilTask-%d",
               int periodSecs = DEF_LOG_FLUSH_PERIOD,
               int maxRequestQMsgs = DEF_MAX_MSGS);
   //:Default constructor

   OsStatus processFlushLog(OsEvent* pEvent);

   OsStatus processSetFlushPeriod(int seconds);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
   static IvrUtilTask*    spInstance;    // pointer to the single instance of
   //  the IvrUtilTask class
   static OsBSem     sLock;         // semaphore used to ensure that there

   IvrUtilTask(const IvrUtilTask& rIvrUtilTask);
   //:Copy constructor

   IvrUtilTask& operator=(const IvrUtilTask& rhs);
   //:Assignment operator   

#ifdef TEST
   static bool sIsTested;
   //:Set to true after the tests for this class have been executed once

   void test();
   //:Verify assertions for this class

   // Test helper functions
   void testCreators();
   void testManipulators();
   void testAccessors();
   void testInquiry();

#endif /* TEST */
};

/* ============================ INLINE METHODS ============================ */

#endif  /* _IvrUtilTask_h_ */
