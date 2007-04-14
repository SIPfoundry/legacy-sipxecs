// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <stdlib.h>

#ifdef TEST
    #include "utl/UtlMemCheck.h"
#endif

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "os/OsSysLog.h"
#include "os/OsConfigDb.h"
#include "os/OsServerSocket.h"
#include "os/OsSSLServerSocket.h"
#include "cp/CallManager.h"
#include "net/Url.h"
#include "net/HttpServer.h"
#include "net/SipUserAgent.h"
#include "net/SipMessage.h"
#include "net/SipMessageEvent.h"
#include "EmailNotifier/Notifier.h"
#include "emailnotifier/WebServer.h"
#include "emailnotifier/emailnotifier.h"
#include "emailnotifier/SubscribeServerThread.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define HTTP_SERVER_PORT        8200
#define HTTPS_SERVER_PORT       8201
// Configuration names pulled from config-file
#define CONFIG_SETTING_LOG_LEVEL      "SIP_STATUS_LOG_LEVEL"
#define CONFIG_SETTING_LOG_CONSOLE    "SIP_STATUS_LOG_CONSOLE"
#define CONFIG_SETTING_LOG_DIR        "SIP_STATUS_LOG_DIR"

// STATIC VARIABLE INITIALIZATIONS
EmailNotifier* EmailNotifier::spInstance = NULL;
OsBSem EmailNotifier::sLock( OsBSem::Q_PRIORITY, OsBSem::FULL );

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
EmailNotifier::EmailNotifier(
    Notifier* notifier,
    int maxExpiresTime,
    const UtlString& defaultDomain,
    const UtlString& defaultMinExpiresTime,
    const UtlBoolean& useCredentialDB,
    const UtlString& defaultAuthAlgorithm,
    const UtlString& defaultAuthQop,
    const UtlString& defaultRealm,
    const UtlString& configDir,
    HttpServer* httpServer) :
    OsServerTask("SipStatusMain"),
    mSubscribeServerThread(NULL),
    mSubscribeServerThreadQ(NULL),
    mSubscribeThreadInitialized(FALSE),
    mHttpServer(httpServer),
    mNotifier(NULL)
{
#ifdef TEST
    if ( !sIsTested )
    {
        sIsTested = true;
        test();
    }
#endif //TEST

    mConfigDirectory.remove(0);
    mConfigDirectory.append(configDir);

    mDefaultRegistryPeriod = maxExpiresTime;
    mNotifier = notifier;

    if ( mNotifier )
    {
        mpSipUserAgent = mNotifier->getUserAgent();
        mpSipUserAgent->addMessageConsumer(this);
        mpSipUserAgent->allowMethod(SIP_NOTIFY_METHOD);
        mpSipUserAgent->allowMethod(SIP_SUBSCRIBE_METHOD);
        mpSipUserAgent->allowExtension("sip-cc-01");
        mpSipUserAgent->allowExtension("timer");
    }

    if(!defaultAuthAlgorithm.isNull())
    {
        mAuthAlgorithm = defaultAuthAlgorithm;
    }

    if(!defaultAuthQop.isNull())
    {
        mAuthQop = defaultAuthQop;
    }

    if ( !defaultDomain.isNull() )
    {
        mDefaultDomain.remove(0);
        mDefaultDomain.append(defaultDomain);
    }

    if ( !defaultMinExpiresTime.isNull() )
    {
        mMinExpiresTime.remove(0);
        mMinExpiresTime.append(defaultMinExpiresTime);
    }

    if ( !defaultDomain.isNull() )
    {
        mDefaultDomain.remove(0);
        mDefaultDomain.append(defaultDomain);
    }

    if ( !defaultRealm.isNull() )
    {
        mRealm.remove(0);
        mRealm.append(defaultRealm);
    }
    
    Url domain(mDefaultDomain);
    domain.getHostAddress(mlocalDomainHost);

    mIsCredentialDB = useCredentialDB;
    
    UtlString fileName = mConfigDirectory + 
        OsPathBase::separator + "status-plugin.xml";

    mPluginTable.loadPlugins(fileName, mNotifier);
    
    // Start Webserver and initialize the CGIs
    WebServer::getWebServerTask(&mPluginTable)->initWebServer(mHttpServer) ;

    startSubscribeServerThread();
}

