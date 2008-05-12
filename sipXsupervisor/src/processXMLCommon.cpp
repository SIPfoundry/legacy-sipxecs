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
#ifdef _WIN32
    #include <process.h>
#endif
#ifdef __pingtel_on_posix__
#endif

// APPLICATION INCLUDES
#include "processXMLCommon.h"
#include "os/OsFS.h"
#include "os/OsFileIteratorBase.h"
#include "os/OsTask.h"
#include "os/OsTokenizer.h"

// DEFINES
#define ACTION_START    "start"
#define ACTION_STOP     "stop"
#define ACTION_RESTART  "restart"
#define ACTION_STATUS   "status"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
void getProcessElements(
    const TiXmlElement &processElement,
    const char *pCommand,
    UtlBoolean& rbEnabled,
    UtlString& rStrExecute,
    UtlString& rStrParameters,
    UtlString& rStartupDir);

// GLOBALS
DependencyList processDepList[1000];
int processCount = 0;
//place to store child dependencies for each parent
UtlString childDepList[1000];
int childDepCount = 0;

//default wait if no delay is found
int gGlobalDependentDelay = 0;

//=====================  START OF FUNCTIONS FOR XML PARSING =====================
OsStatus
loadAuthorizationXML(TiXmlDocument &doc)
{
    OsStatus retval = OS_FAILED;
    return retval;
}

OsStatus
loadProcessXML(UtlString &rProcessXMLPath, TiXmlDocument &doc)
{
    OsStatus retval = OS_FAILED;
    UtlString rProcessXMLFullPath = rProcessXMLPath;
    rProcessXMLFullPath.append(OsPath::separator);
    // :TODO:  Extract this so that it is parameterized and can be overridden.
    // Need to put the file name into rProcessXMLPath and have it given to us
    // by our caller.  Also, an error message in main() in WatchDogMain.cpp
    // needs to know this file name.
    rProcessXMLFullPath.append("ProcessDefinitions.xml");

    if ( doc.LoadFile(rProcessXMLFullPath.data()) )
    {
        //        doc.Print();
       OsPath processDefsFilename(rProcessXMLFullPath);
       OsPath subdocDir = processDefsFilename.getDirName() + OsPath::separator + PROCESS_DIR;
       retval = findSubDocs(subdocDir, doc, &addProcessDefSubDoc);
    }


    return retval;
}

const char * const 
getProcessStatusString(int state)
{
    switch (state)
    {
    case PROCESS_STARTED:
        return "Started";
    case PROCESS_FAILED:
        return "Failed";
    case PROCESS_STOPPING:
        return "Stopping";    
    case PROCESS_STOPPED:
    case PROCESS_NEVERRUN:        
        return "Stopped";
    case PROCESS_STARTING:
        return "Starting";
    default:
        return "Unknown";
    }
}

OsStatus
adjustProcessList(TiXmlDocument &doc)
{
    OsStatus retval = OS_FAILED;

    OsProcessMgr *pProcessMgr = OsProcessMgr::getInstance();

    TiXmlElement*rootElement = doc.RootElement();
    if (rootElement)
    {
        for ( TiXmlNode *dbGroupNode = rootElement->FirstChild( "group" );
            dbGroupNode;
            dbGroupNode = dbGroupNode->NextSibling( "group" ) )
        {
            for ( TiXmlNode *dbNode = dbGroupNode->FirstChild( "process" );
                dbNode;
                dbNode = dbNode->NextSibling( "process" ) )
            {

                TiXmlElement *pElement = dbNode->ToElement();

                // Determine the DB to insert the items into
                OsProcess process;

                UtlString processAlias = dbNode->ToElement()->Attribute("name");

                TiXmlElement pidElement("pid");

                pProcessMgr->getProcessByAlias(processAlias, process);
                char pidbuf[PID_STR_LEN];
                sprintf(pidbuf,"%ld",(long)process.getPID());
                TiXmlText pidText(pidbuf);

                pElement->SetAttribute("status",getProcessStatusString(pProcessMgr->getAliasState(processAlias)));
                if (PROCESS_STARTED == pProcessMgr->getAliasState(processAlias))
                {
                    pidElement.InsertAfterChild(dbNode,pidText);
                    pElement->InsertAfterChild(dbNode,pidElement);
                }
            }
        }
    }

    retval = OS_SUCCESS;
    return retval;
}

