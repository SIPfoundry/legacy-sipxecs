// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "FailureReporterBase.h"
#include "MonitoredProcess.h"
#include "processcgi/processXMLCommon.h"
#include "os/OsSysLog.h"
#include "os/OsTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern int gnCheckPeriod;

#define STARTING_TIME 5 /* seconds */

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MonitoredProcess::MonitoredProcess(TiXmlDocument *processDoc) :
mbRestartEnabled(FALSE),
mbReportEnabled(FALSE),
mnMaxRestartsPeriod(300),
mnMaxRestarts(0),
mnTotalRestarts(0),
mnMaxReports(0),
mnTotalReports(0),
mnMaxRestartElapsedSecs(0),
mnLastProcessState(-1),
mpProcessDoc(NULL),
mNumReporters(0),
mbStartedOnce(FALSE)
{
    if ( processDoc != NULL )
        mpProcessDoc = processDoc;


}

// Copy constructor
MonitoredProcess::MonitoredProcess(const MonitoredProcess& rMonitoredProcess)
{
}

// Destructor
MonitoredProcess::~MonitoredProcess()
{
    //remove all the reporters
    for ( int loop = 0;loop < mNumReporters;loop++ )
        delete mpReporters[loop];

}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
MonitoredProcess&
MonitoredProcess::operator=(const MonitoredProcess& rhs)
{
    if ( this == &rhs )            // handle the assignment to self case
        return *this;

    return *this;
}

void MonitoredProcess::setAlias(UtlString &rAlias)
{
    mAliasStr = rAlias;
}

void MonitoredProcess::setMaxRestarts(int nMaxRestartCount)
{
    mnMaxRestarts = nMaxRestartCount;
}

void MonitoredProcess::setMaxRestartPeriod(int nMaxRestartPeriod)
{
    mnMaxRestartsPeriod = nMaxRestartPeriod;
}

void MonitoredProcess::setMaxReports(int nMaxReportCount)
{
    mnMaxReports = nMaxReportCount;
}

void MonitoredProcess::enableReports(UtlBoolean bEnable)
{
    mbReportEnabled = bEnable;
}

void MonitoredProcess::enableRestart(UtlBoolean bEnable)
{
    mbRestartEnabled = bEnable;
}

//this function will check if a process is in the stopped state
//and then clear it.  This will cause the next check call to attempt
//a restart of the process
void MonitoredProcess::resetStoppedState()
{
    OsProcessMgr *pProcessMgr = OsProcessMgr::getInstance(SIPX_TMPDIR);

    int processstate = pProcessMgr->getAliasState(mAliasStr);
    if (processstate == PROCESS_STOPPED)
    {
        //setting the process state to NEVERRUN, means that the process
        //alias and sta eare removed from the processAlias.dat file
        //This will cause watchdog to attempt a restart and next check()
        pProcessMgr->setAliasState(mAliasStr,PROCESS_NEVERRUN);
    }
}

//adds reporters (objects will tell users of interesting events) to the process
//under watch
OsStatus MonitoredProcess::AddReporter(FailureReporterBase *pReporter)
{
    OsStatus retval = OS_FAILED;
    if ( mNumReporters < MAX_REPORTERS )
    {

        OsSysLog::add(FAC_WATCHDOG,PRI_INFO,"Adding reporter to process alias %s\n",
                      mAliasStr.data());
        mpReporters[mNumReporters] = pReporter;
        mNumReporters++;
        retval = OS_SUCCESS;
    } else
        OsSysLog::add(FAC_WATCHDOG,PRI_ERR,"MAX_REPORTERS reached for process alias %s\n",
                      mAliasStr.data());
    return retval;
}

OsStatus MonitoredProcess::sendReports()
{
    OsStatus retval = OS_FAILED;

    OsSysLog::add(FAC_WATCHDOG,PRI_INFO," Flushing (Sending) reports STARTED for alias %s\n",
                  mAliasStr.data());

    for ( int loop = 0; loop < mNumReporters;loop++ )
    {
        mpReporters[loop]->send();
    }

    OsSysLog::add(FAC_WATCHDOG,PRI_INFO," Send reports COMPLETE for alias %s\n",
                  mAliasStr.data());

    return retval;
}
/* ============================ ACCESSORS ================================= */
UtlString MonitoredProcess::getAlias()
{
    return mAliasStr;
}

/* ============================ INQUIRY =================================== */
UtlBoolean MonitoredProcess::isReportsEnabled()
{
    return mbReportEnabled;
}

UtlBoolean MonitoredProcess::isRestartEnabled()
{
    return mbRestartEnabled;
}

