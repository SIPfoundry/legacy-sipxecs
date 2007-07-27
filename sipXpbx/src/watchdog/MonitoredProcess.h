// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _MonitoredProcess_h_
#define _MonitoredProcess_h_

// SYSTEM INCLUDES
#include "os/OsStatus.h"
#include "os/OsDefs.h"

// APPLICATION INCLUDES
#include "processXMLCommon.h"

// DEFINES
#define MAX_REPORTERS 100
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class FailureReporterBase;
class OsProcessMgr;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class MonitoredProcess
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   MonitoredProcess(TiXmlDocument *processDoc);
     //:Default constructor

   MonitoredProcess(const MonitoredProcess& rMonitoredProcess);
     //:Copy constructor

   virtual
   ~MonitoredProcess();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   MonitoredProcess& operator=(const MonitoredProcess& rhs);
     //:Assignment operator

   OsStatus AddReporter(FailureReporterBase *pReporter);
   //: Added a reporter (to send messages to users) to process object

   void setAlias(UtlString &rAliasStr);
   void setMaxRestarts(int nMaxRestartCount);
   void setMaxReports(int nMaxReportCount);
   void setMaxRestartPeriod(int nMaxRestartPeriod); //in secs
   void enableReports(UtlBoolean bEnable);
   void enableRestart(UtlBoolean bEnable);
   void resetStoppedState();
   //:causes entry in alias file to be removed.
   //:next check() will restart.

   OsStatus sendReports();
   //: Causes all reporters to flush and report.

/* ============================ ACCESSORS ================================= */
   UtlString getAlias();
/* ============================ INQUIRY =================================== */

   void ApplyUserRequestedState();
   //: If the user has requested to change the state of the running process
   //: then this is the function to do it.

   OsStatus check();
   //: Performs check if running or not and notifies reporters
   //: Will restart if there is a problem.  It won't restart if max restarts has 
   //: been reached.

   UtlBoolean isReportsEnabled();
   //: Returns TRUE if reports are enabled.

   UtlBoolean isRestartEnabled();
   //: Returns TRUE if restarts are enabled.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlString mAliasStr;
   UtlBoolean mbRestartEnabled;
   UtlBoolean mbReportEnabled;
   int mnMaxRestartsPeriod;  //when max restarts occurs in this time, its a failure
   int mnMaxRestarts;  //what is the maximum number of restarts allowed
   int mnTotalRestarts; //how many times have we restarted so far
   int mnMaxReports;    //what is the maximum number of reports to send
   int mnTotalReports;  //how many reports we have sent so far
   int mnMaxRestartElapsedSecs;
   int mnLastProcessState;
   TiXmlDocument  *mpProcessDoc; //pointer to XML doc containing the process we will monitor
   FailureReporterBase *mpReporters[MAX_REPORTERS];
   int mNumReporters;
   UtlBoolean mbStartedOnce;
   //has watchdog started just process once
   //we dont want to send reports
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MonitoredProcess_h_