OsStatus
VerifyProcess(UtlString &rAlias)
{
    OsStatus retval = OS_FAILED;

    for ( int loop = 0; loop < processCount;loop++ )
    {
        //check if this is the process they want
        if ( processDepList[loop].getName() == rAlias )
        {
            UtlString verifyCommand = processDepList[loop].getVerifyCommand();
            UtlString verifyParameters = processDepList[loop].getVerifyParameters();
            UtlString verifyDefaultDir = processDepList[loop].getVerifyDefaultDir();

            if ( verifyCommand.length() )
            {
                //break apart the command before calling spawnlp
                //copy to our workbuf before we parse
                char *workbuf = strdup(verifyParameters.data());
                int numargs = 0;
                pt_token_t *toklist = parse_tokenize(workbuf, &numargs);

                //now allocate space for arg list
                UtlString *args = new UtlString[numargs];

                for ( int loop3 = 0;loop3 < numargs;loop3++ )
                {
                    args[loop3] = parse_token(toklist, loop3);
                }
                parse_kill(toklist);
                free(workbuf);

                //this is the code which will wait for a test to return ok.
                OsProcess process;
                if ( process.launch(verifyCommand, &args[0], static_cast< OsPath& >(verifyDefaultDir) ) == OS_SUCCESS )
                {
                    //now wait around for the thing to finish
                    //it will wait up to delay
                    //0 means wait until it's finished
                    if ( process.wait(0) == 0 )
                    {
                        retval = OS_SUCCESS;
                    }
                }
                delete [] args;
            } else
            {
                //since there is no verify string, we just return OS_SUCCESS
                retval = OS_SUCCESS;
            }
            break;
        }
    }
    return retval;
}

OsStatus
BuildDependencyList(TiXmlDocument &doc)
{
    OsStatus retval = OS_SUCCESS;

    TiXmlElement*rootElement = doc.RootElement();
    if (rootElement)
    {
        for ( TiXmlNode *dbGroupNode = rootElement->FirstChild( "group" );
            dbGroupNode;
            dbGroupNode = dbGroupNode->NextSibling( "group" ) )
        {
            for ( TiXmlNode *dbNode = dbGroupNode->FirstChild( "process" );
                dbNode;
                dbNode = dbNode->NextSibling( "process" ) )
            {

                TiXmlElement *node = dbNode->ToElement();

                UtlString processAlias = node->ToElement()->Attribute("name");
                processDepList[processCount].setName(processAlias);

                UtlString strVerifyCommand, strVerifyParameters, strVerifyDefaultDir;
                UtlBoolean bEnabled;

                //now get the verify command
                getProcessElements(
                    *node, ACTION_VERIFY, 
                    bEnabled, strVerifyCommand, 
                    strVerifyParameters, strVerifyDefaultDir );

                if ( strVerifyCommand.length() )
                {
                    processDepList[processCount].setVerifyCommand(strVerifyCommand);
                    processDepList[processCount].setVerifyParameters(strVerifyParameters);
                    processDepList[processCount].setVerifyDefaultDir(strVerifyDefaultDir);
                }

                //now get the dependentdelay
                TiXmlNode *delayNode = node->FirstChild("dependentdelay");
                if ( delayNode != NULL )
                {
                    TiXmlElement * delayElement = delayNode->ToElement();
                    const char *pWait = delayElement->Attribute("wait");
                    if ( pWait )
                    {
                        processDepList[processCount].setDelay(atoi(pWait));

                    } else
                        processDepList[processCount].setDelay(gGlobalDependentDelay);

                }

                //now we need to find out if the user can start or stop this process
                UtlBoolean bCanStart = FALSE;
                UtlBoolean bCanStop = FALSE;
                UtlBoolean bCanRestart = FALSE;
                UtlString dummyStr;

                //use this func to see if it's enabled for startand stop
                getProcessElements(*node, "start", bCanStart, dummyStr, dummyStr, dummyStr);
                getProcessElements(*node, "stop", bCanStop, dummyStr, dummyStr, dummyStr);
                getProcessElements(*node, "restart", bCanRestart, dummyStr, dummyStr, dummyStr);
                processDepList[processCount].setCanStart(bCanStart);
                processDepList[processCount].setCanStop(bCanStop);
                processDepList[processCount].setCanRestart(bCanRestart);


                //now let get al the dependencies
                for ( TiXmlNode *dbNode = node->FirstChild( "dependency" );
                    dbNode;
                    dbNode = node->NextSibling( "dependency" ) )
                {

                    TiXmlElement *depElement = dbNode->ToElement();

                    if ( !depElement->NoChildren() )
                    {
                        const char* payLoadData = depElement->FirstChild()->Value();
                        if ( payLoadData )
                        {
                            UtlString tmpPayload = payLoadData;
                            processDepList[processCount].addDependent(tmpPayload);
                        }
                    }
                }
                processCount++;
            }
        }
    }

    return retval;
}

