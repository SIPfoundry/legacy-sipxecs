//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
// SYSTEM INCLUDES
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <pwd.h>
#include <grp.h>
#include <signal.h>

// APPLICATION INCLUDES
#include "net/NameValueTokenizer.h"
#include "processXMLCommon.h"
#include "WatchDog.h"
#include "os/OsSysLog.h"
#include "os/OsConfigDb.h"
#include "os/OsTask.h"
#include "AlarmServer.h"
#include "sipdb/SIPDBManager.h"
#include "sipXecsService/SipXecsService.h"
#include "SipxProcessManager.h"
#include "config/sipxsupervisor-buildstamp.h"

#define DEBUG

//The worker who does all the checking... based on OsServerTask
WatchDog *pDog;

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* CONFIG_SETTINGS_FILE = "sipxsupervisor-config";

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// GLOBALS
int gnCheckPeriod = 10;
UtlBoolean gbDone = FALSE;
MonitoredProcess *gpProcessList[1000]; //should be enough  :)
int gnProcessCount = 0; //how many processes are we checking?
UtlString strWatchDogFilename;
UtlBoolean gbPreloadedDatabases = FALSE;

//retrieve the update rate from the xml file
OsStatus getSettings(TiXmlDocument &doc, int &rCheckPeriod)
{
    OsStatus retval = OS_FAILED;

    OsSysLog::add(FAC_SUPERVISOR,PRI_INFO,"Getting check_period settings");

    TiXmlElement*rootElement = doc.RootElement();
    if (rootElement)
    {

        TiXmlNode *settingsItem = rootElement->FirstChild("settings");
        if (settingsItem)
        {
            TiXmlNode *healthItem = settingsItem->FirstChild("health");

            if ( healthItem != NULL )
            {
                // This is actually an individual row
                TiXmlElement *nextElement = healthItem->ToElement();


                const char *pCheckStr = nextElement->Attribute("check_period");
                if ( pCheckStr )
                {
                    rCheckPeriod = atoi(pCheckStr);

                    retval = OS_SUCCESS;
                }

            }
            else
            {
                OsSysLog::add(FAC_SUPERVISOR,PRI_ERR,"Can not read health settings");
            }
        }
        else
        {
            OsSysLog::add(FAC_SUPERVISOR,PRI_ERR,"Can not read settings node");
        }
    }
    else
    {
            OsSysLog::add(FAC_SUPERVISOR,PRI_ERR,"Can not read root element in getSettings!");
    }

    return retval;
}

// Parses the sipxsupervisor-config file and extracts the XML RPC port and  
// peer hostnames.  Returns OS_SUCCESS only if a non-zero port was found 
// *and* at least one peer hostname was found.  On success, the 'allowedPeers' 
// will contain new'd UtlString objects, which the caller is responsible  
// for deleting.  If OS_SUCCESS is not returned, then no memory is new'd.
OsStatus initXMLRPCsettings(int & port, UtlSList& allowedPeers)
{
    OsStatus retval = OS_FAILED;

    UtlString fileName =  SipXecsService::Path(SipXecsService::ConfigurationDirType,
                                               CONFIG_SETTINGS_FILE);
    
    OsConfigDb configDb;    
    bool configLoaded = ( configDb.loadFromFile(fileName) == OS_SUCCESS );
    if (!configLoaded)
    {
        OsSysLog::add(FAC_SUPERVISOR,PRI_ERR,"Can not open '%s'!", fileName.data());
    }
    else
    {
        port = configDb.getPort("SIP_WATCHDOG_XMLRPC_PORT");
        if (PORT_NONE == port)
        {
            OsSysLog::add(FAC_SUPERVISOR,PRI_ERR,"Can not read SIP_WATCHDOG_XMLRPC_PORT!");
        }
        else
        {
            UtlString peerNames;
            configDb.get("SIP_WATCHDOG_XMLRPC_PEERS", peerNames);
            if (peerNames.isNull())
            {
                OsSysLog::add(FAC_SUPERVISOR,PRI_ERR,
                              "Can not read SIP_WATCHDOG_XMLRPC_PEERS!");
            }
            else
            {
                UtlString peerName;
                for (int peerIndex = 0;
                     NameValueTokenizer::getSubField(peerNames.data(), peerIndex, ", \t", &peerName);
                     peerIndex++)
                {
                    // Found a port and at least one peer hostname.
                    if (!allowedPeers.contains(&peerName))
                    {
                       allowedPeers.insert(new UtlString(peerName));
                    }
                    retval = OS_SUCCESS;
                }
                
                if (OS_SUCCESS != retval)
                {
                    OsSysLog::add(FAC_SUPERVISOR,PRI_ERR,"No peers found.");
                }
            }
        }
    }

    return retval;
}