//helper func. converts state to string
void stateToString(int state, UtlString &rStateStr)
{
    switch(state)
    {
        case PROCESS_STARTED:
            rStateStr = "RUNNING";
            break;
        case PROCESS_STARTING:
            rStateStr = "STARTING";
            break;
        case PROCESS_STOPPED:
            rStateStr = "STOPPED";
            break;
        case PROCESS_STOPPING:
            rStateStr = "STOPPING";
            break;
        case PROCESS_FAILED:
            rStateStr = "FAILED";
            break;
        case PROCESS_NEVERRUN:
            rStateStr = "NEVERRUN";
            break;
        default:
            rStateStr = "UNKNOWN:  Check code, something is weird!";
            break;
    }

}

void MonitoredProcess::ApplyUserRequestedState()
{
    OsProcessMgr *pProcessMgr = OsProcessMgr::getInstance(SIPX_TMPDIR);

    pProcessMgr->lockAliasFile() ;

    int userRequestedState = pProcessMgr->getUserRequestState(mAliasStr);
    
    //the user has requested a process state change
    if (userRequestedState != USER_PROCESS_NONE)
    {
        TiXmlDocument *doc = mpProcessDoc;
        UtlString verb = "";

        if (userRequestedState == USER_PROCESS_START)
            verb = "start";
        else
        if (userRequestedState == USER_PROCESS_STOP)
            verb = "stop";
        else
        if (userRequestedState == USER_PROCESS_RESTART)
            verb = "restart";

        OsSysLog::add(FAC_WATCHDOG,PRI_INFO,"User has requested a %s on process %s\n",
                      verb.data(),mAliasStr.data());
        
        if ( startstopProcessTree(*doc,mAliasStr,verb) == OS_SUCCESS )
        {
            OsSysLog::add(FAC_WATCHDOG,PRI_INFO,"[SUCCESS] User with %s on process %s\n",
                          verb.data(),mAliasStr.data());

        }
        else
            OsSysLog::add(FAC_WATCHDOG,PRI_INFO,"[FAILED] User could not %s process %s\n",
                          verb.data(),mAliasStr.data());
    }
        
    pProcessMgr->unlockAliasFile() ;

}