void
getProcessElements(
    const TiXmlElement &processElement,
    const char *pCommand,
    UtlBoolean& rbEnabled,
    UtlString& rStrExecute,
    UtlString& rStrParameters,
    UtlString& rStartupDir)
{
    TiXmlNode *commandNode = (TiXmlNode*)processElement.FirstChild(pCommand);
    if ( commandNode != NULL )
    {                   // This is actually an individual row
        TiXmlElement *element = commandNode->ToElement();
        const char *ptrControlElement  = element->Attribute("control");
        if ( ptrControlElement != NULL && memcmp(ptrControlElement,"true",4) == 0 )
        {
            rbEnabled = TRUE;
            //now get the execute element
            TiXmlNode *executeNode = element->FirstChild("execute");
            if ( executeNode != NULL )
            {
                TiXmlElement *executeElement = executeNode->ToElement();
                const char *ptrCommand = executeElement->Attribute("command");
                if ( ptrCommand )
                {
                    rStrExecute = ptrCommand;
                }
                const char *ptrParameters  = executeElement->Attribute("parameters");
                if ( ptrParameters )
                {
                    rStrParameters = ptrParameters;
                }
                const char *ptrDefaultDir  = executeElement->Attribute("defaultdir");
                if ( ptrDefaultDir )
                {
                    rStartupDir = ptrDefaultDir;
                    if ( rStartupDir == "" )
                        rStartupDir = ".";
                }
            }
        }
    }
}