OsStatus initLogfile(TiXmlDocument &doc)
{
    OsStatus retval = OS_FAILED;
    OsSysLogPriority logging_level = (OsSysLogPriority) (-1);

    TiXmlElement*rootElement = doc.RootElement();
    if (rootElement)
    {
        TiXmlNode *healthItem = rootElement->FirstChild("logfile");

        if ( healthItem != NULL )
        {
            // This is actually an individual row
            TiXmlElement *nextElement = healthItem->ToElement();

            if ( !nextElement->NoChildren() )
            {
                const char *pLevelStr = nextElement->Attribute("level");
                if ( pLevelStr )
                {

                    if ( strcmp(pLevelStr,"debug") == 0 )
                        logging_level = PRI_DEBUG;
                    else
                        if ( strcmp(pLevelStr,"info") == 0 )
                        logging_level = PRI_INFO;
                    else
                        if ( strcmp(pLevelStr,"notice") == 0 )
                        logging_level = PRI_NOTICE;
                    else
                        if ( strcmp(pLevelStr,"warning") == 0 )
                        logging_level = PRI_WARNING;
                    else
                        if ( strcmp(pLevelStr,"err") == 0 )
                        logging_level = PRI_ERR;
                    else
                        if ( strcmp(pLevelStr,"crit") == 0 )
                        logging_level = PRI_CRIT;
                    else
                        if ( strcmp(pLevelStr,"alert") == 0 )
                        logging_level = PRI_ALERT;
                    else
                        if ( strcmp(pLevelStr,"emerg") == 0 )
                        logging_level = PRI_EMERG;
                    else
                    {
                       OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT,
                                     "initLogfile: Incomprehensible logging level string '%s'!",
                                     pLevelStr);
#ifdef DEBUG
                       osPrintf("initLogfile: Incomprehensible logging level string '%s'!\n", pLevelStr);
#endif /* DEBUG */
                    }


                }

                if ( logging_level >= 0 )
                {
                    const char*  payLoadData = nextElement->FirstChild()->Value();
                    if ( payLoadData )
                    {
                        retval = OsSysLog::initialize(0, "WatchDog") ;
                        OsSysLog::setOutputFile(0, payLoadData) ;
                        OsSysLog::setLoggingPriority(logging_level);

                        if ( retval == OS_SUCCESS )
                        {
                           OsSysLog::add(FAC_SUPERVISOR, PRI_NOTICE,
                                         ">>>>> Starting sipxsupervisor version %s",
                                         SipXsupervisorVersion);
                        }
                        else
                        {
                           // This is one case where we need to output an
                           // error message.
                           osPrintf("ERROR: initLogfile: Could not initialize SysLog!\n");
                        }
                    }
                }
            }
        }
        else
            OsSysLog::add(FAC_SUPERVISOR,PRI_ALERT,"Couldn't get health element in init logfile");
    }
    else
        OsSysLog::add(FAC_SUPERVISOR,PRI_ALERT,"Couldn't get root Element in init logfile");


    return retval;
}

