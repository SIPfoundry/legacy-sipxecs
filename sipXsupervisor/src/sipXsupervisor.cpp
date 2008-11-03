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
#include "os/OsSysLog.h"
#include "os/OsConfigDb.h"
#include "os/OsTask.h"
#include "config/sipxsupervisor-buildstamp.h"
#include "sipdb/SIPDBManager.h"
#include "sipXecsService/SipXecsService.h"
#include "SipxRpc.h"
#include "AlarmServer.h"
#include "SipxProcessManager.h"

#define DEBUG

//The worker who manages xmlrpc requests
SipxRpc *pSipxRpcImpl;

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* CONFIG_SETTINGS_FILE = "sipxsupervisor-config";
const int   DEFAULT_SUPERVISOR_PORT = 8092;
const char* SUPERVISOR_HOST = "SUPERVISOR_HOST";

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// GLOBALS
int gnCheckPeriod = 10;
UtlBoolean gbDone = FALSE;
UtlBoolean gbShutdown = FALSE;
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

/// Parses the domain-config and sipxsupervisor-config files for control configuration data.
/**
 *  Gets:
 *  - XML RPC port
 *  - sipXconfig hostnames
 *  - the local hostname
 * The combined list of these hostnames is inserted in allowedPeers
 *
 * The caller is responsible for the UtlString objects in the 'allowedPeers' list.
 */
void initXMLRPCsettings(int & port, UtlSList& allowedPeers)
{
    OsStatus retval = OS_FAILED;
    port=DEFAULT_SUPERVISOR_PORT;

    OsConfigDb domainConfiguration;
    OsPath domainConfigPath = SipXecsService::domainConfigPath();
    if (OS_SUCCESS == domainConfiguration.loadFromFile(domainConfigPath.data()))
    {
       port = domainConfiguration.getPort(SipXecsService::DomainDbKey::SUPERVISOR_PORT);
       if (PORT_NONE == port)
       {
          OsSysLog::add(FAC_SUPERVISOR,PRI_ERR,
                        "initXMLRPCsettings: %s not configured in '%s'",
                        SipXecsService::DomainDbKey::SUPERVISOR_PORT,
                        domainConfigPath.data()
                        );
          port=DEFAULT_SUPERVISOR_PORT;
       }
       else if (PORT_DEFAULT == port)
       {
          OsSysLog::add(FAC_SUPERVISOR,PRI_NOTICE,"initXMLRPCsettings: %s set to default",
                        SipXecsService::DomainDbKey::SUPERVISOR_PORT
                        );
          port=DEFAULT_SUPERVISOR_PORT;
       }
       
       UtlString configHosts;
       domainConfiguration.get(SipXecsService::DomainDbKey::CONFIG_HOSTS, configHosts);
       if (!configHosts.isNull())
       {
          UtlString hostName;
          for (int hostIndex = 0;
               NameValueTokenizer::getSubField(configHosts.data(), hostIndex, ", \t", &hostName);
               hostIndex++)
          {
             // Found at least one config hostname.
             if (!allowedPeers.contains(&hostName))
             {
                OsSysLog::add(FAC_SUPERVISOR,PRI_DEBUG,
                              "initXMLRPCsettings: %s value '%s'",
                              SipXecsService::DomainDbKey::CONFIG_HOSTS,
                              hostName.data()
                        );
                allowedPeers.insert(new UtlString(hostName));
             }
             retval = OS_SUCCESS;
          }                
       }
       else
       {
          OsSysLog::add(FAC_SUPERVISOR,PRI_ERR,
                        "initXMLRPCsettings: %s not configured in '%s'",
                        SipXecsService::DomainDbKey::CONFIG_HOSTS, domainConfigPath.data()
                        );
       }
    }
    else
    {
       OsSysLog::add(FAC_SUPERVISOR,PRI_ERR,
                     "initXMLRPCsettings: failed to open '%s'",
                     domainConfigPath.data()
                     );
    }

    OsConfigDb supervisorConfiguration;
    OsPath supervisorConfigPath = SipXecsService::Path(SipXecsService::ConfigurationDirType,
                                                       CONFIG_SETTINGS_FILE);
    if (OS_SUCCESS == supervisorConfiguration.loadFromFile(supervisorConfigPath.data()))
    {
       UtlString superHost;
       supervisorConfiguration.get(SUPERVISOR_HOST, superHost);
       if (!superHost.isNull())
       {
          if (!allowedPeers.contains(&superHost))
          {
             allowedPeers.insert(new UtlString(superHost));
          }
       }
       else
       {
          OsSysLog::add(FAC_SUPERVISOR,PRI_ERR,
                        "initXMLRPCsettings: %s not configured in '%s'",
                        SUPERVISOR_HOST, supervisorConfigPath.data()
                        );
       }
    }
    else
    {
       OsSysLog::add(FAC_SUPERVISOR,PRI_WARNING,
                     "initXMLRPCsettings: failed to open '%s'",
                     supervisorConfigPath.data()
                     );
    }
    

    if (allowedPeers.isEmpty())
    {
       OsSysLog::add(FAC_SUPERVISOR,PRI_ERR,
                     "initXMLRPCsettings: No configuration servers configured.");
    }
}

