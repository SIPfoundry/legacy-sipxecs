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
#include <signal.h>

// APPLICATION INCLUDES
#include "net/NameValueTokenizer.h"
#include "processcgi/processXMLCommon.h"
#include "WatchDog.h"
#include "os/OsSysLog.h"
#include "os/OsTask.h"
#include "EmailReporter.h"

//The worker who does all the checking... based on OsServerTask
WatchDog *pDog;

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// GLOBALS
int gnCheckPeriod = 10;
UtlBoolean gbDone = FALSE;
MonitoredProcess *gpProcessList[1000]; //should be enough  :)
int gnProcessCount = 0; //how many processes are we checking?
UtlString strWatchDogFilename;
UtlString strWatchDogPath = ".";
UtlString gEmailExecuteStr;


//retrieve the update rate from the xml file
OsStatus getSettings(TiXmlDocument &doc, int &rCheckPeriod)
{
    OsStatus retval = OS_FAILED;

    OsSysLog::add(FAC_WATCHDOG,PRI_INFO,"Getting check_period settings");

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
                    rCheckPeriod = atoi(pCheckStr);


                OsSysLog::add(FAC_WATCHDOG,PRI_INFO,"Getting email settings");

                TiXmlNode *emailitem = settingsItem->FirstChild("email");


                if ( emailitem != NULL )
                {
                    // This is actually an individual row
                    TiXmlElement *nextElement = emailitem->ToElement();


                    const char *pCommandStr = nextElement->Attribute("execute");
                    if ( pCommandStr )
                    {
                        gEmailExecuteStr = pCommandStr;

                        retval = OS_SUCCESS;
                    } else
                        OsSysLog::add(FAC_WATCHDOG,PRI_ERR,"Error getting email execute setting");
                } else
                    OsSysLog::add(FAC_WATCHDOG,PRI_ERR,"Error getting email settings");
            } else
                OsSysLog::add(FAC_WATCHDOG,PRI_ERR,"Error getting health settings");
        }else
            OsSysLog::add(FAC_WATCHDOG,PRI_ERR,"Error getting settings node");
    } else
            OsSysLog::add(FAC_WATCHDOG,PRI_ERR,"Error Couldn't get root element in getSettings!");

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
                       OsSysLog::add(FAC_WATCHDOG, PRI_CRIT,
                                     "initLogfile: Incomprehensible logging level string '%s'!\n",
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
                            OsSysLog::add(FAC_WATCHDOG, PRI_NOTICE, "Starting Watchdog");
                        else
                           // This is one case where we need to output an
                           // error message.
                           osPrintf("ERROR: initLogfile: Could not initialize SysLog!\n");
                    }

                }
            }
        }
        else
            OsSysLog::add(FAC_WATCHDOG,PRI_ALERT,"Couldn't get health element in init logfile");
    }
    else
        OsSysLog::add(FAC_WATCHDOG,PRI_ALERT,"Couldn't get root Element in init logfile");


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

                EmailReporter *pEmailReporter = NULL;

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

                    //now get & set the max reports value
                    const char *pMaxReportsStr = nextProcessElement->Attribute("max_reports");
                    if ( pMaxReportsStr )
                    {
                        processList[numProcesses]->setMaxReports(atoi(pMaxReportsStr));

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

                    //now get if report is enabled
                    const char *pReportEnableStr = nextProcessElement->Attribute("report");
                    if ( pReportEnableStr )
                    {
                        UtlBoolean enabled = FALSE;
                        if ( strcmp(pReportEnableStr,"enable") == 0 )
                            enabled = TRUE;

                        processList[numProcesses]->enableReports(enabled);

                    }


                    //now add reporters if enabled
                    if ( processList[numProcesses]->isReportsEnabled() )
                    {
                        for ( TiXmlNode *dbNode = dbProcessNode->FirstChild( "failure_contact" );
                            dbNode;
                            dbNode = dbNode->NextSibling( "failure_contact" ) )
                        {
                            TiXmlElement *nextContactElement = dbNode->ToElement();

                            UtlString contact;
                            const char*  payLoadData = nextContactElement->FirstChild()->Value();
                            if ( payLoadData )
                            {
                                contact = payLoadData;
                            }

                            const char *pMethodStr = nextContactElement->Attribute("method");
                            if ( pMethodStr )
                            {
                                if ( strcmp(pMethodStr,"email") == 0 )
                                {
                                    if ( pEmailReporter == NULL )
                                        pEmailReporter = new EmailReporter();

                                    pEmailReporter->addContact(contact);

                                }
                            }
                        }


                        //now add the reporter to our process object
                        pEmailReporter->setEmailExecuteCommand(gEmailExecuteStr);
                        processList[numProcesses]->AddReporter(pEmailReporter);
                    }

                    numProcesses++;

                }
                retval = OS_SUCCESS;
            }
        }
    }
    else
        OsSysLog::add(FAC_WATCHDOG,PRI_ALERT,"Couldn't get root Element in createProcessList");

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
                        rProcessXMLPath = strWatchDogPath;
                    retval = OS_SUCCESS;

                }
            }
        }
        else
            OsSysLog::add(FAC_WATCHDOG,PRI_ALERT,"Couldn't get settings Element in getProcessXMLPath");
    }
    else
        OsSysLog::add(FAC_WATCHDOG,PRI_ALERT,"Couldn't get root Element in getProcessXMLPath");

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


    OsSysLog::add(FAC_WATCHDOG,PRI_ALERT,"Execution Completed.");

    //cause main loop to exit
    gbDone = TRUE;
}