//fills in the process objects
OsStatus createProcessList(TiXmlDocument &watchdogDoc, TiXmlDocument &processDoc, MonitoredProcess **processList, int &numProcesses)
{
    OsStatus retval = OS_FAILED;

    numProcesses = 0;

    TiXmlElement*rootElement = watchdogDoc.RootElement();
    if (rootElement)
    {
        TiXmlNode *monitorNode = rootElement->FirstChild( "monitor" );
        if ( monitorNode )
        {
            for ( TiXmlNode *dbProcessNode = monitorNode->FirstChild( "monitor-process" );
                dbProcessNode;
                dbProcessNode = dbProcessNode->NextSibling( "monitor-process" ) )
            {

                TiXmlElement *nextProcessElement = dbProcessNode->ToElement();

                const char *pAliasStr = nextProcessElement->Attribute("name");

                if ( pAliasStr )
                {
                    processList[numProcesses] = new MonitoredProcess(&processDoc);

                    UtlString alias = pAliasStr;
                    processList[numProcesses]->setAlias(alias);

                    //now get & set the max restart period
                    const char *pMaxRestartPeriodStr = nextProcessElement->Attribute("reset_restarts_after");

                    if ( pMaxRestartPeriodStr && strlen(pMaxRestartPeriodStr) )
                    {
                        processList[numProcesses]->setMaxRestartPeriod(atoi(pMaxRestartPeriodStr));

                    } else
                        processList[numProcesses]->setMaxRestartPeriod(300); //default is 5 mins

                    //now get & set the max restarts value
                    const char *pMaxRestartStr = nextProcessElement->Attribute("max_restarts");
                    if ( pMaxRestartStr )
                    {
                        processList[numProcesses]->setMaxRestarts(atoi(pMaxRestartStr));

                    }

                    //now get if restart is enabled
                    const char *pRestartEnableStr = nextProcessElement->Attribute("restart");
                    if ( pRestartEnableStr )
                    {
                        UtlBoolean enabled = FALSE;
                        if ( strcmp(pRestartEnableStr,"enable") == 0 )
                            enabled = TRUE;

                        processList[numProcesses]->enableRestart(enabled);

                    }


                    numProcesses++;

                }
                retval = OS_SUCCESS;
            }
        }
    }
    else
        OsSysLog::add(FAC_SUPERVISOR,PRI_ALERT,"Couldn't get root Element in createProcessList");

    return retval;

}



OsStatus loadWatchDogXML(TiXmlDocument &doc, UtlString &rStrFilename)
{
    OsStatus retval = OS_FAILED;

    if ( doc.LoadFile(rStrFilename.data()) )
    {
        OsPath watchdogFilename(rStrFilename);
        OsPath subprocessDir = watchdogFilename.getDirName() + OsPath::separator + PROCESS_DIR;
        retval = findSubDocs(subprocessDir, doc, &addWatchDogSubDoc);
    }


    return retval;
}


OsStatus getProcessXMLPath(TiXmlDocument &doc, UtlString &rProcessXMLPath)
{
    OsStatus retval = OS_FAILED;

    TiXmlElement*rootElement = doc.RootElement();
    if (rootElement)
    {
        TiXmlNode *settingsNode = rootElement->FirstChild( "settings");
        if (settingsNode)
        {
            TiXmlNode *worknode = settingsNode->FirstChild( "processxmlpath");
            if ( worknode != NULL )
            {                   // This is actually an individual row
                TiXmlElement *nextItemsContainer = worknode->ToElement();

                // Determine the DB to insert the items into
                OsProcess process;
                const char *pWorkDir = nextItemsContainer->Attribute("location");
                if ( pWorkDir )
                {
                    rProcessXMLPath = pWorkDir;

                    //if empty, then use watchdogs path
                    if ( rProcessXMLPath == "" )
                    {
                       OsPath watchdogFile(strWatchDogFilename);
                       rProcessXMLPath = watchdogFile.getDirName();
                    }
                    retval = OS_SUCCESS;
                }
            }
        }
        else
        {
            OsSysLog::add(FAC_SUPERVISOR,PRI_ALERT,
                          "Couldn't get settings Element in getProcessXMLPath");
        }
    }
    else
    {
        OsSysLog::add(FAC_SUPERVISOR,PRI_ALERT,"Couldn't get root Element in getProcessXMLPath");
    }

    return retval;
}



void doWaitLoop()
{
    while ( !gbDone )
        OsTask::delay(1000);

    if ( pDog )
        pDog->requestShutdown();
}

