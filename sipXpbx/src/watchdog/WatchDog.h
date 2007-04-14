// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _WatchDog_h_
#define _WatchDog_h_

// SYSTEM INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsServerTask.h"

// APPLICATION INCLUDES
#include "processcgi/processXMLCommon.h"
#include "MonitoredProcess.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class FailureReporterBase;
class OsQueuedEvent;
class OsTimer;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class WatchDog : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   WatchDog(int nWatchInterval,MonitoredProcess **processList,int processCount);
   //: Default constructor (takes time in secs, process list, process count)

   WatchDog(const WatchDog& rWatchDog);
     //:Copy constructor

   virtual
   ~WatchDog();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   WatchDog& operator=(const WatchDog& rhs);
     //:Assignment operator
   
   UtlBoolean handleMessage(OsMsg &rMsg);
    //: Handles our timer message
/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsTimer*            mpWatchDogTimer; // Timer evaluating time to check processes
   OsQueuedEvent*      mpWatchDogEvent;  // Event marking time to check processes

   OsTimer*            mpWatchDogUserChangeTimer; // Timer evaluating time to check if user modified processes
   OsQueuedEvent*      mpWatchDogUserChangeEvent;  // Event marking time to check if user modified processes

   MonitoredProcess**  mpProcessList;    //list of processes to monitor
   int mnProcessCount;   //How many are there to monitor
};

/* ============================ INLINE METHODS ============================ */

#endif  // _WatchDog_h_