void clearStoppedProcesses()
{
    //now walk the processList and whack em
    for ( int loop = 0; loop < gnProcessCount; loop++ )
        gpProcessList[loop]->resetStoppedState();
}

void sig_routine(int sig)
{

    switch ( sig )
    {
    case SIGINT:
#ifdef DEBUG
       osPrintf("Execution USER ABORTED!\n");
#endif /* DEBUG */
       OsSysLog::add(FAC_WATCHDOG,PRI_ALERT,"Execution USER ABORTED!");
       break;
    case SIGABRT:
#ifdef DEBUG
       osPrintf("Execution ABORTED!\n");
#endif /* DEBUG */
       OsSysLog::add(FAC_WATCHDOG,PRI_ALERT,"Execution ABORTED!");
       break;
    case SIGTERM:
#ifdef DEBUG
       osPrintf("Execution TERMINATED!");
#endif /* DEBUG */
       OsSysLog::add(FAC_WATCHDOG,PRI_ALERT,"Execution TERMINATED!");
       break;
    case SIGKILL:
#ifdef DEBUG
       osPrintf("Execution KILLED!");
#endif /* DEBUG */
       OsSysLog::add(FAC_WATCHDOG,PRI_ALERT,"Execution KILLED!");
       break;
    default:
#ifdef DEBUG
       osPrintf("Received unexpected signal %d, shutting down.\n", sig);
#endif /* DEBUG */
       OsSysLog::add(FAC_WATCHDOG,PRI_EMERG,"UNEXPECTED SIGNAL: shutting down!");
       break;
    }
    gbDone = TRUE;
}

int main(int argc, char* argv[])
{
    UtlString outBuffer;
    UtlString commandStr;
    UtlString strVerb = "";
    UtlString gstrErrorMsg;
    UtlString processXMLPath = ".";

    UtlString argString;
    const char * sipxpbxuser = NULL;
    for (int argIndex = 1; argIndex < argc; argIndex++) 
    {
        bool usageExit = false;

        argString = argv[argIndex];
        NameValueTokenizer::frontBackTrim(&argString, "\t ");

        if (argString.compareTo("-h") == 0) 
        {
            usageExit = true;
        }
        else if (argString.compareTo("-u") == 0)
        {
            if (argIndex+1 >= argc)
            {
                // Missing the username.
                usageExit = true;
            }
            else
            {
                sipxpbxuser = argv[++argIndex];
            }
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
                osPrintf("WatchDog XML configuration set to %s\n", strWatchDogFilename.data());
            }
        }

        if (usageExit)
        {
            enableConsoleOutput(true);
            osPrintf("usage: %s [-h] [-u username]\n", argv[0]);
            osPrintf(" -h           Print this help and exit.\n");
            osPrintf(" -u username  Drop privileges down to the specified user.\n");
            osPrintf(" -f filename  Use the specified watchdog config file.\n");
            return 1;
        }
    }

    // Drop privileges down to the specified user.
    if (NULL != sipxpbxuser)
    {
        errno = 0;
        struct passwd * pwd = getpwnam(sipxpbxuser);
        if (NULL == pwd)
        {
            if (0 != errno)
            {
                OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "getpwnam(%s) failed, errno = %d.", sipxpbxuser, errno);
            }
            else
            {
                OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "getpwnam(%s) failed, user does not exist.", sipxpbxuser);
            }
        }
        else
        {
            errno = 0;
            if (0 == setuid(pwd->pw_uid))
            {
                OsSysLog::add(FAC_WATCHDOG, PRI_INFO, "Drop privileges with setuid() to '%s'.", sipxpbxuser);
            }
            else
            {
                OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "setuid(%d) failed, errno = %d.", (int)pwd->pw_uid, errno);
            }
        }
    }

    signal(SIGABRT,sig_routine);
    signal(SIGTERM,sig_routine);
    signal(SIGINT,sig_routine);

    //set our exit routine
    atexit(cleanup);