OsStatus
startstopProcess(
    const TiXmlDocument &doc,
    const UtlString &rProcessToStart,
    const UtlBoolean bOnlyStop = FALSE,
    const UtlBoolean bShouldRestart = FALSE)
{
    OsStatus retval = OS_FAILED;

    //set to true if both bools passed in are false
    UtlBoolean bAskToStart = FALSE;

    OsProcessMgr *pProcessMgr = OsProcessMgr::getInstance();

    UtlBoolean bDone = FALSE;

    TiXmlElement* rootElement = (TiXmlElement*)doc.RootElement();
    if (rootElement)
    {
        TiXmlNode *pGroupNode = rootElement->FirstChild( "group" );
        if (pGroupNode)
        {
            while ( !bDone && pGroupNode )
            {
                for ( TiXmlNode *dbNode = pGroupNode->FirstChild( "process" );
                    !bDone && dbNode && pGroupNode;
                    dbNode = dbNode->NextSibling( "process" ) )
                {

                    TiXmlElement *processElement = dbNode->ToElement();

                    // Determine the DB to insert the items into
                    OsProcess process;

                    const char *pMsg = processElement->Attribute("name");
                    UtlString processAlias = pMsg;

                    if ( processAlias == rProcessToStart )
                    {
                        //first get all the redirect filenames
                        OsPath strRedirectOutput = "";
                        OsPath strRedirectError = "";
                        OsPath strRedirectInput = "";

                        //initialize these variables for this process
                        UtlBoolean bStartEnabled = FALSE;
                        UtlString  strStartExecute = "";
                        UtlString  strStartParams = "";
                        UtlString  strStartDir = "";

                        UtlBoolean bStopEnabled = FALSE;
                        UtlString  strStopExecute = "";
                        UtlString  strStopParams = "";
                        UtlString  strStopDir = "";

                        UtlBoolean bRestartEnabled = FALSE;
                        UtlString  strRestartExecute = "";
                        UtlString  strRestartParams = "";
                        UtlString  strRestartDir = "";

                        UtlString strVerifyCommand = "";

                        //get the stdout
                        TiXmlElement *redirectElementOut = processElement->FirstChild("stdout")->ToElement();
                        if ( redirectElementOut != NULL )
                        {                   // This is actually an individual row
                            const char *ptrRedirectFile  = redirectElementOut->Attribute("file");
                            if ( ptrRedirectFile )
                            {
                                strRedirectOutput = ptrRedirectFile;
                            }
                        }

                        //get the stderr
                        TiXmlElement *redirectElementErr = processElement->FirstChild("stderr")->ToElement();
                        if ( redirectElementErr != NULL )
                        {                   // This is actually an individual row
                            const char *ptrRedirectFile  = redirectElementErr->Attribute("file");
                            if ( ptrRedirectFile )
                            {
                                strRedirectError = ptrRedirectFile;
                            }
                        }

                        //get the stdin
                        TiXmlElement *redirectElementIn = processElement->FirstChild("stdin")->ToElement();
                        if ( redirectElementIn != NULL )
                        {                   // This is actually an individual row
                            const char *ptrRedirectFile  = redirectElementIn->Attribute("file");
                            if ( ptrRedirectFile )
                            {
                                strRedirectInput = ptrRedirectFile;
                            }
                        }

                        getProcessElements(*processElement,"start",bStartEnabled,strStartExecute,strStartParams,strStartDir);
                        getProcessElements(*processElement,"stop",bStopEnabled,strStopExecute,strStopParams,strStopDir);
                        getProcessElements(*processElement,"restart",bRestartEnabled,strRestartExecute,strRestartParams,strRestartDir);\

                        UtlString processString;
                        UtlString paramString;
                        UtlString changeDir;
                        UtlBoolean bOkToExecute = TRUE;


                        if ( (strStartExecute.length() && !bOnlyStop && !bShouldRestart) ||
                             (strStopExecute.length() && bOnlyStop) || bShouldRestart )
                            pProcessMgr->setIORedirect(strRedirectInput,strRedirectOutput,strRedirectError);

                        //the user is asking to restart by execute
                        if ( bShouldRestart && !strRestartExecute.length() )
                        {
                            pProcessMgr->stopProcess(processAlias);
                            processString = strStartExecute;
                            paramString = strStartParams;
                            changeDir = strStartDir;
                        } else
                            if ( bShouldRestart && strRestartExecute.length() )
                        {
                            pProcessMgr->stopProcess(processAlias);
                            processString = strRestartExecute;
                            paramString = strRestartParams;
                            changeDir = strRestartDir;
                        } else
                            if ( bOnlyStop && bStopEnabled && strStopExecute.length() )
                        {
                            processString = strStopExecute;
                            paramString = strStopParams;
                            changeDir = strStopDir;
                        } else
                            if ( bOnlyStop && bStopEnabled && !strStopExecute.length() )
                        {
                            if ( pProcessMgr->stopProcess(processAlias) == OS_SUCCESS )
                            {
                                retval = OS_SUCCESS;
                                pProcessMgr->setAliasStopped(processAlias);
                            }
                            bOkToExecute = FALSE;
                        } else
                        {
                            processString = strStartExecute;
                            paramString = strStartParams;
                            changeDir = strStartDir;
                            bAskToStart = TRUE;
                        }

                        //break apart single param string into many
                        //copy to our workbuf before we parse
                        char *workbuf = strdup(paramString.data());
                        int numargs = 0;
                        pt_token_t *toklist = parse_tokenize(workbuf, &numargs);

                        //now allocate space for arg list
                        UtlString *args = new UtlString[numargs+1];

                        for ( int loop = 0;loop < numargs;loop++ )
                        {
                            args[loop] = parse_token(toklist, loop);
                        }
                        parse_kill(toklist);
                        free(workbuf);

                        if ( bAskToStart
                            && pProcessMgr->getAliasState(processAlias) == PROCESS_STOPPING )
                        {
                            OsSysLog::add(FAC_PROCESSMGR, PRI_WARNING,
                                          "ProcessCommon asked to start"
                                          " but process is in STOPPING STATE  %s",
                                          processAlias.data());
                            retval = OS_FAILED;
                        }
                        else if ( bAskToStart
                                 && pProcessMgr->getAliasState(processAlias) == PROCESS_STARTED )
                        {
                            OsSysLog::add(FAC_PROCESSMGR, PRI_INFO,
                                          "ProcessCommon PROCESS ALREADY STARTED %s",
                                          processAlias.data());
                            retval = OS_SUCCESS;
                        }
                        else if ( bOkToExecute)
                        {
                            retval =  pProcessMgr->startProcess(processAlias, processString,
                                                                args,changeDir);
                            if (retval == OS_SUCCESS)
                            {
                               OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,
                                             "ProcessCommon SUCCESS STARTING process %s",
                                             processAlias.data());
                            }
                            else
                            {
                               OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,
                                             "ProcessCommon ERROR STARTING process %s",
                                             processAlias.data());
                            }
                            
                            if ( bOnlyStop )
                            {
                               pProcessMgr->setAliasStopped(processAlias);
                            }
                        }

                        delete [] args;

                        bDone = TRUE;
                    }
                }
                pGroupNode = pGroupNode->NextSibling( "group" );
            }
        }
        else
            osPrintf("Couldn't load group node !");
    }
    else
        osPrintf("Couldn't load root element !");

    return retval;
}