OsStatus MonitoredProcess::check()
{
    TiXmlDocument *doc = mpProcessDoc;
    OsStatus retval = OS_FAILED;
    char msgbuf[160];
    UtlString msgStr = mAliasStr;

    OsProcessMgr *pProcessMgr = OsProcessMgr::getInstance(SIPX_TMPDIR);

    pProcessMgr->lockAliasFile() ;
    
    int processstate = pProcessMgr->getAliasState(mAliasStr);
    UtlBoolean bIsGhost = FALSE;
    UtlString currentStateStr;
    UtlString lastStateStr;
    //save off current state so we can catch a transistion

    //dont check if the user has stopped the process
    if (processstate != PROCESS_STOPPED && processstate != PROCESS_STOPPING)
    {

        stateToString(processstate,currentStateStr);

        if (mnLastProcessState != processstate && mnLastProcessState != -1)
        {
            //we don't want to see started --> starting because it will be reported later
            if (mnLastProcessState == PROCESS_STARTED && processstate == PROCESS_STARTING)
                ; //do nothing
            else
            {
                stateToString(mnLastProcessState,lastStateStr);
                OsSysLog::add(FAC_WATCHDOG,PRI_WARNING,"Process %s state changed: %s --> %s",
                            mAliasStr.data(), lastStateStr.data(),currentStateStr.data() );
            }
        }

        mnLastProcessState = processstate;

        //if debug level is set then monitor all states
         OsSysLog::add(FAC_WATCHDOG,PRI_DEBUG,"Process %s in %s state.", mAliasStr.data(),currentStateStr.data());


        //if it is running... see if we can get its process
        if ( processstate == PROCESS_STARTING )
        {
#ifdef DEBUG
            osPrintf("DWW Process Marked as starting... checking if it's true\n");
#endif /* DEBUG */
            //wait up to STARTING_TIME seconds to see if we can get the process info
            //if after STARTING_TIME seconds we can't, then consider this process a ghost
            OsSysLog::add(FAC_WATCHDOG,PRI_WARNING,"Process %s found in non-running state. (Waiting for it to start).", mAliasStr.data());
            int trycount = 0;
            int tmpProcessState = PROCESS_STARTING;
            
            while (tmpProcessState == PROCESS_STARTING && trycount < STARTING_TIME)
            {
                bIsGhost = FALSE;
                OsProcess process;
                tmpProcessState = pProcessMgr->getAliasState(mAliasStr);
                if (tmpProcessState == PROCESS_STARTING)
                {
                    bIsGhost = TRUE;
                    OsTask::delay(1000);
                }

                trycount++;
            }

            if (bIsGhost)             
                OsSysLog::add(FAC_WATCHDOG,PRI_WARNING,"Process %s found in non-running state (it was in STARTING state for more than %d seconds).", mAliasStr.data(), STARTING_TIME);

        }


#ifdef DEBUG
        osPrintf("%s is state : %d\n",mAliasStr,processstate);
#endif /* DEBUG */

        //make sure it attempts start the first time no matter what the state is
        if ( !mbStartedOnce && processstate != PROCESS_STARTED )
        {
            processstate = PROCESS_NEVERRUN;
            pProcessMgr->setAliasState(mAliasStr,PROCESS_NEVERRUN);
        }

        if ( processstate == PROCESS_STARTED && mbRestartEnabled )
        {
            sprintf(msgbuf,"Checking process %s", mAliasStr.data());
            OsSysLog::add(FAC_WATCHDOG,PRI_INFO,msgbuf);
            UtlBoolean bVerified = FALSE;
            OsSysLog::add(FAC_WATCHDOG,PRI_INFO,"[running]\n");

            //now check if the process can be verified as running
            if ( processstate == PROCESS_STARTED && VerifyProcess(mAliasStr) == OS_SUCCESS )
            {
                OsSysLog::add(FAC_WATCHDOG,PRI_INFO,"[verified]\n");
                bVerified = TRUE;
            } else
            {
                OsSysLog::add(FAC_WATCHDOG,PRI_ERR,"[verify failed]\n");
            }

            if ( !bVerified )
            {
                if ( processstate != PROCESS_STARTED )
                    OsSysLog::add(FAC_WATCHDOG,PRI_INFO,"[PENDING]\n");
                else
                {
/* @JC - Dan figure out what verify should do in this case
                    OsSysLog::add(FAC_WATCHDOG,PRI_WARNING,"Attempting kill.\n");
                    UtlString stopverb = "stop";
                    bIsGhost = TRUE;
                    if ( startstopProcessTree(*doc,mAliasStr,stopverb) == OS_SUCCESS )
                    {
                        OsSysLog::add(FAC_WATCHDOG,PRI_WARNING,"[Stopped]\n");
                    } else
                    {
                        OsSysLog::add(FAC_WATCHDOG,PRI_ERR,"[Stop Failed]\n");
                    }
*/
                }

            }
        }

        // If the process is not running, and it should be started/restarted,
        // do so.
        if ((mbRestartEnabled || !mbStartedOnce) &&
            (processstate == PROCESS_NEVERRUN || bIsGhost))
        {
            //reset restarts if after the MaxRestartsPeriod
            if ( mnMaxRestartElapsedSecs > mnMaxRestartsPeriod )
            {
                mnMaxRestartElapsedSecs = 0;
                mnTotalRestarts = 0;
            }

            if ( mnTotalRestarts < mnMaxRestarts )
            {

                UtlString verb = "start";

                if ( mbStartedOnce ) //dont send mail first time
                {
                    msgStr = mAliasStr;
                    msgStr.append(" was found in a non-running state.");

                    for ( int loop = 0; loop < mNumReporters;loop++ )
                    {
                        mpReporters[loop]->report(mAliasStr,msgStr);
                    }
                }

                OsSysLog::add(FAC_WATCHDOG,PRI_WARNING,"Attempting %s startup...", mAliasStr.data());
                mnTotalRestarts++;
                if ( startstopProcessTree(*doc,mAliasStr,verb) == OS_SUCCESS )
                {
                    OsSysLog::add(FAC_WATCHDOG,PRI_WARNING,"[success]");
                    retval = OS_SUCCESS;

                    if ( mbStartedOnce ) //dont send mail first time
                    {
                        msgStr = mAliasStr;
                        msgStr.append(" was successfully restarted.");

                        for ( int loop = 0; loop < mNumReporters;loop++ )
                        {
                            mpReporters[loop]->report(mAliasStr,msgStr);
                        }
                    }
                    mbStartedOnce = TRUE;

                } else
                {
                    sprintf(msgbuf,"%s failed restarting.", mAliasStr.data());
                    OsSysLog::add(FAC_WATCHDOG,PRI_ERR,msgbuf);

                    msgStr = mAliasStr;
                    msgStr.append(" failed restart.");

                    for ( int loop = 0; loop < mNumReporters;loop++ )
                    {
                        mpReporters[loop]->report(mAliasStr,msgStr);
                    }

                }

            } else
            {
                char msg[256];
                sprintf(msg,"%s Failed MAX restarts. Process can not be restarted!", mAliasStr.data());
                OsSysLog::add(FAC_WATCHDOG,PRI_ERR,msg);
                mbRestartEnabled = FALSE;
                mbStartedOnce = TRUE; //so it wont keep changing the state
                UtlString msgStr = msg;

                //store message
                for ( int loop = 0; loop < mNumReporters;loop++ )
                {
                    mpReporters[loop]->report(mAliasStr,msgStr);
                }

                pProcessMgr->setAliasState(mAliasStr,PROCESS_FAILED);
            }
        }

        //only  allow the time to increment if process not failed
        if ( processstate != PROCESS_FAILED )
            mnMaxRestartElapsedSecs += gnCheckPeriod; //add check period time to failure time
    } //if process !stopped

    pProcessMgr->unlockAliasFile() ;

    return retval;
}
/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

