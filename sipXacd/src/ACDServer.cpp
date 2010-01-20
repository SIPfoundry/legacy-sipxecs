//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsFS.h>
#include <os/OsProcess.h>
#include <os/OsTask.h>
#include <os/OsSysLog.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlInt.h>
#include <utl/UtlSList.h>
#include <utl/UtlString.h>
#include <net/ProvisioningAgent.h>
#include <net/ProvisioningAgentXmlRpcAdapter.h>
#include <net/ProvisioningAttrList.h>
#include <net/Url.h>
#include <net/XmlRpcRequest.h>
#include <sipXecsService/SipXecsService.h>
#include "sipXecsService/SharedSecret.h"
#include "net/SipXauthIdentity.h"

#include "ACDCallManager.h"
#include "ACDLineManager.h"
#include "ACDAgentManager.h"
#include "ACDQueueManager.h"
#include "ACDAudioManager.h"
#include "ACDRtRecord.h"
#ifdef CML
#include "ACDRpcServer.h"
#endif
#include "ACDServer.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern UtlBoolean gShutdownFlag;
extern UtlBoolean gRestartFlag;

// CONSTANTS
#define MAX_CONNECTIONS    30

const char* ACDServer::ID_TOKEN = "~~id~acd";

// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::ACDServer
//
//  SYNOPSIS:
//
//  DESCRIPTION: Default constructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDServer::ACDServer(int provisioningAgentPort, int watchdogRpcServerPort)
: ProvisioningClass(ACD_SERVER_TAG),
  mWatchdogRpcServerPort(watchdogRpcServerPort)
{

   mpProvisioningAgent              = NULL;
   mpProvisioningAgentXmlRpcAdapter = NULL;
   mpAcdCallManager                 = NULL;
   mpAcdLineManager                 = NULL;
   mpAcdAgentManager                = NULL;
   mpAcdQueueManager                = NULL;
   mpAcdAudioManager                = NULL;
#ifdef CML
   mpAcdRpcServer                   = NULL;
#endif

   // Start initially in the STANDBY state.
   mAdministrativeState = STANDBY;
   mOperationalState    = STANDBY;
   mServerStarted       = false;

   // Set up some initial default attributes
   mLogDirectory            = "";
   mLogFile                 = "";
   mLogLevel                = "DEBUG";
   mLogToConsole            = false;
   mPresenceServerUriString = "";
   mPresenceServiceUriString = "";
   mMaxAcdCallsAllowed      = MAX_CONNECTIONS;
#ifdef CML
   mAcdRpcServerPort           = -1;
#endif

   // Start up the Provisioning Agent
   mpProvisioningAgent = new ProvisioningAgent(ACD_SERVER_NAME, true);

   // Bind the Provisioning Agent to the XmlRpc Adapter
   mpProvisioningAgentXmlRpcAdapter = new ProvisioningAgentXmlRpcAdapter(mpProvisioningAgent, provisioningAgentPort, false);

   // Register this class with the Provisioning Agent
   mpProvisioningAgent->registerClass(this);

   // Load the configuration
   loadConfiguration();

   initSysLog(ACD_SERVER_NAME, mLogDirectory, mLogLevel, mLogToConsole);

   // Prepare for writing real-time record
   UtlString mRtLogDir = "";
   mpAcdRtRecord = new ACDRtRecord(mRtLogDir);

   if (mpAcdRtRecord) {
      ACDRtRecord* pACDRtRec;

      if (NULL != (pACDRtRec = getAcdRtRecord())) {
         pACDRtRec->appendAcdEvent(ACDRtRecord::START_ACD);
      }
   }

   OsConfigDb  domainConfiguration;
   OsPath      domainConfigPath = SipXecsService::domainConfigPath();
   if (OS_SUCCESS == domainConfiguration.loadFromFile(domainConfigPath.data()))
   {
      if (OS_SUCCESS == domainConfiguration.get(SipXecsService::DomainDbKey::SIP_REALM, mRealm))
      {
         OsSysLog::add(FAC_ACD, PRI_INFO,
                       "ACDServer::ACDServer "
                       "realm='%s'",
                       mRealm.data());
      }
      else
      {
         OsSysLog::add(FAC_ACD, PRI_ERR,
                       "ACDServer::ACDServer"
                       " realm not configured: transfer functions will not work."
                       );
      }
      // Set secret for signing SipXauthIdentity
      SharedSecret secret(domainConfiguration);
      SipXauthIdentity::setSecret(secret.data());
   }
   else
   {
      OsSysLog::add(LOG_FACILITY, PRI_ERR,
                    "ACDServer::ACDServer failed to load domain configuration from '%s'"
                    " realm not configured: transfer functions will not work.",
                    domainConfigPath.data()
                    );
   }

   // If the configuration was succesfully loaded, instantiate the remainder of the components
   if (mConfigurationLoaded) {
      mDefaultIdentity.setUserId(ID_TOKEN);
      mDefaultIdentity.setHostAddress(mDomain);

      // Create the remainder of the server components
      mpAcdCallManager  = new ACDCallManager(this, mUdpPort, mTcpPort, mTlsPort,
                                             mRtpBase, mMaxAcdCallsAllowed,
                                             mDefaultIdentity.toString().data());
      if (mpAcdCallManager->getAcdCallManagerHandle() != SIPX_INST_NULL)
      {
         mpAcdLineManager  = new ACDLineManager(this);
         mpAcdAgentManager = new ACDAgentManager(this, mPresenceMonitorPort, mPresenceServerUriString, mPresenceServiceUriString);
         mpAcdQueueManager = new ACDQueueManager(this);
         mpAcdAudioManager = new ACDAudioManager(this);

         // Initialize the server components
         mpAcdAudioManager->initialize();
         mpAcdQueueManager->initialize();
         mpAcdAgentManager->initialize();
         mpAcdLineManager->initialize();
         mpAcdCallManager->initialize();

         // If the Administrative State == ACTIVE, start the server components
         // The order of component startup is critical.  AgentManager must
         // be started before QueueManager, and CallManager must be last.
         if (mAdministrativeState == ACTIVE) {
            mpAcdAudioManager->start();
            mpAcdAgentManager->start();
            mpAcdLineManager->start();
            mpAcdQueueManager->start();
            mpAcdCallManager->start();

#ifdef CML
            // If a valid ACDRpcServer port has been defined, start it
            if (mAcdRpcServerPort != -1) {
               mpAcdRpcServer = new ACDRpcServer(mpAcdAgentManager, mAcdRpcServerPort);
            }
#endif

            // Indicate that the server is fully operational
            mServerStarted = true;

            if (!mpAcdAgentManager->isOk())
            {
               OsSysLog::add(FAC_ACD, PRI_EMERG,
                     "AcdAgentManager failed to initialize, requesting shutdown");
               gShutdownFlag = true;
            }
         }

         mOperationalState = mAdministrativeState;
      }
      else
      {
         OsSysLog::add(FAC_ACD, PRI_EMERG,
               "AcdCallManager failed to initialize sipXtapi, requesting shutdown");
         gShutdownFlag = true;
      }
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::~ACDServer
//
//  SYNOPSIS:
//
//  DESCRIPTION: Destructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDServer::~ACDServer()
{
   // Shut down the server components
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "SHUTTING DOWN ACD SERVER");

#ifdef CML
   if (mpAcdRpcServer) {
       delete mpAcdRpcServer;
   }
#endif

   if (mpProvisioningAgentXmlRpcAdapter) {
      delete mpProvisioningAgentXmlRpcAdapter;
   }

   if (mpProvisioningAgent) {
      delete mpProvisioningAgent;
   }

   if (mpAcdAudioManager) {
      delete mpAcdAudioManager;
   }

   if (mpAcdCallManager) {
      delete mpAcdCallManager;
   }

   if (mpAcdAgentManager) {
      delete mpAcdAgentManager;
   }

   if (mpAcdQueueManager) {
      delete mpAcdQueueManager;
   }

   if (mpAcdLineManager) {
      delete mpAcdLineManager;
   }

   if (mpAcdRtRecord) {
      ACDRtRecord* pACDRtRec;
      if (NULL != (pACDRtRec=getAcdRtRecord())) {
         pACDRtRec->appendAcdEvent(ACDRtRecord::STOP_ACD);
      }
      delete mpAcdRtRecord;
   }
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::initSysLog
//
//  SYNOPSIS:
//
//  DESCRIPTION: Initialize the OsSysLog
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDServer::initSysLog(const char* pApplication, UtlString& rLogDirectory, UtlString& rLogLevel, bool consoleLogging)
{
   OsSysLog::initialize(0, pApplication);

   //
   // Get/Apply Log Filename
   //
   if (rLogDirectory.isNull() || !OsFileSystem::exists(rLogDirectory)) {
      // If the log file directory exists use that, otherwise place the log
      // in the current directory
      OsPath workingDirectory;
      if (OsFileSystem::exists(CONFIG_LOG_DIR)) {
         rLogDirectory = CONFIG_LOG_DIR;
         OsPath path(rLogDirectory);
         path.getNativePath(workingDirectory);

         osPrintf("<%s> : %s\n", LOG_DIR_TAG, workingDirectory.data());
         OsSysLog::add(LOG_FACILITY, PRI_INFO, "<%s> : %s", LOG_DIR_TAG, workingDirectory.data());
      }
      else {
         OsPath path;
         OsFileSystem::getWorkingDirectory(path);
         path.getNativePath(workingDirectory);

         osPrintf("<%s> : %s\n", LOG_DIR_TAG, workingDirectory.data());
         OsSysLog::add(LOG_FACILITY, PRI_INFO, "<%s> : %s", LOG_DIR_TAG, workingDirectory.data());
      }

      rLogDirectory = workingDirectory;
      mLogFile = workingDirectory +
                 OsPathBase::separator +
                 CONFIG_LOG_FILE;
   }
   else {
      osPrintf("<%s> : %s\n", LOG_DIR_TAG, rLogDirectory.data());
      OsSysLog::add(LOG_FACILITY, PRI_INFO, "<%s> : %s", LOG_DIR_TAG, rLogDirectory.data());

      mLogFile = rLogDirectory +
                 OsPathBase::separator +
                 CONFIG_LOG_FILE;
   }

   OsSysLog::setOutputFile(0, mLogFile);

   // Set the OsSysLog Logging Level
   setSysLogLevel(rLogLevel);

   //
   // Get/Apply console logging
   //
   OsSysLog::enableConsoleOutput(false);
   if (consoleLogging) {
      OsSysLog::enableConsoleOutput(true);
   }

   osPrintf("<%s> : %s\n", LOG_TO_CONSOLE_TAG, consoleLogging ? "ENABLE" : "DISABLE") ;
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "<%s> : %s", LOG_TO_CONSOLE_TAG, consoleLogging ? "ENABLE" : "DISABLE") ;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::setSysLogLevel
//
//  SYNOPSIS:
//
//  DESCRIPTION: Set the OsSysLog Logging Level
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDServer::setSysLogLevel(UtlString& rLogLevel)
{
   OsSysLogPriority newPriority;
   if (! OsSysLog::priority(rLogLevel.data(), newPriority))
   {
      newPriority = PRI_NOTICE;
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDServer::setSysLogLevel invalid log level %s",
                    rLogLevel.data());
   }
   OsSysLog::setLoggingPriority(newPriority);
   OsSysLog::setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::Create
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProvisioningAttrList* ACDServer::Create(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;

   osPrintf("{method} = create\n{object-class} = acd-server\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   // Verify that the required set of attributes are there.
   try {
      rRequestAttributes.validateAttribute(SERVER_NAME_TAG,           ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttribute(DOMAIN_TAG,                ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttribute(FQDN_TAG,                  ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttribute(UDP_PORT_TAG,              ProvisioningAttrList::INT);
      rRequestAttributes.validateAttribute(TCP_PORT_TAG,              ProvisioningAttrList::INT);
      rRequestAttributes.validateAttribute(RTP_PORT_TAG,              ProvisioningAttrList::INT);
      rRequestAttributes.validateAttribute(TLS_PORT_TAG,              ProvisioningAttrList::INT);
      rRequestAttributes.validateAttribute(PRESENCE_MONITOR_PORT_TAG, ProvisioningAttrList::INT);
      rRequestAttributes.validateAttribute(ADMINISTRATIVE_STATE_TAG,  ProvisioningAttrList::INT);
   }
   catch (UtlString error) {
      // We're missing at least one mandatory attribute.
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "create");
      pResponse->setAttribute("result-code", ProvisioningAgent::MISSING_ATTR);
      pResponse->setAttribute("result-text", error);
      return pResponse;
   }

   // Verify that this is for the special "sipxacd" instance.
   if (UtlString(*dynamic_cast<UtlString*>(rRequestAttributes.getAttribute(SERVER_NAME_TAG))) != ACD_SERVER_NAME) {
      // No, Build up the response.
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "create");
      pResponse->setAttribute("result-code", ProvisioningAgent::INVALID_ATTR);
      pResponse->setAttribute("result-text", "Attribute 'server-name' must be set to 'sipxacd'");
      return pResponse;
   }

   // Verify that an instance of this object has not already been created.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_SERVER_TAG, SERVER_NAME_TAG, ACD_SERVER_NAME);
   if (pInstanceNode != NULL) {
      //The instance has already been created, send back the response.
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "create");
#ifdef CML
      pResponse->setAttribute("result-code", ProvisioningAgent::ALREADY_EXISTS);
#else
      pResponse->setAttribute("result-code", ProvisioningAgent::DUPLICATE);
#endif
      pResponse->setAttribute("result-text", "Managed Object Instance already exists");
      return pResponse;
   }

   // Validate that the optional attribute types are correct, ignoring any that are missing
   try {
      rRequestAttributes.validateAttributeType(LOG_DIR_TAG,                 ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(LOG_LEVEL_TAG,               ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(LOG_TO_CONSOLE_TAG,          ProvisioningAttrList::BOOL);
      rRequestAttributes.validateAttributeType(PRESENCE_SERVER_URI_TAG,     ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(PRESENCE_SERVICE_URI_TAG,    ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(RPC_SERVER_PORT_TAG, ProvisioningAttrList::INT);
   }
   catch (UtlString error) {
      // One of the optional attributes is not set to the correct type
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "create");
      pResponse->setAttribute("result-code", ProvisioningAgent::INVALID_ATTR);
      pResponse->setAttribute("result-text", error);
      return pResponse;
   }

   // Create the instance in the configuration file
   pInstanceNode = createPSInstance("acd-server", "server-name", ACD_SERVER_NAME);
   if (pInstanceNode == NULL) {
      // Instance creation failed.
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "create");
      pResponse->setAttribute("result-code", ProvisioningAgent::UNDEFINED);
      pResponse->setAttribute("result-text", "Managed Object Instance creation failed");
      return pResponse;
   }

   // Now save the individual attributes

   // log-dir
   if (rRequestAttributes.getAttribute(LOG_DIR_TAG, mLogDirectory)) {
      setPSAttribute(pInstanceNode, LOG_DIR_TAG, mLogDirectory);
   }

   // log-level
   if (rRequestAttributes.getAttribute(LOG_LEVEL_TAG, mLogLevel)) {
      setPSAttribute(pInstanceNode, LOG_LEVEL_TAG, mLogLevel);
      setSysLogLevel(mLogLevel);
   }

   // log-to-console
   if (rRequestAttributes.getAttribute(LOG_TO_CONSOLE_TAG, mLogToConsole)) {
      setPSAttribute(pInstanceNode, LOG_TO_CONSOLE_TAG, mLogToConsole);
   }

   // domain
   if (rRequestAttributes.getAttribute(DOMAIN_TAG, mDomain)) {
      setPSAttribute(pInstanceNode, DOMAIN_TAG, mDomain);
   }

   // fqdn
   if (rRequestAttributes.getAttribute(FQDN_TAG, mFqdn)) {
      setPSAttribute(pInstanceNode, FQDN_TAG, mFqdn);
   }


   // udp-port
   rRequestAttributes.getAttribute(UDP_PORT_TAG, mUdpPort);
   setPSAttribute(pInstanceNode, UDP_PORT_TAG, mUdpPort);

   // tcp-port
   rRequestAttributes.getAttribute(TCP_PORT_TAG, mTcpPort);
   setPSAttribute(pInstanceNode, TCP_PORT_TAG, mTcpPort);

   // rtp-port
   rRequestAttributes.getAttribute(RTP_PORT_TAG, mRtpBase);
   setPSAttribute(pInstanceNode, RTP_PORT_TAG, mRtpBase);

   // tls-port
   rRequestAttributes.getAttribute(TLS_PORT_TAG, mTlsPort);
   setPSAttribute(pInstanceNode, TLS_PORT_TAG, mTlsPort);

   // dialog-monitor-port
   rRequestAttributes.getAttribute(PRESENCE_MONITOR_PORT_TAG, mPresenceMonitorPort);
   setPSAttribute(pInstanceNode, PRESENCE_MONITOR_PORT_TAG, mPresenceMonitorPort);

   // presence-server-uri
   rRequestAttributes.getAttribute(PRESENCE_SERVER_URI_TAG, mPresenceServerUriString);
   setPSAttribute(pInstanceNode, PRESENCE_SERVER_URI_TAG, mPresenceServerUriString);

   // presence-service-uri
   rRequestAttributes.getAttribute(PRESENCE_SERVICE_URI_TAG, mPresenceServiceUriString);
   setPSAttribute(pInstanceNode, PRESENCE_SERVICE_URI_TAG, mPresenceServiceUriString);


#ifdef CML
   // rpc-server-port
   if (rRequestAttributes.getAttribute(RPC_SERVER_PORT_TAG, mAcdRpcServerPort)) {
      setPSAttribute(pInstanceNode, RPC_SERVER_PORT_TAG, mAcdRpcServerPort);
   }
#endif

   // max-acd-calls-allowed
   if (rRequestAttributes.getAttribute(MAX_CALLS_TAG, mMaxAcdCallsAllowed)) {
      setPSAttribute(pInstanceNode, MAX_CALLS_TAG, mMaxAcdCallsAllowed);
   }

   // administrative-state
   rRequestAttributes.getAttribute(ADMINISTRATIVE_STATE_TAG, mAdministrativeState);

   // Check for DOWN state.
   // If so, set Admin State to STANDBY and shutdown the server.
   if (mAdministrativeState == DOWN) {
      mAdministrativeState = STANDBY;
      mOperationalState = DOWN;
      // Save the configuration.
      setPSAttribute(pInstanceNode, ADMINISTRATIVE_STATE_TAG, mAdministrativeState);
      OsSysLog::add(LOG_FACILITY, PRI_INFO, "ACDServer::Create - Writing out the config file before shutting down the server");
      mpXmlConfigDoc->SaveFile();
      // Flag to restart the server.
      gShutdownFlag = true;
   }
   else {
      mOperationalState = mAdministrativeState;
      setPSAttribute(pInstanceNode, ADMINISTRATIVE_STATE_TAG, mAdministrativeState);

      // Update the configuration file
      OsSysLog::add(LOG_FACILITY, PRI_INFO, "ACDServer::Create - Updating the config file");
      mpXmlConfigDoc->SaveFile();

      // Now that the ACDServer instance has been initialized,
      // create the remainder of the server components
      mpAcdCallManager  = new ACDCallManager(this, mUdpPort, mTcpPort, mTlsPort, mRtpBase, mMaxAcdCallsAllowed);
      mpAcdLineManager  = new ACDLineManager(this);
      mpAcdAgentManager = new ACDAgentManager(this, mPresenceMonitorPort, mPresenceServerUriString, mPresenceServiceUriString);
      mpAcdQueueManager = new ACDQueueManager(this);
      mpAcdAudioManager = new ACDAudioManager(this);

      // Initialize the server components
      mpAcdAudioManager->initialize();
      mpAcdQueueManager->initialize();
      mpAcdAgentManager->initialize();
      mpAcdLineManager->initialize();
      mpAcdCallManager->initialize();

      // If the Administrative State == ACTIVE, start the server components
      // The order of component startup is critical.  AgentManager must
      // be started before QueueManager, and CallManager must be last.
      if (mAdministrativeState == ACTIVE) {
         mpAcdAudioManager->start();
         mpAcdAgentManager->start();
         mpAcdLineManager->start();
         mpAcdQueueManager->start();
         mpAcdCallManager->start();

#ifdef CML
         // If a valid ACDRpcServer port has been defined, start it
         if (mAcdRpcServerPort != -1) {
            mpAcdRpcServer = new ACDRpcServer(mpAcdAgentManager, mAcdRpcServerPort);
         }
#endif

         // Indicate that the server is fully operational
         mServerStarted = true;
      }
   }

   // Build up the response.
   pResponse = new ProvisioningAttrList;
   pResponse->setAttribute("method-name", "create");
   pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
   pResponse->setAttribute("result-text", "SUCCESS");
   return pResponse;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::Delete
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProvisioningAttrList* ACDServer::Delete(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;

   osPrintf("{method} = delete\n{object-class} = acd-server\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   // Verify that this is for the special "sipxacd" instance.
   if (UtlString(*dynamic_cast<UtlString*>(rRequestAttributes.getAttribute(SERVER_NAME_TAG))) != ACD_SERVER_NAME) {
      // No, Build up the response.
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "delete");
      pResponse->setAttribute("result-code", ProvisioningAgent::UNKNOWN_OBJECT);
      pResponse->setAttribute("result-text", "Unknown 'server-name' instance");
      return pResponse;
   }

   // Verify that the instance has already been created.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_SERVER_TAG, SERVER_NAME_TAG, ACD_SERVER_NAME);
   if (pInstanceNode == NULL) {
      // There is no instance.
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "delete");
      pResponse->setAttribute("result-code", ProvisioningAgent::UNKNOWN_OBJECT);
      pResponse->setAttribute("result-text", "Unknown 'server-name' instance");
      return pResponse;
   }

   if (mpAcdCallManager->isThereActiveCalls() == false) {
      // Delete the instance
      deletePSInstance(ACD_SERVER_TAG, SERVER_NAME_TAG, ACD_SERVER_NAME);

      // Update the configuration file
      OsSysLog::add(LOG_FACILITY, PRI_INFO, "ACDServer::Delete - Updating the config file");
      mpXmlConfigDoc->SaveFile();

      // Build up the response.
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "delete");
      pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
      pResponse->setAttribute("result-text", "SUCCESS");

      // Unregister this class with the Provisioning Agent
      mpProvisioningAgent->unregisterClass(this);

      // Now we must restart the server.
      mOperationalState = DOWN;
      gShutdownFlag = true;

      return pResponse;
   }
   else {
      OsSysLog::add(LOG_FACILITY, PRI_WARNING, "ACDServer::Delete There are active calls on this server. Deleting this server instance is prohibited.");

      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "delete");
      pResponse->setAttribute("result-code", ProvisioningAgent::FAILURE);
      pResponse->setAttribute("result-text", "'server-name' is in active mode");

      return pResponse;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::Set
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProvisioningAttrList* ACDServer::Set(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;

   osPrintf("{method} = set\n{object-class} = acd-server\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   // Verify that this is for the special "sipxacd" instance.
   if (UtlString(*dynamic_cast<UtlString*>(rRequestAttributes.getAttribute(SERVER_NAME_TAG))) != ACD_SERVER_NAME) {
      // No, Build up the response.
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "set");
      pResponse->setAttribute("result-code", ProvisioningAgent::UNKNOWN_OBJECT);
      pResponse->setAttribute("result-text", "Unknown instance");
      return pResponse;
   }

   // Verify that the instance has already been created.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_SERVER_TAG, SERVER_NAME_TAG, ACD_SERVER_NAME);
   if (pInstanceNode == NULL) {
      // There is no instance.
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "set");
      pResponse->setAttribute("result-code", ProvisioningAgent::UNKNOWN_OBJECT);
      pResponse->setAttribute("result-text", "Unknown instance");
      return pResponse;
   }

   // Validate that the attribute types are correct, ignoring any that are missing
   try {
      rRequestAttributes.validateAttributeType(LOG_DIR_TAG,                 ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(LOG_LEVEL_TAG,               ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(LOG_TO_CONSOLE_TAG,          ProvisioningAttrList::BOOL);
      rRequestAttributes.validateAttributeType(DOMAIN_TAG,                  ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(FQDN_TAG,                    ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(UDP_PORT_TAG,                ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(TCP_PORT_TAG,                ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(RTP_PORT_TAG,                ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(TLS_PORT_TAG,                ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(PRESENCE_MONITOR_PORT_TAG,   ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(RPC_SERVER_PORT_TAG,         ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(ADMINISTRATIVE_STATE_TAG,    ProvisioningAttrList::INT);
   }
   catch (UtlString error) {
      // One of the attributes is not set to the correct type
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "set");
      pResponse->setAttribute("result-code", ProvisioningAgent::INVALID_ATTR);
      pResponse->setAttribute("result-text", error);
      return pResponse;
   }

   // Now set the individual attributes

   // log-dir
   if (rRequestAttributes.getAttribute(LOG_DIR_TAG, mLogDirectory)) {
      setPSAttribute(pInstanceNode, LOG_DIR_TAG, mLogDirectory);
   }

   // log-level
   if (rRequestAttributes.getAttribute(LOG_LEVEL_TAG, mLogLevel)) {
      setPSAttribute(pInstanceNode, LOG_LEVEL_TAG, mLogLevel);
      setSysLogLevel(mLogLevel);
   }

   // log-to-console
   if (rRequestAttributes.getAttribute(LOG_TO_CONSOLE_TAG, mLogToConsole)) {
      setPSAttribute(pInstanceNode, LOG_TO_CONSOLE_TAG, mLogToConsole);
   }

   // domain
   if (rRequestAttributes.getAttribute(DOMAIN_TAG, mDomain)) {
      setPSAttribute(pInstanceNode, DOMAIN_TAG, mDomain);
   }

   // fqdn
   if (rRequestAttributes.getAttribute(FQDN_TAG, mFqdn)) {
      setPSAttribute(pInstanceNode, FQDN_TAG, mFqdn);
   }


   // udp-port
   if (rRequestAttributes.getAttribute(UDP_PORT_TAG, mUdpPort)) {
      setPSAttribute(pInstanceNode, UDP_PORT_TAG, mUdpPort);
   }

   // tcp-port
   if (rRequestAttributes.getAttribute(TCP_PORT_TAG, mTcpPort)) {
      setPSAttribute(pInstanceNode, TCP_PORT_TAG, mTcpPort);
   }

   // rtp-port
   if (rRequestAttributes.getAttribute(RTP_PORT_TAG, mRtpBase)) {
      setPSAttribute(pInstanceNode, RTP_PORT_TAG, mRtpBase);
   }

   // tls-port
   if (rRequestAttributes.getAttribute(TLS_PORT_TAG, mTlsPort)) {
      setPSAttribute(pInstanceNode, TLS_PORT_TAG, mTlsPort);
   }

   // dialog-monitor-port
   if (rRequestAttributes.getAttribute(PRESENCE_MONITOR_PORT_TAG, mPresenceMonitorPort)) {
      setPSAttribute(pInstanceNode, PRESENCE_MONITOR_PORT_TAG, mPresenceMonitorPort);
   }

#ifdef CML
   // rpc-server-port
   if (rRequestAttributes.getAttribute(RPC_SERVER_PORT_TAG, mAcdRpcServerPort)) {
      setPSAttribute(pInstanceNode, RPC_SERVER_PORT_TAG, mAcdRpcServerPort);
   }
#endif

   // presence-server-uri
   if (rRequestAttributes.getAttribute(PRESENCE_SERVER_URI_TAG, mPresenceServerUriString)) {
      setPSAttribute(pInstanceNode, PRESENCE_SERVER_URI_TAG, mPresenceServerUriString);
   }

   // presence-service-uri
   if (rRequestAttributes.getAttribute(PRESENCE_SERVICE_URI_TAG, mPresenceServiceUriString)) {
      setPSAttribute(pInstanceNode, PRESENCE_SERVICE_URI_TAG, mPresenceServiceUriString);
   }

   // max-acd-calls-allowed
   if (rRequestAttributes.getAttribute(MAX_CALLS_TAG, mMaxAcdCallsAllowed)) {
      setPSAttribute(pInstanceNode, MAX_CALLS_TAG, mMaxAcdCallsAllowed);
   }

   // administrative-state
   if (rRequestAttributes.getAttribute(ADMINISTRATIVE_STATE_TAG, mAdministrativeState)) {
      // Check for DOWN state.
      // If so, set Admin State to STANDBY and shutdown the server.
      if (mAdministrativeState == DOWN) {
         mAdministrativeState = STANDBY;
         mOperationalState = DOWN;
         // Save the configuration.
         setPSAttribute(pInstanceNode, ADMINISTRATIVE_STATE_TAG, mAdministrativeState);
         OsSysLog::add(LOG_FACILITY, PRI_INFO, "ACDServer::Set - Writing out the config file before shutting down the server");
         mpXmlConfigDoc->SaveFile();
         // Flag to restart the server.
         gShutdownFlag = true;
      }
      else {
         // sipXconfig always sets ADMIN_STATE to active as the last step
         // in configuration.  As we don't yet track which items have changed
         // and thus require a restart, for now just always force a restart
         // on configuration change/update.

         // At some point in the future, the individual settings need to
         // determine if they changed, and if that change requires a restart.
         // If so, set gRestartFlag to true.
         //
         // A better approach would be to make the restart flag a parameter
         // and sipXconfig would query it after making a change to see if a
         // restart is required.  If so, it would send a RESTART action command
         // (currently unused and uncoded)
         gRestartFlag = true ;

         mOperationalState = mAdministrativeState;
         setPSAttribute(pInstanceNode, ADMINISTRATIVE_STATE_TAG, mAdministrativeState);

         // If the Administrative State == ACTIVE, start the server components
         // The order of component startup is critical.  AgentManager must
         // be started before QueueManager, and CallManager must be last.
         if ((mAdministrativeState == ACTIVE) && (mServerStarted == false)) {
            mpAcdAudioManager->start();
            mpAcdAgentManager->start();
            mpAcdLineManager->start();
            mpAcdQueueManager->start();
            mpAcdCallManager->start();

#ifdef CML
            // If a valid ACDRpcServer port has been defined, start it
            if (mAcdRpcServerPort != -1) {
               mpAcdRpcServer = new ACDRpcServer(mpAcdAgentManager, mAcdRpcServerPort);
            }
#endif

            // Indicate that the server is fully operational
            mServerStarted = true;
            gRestartFlag = false ;
         }
      }
   }

   // Update the configuration file
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "ACDServer::Set - Updating the config file");
   mpXmlConfigDoc->SaveFile();

   // Build up the response.
   pResponse = new ProvisioningAttrList;
   pResponse->setAttribute("method-name", "set");
   pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
   pResponse->setAttribute("result-text", "SUCCESS");


   // This needs a better place to live.  It is here because of the current
   // behavior of sipXconfig always setting ADMINISTRATIVE_STATE_TAG to
   // active when it has finished sending all the other config changes.
   if (gRestartFlag == true) {
      bool bWatchDogRestarted = false;

      // Construct the URL to the watchdog's XMLRPC server.
      Url targetURL;
      targetURL.setUrlType("https");
      targetURL.setHostAddress(mFqdn.data());
      targetURL.setHostPort(mWatchdogRpcServerPort);
      targetURL.setPath("/RPC2");

      // Make the XMLRPC request to restart this process.
      UtlSList aliases;
      UtlString alias(ACD_SERVER_ALIAS);
      aliases.append(&alias);
      XmlRpcRequest requestRestart(targetURL, "ProcMgmtRpc.restart");
      requestRestart.addParam(&mFqdn);
      requestRestart.addParam(&aliases);
      UtlBool bBlock(false);
      requestRestart.addParam(&bBlock); // No, don't block for the state change.
      XmlRpcResponse response;
      if (!requestRestart.execute(response))
      {
         // The XMLRPC request failed.
         int faultCode;
         UtlString faultString;
         response.getFault(&faultCode, faultString);
         OsSysLog::add(LOG_FACILITY, PRI_CRIT, "ProcMgmtRpc.restart failed, fault %d : %s",
         faultCode, faultString.data());
      }
      else
      {
         // Apparently successful, so extract the result.
         UtlContainable* pValue = NULL;
         if (!response.getResponse(pValue) || !pValue)
         {
            OsSysLog::add(LOG_FACILITY, PRI_CRIT, "ProcMgmtRpc.restart response had no result.");
         }
         else
         {
            UtlHashMap* pResult = dynamic_cast<UtlHashMap*>(pValue);
            if (!pResult)
            {
               OsSysLog::add(LOG_FACILITY, PRI_CRIT,
               "ProcMgmtRpc.restart response result had unexpected type: %s",
               pValue->getContainableType());
            }
            else
            {
               if (!pResult->findValue(&alias))
               {
                  OsSysLog::add(LOG_FACILITY, PRI_ERR, "ProcMgmtRpc.restart could not restart the process.");
               }
               else
               {
                  OsSysLog::add(LOG_FACILITY, PRI_INFO, "ACDServer::Set - Restart %s", mDomain.data());
                  bWatchDogRestarted = true;
               }
            }
         }
      }

      if (!bWatchDogRestarted)
      {
         OsSysLog::add(LOG_FACILITY, PRI_CRIT, "ACDServer::Set - This process could not be controlled by the watchdog.  Exit, stage left.");
         exit(0) ;
      }
   }

   return pResponse;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::Get
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProvisioningAttrList* ACDServer::Get(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;
   UtlString             serverName;

   osPrintf("{method} = get\n{object-class} = acd-server\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   // Extract the instance index from the request attributes.
   if (rRequestAttributes.getAttribute(SERVER_NAME_TAG, serverName)) {
      // A specific instance has been specified, verify that it exists
      TiXmlNode* pInstanceNode = findPSInstance(ACD_SERVER_TAG, SERVER_NAME_TAG, serverName);
      if (pInstanceNode == NULL) {
         // There is no instance.
         pResponse = new ProvisioningAttrList;
         pResponse->setAttribute("method-name", "get");
         pResponse->setAttribute("result-code", ProvisioningAgent::UNKNOWN_OBJECT);
         pResponse->setAttribute("result-text", "Unknown instance");
         return pResponse;
      }

      // Now see if there are any specific attributes listed in the request
      if (rRequestAttributes.attributePresent(LOG_DIR_TAG) ||
          rRequestAttributes.attributePresent(LOG_LEVEL_TAG) ||
          rRequestAttributes.attributePresent(LOG_TO_CONSOLE_TAG) ||
          rRequestAttributes.attributePresent(DOMAIN_TAG) ||
          rRequestAttributes.attributePresent(FQDN_TAG) ||
          rRequestAttributes.attributePresent(UDP_PORT_TAG) ||
          rRequestAttributes.attributePresent(TCP_PORT_TAG) ||
          rRequestAttributes.attributePresent(RTP_PORT_TAG) ||
          rRequestAttributes.attributePresent(TLS_PORT_TAG) ||
          rRequestAttributes.attributePresent(PRESENCE_MONITOR_PORT_TAG) ||
          rRequestAttributes.attributePresent(RPC_SERVER_PORT_TAG) ||
          rRequestAttributes.attributePresent(ADMINISTRATIVE_STATE_TAG)) {
         // At least one attribute has been requested, go through list and retrieve
         pResponse = new ProvisioningAttrList;

         // log-dir
         if (rRequestAttributes.attributePresent(LOG_DIR_TAG)) {
            pResponse->setAttribute(LOG_DIR_TAG, mLogDirectory);
         }

         // log-level
         if (rRequestAttributes.attributePresent(LOG_LEVEL_TAG)) {
            pResponse->setAttribute(LOG_LEVEL_TAG, mLogLevel);
         }

         // log-to-console
         if (rRequestAttributes.attributePresent(LOG_TO_CONSOLE_TAG)) {
            pResponse->setAttribute(LOG_TO_CONSOLE_TAG, mLogToConsole);
         }

         // domain
         if (rRequestAttributes.attributePresent(DOMAIN_TAG)) {
            pResponse->setAttribute(DOMAIN_TAG, mDomain);
         }
         // fqdn
         if (rRequestAttributes.attributePresent(FQDN_TAG)) {
            pResponse->setAttribute(FQDN_TAG, mFqdn);
         }

         // udp-port
         if (rRequestAttributes.attributePresent(UDP_PORT_TAG)) {
            pResponse->setAttribute(UDP_PORT_TAG, mUdpPort);
         }

         // tcp-port
         if (rRequestAttributes.attributePresent(TCP_PORT_TAG)) {
            pResponse->setAttribute(TCP_PORT_TAG, mTcpPort);
         }

         // rtp-port
         if (rRequestAttributes.attributePresent(RTP_PORT_TAG)) {
            pResponse->setAttribute(RTP_PORT_TAG, mRtpBase);
         }

         // tls-port
         if (rRequestAttributes.attributePresent(TLS_PORT_TAG)) {
            pResponse->setAttribute(TLS_PORT_TAG, mTlsPort);
         }

         // dialog-monitor-port
         if (rRequestAttributes.attributePresent(PRESENCE_MONITOR_PORT_TAG)) {
            pResponse->setAttribute(PRESENCE_MONITOR_PORT_TAG, mPresenceMonitorPort);
         }

         // presence-server-uri
         if (rRequestAttributes.attributePresent(PRESENCE_SERVER_URI_TAG)) {
            pResponse->setAttribute(PRESENCE_SERVER_URI_TAG, mPresenceServerUriString);
         }

         // presence-service-uri
         if (rRequestAttributes.attributePresent(PRESENCE_SERVICE_URI_TAG)) {
            pResponse->setAttribute(PRESENCE_SERVICE_URI_TAG, mPresenceServiceUriString);
         }

#ifdef CML
         // rpc-server-port
         if (rRequestAttributes.attributePresent(RPC_SERVER_PORT_TAG)) {
            pResponse->setAttribute(RPC_SERVER_PORT_TAG, mAcdRpcServerPort);
         }
#endif

         // administrative-state
         if (rRequestAttributes.attributePresent(ADMINISTRATIVE_STATE_TAG)) {
            pResponse->setAttribute(ADMINISTRATIVE_STATE_TAG, mAdministrativeState);
         }

         // operational-state
         if (rRequestAttributes.attributePresent(OPERATIONAL_STATE_TAG)) {
            pResponse->setAttribute(OPERATIONAL_STATE_TAG, mOperationalState);
         }

         // max-acd-calls-allowed
         if (rRequestAttributes.attributePresent(MAX_CALLS_TAG)) {
            pResponse->setAttribute(MAX_CALLS_TAG, mMaxAcdCallsAllowed);
         }

         // Send back the response
         pResponse->setAttribute("method-name", "get");
         pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
         pResponse->setAttribute("result-text", "SUCCESS");
         pResponse->setAttribute("object-class", ACD_SERVER_TAG);
         pResponse->setAttribute(SERVER_NAME_TAG, serverName);
         return pResponse;
      }
      else {
         // No specific attributes were requested, send them all back
         pResponse = new ProvisioningAttrList;

         // log-dir
         pResponse->setAttribute(LOG_DIR_TAG, mLogDirectory);

         // log-level
         pResponse->setAttribute(LOG_LEVEL_TAG, mLogLevel);

         // log-to-console
         pResponse->setAttribute(LOG_TO_CONSOLE_TAG, mLogToConsole);

         // domain
         pResponse->setAttribute(DOMAIN_TAG, mDomain);

         // fqdn
         pResponse->setAttribute(FQDN_TAG, mFqdn);

         // udp-port
         pResponse->setAttribute(UDP_PORT_TAG, mUdpPort);

         // tcp-port
         pResponse->setAttribute(TCP_PORT_TAG, mTcpPort);

         // rtp-port
         pResponse->setAttribute(RTP_PORT_TAG, mRtpBase);

         // tls-port
         pResponse->setAttribute(TLS_PORT_TAG, mTlsPort);

         // dialog-monitor-port
         pResponse->setAttribute(PRESENCE_MONITOR_PORT_TAG, mPresenceMonitorPort);

         // presence-server-uri
         pResponse->setAttribute(PRESENCE_SERVER_URI_TAG, mPresenceServerUriString);

         // presence-service-uri
         pResponse->setAttribute(PRESENCE_SERVICE_URI_TAG, mPresenceServiceUriString);
#ifdef CML
         // rpc-server-port
         pResponse->setAttribute(RPC_SERVER_PORT_TAG, mAcdRpcServerPort);
#endif

         // administrative-state
         pResponse->setAttribute(ADMINISTRATIVE_STATE_TAG, mAdministrativeState);

         // operational-state
         pResponse->setAttribute(OPERATIONAL_STATE_TAG, mOperationalState);

         // max-acd-calls-allowed
         pResponse->setAttribute(MAX_CALLS_TAG, mMaxAcdCallsAllowed);

         // Send back the response
         pResponse->setAttribute("method-name", "get");
         pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
         pResponse->setAttribute("result-text", "SUCCESS");
         pResponse->setAttribute("object-class", ACD_SERVER_TAG);
         pResponse->setAttribute(SERVER_NAME_TAG, serverName);
         return pResponse;
      }
   }
   else {
      // No specific instance was requested, send back a list of the available instances
      ProvisioningAttrList* pServerObject;
      UtlSList* pServerList = new UtlSList;

      // Create the list of available instances
      if (findPSInstance(ACD_SERVER_TAG) != NULL) {
         pServerObject = new ProvisioningAttrList;
         pServerObject->setAttribute("object-class", ACD_SERVER_TAG);
         pServerObject->setAttribute(SERVER_NAME_TAG, ACD_SERVER_NAME);
         pServerObject->setAttribute(LOG_DIR_TAG, mLogDirectory);
         pServerObject->setAttribute(LOG_LEVEL_TAG, mLogLevel);
         pServerList->append(pServerObject->getData());
      }

      // Send back the response
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "get");
      pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
      pResponse->setAttribute("result-text", "SUCCESS");
      pResponse->setAttribute("object-class-list", pServerList);
      return pResponse;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::Action
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProvisioningAttrList* ACDServer::Action(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;
   UtlString             timeString;

   osPrintf("{method} = action\n{object-class} = acd-server\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   // Extract the action attribute from the request
   // operate on either restart or shutdown attributes
   if (rRequestAttributes.getAttribute(ACTION_RESTART_TAG, timeString)) {
      // restart operation

      // Build up the response.
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "action");
      pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
      pResponse->setAttribute("result-text", "SUCCESS");
      return pResponse;
   }
   else if (rRequestAttributes.getAttribute(ACTION_SHUTDOWN_TAG, timeString)) {
      // shutdown operation

      // Build up the response.
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "action");
      pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
      pResponse->setAttribute("result-text", "SUCCESS");
      return pResponse;
   }
   else {
      // Unrecognized or missing action-object
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "action");
      pResponse->setAttribute("result-code", ProvisioningAgent::FAILURE);
      pResponse->setAttribute("result-text", "Invalid action operation");
      return pResponse;
   }
}

/* ============================ ACCESSORS ================================= */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::loadConfiguration
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ACDServer::loadConfiguration(void)
{
   // Verify that the instance has already been created.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_SERVER_TAG, SERVER_NAME_TAG, ACD_SERVER_NAME);
   if (pInstanceNode == NULL) {
      // There is no instance.
      return false;
   }

   // Read in the parameters from the configuration file
   getPSAttribute(pInstanceNode, LOG_DIR_TAG,                 mLogDirectory);
   getPSAttribute(pInstanceNode, LOG_LEVEL_TAG,               mLogLevel);
   getPSAttribute(pInstanceNode, LOG_TO_CONSOLE_TAG,          mLogToConsole);
   getPSAttribute(pInstanceNode, DOMAIN_TAG,                  mDomain);
   getPSAttribute(pInstanceNode, FQDN_TAG,                    mFqdn);
   getPSAttribute(pInstanceNode, UDP_PORT_TAG,                mUdpPort);
   getPSAttribute(pInstanceNode, TCP_PORT_TAG,                mTcpPort);
   getPSAttribute(pInstanceNode, RTP_PORT_TAG,                mRtpBase);
   getPSAttribute(pInstanceNode, TLS_PORT_TAG,                mTlsPort);
   getPSAttribute(pInstanceNode, PRESENCE_MONITOR_PORT_TAG,   mPresenceMonitorPort);
   getPSAttribute(pInstanceNode, PRESENCE_SERVER_URI_TAG,     mPresenceServerUriString);
   getPSAttribute(pInstanceNode, PRESENCE_SERVICE_URI_TAG,    mPresenceServiceUriString);
#ifdef CML
   getPSAttribute(pInstanceNode, RPC_SERVER_PORT_TAG,         mAcdRpcServerPort);
#endif
   getPSAttribute(pInstanceNode, ADMINISTRATIVE_STATE_TAG,    mAdministrativeState);
   getPSAttribute(pInstanceNode, MAX_CALLS_TAG,               mMaxAcdCallsAllowed);

   mConfigurationLoaded = true;

   return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::getProvisioningAgent
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProvisioningAgent* ACDServer::getProvisioningAgent(void)
{
   return mpProvisioningAgent;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::getAcdCallManager
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallManager* ACDServer::getAcdCallManager(void)
{
   return mpAcdCallManager;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::getAcdLineManager
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDLineManager* ACDServer::getAcdLineManager(void)
{
   return mpAcdLineManager;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::getAcdAgentManager
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDAgentManager* ACDServer::getAcdAgentManager(void)
{
   return mpAcdAgentManager;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::getAcdQueueManager
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDQueueManager* ACDServer::getAcdQueueManager(void)
{
   return mpAcdQueueManager;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::getAcdAudioManager
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDAudioManager* ACDServer::getAcdAudioManager(void)
{
   return mpAcdAudioManager;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::getAcdRtRecord
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDRtRecord* ACDServer::getAcdRtRecord(void)
{
   return mpAcdRtRecord;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::getDomain
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

const char* ACDServer::getDomain(void)
{
   return mDomain.data();
}

const char* ACDServer::getRealm(void)
{
   return mRealm.data();
}

void ACDServer::getDefaultIdentity(Url& id)
{
   id = mDefaultIdentity;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDServer::getAdministrativeState
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

int ACDServer::getAdministrativeState(void)
{
   return mAdministrativeState;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