// Destructor
EmailNotifier::~EmailNotifier()
{
   // Wait for the owned servers to shutdown first
   if ( mSubscribeServerThread )
   {
      // Deleting a server task is the only way of 
      // waiting for shutdown to complete cleanly
      mSubscribeServerThread->requestShutdown();
      delete mSubscribeServerThread;
      mSubscribeServerThread = NULL;
      mSubscribeServerThreadQ = NULL;
   }
   // HTTP server shutdown
   if (mHttpServer)
   {
      mHttpServer->requestShutdown();
      delete mHttpServer;
      mHttpServer = NULL;
   }
   if( mNotifier)
   {
      delete mNotifier;
      mNotifier = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
/*EmailNotifier&
EmailNotifier::operator=(const EmailNotifier& rhs)
{
   if (this == &rhs)              // handle the assignment to self case
      return *this;

   return *this;
}*/

/* ============================ MANIPULATORS ============================== */
UtlBoolean 
EmailNotifier::handleMessage( OsMsg& eventMessage )
{
    syslog(FAC_SIP, PRI_DEBUG, "EmailNotifier::handleMessage() :: Start processing SIP message") ;

    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();

    if ( msgType == OsMsg::PHONE_APP && msgSubType == CallManager::CP_SIP_MESSAGE )
    {
        const SipMessage* message = ((SipMessageEvent&)eventMessage).getMessage();
        UtlString callId;

        if ( message )
        {
            message->getCallIdField(&callId);
            UtlString method;
            message->getRequestMethod(&method);

            if ( !message->isResponse() )
            {
               if ( method.compareTo(SIP_SUBSCRIBE_METHOD) == 0 )
               {
                   //send to SubscribeThread
                   sendToSubscribeServerThread(eventMessage);
               } 
               else
               {
                   //send to redirect thread
                   //sendToRedirectServer(eventMessage);
               }
            }
        }
    }
    return(TRUE);
}

EmailNotifier* 
EmailNotifier::startEmailNotifier ( 
    const UtlString workingDir, 
    const char* configFileName )
{
    int httpPort = 0;
    int tcpPort  = 0;
    int udpPort  = 0;
    UtlString defaultMaxExpiresTime;
    UtlString defaultMinExpiresTime;

    UtlString authAlgorithm;
    UtlString authQop;
    UtlString authRealm;
    UtlString authScheme;
    UtlString domainName;

    UtlBoolean isCredentialDB = TRUE;

    OsConfigDb configDb;

    // if the configuration file exists, load the name value pairs
    if ( configDb.loadFromFile(configFileName) == OS_SUCCESS )
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "Found config file: %s", configFileName);
    } else
    {
        configDb.set("SIP_STATUS_AUTHENTICATE_ALGORITHM", "");
        configDb.set("SIP_STATUS_AUTHENTICATE_QOP", "");
        configDb.set("SIP_STATUS_AUTHENTICATE_REALM", "");
        // configDb.set("SIP_STATUS_AUTHENTICATE_SCHEME", "");
        configDb.set("SIP_STATUS_DOMAIN_NAME", "");
        configDb.set("SIP_STATUS_HTTP_AUTH_DB.", "");
        configDb.set("SIP_STATUS_HTTP_PORT", "");
        configDb.set("SIP_STATUS_HTTP_VALID_IP.", "");
        configDb.set("SIP_STATUS_MAX_EXPIRES", "");
        configDb.set("SIP_STATUS_MIN_EXPIRES", "");
        configDb.set("SIP_STATUS_TCP_PORT", "");
        configDb.set("SIP_STATUS_UDP_PORT", "");
        configDb.set(CONFIG_SETTING_LOG_LEVEL, "");
        configDb.set(CONFIG_SETTING_LOG_CONSOLE, "");
        configDb.set(CONFIG_SETTING_LOG_DIR, "");
    
        if ( configDb.storeToFile(configFileName) != OS_SUCCESS )
        {
            OsSysLog::add(FAC_SIP, PRI_INFO,
                          "Could not write config file: %s", configFileName);
        }
    }

    configDb.get("SIP_STATUS_AUTHENTICATE_ALGORITHM", authAlgorithm);
    configDb.get("SIP_STATUS_AUTHENTICATE_QOP", authQop);
    configDb.get("SIP_STATUS_AUTHENTICATE_REALM", authRealm);
    configDb.get("SIP_STATUS_AUTHENTICATE_SCHEME", authScheme);
    configDb.get("SIP_STATUS_DOMAIN_NAME", domainName);
    configDb.get("SIP_STATUS_HTTP_PORT", httpPort);
    configDb.get("SIP_STATUS_MAX_EXPIRES", defaultMaxExpiresTime);
    configDb.get("SIP_STATUS_MIN_EXPIRES", defaultMinExpiresTime);
    configDb.get("SIP_STATUS_TCP_PORT", tcpPort);
    configDb.get("SIP_STATUS_UDP_PORT", udpPort);
    
    // Get the HTTP server authentication database
    OsConfigDb* pUserPasswordDigestDb = new OsConfigDb() ;
    configDb.getSubHash("SIP_STATUS_HTTP_AUTH_DB.", *pUserPasswordDigestDb) ;
    if( pUserPasswordDigestDb->isEmpty() )
    {
        delete pUserPasswordDigestDb;
        pUserPasswordDigestDb = NULL;
    }

    // Get the HTTP server Valid IP address database
    OsConfigDb* pValidIpAddressDB = new OsConfigDb() ;
    configDb.getSubHash("SIP_STATUS_HTTP_VALID_IP.", *pValidIpAddressDB);
    if( pValidIpAddressDB->isEmpty() )
    {
        delete pValidIpAddressDB;
        pValidIpAddressDB = NULL;
    }

    // Check for default values and, if not specified, set to defaults

    // SIP_STATUS_AUTHENTICATE_ALGORITHM
    if ( authAlgorithm.isNull() ) /* MD5/MD5SESS */
    {
        authAlgorithm.append("MD5"); 
    }
    OsSysLog::add(FAC_SIP, PRI_INFO,
                  "SIP_STATUS_AUTHENTICATE_ALGORITHM : %s", 
                  authAlgorithm.data());

    // SIP_STATUS_AUTHENTICATE_QOP
    if ( authQop.isNull() ) /* AUTH/AUTH-INT/NONE */
    {
        authQop.append("NONE"); 
    }
    OsSysLog::add(FAC_SIP, PRI_INFO,
                  "SIP_STATUS_AUTHENTICATE_QOP : %s", 
                  authQop.data());
    
    // SIP_STATUS_DOMAIN_NAME - need this before the SIP_STATUS_AUTHENTICATE_REALM
    // below since we get the domain name from the socket
    if ( domainName.isNull() )
    {
        OsSocket::getHostIp(&domainName);
    }
    OsSysLog::add(FAC_SIP, PRI_INFO,
                  "SIP_STATUS_DOMAIN_NAME : %s", 
                  domainName.data());
    
    // SIP_STATUS_AUTHENTICATE_REALM
    if(authRealm.isNull())
    {
        authRealm.append(domainName);
    }
    OsSysLog::add(FAC_SIP, PRI_INFO,
                  "SIP_STATUS_AUTHENTICATE_REALM : %s", 
                  authRealm.data());

    // SIP_STATUS_AUTHENTICATE_SCHEME (Hidden) NONE/DIGEST
    if ( authScheme.compareTo("NONE" , UtlString::ignoreCase) == 0 ) 
    {
        isCredentialDB = FALSE;
    }

    // SIP_STATUS_HTTP_PORT
    OsStatus result;
    UtlString httpPortStr;
    result = configDb.get("SIP_STATUS_HTTP_PORT", httpPortStr);
    // If the key is not found, disable the web sever
    if ( result == OS_NOT_FOUND )
    {
        httpPort = -1;    
    } else if ( httpPortStr.isNull() )
    {
        // If the key is present, but not set, set it to the default
        // non-secure port number
        httpPort = HTTP_SERVER_PORT;
    }
    OsSysLog::add(FAC_SIP, PRI_INFO,
                  "SIP_STATUS_HTTP_PORT : %d", httpPort);

    // SIP_STATUS_MAX_EXPIRES
    if ( defaultMaxExpiresTime.isNull() )
    {
        defaultMaxExpiresTime.append("604800"); // default to 1 week
    }
    OsSysLog::add(FAC_SIP, PRI_INFO,
                  "SIP_STATUS_MAX_EXPIRES : %s", defaultMaxExpiresTime.data());

    int maxExpiresTime = atoi(defaultMaxExpiresTime.data());

    // SIP_STATUS_MIN_EXPIRES
    if ( defaultMinExpiresTime.isNull() )
    {
        defaultMinExpiresTime.append("300");  // default to 300 seconds
    }
    OsSysLog::add(FAC_SIP, PRI_INFO,
                  "SIP_STATUS_MIN_EXPIRES : %s", defaultMinExpiresTime.data());

    // SIP_STATUS_TCP_PORT
    if ( tcpPort <= 0 )
    {
        tcpPort = 5110;
    }
    OsSysLog::add(FAC_SIP, PRI_INFO,
                  "SIP_STATUS_TCP_PORT : %d", tcpPort);

    // SIP_STATUS_UDP_PORT
    if ( udpPort <= 0 )
    {
        udpPort = 5110;
    }
    OsSysLog::add(FAC_SIP, PRI_INFO,
                  "SIP_STATUS_UDP_PORT : %d", tcpPort);

    // Start the HTTP server
    HttpServer* httpServer = 
        initHttpServer(
            httpPort,
            authRealm,
            pUserPasswordDigestDb,
            pValidIpAddressDB );

    // Start the SIP stack
    SipUserAgent* sipUserAgent = 
        new SipUserAgent( 
            tcpPort, 
            udpPort,
            NULL,   // public IP address (nopt used in proxy)
            NULL,   // default user (not used in proxy)
            NULL,   // default SIP address (not used in proxy)
            NULL,   // outbound proxy
            NULL,   // directory server
            NULL,   // registry server
            NULL,   // auth scheme
            NULL,   // auth realm
            NULL,   // auth DB
            NULL,   // auth user IDs
            NULL,   // auth passwords
            NULL,   // nat ping URL
            0,      // nat ping frequency
            "PING", // nat ping method
            NULL,   // line mgr
            SIP_DEFAULT_RTT, // first resend timeout
            TRUE,   // default to UA transaction
            1000000 // socket layer read buffer size
         );
    sipUserAgent->startMessageLog(100000);
    sipUserAgent->start();

    Notifier* notifier = new Notifier(sipUserAgent);

    // Start the status server
    EmailNotifier* status = new EmailNotifier(
                notifier,
                maxExpiresTime,
                domainName,
                defaultMinExpiresTime,
                isCredentialDB,
                authAlgorithm,
                authQop,
                authRealm,
                workingDir,
                httpServer);
    status->start();
    return(status);
}


