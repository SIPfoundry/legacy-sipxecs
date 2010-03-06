//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "os/OsSysLog.h"
#include "AppearanceAgent.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

const UtlContainableType AppearanceAgent::TYPE = "AppearanceAgent";

// Milliseconds of delay to allow between calls that add or delete resources
// to/from Appearance Groups when doing bulk updating of Appearance Groups.
const int AppearanceAgent::sChangeDelay = 10;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
AppearanceAgent::AppearanceAgent(const UtlString& domainName,
                                       const UtlString& realm,
                                       SipLineMgr* lineMgr,
                                       int tcpPort,
                                       int udpPort,
                                       int tlsPort,
                                       const UtlString& bindIp,
                                       UtlString* appearanceGroupFile,
                                       int refreshInterval,
                                       int resubscribeInterval,
                                       int minResubscribeInterval,
                                       int seizedResubscribeInterval,
                                       int publishingDelay,
                                       int maxRegSubscInGroup,
                                       int serverMinExpiration,
                                       int serverDefaultExpiration,
                                       int serverMaxExpiration,
                                       const UtlString&  subscriptionDbName,
                                       const UtlString&  credentialDbName) :
   mDomainName(domainName),
   mAppearanceGroupFile(*appearanceGroupFile),
   mRefreshInterval(refreshInterval),
   mResubscribeInterval(resubscribeInterval),
   mMinResubscribeInterval(minResubscribeInterval),
   mSeizedResubscribeInterval(seizedResubscribeInterval),
   mPublishingDelay(publishingDelay),
   mMaxRegSubscInGroup(maxRegSubscInGroup),
   mServerUserAgent(
      tcpPort, // sipTcpPort
      udpPort, // sipUdpPort
      tcpPort, // sipTlsPort
      NULL, // publicAddress
      NULL, // defaultUser
      bindIp, // defaultSipAddress
      domainName.data(), // sipProxyServers (outbound proxy)
      NULL, // sipDirectoryServers
      NULL, // sipRegistryServers
      NULL, // authenicateRealm
      NULL, // authenticateDb
      NULL, // authorizeUserIds
      NULL, // authorizePasswords
      NULL, // lineMgr
      SIP_DEFAULT_RTT, // sipFirstResendTimeout
      TRUE, // defaultToUaTransactions
      -1, // readBufferSize
      OsServerTask::DEF_MAX_MSGS, // queueSize
      FALSE // bUseNextAvailablePort
      ),
   mSubscriptionMgr(SUBSCRIPTION_COMPONENT_SAA, mDomainName, subscriptionDbName),
   mPolicyHolder(domainName, realm, credentialDbName),
   mSubscribeServer(SipSubscribeServer::terminationReasonSilent,
                    mServerUserAgent, mEventPublisher, mSubscriptionMgr,
                    mPolicyHolder),
   mRefreshMgr(mServerUserAgent, mDialogManager),
   mSubscribeClient(mServerUserAgent, mDialogManager, mRefreshMgr),
   mAppearanceAgentTask(this),
   mAppearanceGroupSet(this),
   // Do not set the appearance group file name yet, so the AppearanceGroupFileReader
   // doesn't add elements to the AppearanceGroupSet before we have the
   // SIP tasks set up.
   mAppearanceGroupFileReader(UtlString(""), &mAppearanceGroupSet)
{
   OsSysLog::add(FAC_SAA, PRI_DEBUG,
                 "AppearanceAgent::_ this = %p, mDomainName = '%s', "
                 "mRefreshInterval = %d, mResubscribeInterval = %d",
                 this, mDomainName.data(), mRefreshInterval, mResubscribeInterval);
   OsSysLog::add(FAC_SAA, PRI_DEBUG,
                 "AppearanceAgent::_ this = %p, mPublishingDelay = %d, mMaxRegSubscInGroup = %zu",
                 this, publishingDelay, mMaxRegSubscInGroup);

   // Initialize the call processing objects.

   // contact address to be used in outgoing requests (primarily SUBSCRIBEs)
   mServerFromURI = "sip:sipXsaa@";
   mServerFromURI.append(mDomainName);
   mServerUserAgent.getContactURI(mServerContactURI);

   // Initialize the SipUserAgents.
   // Set the user-agent strings.
   mServerUserAgent.setUserAgentHeaderProperty("sipXecs/saa");

   // Set the subscribe server grant times.
   if (!mSubscriptionMgr.setSubscriptionTimes(serverMinExpiration,
                                              serverDefaultExpiration,
                                              serverMaxExpiration))
   {
      OsSysLog::add(FAC_SAA, PRI_ERR,
                    "AppearanceAgent given unacceptable server subscription times: min = %d, default = %d, max = %d.  Using the default subscription times.",
                    serverMinExpiration,
                    serverDefaultExpiration,
                    serverMaxExpiration);
   }
}

