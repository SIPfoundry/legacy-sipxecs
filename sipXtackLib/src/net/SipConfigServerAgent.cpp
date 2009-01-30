//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <net/SipConfigServerAgent.h>
#include <net/SipUserAgent.h>
#include <os/OsConfigDb.h>
#include "os/OsFS.h"
#include <os/OsDateTime.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

#define CONFIG_ETC_DIR        SIPX_CONFDIR
#define CONFIG_LOG_DIR        SIPX_LOGDIR
#define CONFIG_LOG_FILE       "sds.log"
#define CONFIG_SETTINGS_FILE  "sds-config"

#define DEFAULT_EXPIRES 60*60*24 //24 hrs


// Configuration names pulled from config-file
#define CONFIG_SETTING_LOG_LEVEL      "SIP_SDS_LOG_LEVEL"
#define CONFIG_SETTING_LOG_CONSOLE    "SIP_SDS_LOG_CONSOLE"
#define CONFIG_SETTING_LOG_DIR        "SIP_SDS_LOG_DIR"


// STATIC VARIABLE INITIALIZATIONS
SipConfigServerAgent* SipConfigServerAgent::spInstance = NULL ;
OsBSem SipConfigServerAgent::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipConfigServerAgent::SipConfigServerAgent(SipUserAgent* userAgent) :
  OsServerTask("SipConfigServerAgent-%d", NULL, 2000)
{
   osPrintf("---> BEGIN Starting SipConfigServerAgent...\r\n") ;

   if(userAgent)
   {
       mpSipUserAgent = userAgent;
       mpSipUserAgent->addMessageObserver(*(this->getMessageQueue()),
           SIP_NOTIFY_METHOD,
           FALSE, // do not want to get requests
           TRUE, // do want responses
           TRUE, // Incoming messages
           FALSE); // Don't want to see out going messages

       mpSipUserAgent->addMessageObserver(*(this->getMessageQueue()),
           SIP_SUBSCRIBE_METHOD,
           TRUE, // do want to get requests
           FALSE, // do not want responses
           TRUE, // Incoming messages
           FALSE); // Don't want to see out going messages
   } else
       osPrintf("---> NULL user agent passed to SipConfigServerAgent constructor\r\n") ;

    osPrintf("---> END Starting SipConfigServerAgent...\r\n") ;
}

// Copy constructor
SipConfigServerAgent::SipConfigServerAgent(const SipConfigServerAgent& rSipConfigServerAgent)
{
}

