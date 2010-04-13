//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ACDServer_h_
#define _ACDServer_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <net/Url.h>
#include <net/ProvisioningClass.h>
#include <net/ProvisioningAttrList.h>

// DEFINES
#define CONFIG_LOG_FILE               "sipxacd.log"
#define CONFIG_LOG_DIR                SIPX_LOGDIR
#define LOG_FACILITY                  FAC_ACD

#define ACD_SERVER_NAME               "sipxacd"
#define ACD_SERVER_ALIAS              "ACDServer"   // matches the name from the process definition file

// Provisioning Tag Definitions
#define ACD_SERVER_TAG                "acd-server"
#define SERVER_NAME_TAG               "server-name"
#define LOG_DIR_TAG                   "log-dir"
#define LOG_LEVEL_TAG                 "log-level"
#define LOG_TO_CONSOLE_TAG            "log-to-console"
#define DOMAIN_TAG                    "domain"
#define FQDN_TAG                      "fqdn"
#define UDP_PORT_TAG                  "udp-port"
#define TCP_PORT_TAG                  "tcp-port"
#define RTP_PORT_TAG                  "rtp-port"
#define TLS_PORT_TAG                  "tls-port"
#define MAX_CALLS_TAG                 "max-calls-allowed"
#define RPC_SERVER_PORT_TAG           "rpc-server-port"
#define PRESENCE_MONITOR_PORT_TAG     "presence-monitor-port"
#define PRESENCE_SERVER_URI_TAG       "presence-server-uri"
#define PRESENCE_SERVICE_URI_TAG      "presence-service-uri"
#define ADMINISTRATIVE_STATE_TAG      "administrative-state"
#define OPERATIONAL_STATE_TAG         "operational-state"
#define ACTION_RESTART_TAG            "restart"
#define ACTION_SHUTDOWN_TAG           "shutdown"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlContainable;
class ProvisioningAgent;
class ProvisioningAgentXmlRpcAdapter;
class ACDCallManager;
class ACDLineManager;
class ACDAgentManager;
class ACDQueueManager;
class ACDAudioManager;
class ACDRtRecord;

#ifdef CML
class ACDRpcServer;
#endif

/**
 *
 */
class ACDServer : public ProvisioningClass {
public:

   enum eServerState {
      ACTIVE      = 1,
      STANDBY     = 2,
      DOWN        = 3
   };

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

   // Default constructor
   ACDServer(int provisioningAgentPort, int watchdogRpcServerPort);

   // Destructor
   virtual ~ACDServer();

/* ============================ MANIPULATORS ============================== */

   bool loadConfiguration(void);

   void initSysLog(const char* pApplication, UtlString& rLogDirectory, UtlString& rLogLevel, bool consoleLogging);

   void setSysLogLevel(UtlString& rLogLevel);

   ProvisioningAttrList* Create(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Delete(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Set(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Get(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Action(ProvisioningAttrList& rRequestAttributes);

/* ============================ ACCESSORS ================================= */

   ProvisioningAgent* getProvisioningAgent(void);

   ACDCallManager*    getAcdCallManager(void);

   ACDLineManager*    getAcdLineManager(void);

   ACDAgentManager*   getAcdAgentManager(void);

   ACDQueueManager*   getAcdQueueManager(void);

   ACDAudioManager*   getAcdAudioManager(void);

   ACDRtRecord*       getAcdRtRecord(void);

   const char*        getDomain(void);

   const char*        getRealm(void);

   void               getDefaultIdentity(Url& id);

   int                getAdministrativeState(void);

   size_t             getMaxCallAllowed(void) { return mMaxAcdCallsAllowed; }

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static const char* ID_TOKEN;

   ProvisioningAgent* mpProvisioningAgent;
   ProvisioningAgentXmlRpcAdapter* mpProvisioningAgentXmlRpcAdapter;
   ACDCallManager*    mpAcdCallManager;
   ACDLineManager*    mpAcdLineManager;
   ACDAgentManager*   mpAcdAgentManager;
   ACDQueueManager*   mpAcdQueueManager;
   ACDAudioManager*   mpAcdAudioManager;
   ACDRtRecord*       mpAcdRtRecord;
#ifdef CML
   ACDRpcServer*      mpAcdRpcServer;
#endif

   UtlString          mLogDirectory;            ///< Log directory path.
   UtlString          mLogFile;                 ///< Log filename including the path.
   UtlString          mLogLevel;                ///< The logging level.
   bool               mLogToConsole;            ///< Console logging switch.
   UtlString          mDomain;                  ///< The SIP domain of this server.
   UtlString          mFqdn;                    ///< The FQDN of this server.
   UtlString          mRealm;                   ///< The SIP authentication realm of this server.
   Url                mDefaultIdentity;         ///< Identity to use if the line does not have one.
   int                mUdpPort;                 ///< The UDP Port used by this server.
   int                mTcpPort;                 ///< The TCP Port used by this server.
   int                mRtpBase;                 ///< The starting RTP Port used by this server.
   int                mTlsPort;                 ///< The TLS Port used by this server.
   int                mPresenceMonitorPort;     ///< The Presence Monitor Port used by this server.
   UtlString          mPresenceServerUriString; /**< The Uri of the Presence Server
                                                 *   used for TUI sign-in/out. */
   UtlString          mPresenceServiceUriString; /**< The Uri of the Presence Server
                                                 *   used for RPC sign-in/out. */
   int                mAdministrativeState;     /**< The desired Administrative State of this
                                                 *   Server. */
   int                mOperationalState;        ///< The actual Operational State of this Server.
   bool               mServerStarted;           /**< Flag indicating that the server is completely
                                                 *   up. */
   int                mMaxAcdCallsAllowed;      ///< The maximum number of calls allowed on a ACD.
#ifdef CML
   int                mAcdRpcServerPort;        ///< Port to be used for call pickup XML-RPC server
#endif
   int                mWatchdogRpcServerPort;   /**< Port that the Watchdog's XML-RPC server
                                                 *   is using. */
};

#endif  // _ACDServer_h_
