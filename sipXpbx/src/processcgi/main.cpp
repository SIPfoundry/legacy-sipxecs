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
#include <string>
#include <memory>

// APPLICATION INCLUDES
#include "cgicc/Cgicc.h"
#include "os/OsTask.h"
#include "os/OsFS.h"
#include "processcgi/processXMLCommon.h"
#include "os/OsSysLog.h"

// MACROS
// EXTERNAL FUNCTIONS
extern OsStatus adjustProcessList(TiXmlDocument &doc);
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

UtlString gstrErrorMsg = "";
UtlBoolean bUnderApache = FALSE;

OsStatus initLogfile(TiXmlDocument &doc)
{
    OsStatus retval = OS_FAILED;
    OsSysLogPriority logging_level = (OsSysLogPriority)-1;

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
                        osPrintf("ERROR reading warning level!\n");
                    }


                }

                if ( logging_level >= 0 )
                {
                    const char*  payLoadData = nextElement->FirstChild()->Value();
                    if ( payLoadData )
                    {
                        retval = OsSysLog::initialize(0, "processcgi") ;
                        OsSysLog::setOutputFile(0, payLoadData) ;
                        OsSysLog::setLoggingPriority(logging_level);

                        if ( retval == OS_SUCCESS )
                            OsSysLog::add(FAC_PROCESSCGI,PRI_INFO,"Starting processcgi");
                        else
                            osPrintf("ERROR: Could not initialize SysLog!\n");

                    }

                }
            }
        }
    }
    else
        OsSysLog::add(FAC_PROCESSCGI,PRI_INFO,"Couldn't get root node in initLogFile for processCGI");


    return retval;   
}