/*
//Used to write out an XML formatted process list
void WriteNodeList(TiXmlNode *in_node, int level, UtlString &buffer)
{
    for( TiXmlNode *dbNode = in_node; dbNode; dbNode = dbNode->NextSibling())
    {

        TiXmlElement *node = dbNode->ToElement();

        char *name = node.getNodeName().transcode();
        char *value = node.getNodeValue().transcode();

        if ( memcmp(name,"#text",5) != 0 && memcmp(name,"#comment",5) != 0 )
        {
            //add level spacing
            for ( int loop = 0; loop < level;loop++ )
                buffer.append("   ");
            buffer.append("<");
            buffer.append(name);

            DOM_NamedNodeMap attrList = node.getAttributes();
            for ( int j = 0;attrList != NULL && j < attrList.getLength();j++ )
            {
                buffer.append(" ");

                DOM_Element attr =
                static_cast<const DOM_Element &>(attrList.item(j));

                char *localname = attr.getLocalName().transcode();
                char *name = attr.getNodeName().transcode();
                char *value = attr.getNodeValue().transcode();

                if ( memcmp(name,"#text",5) != 0 && memcmp(name,"#comment",5) != 0 )
                {
                    buffer.append(name);
                    buffer.append("=\"");
                    buffer.append(value);
                    buffer.append("\"");
                }

            }
            buffer.append(">");

            if ( node.hasChildNodes() )
            {
                char*  payLoadData = node.getFirstChild().getNodeValue().transcode();
                if ( payLoadData )
                {
                    UtlString tmpPayload = payLoadData;
                    buffer.append(tmpPayload);
#ifdef __pingtel_on_posix__
                    delete [] payLoadData;
#endif
                }
                WriteNodeList(node->FirstChildElement(),level+1,buffer);
            }

            for ( int loop2 = 0; loop2 < level;loop2++ )
                buffer.append("   ");

            buffer.append("</");
            buffer.append(name);
            buffer.append(">\n");
        }

#ifdef __pingtel_on_posix__
        delete [] name;
        delete [] value;
#endif
    }
}



OsStatus WriteProcessXML(TiXmlDocument &doc, UtlString &buffer)
{
    OsStatus retval = OS_FAILED;
    DOM_NodeList nodeItems = doc.getChildNodes();

    //recursive func.  must pass 0 as second param
    //this func will fill buffer (UtlString) with the text of the XML file.
    WriteNodeList(nodeItems,0,buffer);

    return retval;
}
*/

OsStatus
verifyUser(TiXmlDocument &doc, UtlString &rUsername, UtlString &rPassword)
{
    OsStatus retval = OS_FAILED;

    TiXmlElement*rootElement = doc.RootElement();
    if (rootElement)
    {

        for ( TiXmlNode *dbNode = rootElement->FirstChild( "user" );
            dbNode;
            dbNode = dbNode->NextSibling( "user" ) )
        {

            TiXmlElement *nextItemsContainer = dbNode->ToElement();


            // Determine the DB to insert the items into
            OsProcess process;
            const char *pUsername = nextItemsContainer->Attribute("name");
            const char *pPassword = nextItemsContainer->Attribute("password");
            UtlString strUsername = pUsername;
            UtlString strPassword = pPassword;
            if ( rUsername == strUsername && rPassword == strPassword )
                retval = OS_SUCCESS;

        }
    }

    // FOR DEBUGGING JUST RETURN OS_SUCCESS
    retval = OS_SUCCESS;
    return retval;
}

