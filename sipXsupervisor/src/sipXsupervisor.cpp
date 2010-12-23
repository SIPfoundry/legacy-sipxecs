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
#include "os/OsSSLServerSocket.h"
#include "net/XmlRpcDispatch.h"
#include "sipXecsService/SipXecsService.h"
#include "SipxRpc.h"
#include "HttpFileAccess.h"
#include "AlarmServer.h"
#include "SipxProcessManager.h"
#include "config.h"

#define DEBUG

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* CONFIG_SETTINGS_FILE = "sipxsupervisor-config";
const char* SUPERVISOR_PID_FILE = "sipxsupervisor.pid";
const int   DEFAULT_SUPERVISOR_PORT = 8092;
const char* SUPERVISOR_PREFIX = "SUPERVISOR";    // prefix for config file entries
const char* SUPERVISOR_HOST = "SUPERVISOR_HOST";
const char* SUPERVISOR_LOG_LEVEL = "SUPERVISOR_LOG_LEVEL";
const char* EXIT_KEYWORD = "exit";

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
int supervisorMain(bool bOriginalSupervisor);
void upgradeFrom3_0();

// GLOBALS
UtlBoolean gbDone = FALSE;
UtlBoolean gbShutdown = FALSE;
SipxRpc *pSipxRpcImpl;           // The worker who manages xmlrpc requests
int fdin[2];                     // stdin pipe between supervisor and supervisor-in-waiting