void cleanup()
{
    //cleanup our dog
    if ( pDog )
        delete pDog;

    //now walk the processList and whack em
    for ( int loop = 0; loop < gnProcessCount; loop++ )
        delete gpProcessList[loop];

    // Release preloaded databases.
    if (gbPreloadedDatabases)
    {
        OsStatus rc = SIPDBManager::getInstance()->releaseAllDatabase();
        if (OS_SUCCESS == rc)
        {
            OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "Released the preloaded the databases.");
        }
        else
        {
            OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, 
                          "releaseAllDatabase() failed, rc = %d.", (int)rc); 
        }
        delete SIPDBManager::getInstance();
    }

    cAlarmServer::getInstance()->cleanup();
    
    OsSysLog::add(FAC_SUPERVISOR,PRI_ALERT,"Execution Completed.");

    //cause main loop to exit
    gbDone = TRUE;
}

void sig_routine(int sig)
{

    switch ( sig )
    {
    case SIGINT:
#ifdef DEBUG
       osPrintf("Execution USER ABORTED!\n");
#endif /* DEBUG */
       OsSysLog::add(FAC_SUPERVISOR,PRI_ALERT,"Execution USER ABORTED!");
       break;
    case SIGABRT:
#ifdef DEBUG
       osPrintf("Execution ABORTED!\n");
#endif /* DEBUG */
       OsSysLog::add(FAC_SUPERVISOR,PRI_ALERT,"Execution ABORTED!");
       break;
    case SIGTERM:
#ifdef DEBUG
       osPrintf("Execution TERMINATED!\n");
#endif /* DEBUG */
       OsSysLog::add(FAC_SUPERVISOR,PRI_ALERT,"Execution TERMINATED!");
       break;
    case SIGKILL:
#ifdef DEBUG
       osPrintf("Execution KILLED!\n");
#endif /* DEBUG */
       OsSysLog::add(FAC_SUPERVISOR,PRI_ALERT,"Execution KILLED!");
       break;
    default:
#ifdef DEBUG
       osPrintf("Received unexpected signal %d, shutting down.\n", sig);
#endif /* DEBUG */
       OsSysLog::add(FAC_SUPERVISOR,PRI_EMERG,"UNEXPECTED SIGNAL: %d shutting down!", sig);
       break;
    }
    gbDone = TRUE;
}


class SignalTask : public OsTask
{
public:
   SignalTask() : OsTask() {}

   int
   run(void *pArg)
   {
       int sig_num ;
       OsStatus res ;

       // Wait for a signal.  This will unblock signals
       // for THIS thread only, so this will be the only thread
       // to catch an async signal directed to the process 
       // from the outside.
       res = awaitSignal(sig_num);
       if (res != OS_SUCCESS)
       {
          OsSysLog::add(FAC_SUPERVISOR,PRI_ALERT,"awaitSignal() errno=%d", errno);
       }

       // Call the original signal handler.  Pass -1 on error from awaitSignal
       sig_routine( OS_SUCCESS == res ? sig_num : -1);
       return 0;
   }
};