OsStatus getGlobalDependentDelay(TiXmlDocument &doc)
{
    OsStatus retval = OS_FAILED;

    TiXmlElement*rootElement = doc.RootElement();
    if (rootElement)
    {
        TiXmlNode *delaynode = rootElement->FirstChild( "dependentdelay" );


        if ( delaynode != NULL )
        {
            TiXmlElement *nextItemsContainer = delaynode->ToElement();

            // Determine the DB to insert the items into
            const char *pDelay = nextItemsContainer->Attribute("wait");
            if ( pDelay )
            {

                gGlobalDependentDelay = atoi(pDelay);
                retval = OS_SUCCESS;

            }
        } else
            printf("ERROR: delay not set in xml file!\n");
    }
    else
        OsSysLog::add(FAC_WATCHDOG,PRI_ALERT,"Couldn't get root Element in getGlobalDependentDelay");


    return retval;
}

/**
 * Returns true if the the specified alias can be matched to a process, and that
 * process can undergo the specified state change.  Returns false otherwise.
 * The state must be one of: 
 *   - USER_PROCESS_START
 *   - USER_PROCESS_STOP
 *   - USER_PROCESS_RESTART
 */
bool canProcessStateChange(const UtlString& alias, const int state)
{
    bool bDone = false;
    bool bResult = false;
    
    for (int loop=0; !bDone && loop < processCount; loop++)
    {
        if (processDepList[loop].getName() == alias)
        {
            bDone = true;
            switch (state)
            {
            case USER_PROCESS_START:
                bResult = processDepList[loop].getCanStart();                
                break;
                
            case USER_PROCESS_STOP:
                bResult = processDepList[loop].getCanStop();                
                break;
                
            case USER_PROCESS_RESTART:
                bResult = processDepList[loop].getCanRestart();                
                break;
            }
        }
    }
    
    return bResult;
}

//gets the dependents who can do the specified action command
void GetDependents(UtlString &rParentProcess, UtlString actionStr)
{
    UtlBoolean bDone = FALSE;

    for ( int loop = 0;!bDone && loop < processCount;loop++ )
    {
        if ( (actionStr == "start" && processDepList[loop].getCanStart()) ||
             (actionStr == "stop" && processDepList[loop].getCanStop()) ||
             (actionStr == "restart" && processDepList[loop].getCanRestart()) )
        {
            if ( processDepList[loop].getName() == rParentProcess )
            {

                int numDep = processDepList[loop].getDependencyCount();

                for ( int loop2 = 0;loop2 < numDep;loop2++ )
                {
                    UtlString depStr;
                    processDepList[loop].getDependency(loop2,depStr);

                    //now check if this child is already in the list of children
                    UtlBoolean bOkToAdd = TRUE;
                    for ( int child = 0;child < childDepCount;child++ )
                    {
                        if ( childDepList[child] == depStr )
                        {
                            bOkToAdd = FALSE;
                            break;
                        }
                    }

                    //now check if this child has the action disabled
                    for ( int loop3 = 0;bOkToAdd && loop3 < processCount;loop3++ )
                    {
                        if ( processDepList[loop3].getName() == depStr )
                        {
                            if ( (actionStr == "start" && !processDepList[loop3].getCanStart()) ||
                                 (actionStr == "stop" && !processDepList[loop3].getCanStop()) ||
                                 (actionStr == "restart" && !processDepList[loop3].getCanRestart()) )
                            {
                                bOkToAdd = FALSE;
                            }
                        }
                    }

                    //if still ok to add, then add it
                    if ( bOkToAdd )
                    {
                        childDepList[childDepCount] = depStr;
                        childDepCount++;
                        GetDependents(depStr,actionStr);
                    }
                }

                bDone = TRUE;
            }
        }
    }
}


