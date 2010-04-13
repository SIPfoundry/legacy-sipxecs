//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef STATUSSERVER_H
#define STATUSSERVER_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsBSem.h"
#include "os/OsServerTask.h"
#include "net/SipUserAgent.h"
#include "statusserver/PluginXmlParser.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define CONFIG_ETC_DIR          SIPX_CONFDIR
#define CONFIG_SETTINGS_FILE  "status-config"
// Configuration names pulled from config-file
#define CONFIG_SETTING_PREFIX         "SIP_STATUS"
#define CONFIG_SETTING_LOG_CONSOLE    "SIP_STATUS_LOG_CONSOLE"
#define CONFIG_SETTING_LOG_DIR        "SIP_STATUS_LOG_DIR"

#define HTTP_SERVER_ROOT_DIR    SIPX_CONFDIR
#define HTTP_SERVER_PORT        8100
#define HTTPS_SERVER_PORT       8101
#define HTTPS_PUBLIC_CERTIFICATE_FILE_LOCATION CONFIG_ETC_DIR "/ssl/ssl.crt"
#define HTTPS_PRIVATE_KEY_FILE_LOCATION CONFIG_ETC_DIR "/ssl/ssl.key"

#define LOG_FACILITY          FAC_SIP
#define CONFIG_LOG_DIR        SIPX_LOGDIR
#define CONFIG_LOG_FILE       "sipstatus.log"

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsConfigDb;
class SubscribeServerThread;
class SubscribePersistThread;
class StatusServer;
class HttpServer;
class Notifier;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class StatusServer : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    // Singleton globals
    // Note: this class does not need to be a singleton.  The only method that
    // assumes singleton is getStatusServer
    static StatusServer* spInstance;
    static OsBSem sLock;

/* ============================ CREATORS ================================== */
    //:Default constructor

    virtual ~StatusServer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
    static StatusServer* startStatusServer(
        const UtlString workingDir,
        const char* configFileName);

    static StatusServer* getInstance();
    // Singleton constructor/accessor
    // Note: this class does not need to be a singleton.  The only method that
    // assumes singleton is getStatusServer

    static OsConfigDb& getConfigDb();
    SubscribePersistThread* getSubscribePersistThread();
    SubscribeServerThread* getSubscribeServerThread();

    virtual UtlBoolean handleMessage(OsMsg& eventMessage);

    // Common code to parse a list of name/value pairs in the config file.
    // Fills in an OsConfigDb with that data.
    static void parseList (
        const UtlString& keyPrefix,
        const UtlString& separatedList,
        OsConfigDb& list);


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    static OsConfigDb& sConfigDb;
    Notifier* mNotifier;
    SipUserAgent* mpSipUserAgent;
    SubscribeServerThread* mSubscribeServerThread;
    OsMsgQ* mSubscribeServerThreadQ;
    UtlBoolean mSubscribeThreadInitialized;
    SubscribePersistThread* mSubscribePersistThread;
    UtlBoolean mIsCredentialDB;
    int mDefaultRegistryPeriod;
    int mDefaultQvalue;
    static UtlString sKeyPassword;
    UtlString mDefaultDomain;
    UtlString mMinExpiresTime;
    UtlString mRegistryCacheFileName;
    UtlString mAuthAlgorithm;
    UtlString mAuthQop;
    UtlString mRealm;
    UtlString mConfigDirectory;
    UtlString mlocalDomainHost;

    PluginXmlParser mPluginTable;
    OsServerSocket* mpServerSocket;
    HttpServer* mHttpServer;

    // Private constructor for singleton implementation
    StatusServer (
        Notifier* notifier,
        int maxExpiresTime,
        const UtlString& defaultDomain,
        const UtlString& defaultMinExpiresTime,
        const UtlBoolean& useCredentialDB,
        const UtlString& defaultAuthAlgorithm,
        const UtlString& defaultAuthQop,
        const UtlString& defaultRealm,
        const UtlString& configDir,
        OsServerSocket* serverSocket,
        HttpServer* httpServer );

    /// Start the thread that periodically persists the subscription DB
    void startSubscribePersistThread();

    /* ============================ SUBSCRIBE =================================== */
    void startSubscribeServerThread();
    void sendToSubscribeServerThread(OsMsg& eventMessage);

};

/* ============================ INLINE METHODS ============================ */

#endif  // STATUSSERVER_H