void cleanup()
{
    // tell the supervisor-in-waiting to exit nicely
    ssize_t rc = write(fdin[1], EXIT_KEYWORD, strlen(EXIT_KEYWORD));
    if ( rc < 0 )
    {
       osPrintf("Failed to write to supervisor-in-waiting, errno %d\n", errno);
    }
    rc = write(fdin[1], "\n", 1);

    // Stop handling xmlrpc requests
    if ( pSipxRpcImpl )
    {
        delete pSipxRpcImpl;
        pSipxRpcImpl = NULL;
    }

    // Shut down all processes
    SipxProcessManager::getInstance()->shutdown();

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

    case SIGHUP:
    {
#ifdef DEBUG
       osPrintf("Received SIGHUP; ignoring\n");
#endif /* DEBUG */
       OsSysLog::add(FAC_SUPERVISOR,PRI_ALERT,"Received SIGHUP; ignoring");
       return;
    }

    case SIGTERM:
    {
#ifdef DEBUG
       osPrintf("Execution TERMINATED!\n");
#endif /* DEBUG */
       OsSysLog::add(FAC_SUPERVISOR,PRI_ALERT,"Execution TERMINATED!");
       gbShutdown = true;
       return;
       break;
    }

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
   SignalTask() : OsTask("SignalTask") {}

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

void writePidToFile()
{
   UtlString pidFileName = SipXecsService::Path(SipXecsService::RunDirType, SUPERVISOR_PID_FILE);
   char pidString[1024];
   sprintf(pidString, "%ld\n", (long)getpid());
   OsFile pidFile(pidFileName);
   if ( OS_SUCCESS == pidFile.open(OsFile::CREATE) )
   {
      size_t bytesWritten;
      if ( OS_SUCCESS != pidFile.write(pidString, strlen(pidString), bytesWritten) )
      {
         osPrintf("sipXsupervisor: could not write pidFile %s\n", pidFileName.data());
      }
      pidFile.close();
   }
   else
   {
      osPrintf("sipXsupervisor: could not open pidFile %s\n", pidFileName.data());
   }
}

/// Fork another copy of the supervisor which will do nothing unless/until the parent dies,
/// in which case it will take over by calling main again
void forkSupervisorInWaiting()
{
    // create pipes between the parent and child for stdin
    if ( pipe(fdin) < 0 )
    {
       osPrintf("Failed to create pipe for supervisor, errno %d",
                     errno);
       return;
    }

    //now fork into two processes
    pid_t forkReturnVal = fork();
    switch (forkReturnVal)
    {
        case -1 : //retval = OS_FAILED;
                  break;

        case 0 :
        {
                  // this is the child process
                  // Dupe the stdin pipe, then read from it.
                  // If we ever return from read with EOF, the parent has died
                  // so it's time for the child to take over

                  // child closes the writing end of the stdin pipe
                  close(fdin[1]);

                  // replace stdin with read part of the pipe
                  if (dup2(fdin[0], STDIN_FILENO) < 0)
                  {
                     osPrintf("Failed to dup2 pipe for supervisor, errno %d!\n",
                     errno);
                  }

                  ssize_t rc;
                  char recvbuf[1024];
                  while ( (rc = read(fdin[0], recvbuf, sizeof(recvbuf))) > 0 )
                  {
                     if ( !strncasecmp(recvbuf, EXIT_KEYWORD, strlen(EXIT_KEYWORD)) )
                     {
                        _exit(EXIT_SUCCESS);
                     }
                  }

                  // now the supervisor-in-waiting ascends to the throne
                  // Write this process's pid into the supervisor pidfile (it is a child or grandchild)
                  writePidToFile();

                  OsTask::delay(20000);
                  supervisorMain(false);
                  break;
        }
        default : // this is the parent process

                  // parent closes the reading end of the stdin pipe
                  close(fdin[0]);
                  //retval = OS_SUCCESS;
                  break;
    }
}

int main(int argc, char* argv[])
{
   // All osPrintf output should go to the console until the log file is initialized.
   enableConsoleOutput(true);

   UtlString argString;

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
           osPrintf("sipxsupervisor %s\n\n", VERSION);
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

    // Write this process's pid into the supervisor pidfile
    writePidToFile();

    return supervisorMain(true);
}

int supervisorMain(bool bOriginalSupervisor)
{
    // Create forked process which will do nothing unless parent dies.  Parent continues with initialization.
    forkSupervisorInWaiting();

    // Drop privileges down to the specified user & group
    const char * sipxpbxuser = SipXecsService::User();
    const char * sipxpbxgroup = SipXecsService::Group();

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

    // Block all signals in this the main thread
    // Any threads created after this will have all signals masked.
    OsTask::blockSignals();

    // Create a new task to wait for signals.  Only that task
    // will ever see a signal from the outside.
    SignalTask* signalTask = new SignalTask();
    signalTask->start() ;

    // All osPrintf output should go to the console until the log file is initialized.
    enableConsoleOutput(true);

    // Initialize the log file.
    OsSysLog::initialize(0, "Supervisor") ;
    UtlString logFile = SipXecsService::Path(SipXecsService::LogDirType, "sipxsupervisor.log");
    OsSysLog::setOutputFile(0, logFile) ;
    OsSysLog::setLoggingPriority(PRI_DEBUG);
    if (!bOriginalSupervisor)
    {
       OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT,
                     "Restarting sipxsupervisor after unexpected shutdown");
    }
    OsSysLog::add(FAC_SUPERVISOR, PRI_NOTICE,
                  ">>>>> Starting sipxsupervisor version %s",
                  VERSION);

    // Now that the log file is initialized, stop sending osPrintf to the console.
    // All relevant log messages from this point on must use OsSysLog::add().
    enableConsoleOutput(false);
    fflush(NULL); // Flush all output so children don't get a buffer of output

    // Open the supervisor configuration file
    OsConfigDb supervisorConfiguration;
    OsPath supervisorConfigPath = SipXecsService::Path(SipXecsService::ConfigurationDirType,
                                                       CONFIG_SETTINGS_FILE);
    if (OS_SUCCESS != supervisorConfiguration.loadFromFile(supervisorConfigPath.data()))
    {
       OsSysLog::add(FAC_SUPERVISOR,PRI_WARNING,
                     "Failed to open supervisor configuration file at '%s'",
                     supervisorConfigPath.data()
                     );
    }

    // Set logging based on the supervisor configuration - TODO change default to "NOTICE" ?
    OsSysLogPriority logPri = SipXecsService::setLogPriority(supervisorConfiguration, SUPERVISOR_PREFIX, PRI_INFO );
    OsSysLog::setLoggingPriorityForFacility(FAC_ALARM, logPri);

    // Open the domain configuration file
    OsConfigDb domainConfiguration;
    OsPath domainConfigPath = SipXecsService::domainConfigPath();
    if (OS_SUCCESS != domainConfiguration.loadFromFile(domainConfigPath.data()))
    {
       OsSysLog::add(FAC_SUPERVISOR,PRI_ERR,
                     "Failed to open domain configuration '%s'",
                     domainConfigPath.data()
                     );
    }
    // @TODO const char* managementIpBindAddress;
    int managementPortNumber;
    managementPortNumber = domainConfiguration.getPort(SipXecsService::DomainDbKey::SUPERVISOR_PORT);
    if (PORT_NONE == managementPortNumber)
    {
       OsSysLog::add(FAC_SUPERVISOR,PRI_WARNING,
                     "%s not configured in '%s', using default: %d",
                     SipXecsService::DomainDbKey::SUPERVISOR_PORT,
                     domainConfigPath.data(), DEFAULT_SUPERVISOR_PORT
                     );
       managementPortNumber=DEFAULT_SUPERVISOR_PORT;
    }
    else if (PORT_DEFAULT == managementPortNumber)
    {
       OsSysLog::add(FAC_SUPERVISOR,PRI_NOTICE,"%s set to default: %d",
                     SipXecsService::DomainDbKey::SUPERVISOR_PORT, DEFAULT_SUPERVISOR_PORT
                     );
       managementPortNumber=DEFAULT_SUPERVISOR_PORT;
    }

    UtlSList allowedPeers;
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
                           "%s value '%s'",
                           SipXecsService::DomainDbKey::CONFIG_HOSTS,
                           hostName.data()
                           );
             allowedPeers.insert(new UtlString(hostName));
          }
       }
    }
    else
    {
       OsSysLog::add(FAC_SUPERVISOR,PRI_ERR,
                     "%s not configured in '%s'",
                     SipXecsService::DomainDbKey::CONFIG_HOSTS, domainConfigPath.data()
                     );
    }

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
                     "%s not configured in '%s'",
                     SUPERVISOR_HOST, supervisorConfigPath.data()
                     );
    }

    if (allowedPeers.isEmpty())
    {
       OsSysLog::add(FAC_SUPERVISOR,PRI_ERR,
                     "No configuration peers configured.");
    }

    if (!cAlarmServer::getInstance()->init())
    {
       OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
             "sipXsupervisor failed to init AlarmServer");
    }

    // Initialize management interfaces on the TLS socket
    OsSSLServerSocket serverSocket(50, managementPortNumber /*@TODO managementIpBindAddress */);
    HttpServer        httpServer(&serverSocket); // set up but don't start https server
    XmlRpcDispatch    xmlRpcDispatcher(&httpServer); // attach xml-rpc service to https
    pSipxRpcImpl = new SipxRpc(&xmlRpcDispatcher, allowedPeers); // register xml-rpc methods
    HttpFileAccess    fileAccessService(&httpServer, pSipxRpcImpl); // attach file xfer to https

    if (serverSocket.isOk())
    {
       OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG, "Starting Management HTTP Server");
       httpServer.start();
    }
    else
    {
       OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "Management listening socket failure");
    }

    // Read the process definitions.
    UtlString processDefinitionDirectory =
       SipXecsService::Path(SipXecsService::DataDirType, "process.d");
    SipxProcessManager* processManager = SipxProcessManager::getInstance();
    processManager->instantiateProcesses(processDefinitionDirectory);

    // 3.0 had different process def files.  The only important thing in them is the
    // state of the services.  Transfer this state to the new process def files.
    upgradeFrom3_0();

    doWaitLoop();

    // Successful run.
    return 0;
}