//this function just delays for the required amount for the process
void dependentDelay(UtlString &rAlias)
{
    for ( int loop = 0; loop < processCount;loop++ )
    {
        //check if this is the process they want
        if ( processDepList[loop].getName() == rAlias )
        {
            int delay = processDepList[loop].getDelay();

            if ( delay == 0 )
                OsTask::delay(gGlobalDependentDelay*1000);
            else
                OsTask::delay(delay*1000);

        }
    }
}

OsStatus startstopProcessTree(TiXmlDocument &rProcessXMLDoc, UtlString &rProcessAlias, UtlString &rActionVerb)
{
    UtlBoolean bOnlyStop = FALSE;
    UtlBoolean bOnlyRestart = FALSE;

    OsStatus retval = OS_FAILED;

    //now, depending on which process is requested, do the right thing
    if ( rActionVerb == ACTION_STOP )
    {
        bOnlyStop = TRUE;
    } else
        if ( rActionVerb == ACTION_RESTART )
    {
        bOnlyRestart = TRUE;
    } else
        if ( rActionVerb == ACTION_START )
    {
        bOnlyStop = FALSE;
        bOnlyRestart = FALSE;
    } else
        return retval; //I hate doing this here but it works better.

    //first we must look for any dependencies
    childDepCount = 0;

    //also, we need to add the parent to the top of the list
    childDepList[childDepCount++] = rProcessAlias;

    //ok now find ALL its children
    GetDependents(rProcessAlias,rActionVerb);

    //now the global childDepCount has the total
    //number of children.
    //we can now start executing

    OsProcessMgr *pProcessMgr = OsProcessMgr::getInstance();

    for ( int loop2 = childDepCount-1;loop2 >=0 ; loop2-- )
    {
        int last_state = pProcessMgr->getAliasState(childDepList[loop2]);
        if ( startstopProcess(rProcessXMLDoc,childDepList[loop2],bOnlyStop,bOnlyRestart) == OS_SUCCESS )
        {
            UtlString verifyCommand;

            for ( int loop = 0; loop < processCount;loop++ )
            {
                //check if this is the process they want
                if ( processDepList[loop].getName() == childDepList[loop2] )
                {
                    verifyCommand = processDepList[loop].getVerifyCommand();
                    break;
                }
            }

            if ( verifyCommand.length() )
            {
                retval = VerifyProcess(childDepList[loop2]);
                if ( retval != OS_SUCCESS )
                {
                    //failed verify but it's still running so we need to kill it
                    // Log why we are killing this process at NOTICE level,
                    // because OsProcess::kill will log at NOTICE level.
                    OsSysLog::add(FAC_PROCESSMGR, PRI_NOTICE,
                                  "Process %s was started successfully, "
                                  "but VerifyProcess fails.  Killing process...",
                                  childDepList[loop2].data());
                    OsProcess process;
                    pProcessMgr->getProcessByAlias(childDepList[loop2],process);
                    process.kill();
                }
            } else
            {
                //if we have no command, then we should just wait for delay
                //but we should only delay if the state before attemping start was
                //anything but STARTED
                if ( last_state != PROCESS_STARTED )
                    dependentDelay(childDepList[loop2]);

                retval = OS_SUCCESS;
            }
        }
    }

    return retval;
}

//initialize all structures used for process management
OsStatus initProcessXMLLayer(UtlString &rProcessXMLPath,  TiXmlDocument &rProcessXMLDoc, UtlString &rStrErrorMsg)
{
    OsStatus retval = OS_FAILED;

    //clear out error message
    rStrErrorMsg = "";

    //load process xml template
    if ( loadProcessXML(rProcessXMLPath, rProcessXMLDoc) == OS_SUCCESS )
    {
            //for all processes, grab their dependencies
            if ( getGlobalDependentDelay(rProcessXMLDoc) == OS_SUCCESS )
            {
                if ( BuildDependencyList(rProcessXMLDoc) == OS_SUCCESS )
                {
                    //adjust it to add pid and status
                    if ( adjustProcessList(rProcessXMLDoc) == OS_SUCCESS )
                    {
                        //        rProcessXMLDoc.Print();  //TESTING
                        retval = OS_SUCCESS;
                    } else
                        rStrErrorMsg =  "Error adjusting process list!\n";
                } else
                    rStrErrorMsg =  "Error getting dependencies!\n";
            } else
                rStrErrorMsg = "Couldn't set dependent delay!\n";

    } else
        rStrErrorMsg =  "Error loading process xml file!\n";

    return retval;
}