void cleanup()
{
    // Stop handling xmlrpc requests
    if ( pSipxRpcImpl )
    {
        delete pSipxRpcImpl;
        pSipxRpcImpl = NULL;
    }

    // Shut down all processes
    SipxProcessManager::getInstance()->shutdown();

    // Release preloaded databases.
    if (gbPreloadedDatabases)
    {
        OsStatus rc = SIPDBManager::getInstance()->releaseAllDatabase();
        if (OS_SUCCESS == rc)
        {
            OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "Released the preloaded databases.");
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

void doWaitLoop()
{
    while ( !gbDone )
    {
        if ( gbShutdown )
        {
            cleanup();
            gbDone = true;
        }
        OsTask::delay(1000);
    }
}

void sig_routine(int sig)
{

    switch ( sig )
    {
    case SIGCHLD:
#ifdef DEBUG
       osPrintf("Execution CHILD exited; ignoring\n");
#endif /* DEBUG */
       OsSysLog::add(FAC_SUPERVISOR,PRI_DEBUG,"Execution CHILD exited; ignoring");
       return;
       break;    
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
       gbShutdown = true;
       return;
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
      int sig_num;
      OsStatus res;

      // Wait for a signal.  This will unblock signals
      // for THIS thread only, so this will be the only thread
      // to catch an async signal directed to the process 
      // from the outside.
      while (!gbDone)
      {
         res = awaitSignal(sig_num);
         if (res != OS_SUCCESS)
         {
            OsSysLog::add(FAC_SUPERVISOR, PRI_ALERT, "awaitSignal() errno=%d", errno);
         }

         // Call the original signal handler.  Pass -1 on error from awaitSignal
         sig_routine(OS_SUCCESS == res ? sig_num : -1);
      }
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
        else
        {
            // Unknown argument.
            usageExit = true;
        }

        if (usageExit)
        {
            osPrintf("usage: %s [-h] [-v]\n", argv[0]);
            osPrintf(" -h           Print this help and exit.\n");
            osPrintf(" -v           Print version number.\n");
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
                              
    // Initialize the log file.
    OsSysLog::initialize(0, "Supervisor") ;
    UtlString logFile = SipXecsService::Path(SipXecsService::LogDirType, "sipxsupervisor.log");
    OsSysLog::setOutputFile(0, logFile) ;
    OsSysLog::setLoggingPriority(PRI_DEBUG);
    OsSysLog::add(FAC_SUPERVISOR, PRI_NOTICE,
                  ">>>>> Starting sipxsupervisor version %s",
                  SipXsupervisorVersion);

    // Now that the log file is initialized, stop sending osPrintf to the console.
    // All relevant log messages from this point on must use OsSysLog::add().
    enableConsoleOutput(false);
    fflush(NULL); // Flush all output so children don't get a buffer of output

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
    OsStatus rc = SIPDBManager::getInstance()->preloadAllDatabase();
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
    
    // Get the Supervisor XML-RPC server settings.
    UtlSList allowedPeers;
    int port;
    initXMLRPCsettings(port, allowedPeers);
    
    // Read the process definitions.
    UtlString processDefinitionDirectory =
       SipXecsService::Path(SipXecsService::DataDirType, "process.d");
    SipxProcessManager* processManager = SipxProcessManager::getInstance();
    processManager->instantiateProcesses(processDefinitionDirectory);

    // Create the SipxRpc service which manages xmlrpc requests
    pSipxRpcImpl = new SipxRpc(port, allowedPeers);
    pSipxRpcImpl->startRpcServer();

    doWaitLoop();

    // Successful run.
    return 0;
}