#ifdef DEBUG
    osPrintf("SIPxchange WatchDog - Copyright 2002 Pingtel Corp. All Rights Reserved.\n");
#endif /* DEBUG */


    // The location of the watchdog config file might have been supplied on the command-line.
    // Otherwise, check for the SIPX_HOME env var.  If it is set, the watchdog
    // config file is $SIPX_HOME/watchdog/etc/WatchDog.xml.
    // :TODO: That last file name is now incorrect.
    // Otherwise, the config file is SIPX_CONFDIR/WatchDog.xml.
    if (strWatchDogFilename.isNull())
    {
        //first thing we check is if the path to the processdef xml file is set
        char *pWatchDogXMLPathEnv = getenv("SIPX_HOME");
        if ( pWatchDogXMLPathEnv )
        {
            strWatchDogFilename = pWatchDogXMLPathEnv; //set it to the environment variable
            strWatchDogFilename += OsPath::separator;
            strWatchDogFilename += "watchdog";
            strWatchDogFilename += OsPath::separator;
            strWatchDogFilename += "etc";

            //save this path because we might need it later
            strWatchDogPath = strWatchDogFilename;
        }
        else
        {
            strWatchDogFilename = SIPX_CONFDIR;
        }

        strWatchDogFilename += OsPath::separator;
        strWatchDogFilename += "WatchDog.xml";
    }

#ifdef DEBUG
    osPrintf("Loading WatchDog XML from: %s\n",strWatchDogPath.data());
#endif /* DEBUG */

    TiXmlDocument processXMLDoc;
    TiXmlDocument watchdogXMLDoc;
//  TiXmlDocument AuthXMLDoc;

    if ( loadWatchDogXML(watchdogXMLDoc, strWatchDogFilename) == OS_SUCCESS )
    {
        if ( initLogfile(watchdogXMLDoc) == OS_SUCCESS )
        {
            if ( getProcessXMLPath(watchdogXMLDoc, processXMLPath) ==
                 OS_SUCCESS )
            {
#ifdef DEBUG
                osPrintf("Loading ProcessDefinitions XML from: %s\n",
                         processXMLPath.data());
#endif /* DEBUG */
                OsSysLog::add(FAC_WATCHDOG, PRI_INFO,
                              "Loading WatchDog XML from: %s\n",
                              strWatchDogPath.data());
                OsSysLog::add(FAC_WATCHDOG, PRI_INFO,
                              "Loading ProcessDefinitions XML from: %s\n",
                              processXMLPath.data());

                if ( initProcessXMLLayer(processXMLPath, processXMLDoc,
                                         gstrErrorMsg) ==
                     OS_SUCCESS )
                {

                    if ( getSettings(watchdogXMLDoc,gnCheckPeriod) ==
                         OS_SUCCESS )
                    {
                        if ( createProcessList(watchdogXMLDoc, processXMLDoc,
                                               gpProcessList, gnProcessCount) ==
                             OS_SUCCESS )
                        {
                            // This will remove and processes in the
                            // stopped state from the processAlias.dat file.
                            clearStoppedProcesses();

                            // Now create the "watchdog" which will
                            // monitor the process list.
#ifdef DEBUG
                            osPrintf("Process check occurs every %d seconds\n\n",gnCheckPeriod);
#endif /* DEBUG */
                            pDog = new WatchDog(gnCheckPeriod, gpProcessList,
                                                gnProcessCount);
                            pDog->start();
                            doWaitLoop();
                        } else
                        {
                           // These messages are also only generated at
                           // startup time.
                           // :TODO: All these error messages need to be sent
                           // to stderr.
                           // Note that "ProcessDefinitions.xml" is fixed as
                           // the last component of the process XML file name.
                           // See loadProcessXML in processCommon.cpp.
                           osPrintf("Couldn't load process list: Error in "
                                    "'%s' and/or '%s/ProcessDefinitions.xml'.\n",
                                    strWatchDogFilename.data(),
                                    processXMLPath.data());
                        }
                    } else
                    {
                        osPrintf("Couldn't get check period from xml file!\n");
                    }
                } else
                    osPrintf("Couldn't get logfile path from xml!\n");
            } else
            {
            	osPrintf("Couldn't init logfile!\n");
            }
        } else
	    osPrintf("Couldn't load watchdog XML file!\n");

    }
    // :TODO: This should exit success/failure as appropriate.
    return 0;
}