int main(int argc, char* argv[])
{
    // Block all signals in this the main thread
    // Any threads created after this will have all signals masked.
    OsTask::blockSignals();

    // Create a new task to wait for signals.  Only that task
    // will ever see a signal from the outside.
    SignalTask* signalTask = new SignalTask();
    signalTask->start() ;

    // All osPrintf output should go to the console until the log file is initialized.
    enableConsoleOutput(true);

    UtlString argString;
    const char * sipxpbxuser = SipXecsService::User();
    const char * sipxpbxgroup = SipXecsService::Group();

    for (int argIndex = 1; argIndex < argc; argIndex++) 
    {
        bool usageExit = false;
        int valueExit = 1;

        argString = argv[argIndex];
        NameValueTokenizer::frontBackTrim(&argString, "\t ");

        if (argString.compareTo("-h") == 0) 
        {
           usageExit = true;
           valueExit = 0;
        }
        if (argString.compareTo("-v") == 0) 
        {
           osPrintf("sipxsupervisor %s\n\n", SipXsupervisorVersion);
           return 0;
        }
        else if (argString.compareTo("-f") == 0)
        {
            if (argIndex+1 >= argc)
            {
                // Missing the filename.
                usageExit = true;
            }
            else
            {
                strWatchDogFilename = argv[++argIndex];
                // This is OK to print directly, since it happens at startup and is
                // triggered only by special arguments.
                osPrintf("WatchDog XML configuration set to '%s'.\n", strWatchDogFilename.data());
            }
        }
        else
        {
            // Unknown argument.
            usageExit = true;
        }

        if (usageExit)
        {
            osPrintf("usage: %s [-h] [-v] [-f filename]\n", argv[0]);
            osPrintf(" -h           Print this help and exit.\n");
            osPrintf(" -v           Print version number.\n");
            osPrintf(" -f filename  Use the specified watchdog config file.\n");
            return valueExit;
        }
    }

    // Drop privileges down to the specified user & group
    if (NULL == sipxpbxuser || 0 == strlen(sipxpbxuser))
    {
       osPrintf("sipXsupervisor: Failed to setuid(%s), username not defined.\n",
          sipxpbxuser);
       return 2;
    }
    if (NULL == sipxpbxgroup || 0 == strlen(sipxpbxgroup))
    {
       osPrintf("sipXsupervisor: Failed to setgid(%s), groupname not defined.\n",
          sipxpbxgroup);
       return 2;
    }

    struct group * grp = getgrnam(sipxpbxgroup);
    if (NULL == grp)
    {
       if (0 != errno)
       {
          osPrintf("getgrnam(%s) failed, errno = %d.", 
             sipxpbxgroup, errno);
       }
       else
       {
          osPrintf(
             "sipXsupervisor: getgrnam(%s) failed, user does not exist.",
                sipxpbxgroup);
       }
       return 3;
    }

    struct passwd * pwd = getpwnam(sipxpbxuser);
    if (NULL == pwd)
    {
       if (0 != errno)
       {
          osPrintf("getpwnam(%s) failed, errno = %d.", 
             sipxpbxuser, errno);
       }
       else
       {
          osPrintf(
             "sipXsupervisor: getpwnam(%s) failed, user does not exist.",
                sipxpbxuser);
       }
       return 3;
    }

    // Change group first, cause once user is changed this cannot be done.
    if (0 != setgid(grp->gr_gid))
    {
       osPrintf("sipXsupervisor: setgid(%d) failed, errno = %d.",
          (int)grp->gr_gid, errno);
       return 4;
    }

    if (0 != setuid(pwd->pw_uid))
    {
       osPrintf("sipXsupervisor: setuid(%d) failed, errno = %d.",
          (int)pwd->pw_uid, errno);
       return 4;
    }


# if 0
// Only output problems.  This keeps the startup output clean.
    osPrintf("sipXsupervisor: Dropped privileges with setuid(%s)/setgid(%s).", 
       sipxpbxuser, sipxpbxgroup);
#endif
                              
    // The location of the watchdog config file might have been supplied on the command-line.
    // Otherwise, the config file is SIPX_CONFDIR/WatchDog.xml.
    if (strWatchDogFilename.isNull())
    {
       strWatchDogFilename = SipXecsService::Path(SipXecsService::ConfigurationDirType,
                                                  "WatchDog.xml");
    }

    // Load the WatchDog XML (settings) file.
    TiXmlDocument watchdogXMLDoc;
    OsStatus rc = loadWatchDogXML(watchdogXMLDoc, strWatchDogFilename);
    if (OS_SUCCESS != rc)
    {
        osPrintf("Error: loadWatchDogXML() failed, rc = %d.\n", (int)rc);
        return 5;
    }

    // Initialize the log file.
    rc = initLogfile(watchdogXMLDoc);
    if (OS_SUCCESS != rc)
    {
        osPrintf("Error: initLogfile() failed, rc = %d.\n", (int)rc);
        return 6;
    }

    // Now that the log file is initialized, stop sending osPrintf to the console.
    // All relevant log messages from this point on must use OsSysLog::add().
    enableConsoleOutput(false);
    fflush(NULL); // Flush all output so children don't get a buffer of output
    OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "Loaded WatchDog XML from: %s",
                  strWatchDogFilename.data());

    //set our exit routine
    atexit(cleanup);
    
    // Preload the databases.  This ensures that:               
    //  1. Their reference count does not go to 0 causing an expensive 
    //  load by another process.  (Notably when the system only has apache 
    //  running on it; on such a system, the only processes accessing the 
    //  database would be CGIs, and they are created and destroyed frequently -
    //  the database would be recreated and reloaded a lot if there were not 
    //  a process holding the use count above zero.)
    //
    //  2. The first touch of each database is performed by a non-root 
    //  process, thus allowing processes running as root to subsequently 
    //  access the database  without causing the known fastdb hang problem.  
    rc = SIPDBManager::getInstance()->preloadAllDatabase();
    if (OS_SUCCESS == rc)
    {
       // The databases must be released on exit.
       gbPreloadedDatabases = TRUE;
       OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "Preloaded databases.");
    }
    else
    {
       OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
          "sipXsupervisor preloadAllDatabase() failed, rc = %d", (int)rc); 
    }
        
    if (!cAlarmServer::getInstance()->init())
    {
       OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
             "sipXsupervisor failed to init AlarmServer");
    }
    
    // Determine the ProcessDefinitions XML file path.
    UtlString processXMLPath;
    rc = getProcessXMLPath(watchdogXMLDoc, processXMLPath);
    if (OS_SUCCESS != rc)
    {
        OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, 
           "getProcessXMLPath() failed, rc = %d.", (int)rc);
        return 7; 
    }
    OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, 
       "Using ProcessDefinitions XML from: %s", 
          processXMLPath.data());

    // Load the ProcessDefinitions XML file.
    TiXmlDocument processXMLDoc;
    UtlString gstrErrorMsg;
    rc = initProcessXMLLayer(processXMLPath, processXMLDoc, gstrErrorMsg);
    if (OS_SUCCESS != rc)
    {
        OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, 
           "initProcessXMLLayer() failed, rc = %d, msg='%s'.",
              (int)rc, gstrErrorMsg.data());
        return 8;
    }
    
    // Get the general Watchdog settings.
    rc = getSettings(watchdogXMLDoc, gnCheckPeriod);
    if (OS_SUCCESS != rc)
    {
        OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, 
           "getSettings() failed, rc = %d.", (int)rc);
        return 9;
    }
    OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, 
                  "Process check occurs every %d seconds.", gnCheckPeriod);
    
    // Get the Watchdog XML-RPC server settings.
    UtlSList allowedPeers;
    int port;
    rc = initXMLRPCsettings(port, allowedPeers);
    if (OS_SUCCESS != rc)
    {
       OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, 
                     "initXMLRPCsettings() failed, rc = %d.", (int)rc);
       return 11;
    }
    
    // Read the process definitions.
    UtlString processDefinitionDirectory =
       SipXecsService::Path(SipXecsService::DataDirType, "process.d");
    SipxProcessManager* processManager = SipxProcessManager::getInstance();
    processManager->instantiateProcesses(processDefinitionDirectory);

    // @TODO - old style - to be removed: Create the list of processes to watch.
    rc = createProcessList(watchdogXMLDoc, processXMLDoc, gpProcessList, gnProcessCount);
    if (OS_SUCCESS != rc)
    {
        // Apparently this is a common case, so send useful information to the console too.
        // Note that "ProcessDefinitions.xml" is fixed as the last component of the process XML 
        // file name.  See loadProcessXML in processCommon.cpp.
        OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "createProcessList() failed, rc = %d.", (int)rc);        
        enableConsoleOutput(true);
        osPrintf("Couldn't load process list: Error in '%s' and/or '%s/ProcessDefinitions.xml'.\n",
                 strWatchDogFilename.data(), processXMLPath.data());
        return 10;
    }
    
    // Create the "watchdog" which will monitor the process list.
    pDog = new WatchDog(gnCheckPeriod, gpProcessList, gnProcessCount, port, allowedPeers);
    pDog->start();
    doWaitLoop();

    // Successful run.
    return 0;
}