OsStatus findSubDocs(OsPath &path, TiXmlDocument &rootDoc, ProcessSubDoc addSubDoc)
{
    OsFileIterator subdocs(path);
    OsPath subdocName;
    OsStatus status = subdocs.findFirst(subdocName, "[.]process[.]xml$");
    while (status == OS_SUCCESS && subdocName.length() > 0) {
        TiXmlDocument subdoc;
        OsPath pathCopy(path);
        // iterator doesn't appear to load full path, it's just the file name
        bool success = subdoc.LoadFile(pathCopy + OsPath::separator + subdocName);
        if (!success) {
            // failed to load the process xml file.  Log the issue, skip over it and continue.
            OsSysLog::add(FAC_WATCHDOG,PRI_ALERT,"Failed to load process xml file %s", subdocName.data());

            status = subdocs.findNext(subdocName);
        } else {
            TiXmlElement *subroot = subdoc.RootElement();
            if (subroot != NULL) {
               // Only deal with enabled files
               // Non-enabled files are skipped as if they don't exist
               const char *enableString = subroot->Attribute("enable");
               if (enableString == NULL || strcmp(enableString, "true")==0)
               {
                  status = (*addSubDoc)(rootDoc, subdoc);
               }
            } else {
               // Invalid document format.  Log the issue and continue to the next process xml file.
               OsSysLog::add(FAC_WATCHDOG,PRI_ALERT,"Invalid process xml file %s", subdocName.data());
            }
            if (status == OS_SUCCESS) 
            {
               status = subdocs.findNext(subdocName);
            }
        }
    }

    return status == OS_FILE_NOT_FOUND ? OS_SUCCESS : status;
}

OsStatus addWatchDogSubDoc(TiXmlDocument &rWatchDogXMLDoc, TiXmlDocument &subdoc)
{
    OsStatus status = OS_FAILED;

    // navigate child doc
    TiXmlElement *subroot = subdoc.RootElement();
    if (subroot != NULL) 
    {
        TiXmlNode *subwatchdog = subroot->FirstChild("watchdog");
        if (subwatchdog != NULL) 
        {
            TiXmlNode *submonitor = subwatchdog->FirstChild("monitor");
            if (submonitor != NULL) 
            {
                // navigate root doc
                TiXmlElement *root = rWatchDogXMLDoc.RootElement();
                if (root != NULL) 
                {
                    TiXmlNode *monitor = root->FirstChild("monitor");
                    if (monitor != NULL) 
                    {
                        // copy
                        for (TiXmlNode *child = submonitor->FirstChild("monitor-process");
                              child != NULL;
                              child = child->NextSibling( "monitor-process" ) )
                        {
                            // according to tinyxml docs clone should be free'ed
                            // but watchdog doc is never free'ed so leave as is
                            TiXmlNode *clone = child->Clone();
                            monitor->LinkEndChild(clone);
                            status = OS_SUCCESS;
                        }
                    }
                }
            }
        }
    }

    return status;
}

OsStatus addProcessDefSubDoc(TiXmlDocument &rProcessXMLDoc, TiXmlDocument &subdoc)
{
    OsStatus status = OS_FAILED;

    // navigate child doc
    TiXmlElement *subroot = subdoc.RootElement();
    if (subroot != NULL) 
    {
        TiXmlNode *subproc_defs = subroot->FirstChild("process_definitions");
        if (subproc_defs != NULL) 
        {
            // navigate root doc
            TiXmlElement *root = rProcessXMLDoc.RootElement();
            if (root != NULL) 
            {
                // copy
                for (TiXmlNode *child = subproc_defs->FirstChild("group");
                      child != NULL;
                      child = child->NextSibling( "group" ) )
                {
                    // according to tinyxml docs clone should be free'ed
                    // but process doc is never free'ed so leave as is
                    TiXmlNode *clone = child->Clone();
                    root->LinkEndChild(clone);
                    status = OS_SUCCESS;
                }
            }
        }
    }

    return status;
}