int main(int argc, char* argv[])
{
    UtlString outBuffer;
    UtlString commandStr;
    UtlString strVerb = "";
    UtlString processXMLPath = SIPX_CONFDIR;  //no ending slash

    //first thing we check is if the path to the processdef xml file is set
    // char *pProcessXMLPathEnv = ;

    //multiple process alias may be sent to us.
    UtlString processAliasList[100];
    int aliasCount = 0;

    //for debugging.  comment out, compile, run in browser.
    //then attch with debugger to process.
    //    Sleep(30000);
    //    DebugBreak();
    TiXmlDocument processXMLDoc;

    // need to make sure that osPrint ends up on standard output
	enableConsoleOutput(true);
    if ( getenv("SERVER_NAME") != NULL ) // are we running as a cgi under a server?
    {
        try
        {
            // Create a new Cgicc object containing all the CGI data
            cgicc::Cgicc cgi;
        
            for( cgicc::const_form_iterator i = cgi.getElements().begin(); 
                 i != cgi.getElements().end();
                 ++i
                )
            {
                std::string key   = i->getName();
                std::string value = i->getValue();

                if ( key == "command" )
                {
                    commandStr = value.c_str();
                } 
                else if ( key == "process" )
                {
                    processAliasList[aliasCount++] = value.c_str();
                }
                else
                {
                    // :TBD: invalid argument - error out
                }
            }

//        else
//        {
//            gstrErrorMsg = "You must use:\n<BR>\ncommand=start process=aliasName (to start a process)\n<BR>"
//                           "command=stop process=aliasName (to stop a process)\n<BR>"
//                           "command=status (to see process status)\n<BR>";
//            osPrintf("Content-type: text/html\n\n");
//            osPrintf("%s\n",gstrErrorMsg.data());
//            return 0;
//        }
        } /* end try */
        catch( const std::exception& e)
        {
            /* print a mimimalist error message */
            osPrintf("Content-Type: text/plain\n\n");
            osPrintf("processcgi exception: %s", e.what());
            OsSysLog::add(FAC_PROCESSCGI,PRI_ERR,"ERROR: processcgi exception %s\n",
                          e.what());
        }
    } 
    else //we must not be under a server
    {
        if ( argc < 2 )
        {
            osPrintf("Syntax: processCGI start processname -path=/path/to/xml/file \nor\n");
            osPrintf("Syntax: processCGI stop processname -path=/path/to/xml/file\nor\n");
            osPrintf("Syntax: processCGI status -path=/path/to/xml/file\n");
            return 1;
        } else
        {
            commandStr = argv[1];
            if ( (commandStr == ACTION_START || commandStr == ACTION_RESTART || commandStr == ACTION_STOP) && argc > 2 )
                for ( int loop = 2;loop < argc;loop++ )
                {
                    //only add it if it isn't a parameter
                    if (strstr(argv[loop],"-") == NULL)
                        processAliasList[aliasCount++] = argv[loop];
                }
            //search for other parameters the user may have put
            for (int i = 2; i < argc;i++)
            {
                if (memcmp(argv[i],"-path=",6) == 0)
                    processXMLPath = argv[i]+6;
            }
        }

    }

    if ( initProcessXMLLayer(processXMLPath,processXMLDoc, gstrErrorMsg) == OS_SUCCESS )
    {
        if (initLogfile(processXMLDoc) == OS_SUCCESS)
        {
            if ( commandStr == ACTION_START || commandStr == ACTION_STOP || commandStr == ACTION_RESTART )
            {
                //now start each process alias passed to us

                for ( int loop = 0;loop < aliasCount;loop++ )
                {
                    OsSysLog::add(FAC_PROCESSCGI,PRI_WARNING,"User requested to %s process %s\n",
                          commandStr.data(),processAliasList[loop].data());


                    int waitingState = -1;
                    int userState = -1;
                    if (commandStr == ACTION_START)
                    {
                        waitingState = PROCESS_STARTED;
                        userState = USER_PROCESS_START;
                    }
                    else
                    if (commandStr == ACTION_RESTART)
                    {
                        waitingState = PROCESS_STARTED;
                        userState = USER_PROCESS_RESTART;
                    }
                    else
                    if (commandStr == ACTION_STOP)
                    {
                        waitingState = PROCESS_STOPPED;
                        userState = USER_PROCESS_STOP;
                    }

                    OsProcessMgr *pProcessMgr = OsProcessMgr::getInstance(SIPX_TMPDIR);
                    //wait up to 30 secs for process state to change
                    int secs = 0;
                    int currentState = 0;
                    bool bSuccess = false;
                    
                    
                    pProcessMgr->setUserRequestState(processAliasList[loop],userState);
                    while (secs < 30)
                    {
                        currentState = pProcessMgr->getAliasState(processAliasList[loop]);
                        osPrintf("Current State=%d Waiting State=%d\n",currentState,waitingState);
                        if (currentState == waitingState)
                        {
                            bSuccess = true;
                            break;
                        }
                        secs++;
                        OsTask::delay(1000);
                    }
                    //pass aliasname here
//                    if ( startstopProcessTree(processXMLDoc, processAliasList[loop],commandStr)== OS_SUCCESS )
                    if (bSuccess)
                    {
                        outBuffer += "SUCCESS: ";
                        outBuffer += processAliasList[loop];
                        outBuffer += " process ";
                        outBuffer += commandStr;
                        outBuffer += "<BR>\n";
                        OsSysLog::add(FAC_PROCESSCGI,PRI_WARNING,"SUCCESS for %s on process %s\n",
                                  commandStr.data(),processAliasList[loop].data());
                    } else
                    {
                        gstrErrorMsg += processAliasList[loop];
                        gstrErrorMsg += " process failed to ";
                        gstrErrorMsg += commandStr;
                        gstrErrorMsg += "<BR>\n";
                        OsSysLog::add(FAC_PROCESSCGI,PRI_ERR,"FAILED for %s on process %s\n",
                                  commandStr.data(),processAliasList[loop].data());
                    }

                    //adjust xml file based on results from execute
                    adjustProcessList(processXMLDoc);
                }
            }
            else
            if ( commandStr == ACTION_STATUS )
            {
                OsSysLog::add(FAC_PROCESSCGI,PRI_INFO,"User requested status\n");
            }
            else
            {
                gstrErrorMsg = "Unknown command: command=";
                gstrErrorMsg += commandStr;
                gstrErrorMsg += "\n";
                OsSysLog::add(FAC_PROCESSCGI,PRI_ERR,"ERROR: Unknown command: command %s\n",
                                  commandStr.data());
            }
        }
        else
        {
                gstrErrorMsg = "Unable to initialze logfile specified in XML file in dir ";
                gstrErrorMsg += processXMLPath;
                gstrErrorMsg += "\n";
        }
    }
    else
    {
        gstrErrorMsg = "Unable to initialze processXML located in dir ";
        gstrErrorMsg += processXMLPath;
        gstrErrorMsg += "\n";
    }

    if ( commandStr == "status" && gstrErrorMsg.isNull() )
        osPrintf("Content-type: text/xml\n\n");
    else
        osPrintf("Content-type: text/html\n\n");

    //if no error msg then output the text string which we built
    if ( gstrErrorMsg.isNull() || gstrErrorMsg == "" )
    {
        //output the xml file for the web server to pick up
        processXMLDoc.Print();

    } else
    {
        //we must have an error!
        osPrintf("ERROR: %s",gstrErrorMsg.data());
        OsSysLog::add(FAC_PROCESSCGI,PRI_ERR,"ERROR: %s\n",
                              gstrErrorMsg.data());
    }

    OsSysLog::flush();
    return 0;
}