EmailNotifier* 
EmailNotifier::getInstance()
{
    // crit sec, ensure only one instance starts the status server
    sLock.acquire();

    if ( spInstance == NULL )
    {
        OsPath workingDirectory;
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

        UtlString fileName =  workingDirectory + 
            OsPathBase::separator +
            "status-config";

        spInstance = startEmailNotifier( 
            workingDirectory, 
            fileName );
    }

    // If the task object already exists, and the corresponding low-level task
    // has been started, then use it
    if ( !spInstance->isStarted() )
    {
        UtlBoolean isStarted = spInstance->start();
        assert(isStarted);
    }
    // release the critsec
    sLock.release();

#ifdef TEST
    if ( !sIsTested )
    {
        sIsTested = true;
        spInstance->test();
    }
#endif //TEST
    return spInstance;
}


/* ============================ ACCESSORS ================================= */
/////////////////////////////////////////////////////////////////////////////
void 
EmailNotifier::startSubscribeServerThread()
{
    UtlString localdomain;
    OsSocket::getHostIp(&localdomain);

    mSubscribeServerThread = new SubscribeServerThread();
    if ( mSubscribeServerThread->initialize(
            mpSipUserAgent,
            mDefaultRegistryPeriod,
            mMinExpiresTime,
            mDefaultDomain,
            mIsCredentialDB,
            mRealm,
            &mPluginTable))
    {
        mSubscribeServerThreadQ = mSubscribeServerThread->getMessageQueue();
        mSubscribeThreadInitialized = TRUE;
    }
}