// Destructor
SipConfigServerAgent::~SipConfigServerAgent()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipConfigServerAgent&
SipConfigServerAgent::operator=(const SipConfigServerAgent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


UtlBoolean SipConfigServerAgent::handleMessage(OsMsg& eventMessage)
{

        osPrintf("---> SipConfigServerAgent: handleMessage...\r\n") ;

    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();

    // if this is a SIP message
    if(msgType == OsMsg::PHONE_APP &&
                msgSubType == SipMessage::NET_SIP_MESSAGE)
        {
                const SipMessage* sipMessage = ((SipMessageEvent&)eventMessage).getMessage();
                int messageType = ((SipMessageEvent&)eventMessage).getMessageStatus();
                osPrintf("SipConfigServerAgent::messageType: %d\n", messageType);
        UtlString method;

        // This is a request which failed to get sent
                if(messageType == SipMessageEvent::TRANSPORT_ERROR)
                {
            sipMessage->getRequestMethod(&method);
            osPrintf("SipConfigServerAgent:: Processing message transport error method: %s\n",
                sipMessage->isResponse() ? method.data() : "response");

            if(sipMessage->isResponse())
            {
                int seqNum;
                sipMessage->getCSeqField(&seqNum, &method);
                // SUBSCIBE (enrollment) response
                if(method.compareTo(SIP_SUBSCRIBE_METHOD))
                {
                    // We are sad the device is not there
                    osPrintf("SipConfigServerAgent::handleMessage enroll FAILURE: no response\n");
                }

            }
        }

        else if(messageType == SipMessageEvent::AUTHENTICATION_RETRY)
        {
        }

        // If this is a response
        else if(sipMessage->isResponse())
        {
            int seqNum;
            sipMessage->getCSeqField(&seqNum, &method);
            // SUBSCIBE (enrollment) response
            if(method.compareTo(SIP_NOTIFY_METHOD) &&
                mfpNotifyResponseCallbackFunc)
            {
                mfpNotifyResponseCallbackFunc(*sipMessage);
            }

        }

        // This is a request
        else
        {
            sipMessage->getRequestMethod(&method);
            UtlString eventType;
            sipMessage->getEventField(eventType);
            eventType.toLower();
            // SUBSRIBE (enrollment) request
            if(method.compareTo(SIP_SUBSCRIBE_METHOD) == 0 &&
                eventType.index(SIP_EVENT_CONFIG) >= 0 &&
                mfpEnrollmentCallbackFunc)
            {
               SipMessage response;
               SipMessage copyOfRequest(*sipMessage);

               // add a to tag to the sip message. This to tag will
               // be bubbled up to the Java layer. Also the same to tag will
               // be sent back in the response
               UtlString toAddr;
               UtlString toProto;
               int toPort;
               UtlString toTag;
               sipMessage->getToAddress(&toAddr, &toPort, &toProto, NULL, NULL, &toTag);
               if( toTag.isNull())
               {
                  int epochTime = (int)OsDateTime::getSecsSinceEpoch();
                  // Build a to tag
                  char tagBuffer[100];
                  sprintf(tagBuffer, "%dasd", epochTime);
                  copyOfRequest.setToFieldTag(tagBuffer);
               }

                int responseCode = mfpEnrollmentCallbackFunc(copyOfRequest);
                switch(responseCode)
                {
                    case SIP_ACCEPTED_CODE:
                       {
                          response.setExpiresField(DEFAULT_EXPIRES);
                          response.setResponseData(&copyOfRequest, SIP_ACCEPTED_CODE,
                            SIP_ACCEPTED_TEXT);
                       }
                        break;

                    default:
                        response.setResponseData(&copyOfRequest, SIP_BAD_REQUEST_CODE,
                            SIP_BAD_REQUEST_TEXT);
                        break;

                }
                if(mpSipUserAgent) mpSipUserAgent->send(response);
            }
        }
    }

    return(TRUE);
}


SipConfigServerAgent* SipConfigServerAgent::startAgents(const char* configFileName)
{
    int sipTcpPort;
    int sipUdpPort;
    int sipTlsPort;
    OsConfigDb configDb;

    if(configDb.loadFromFile(configFileName) == OS_SUCCESS)
    {
      osPrintf("Found config file: %s\n", configFileName);
    }
    else
    {
        configDb.set("SIP_SDS_UDP_PORT", "5090");
        configDb.set("SIP_SDS_TCP_PORT", "5090");
        configDb.set("SIP_SDS_TLS_PORT", "5091");
        configDb.set(CONFIG_SETTING_LOG_DIR, "");
        configDb.set(CONFIG_SETTING_LOG_LEVEL, "");
        configDb.set(CONFIG_SETTING_LOG_CONSOLE, "");

        if (configDb.storeToFile(configFileName) != OS_SUCCESS)
                      osPrintf("Could not store config file: %s\n", configFileName);
    }
    
    sipTcpPort = configDb.getPort("SIP_SDS_UDP_PORT") ;
    sipUdpPort = configDb.getPort("SIP_SDS_TCP_PORT") ;
    sipTlsPort = configDb.getPort("SIP_SDS_TLS_PORT") ;

    // Start the sip stack
    SipUserAgent* pAgent = new SipUserAgent(sipTcpPort,
        sipUdpPort,   
        sipTlsPort,
        NULL, // public IP address (nopt used in proxy)
        NULL, // default user (not used in proxy)
        NULL, // default SIP address (not used in proxy)
        NULL, // outbound proxy
        NULL, // directory server
        NULL, // registry server
        NULL, // auth scheme
        NULL, //auth realm
        NULL, // auth DB
        NULL, // auth user IDs
        NULL, // auth passwords
        NULL, // line mgr
        SIP_DEFAULT_RTT, // first resend timeout
        TRUE, // default to UA transaction
        SIPUA_DEFAULT_SERVER_UDP_BUFFER_SIZE, // socket layer read buffer size
        SIPUA_DEFAULT_SERVER_OSMSG_QUEUE_SIZE // OsServerTask message queue size
        );
    pAgent->start();

    // Start the SipConfigServerAgent
    SipConfigServerAgent* pConfigAgent = new SipConfigServerAgent(pAgent) ;
    pConfigAgent->start() ;

    return(pConfigAgent);
}


SipUserAgent* SipConfigServerAgent::getSipUserAgent()
{
    return mpSipUserAgent ;
}


SipConfigServerAgent* SipConfigServerAgent::getSipConfigServerAgent()
{
   UtlBoolean isStarted ;

   // If the task object already exists, and the corresponding low-level task
   // has been started, then use it
   if (spInstance != NULL && spInstance->isStarted())
      return spInstance;

   // If the task does not yet exist or hasn't been started, then acquire
   // the lock to ensure that only one instance of the task is started
   sLock.acquire();
   if (spInstance == NULL)
   {
      OsPath workingDirectory ;
      if ( OsFileSystem::exists( CONFIG_ETC_DIR ) )
      {
         workingDirectory = CONFIG_ETC_DIR;
         OsPath path(workingDirectory);
         path.getNativePath(workingDirectory);
      }
      else
      {
         OsPath path;
         OsFileSystem::getWorkingDirectory(path);
         path.getNativePath(workingDirectory);
      }

      UtlString ConfigfileName =  workingDirectory +
         OsPathBase::separator +
         CONFIG_SETTINGS_FILE  ;

      // Initialize the OsSysLog
      OsConfigDb configDb ;
      configDb.loadFromFile(ConfigfileName) ;
      initializeLog(&configDb) ;

      spInstance = startAgents(ConfigfileName);
   }

   isStarted = spInstance->isStarted();
   if (!isStarted)
   {
      isStarted = spInstance->start();
      // assert(isStarted);
   }
   sLock.release();

   return spInstance;
}

/* ============================ ACCESSORS ================================= */


void SipConfigServerAgent::setEnrollmentCallback(EnrollmentCallbackFunc callback)
{
        mfpEnrollmentCallbackFunc = callback ;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Initialize the OsSysLog
void SipConfigServerAgent::initializeLog(OsConfigDb* pConfig)
{
   UtlString  logLevel;               // Controls Log Verbosity
   UtlString  consoleLogging;         // Enable console logging by default?
   UtlString  fileTarget;             // Path to store log file.
   UtlBoolean bSpecifiedDirError ;   // Set if the specified log dir does not
                                    // exist
   struct tagPrioriotyLookupTable
   {
      const char*      pIdentity;
      OsSysLogPriority ePriority;
   };

   struct tagPrioriotyLookupTable lkupTable[] =
   {
      { "DEBUG",   PRI_DEBUG},
      { "INFO",    PRI_INFO},
      { "NOTICE",  PRI_NOTICE},
      { "WARNING", PRI_WARNING},
      { "ERR",     PRI_ERR},
      { "CRIT",    PRI_CRIT},
      { "ALERT",   PRI_ALERT},
      { "EMERG",   PRI_EMERG},
   };
   OsSysLog::initialize(0, "SDS");

   //
   // Get/Apply Log Filename
   //
   fileTarget.remove(0) ;
   if ((pConfig->get(CONFIG_SETTING_LOG_DIR, fileTarget) != OS_SUCCESS) ||
      fileTarget.isNull() || !OsFileSystem::exists(fileTarget))
   {
      bSpecifiedDirError = !fileTarget.isNull() ;

      // If the log file directory exists use that, otherwise place the log
      // in the current directory
      OsPath workingDirectory;
      if (OsFileSystem::exists(CONFIG_LOG_DIR))
      {
         fileTarget = CONFIG_LOG_DIR;
         OsPath path(fileTarget);
         path.getNativePath(workingDirectory);

         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
         OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
      }
      else
      {
         OsPath path;
         OsFileSystem::getWorkingDirectory(path);
         path.getNativePath(workingDirectory);

         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
         OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
      }

      fileTarget = workingDirectory +
         OsPathBase::separator +
         CONFIG_LOG_FILE;
   }
   else
   {
      bSpecifiedDirError = false ;
      osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, fileTarget.data()) ;
      OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s\n", CONFIG_SETTING_LOG_DIR, fileTarget.data()) ;

      fileTarget = fileTarget +
         OsPathBase::separator +
         CONFIG_LOG_FILE;
   }
   OsSysLog::setOutputFile(0, fileTarget) ;


   //
   // Get/Apply Log Level
   //
   if ((pConfig->get(CONFIG_SETTING_LOG_LEVEL, logLevel) != OS_SUCCESS) ||
         logLevel.isNull())
   {
      logLevel = "ERR";
   }
   logLevel.toUpper();
   OsSysLogPriority priority = PRI_ERR;
   int iEntries = sizeof(lkupTable)/sizeof(struct tagPrioriotyLookupTable);
   for (int i=0; i<iEntries; i++)
   {
      if (logLevel == lkupTable[i].pIdentity)
      {
         priority = lkupTable[i].ePriority;
         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_LEVEL, lkupTable[i].pIdentity) ;
         OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s\n", CONFIG_SETTING_LOG_LEVEL, lkupTable[i].pIdentity) ;
         break;
      }
   }
   OsSysLog::setLoggingPriority(priority);
   OsSysLog::setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);

   //
   // Get/Apply console logging
   //
   UtlBoolean bConsoleLoggingEnabled = false ;
   if ((pConfig->get(CONFIG_SETTING_LOG_CONSOLE, consoleLogging) ==
         OS_SUCCESS))
   {
      consoleLogging.toUpper();
      if (consoleLogging == "ENABLE")
      {
         OsSysLog::enableConsoleOutput(true);
         bConsoleLoggingEnabled = true ;
      }
   }

   osPrintf("%s : %s\n", CONFIG_SETTING_LOG_CONSOLE, bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;
   OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s\n", CONFIG_SETTING_LOG_CONSOLE, bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;

   if (bSpecifiedDirError)
   {
      OsSysLog::add(FAC_LOG, PRI_CRIT, "Cannot access %s directory; please check configuration.", CONFIG_SETTING_LOG_DIR);
   }
}


/* ============================ FUNCTIONS ================================= */