// Destructor
AppearanceAgent::~AppearanceAgent()
{
}

/* ============================ MANIPULATORS ============================== */

// Start the server.
void AppearanceAgent::start()
{
   // Start them.
   mServerUserAgent.start();

   // Set up the SIP Subscribe Client
   mRefreshMgr.start();
   mSubscribeClient.start();

   // Start the AppearanceAgentTask
   mAppearanceAgentTask.start();

   // Start the AppearanceGroupFileReader by giving it the file name.
   // Do this after starting all the subscription client server tasks,
   // as otherwise it will fill their queues.
   mAppearanceGroupFileReader.setFileName(&mAppearanceGroupFile);

   // Start the SIP Subscribe Server after the AppearanceGroupFileReader is
   // done loading the configuration.  This ensures that early subscribers
   // do not get NOTIFYs with incomplete information.
   if (!mSubscribeServer.enableEventType(DIALOG_EVENT_TYPE, NULL, NULL, NULL, NULL,
         SipSubscribeServer::standardVersionCallback, FALSE))
   {
      OsSysLog::add(FAC_SAA, PRI_CRIT, "AppearanceAgent:: enableEventType failed");
   }
   mSubscribeServer.start();

   // Install a listener for MESSAGE requests into the server which queues
   // them for consideration by mAppearanceAgentTask.
   mServerUserAgent.addMessageObserver(*(mAppearanceAgentTask.getMessageQueue()),
         SIP_MESSAGE_METHOD, TRUE, // yes requests
         FALSE, // no responses
         TRUE, // incoming,
         FALSE // outgoing
   );

   OsSysLog::add(FAC_SAA, PRI_DEBUG, "AppearanceAgent::_ Initialization done.");
}

// Shut down the server.
void AppearanceAgent::shutdown()
{
   // Close down the call processing objects.

   OsSysLog::add(FAC_SAA, PRI_INFO,
                 "AppearanceAgent::shutdown this = %p",
                 this);

   mAppearanceGroupSet.deleteAllAppearanceGroups();
   OsSysLog::add(FAC_SAA, PRI_DEBUG,
                 "AppearanceAgent::shutdown back from deleteAllAppearanceGroups"
                 );

   // Stop all the subscriptions so callbacks are no longer activated.
   mSubscribeClient.endAllSubscriptions();
   OsSysLog::add(FAC_SAA, PRI_DEBUG,
                 "AppearanceAgent::shutdown back from mSubscribeClient.endAllSubscriptions"
                 );

   // Stop the SIP subscribe client.
   mSubscribeClient.requestShutdown();
   OsSysLog::add(FAC_SAA, PRI_DEBUG,
                 "AppearanceAgent::shutdown back from mSubscribeClient.requestShutdown"
                 );
   // Stop the refresh manager.
   mRefreshMgr.requestShutdown();
   OsSysLog::add(FAC_SAA, PRI_DEBUG,
                 "AppearanceAgent::shutdown back from mRefreshMgr.requestShutdown"
                 );
   // Stop the subscribe server.
   mSubscribeServer.requestShutdown();
   OsSysLog::add(FAC_SAA, PRI_DEBUG,
                 "AppearanceAgent::shutdown back from mSubscribeServer.requestShutdown"
                 );

   // Shut down SipUserAgent
   mServerUserAgent.shutdown(FALSE);

   // Shut down the AppearanceAgentTask
   mAppearanceAgentTask.requestShutdown();

   // Wait for all tasks to clean up
   while(!(mServerUserAgent.isShutdownDone() &&
           mAppearanceAgentTask.isShutDown()) &&
           mSubscribeClient.isShutDown() &&
           mRefreshMgr.isShutDown() &&
           mSubscribeServer.isShutDown())
   {
      OsTask::delay(100);
   }
}

/* ============================ ACCESSORS ================================= */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ INQUIRY =================================== */

// Dump the object's internal state.
void AppearanceAgent::dumpState()
{
   // indented 0

   OsSysLog::add(FAC_SAA, PRI_INFO,
                 "\tAppearanceAgent %p", this);
   mSubscribeServer.dumpState();
   mSubscribeClient.dumpState();
   mAppearanceGroupSet.dumpState();
}

/**
 * Get the ContainableType for a UtlContainable-derived class.
 */
UtlContainableType AppearanceAgent::getContainableType() const
{
   return AppearanceAgent::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