void 
EmailNotifier::sendToSubscribeServerThread(OsMsg& eventMessage)
{
    if ( mSubscribeThreadInitialized )
    {
        mSubscribeServerThreadQ->send(eventMessage);
    }
}

HttpServer* 
EmailNotifier::initHttpServer (
    int httpServerPort,
    const UtlString authRealm,
    OsConfigDb* pUserPasswordDigestDb,
    OsConfigDb* pValidIpAddressDB )
{
    UtlString osBaseUriDirectory ;
    HttpServer* httpServer = NULL;

    OsPath workingDirectory;
    if ( OsFileSystem::exists( HTTP_SERVER_ROOT_DIR ) )
    {
        workingDirectory = HTTP_SERVER_ROOT_DIR;
        OsPath path(workingDirectory);
        path.getNativePath(workingDirectory);
    } else
    {
        OsPath path;
        OsFileSystem::getWorkingDirectory(path);
        path.getNativePath(workingDirectory);
    }

    osBaseUriDirectory =  workingDirectory + OsPathBase::separator;

    // Determine whether we should start the web server or not.  Use the port
    // value as the decision point.  Anything > 0 means enable. 
    // If the port is odd numberd then start as a secure web server
    // 
    if( httpServerPort > 0 )
    {
        OsSysLog::add(FAC_SIP, PRI_INFO,
                      "Starting Embedded Web Server on port %d...",
                      httpServerPort);

        OsServerSocket *pServerSocket = NULL;  

        // If the port number is odd use SSL
        if ( httpServerPort & 1)
        {
            pServerSocket = new OsSSLServerSocket(50, httpServerPort);
            httpServer = new HttpServer(
                pServerSocket, 
                pUserPasswordDigestDb,
                authRealm, 
                pValidIpAddressDB );
        } else // even port => create a normal non SSL server 
        {
            pServerSocket = new OsServerSocket(50, httpServerPort);
            httpServer = new HttpServer(
                pServerSocket, 
                pUserPasswordDigestDb,
                authRealm, 
                pValidIpAddressDB );
        }
        // Set the web server root to the current directory
        httpServer->addUriMap( "/", osBaseUriDirectory.data() );
    }

    if( httpServer )
    {   // Method to post files to WebServer for processing MWI events
        httpServer->start();
    }
    return httpServer;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

#ifdef TEST

// Set to true after the tests have been executed once
bool EmailNotifier::sIsTested = false;

// Test this class by running all of its assertion tests
void
EmailNotifier::test()
{
    UtlMemCheck* pMemCheck = 0;
    pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

    testCreators();
    testManipulators();
    testAccessors();
    testInquiry();

    assert(pMemCheck->delta() == 0);    // check for memory leak
    delete pMemCheck;
}

// Test the creators (and destructor) methods for the class
void
EmailNotifier::testCreators()
{
    UtlMemCheck* pMemCheck  = 0;

    pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

    // test the default constructor (if implemented)
    // test the copy constructor (if implemented)
    // test other constructors (if implemented)
    //  if a constructor parameter is used to set information in an ancestor
    //      class, then verify it gets set correctly (i.e., via ancestor
    //      class accessor method.
    // test the destructor
    //  if the class contains member pointer variables, verify that the
    //  pointers are getting scrubbed.

    assert(pMemCheck->delta() == 0);    // check for memory leak
    delete pMemCheck;
}

// Test the manipulator methods
void
EmailNotifier::testManipulators()
{
    UtlMemCheck* pMemCheck  = 0;

    pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

    // test the assignment method (if implemented)
    // test the other manipulator methods for the class

    assert(pMemCheck->delta() == 0);    // check for memory leak
    delete pMemCheck;
}

// Test the accessor methods for the class
void
EmailNotifier::testAccessors()
{
    UtlMemCheck* pMemCheck  = 0;

    pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

    // body of the test goes here

    assert(pMemCheck->delta() == 0);    // check for memory leak
    delete pMemCheck;
}

// Test the inquiry methods for the class
void
EmailNotifier::testInquiry()
{
    UtlMemCheck* pMemCheck  = 0;

    pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

    // body of the test goes here

    assert(pMemCheck->delta() == 0);    // check for memory leak
    delete pMemCheck;
}

#endif //TEST

/* ============================ FUNCTIONS ================================= */
