//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include <net/SipMessage.h>
#include <os/OsSysLog.h>
#include <os/OsDefs.h>
#include <sipdb/SubscriptionDB.h>
#include "ResourceListServer.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// URN for the xmlns attribute for Resource List Meta-Information XML.
#define RLMI_XMLNS "urn:ietf:params:xml:ns:rlmi"
// MIME information for RLMI XML.
#define RLMI_CONTENT_TYPE "application/rlmi+xml"

// STATIC VARIABLE INITIALIZATIONS

const UtlContainableType ResourceListServer::TYPE = "ResourceListServer";

// Milliseconds of delay to allow between calls that add or delete resources
// to/from ResourceList's when doing bulk updating of ResourceList's.
const int ResourceListServer::sChangeDelay = 100;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ResourceListServer::ResourceListServer(const UtlString& domainName,
                                       const UtlString& realm,
                                       SipLineMgr* lineMgr,
                                       const char* eventType,
                                       const char* contentType,
                                       int tcpPort,
                                       int udpPort,
                                       int tlsPort,
                                       const UtlString& bindIp,
                                       UtlString* resourceListFile,
                                       int resubscribeInterval,
                                       int minResubscribeInterval,
                                       int publishingDelay,
                                       int maxRegSubscInResource,
                                       int maxContInRegSubsc,
                                       int maxResInstInCont,
                                       int maxDialogsInResInst,
                                       int serverMinExpiration,
                                       int serverDefaultExpiration,
                                       int serverMaxExpiration,
                                       const UtlString&  subscriptionDbName,
                                       const UtlString&  credentialDbName) :
   mDomainName(domainName),
   mEventType(eventType),
   mContentType(contentType),
   mResourceListFile(*resourceListFile),
   mResubscribeInterval(resubscribeInterval),
   mMinResubscribeInterval(minResubscribeInterval),
   mPublishingDelay(publishingDelay),
   mMaxRegSubscInResource(maxRegSubscInResource),
   mMaxContInRegSubsc(maxContInRegSubsc),
   mMaxResInstInCont(maxResInstInCont),
   mMaxDialogsInResInst(maxDialogsInResInst),
   mServerUserAgent(
      tcpPort, // sipTcpPort
      udpPort, // sipUdpPort
      tcpPort, // sipTlsPort
      NULL, // publicAddress
      NULL, // defaultUser
      bindIp, // defaultSipAddress
      NULL, // sipProxyServers
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
   mClientUserAgent(
      PORT_DEFAULT, // sipTcpPort
      PORT_DEFAULT, // sipUdpPort
      PORT_DEFAULT, // sipTlsPort
      NULL, // publicAddress
      NULL, // defaultUser
      bindIp, // defaultSipAddress
      domainName.data(), // sipProxyServers
      NULL, // sipDirectoryServers
      NULL, // sipRegistryServers
      NULL, // authenicateRealm
      NULL, // authenticateDb
      NULL, // authorizeUserIds
      NULL, // authorizePasswords
      lineMgr, // lineMgr
      SIP_DEFAULT_RTT, // sipFirstResendTimeout
      TRUE, // defaultToUaTransactions
      -1, // readBufferSize
      OsServerTask::DEF_MAX_MSGS, // queueSize
      FALSE // bUseNextAvailablePort
      ),
   mSubscriptionMgr(SUBSCRIPTION_COMPONENT_RLS, mDomainName, subscriptionDbName),
   mPolicyHolder(domainName, realm, credentialDbName),
   mSubscribeServer(mServerUserAgent, mEventPublisher, mSubscriptionMgr,
                    mPolicyHolder),
   mRefreshMgr(mClientUserAgent, mDialogManager),
   mSubscribeClient(mClientUserAgent, mDialogManager, mRefreshMgr),
   mResourceListTask(this),
   mResourceListSet(this),
   // Do not set the resource list file name yet, so the ResourceListFileReader
   // doesn't add elements to the ResourceListSet before we have the
   // SIP tasks set up.
   mResourceListFileReader(UtlString(""), &mResourceListSet)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListServer::_ this = %p, mDomainName = '%s', mEventType = '%s', mContentType = '%s', "
                 "mResubscribeInterval = %d",
                 this, mDomainName.data(), mEventType.data(), mContentType.data(),
                 mResubscribeInterval);
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListServer::_ this = %p, mPublishingDelay = %d, mMaxRegSubscInResource = %d, "
                 "mMaxContInRegSubsc = %d, mMaxResInstInCont = %d, mMaxDialogsInResInst = %d",
                 this, publishingDelay, mMaxRegSubscInResource, mMaxContInRegSubsc, mMaxResInstInCont,
                 mMaxDialogsInResInst);

   // Initialize the call processing objects.

   // Construct addresses:
   // our local host-part
   // contact address to be used in client outgoing requests (primarily SUBSCRIBEs)
   {
      // Get our address and the client port.
      UtlString localAddress;
      int localPort;
      mClientUserAgent.getLocalAddress(&localAddress, &localPort);

      char buffer[100];

      // Construct the server's host-part.
      sprintf(buffer, "%s:%d", localAddress.data(), portIsValid(udpPort) ? udpPort : tcpPort);
      mServerLocalHostPart = buffer;

      // Construct the client's From URI.
      sprintf(buffer, "sip:sipXrls@%s:%d", localAddress.data(), localPort);
      mClientFromURI = buffer;

      // Obtain the client's Contact URI.
      mClientUserAgent.getContactURI(mClientContactURI);
   }

   // Initialize the SipUserAgent's.
   // Set the user-agent strings.
   mServerUserAgent.setUserAgentHeaderProperty("sipXecs/rls");
   mClientUserAgent.setUserAgentHeaderProperty("sipXecs/rls");

   // Require the "eventlist" extension in the Resource List clients.
   mServerUserAgent.requireExtension(SIP_EVENTLIST_EXTENSION);

   // Set the subscribe server grant times.
   if (!mSubscriptionMgr.setSubscriptionTimes(serverMinExpiration,
                                              serverDefaultExpiration,
                                              serverMaxExpiration))
   {
      OsSysLog::add(FAC_RLS, PRI_ERR,
                    "ResourceListServer given unacceptable server subscription times: min = %d, default = %d, max = %d.  Using the default subscription times.",
                    serverMinExpiration,
                    serverDefaultExpiration,
                    serverMaxExpiration);
   }
}

// Destructor
ResourceListServer::~ResourceListServer()
{
   // Final stage of closing down the call processing objects.

   // Stop the SipUserAgent's.
   mServerUserAgent.shutdown(TRUE);
   mClientUserAgent.shutdown(TRUE);
}

/* ============================ MANIPULATORS ============================== */

// Start the server.
void ResourceListServer::start()
{
   // Start them.
   mServerUserAgent.start();
   mClientUserAgent.start();

   // Set up the SIP Subscribe Client
   mRefreshMgr.start();
   mSubscribeClient.start();

   // Start the ResourceListTask.
   mResourceListTask.start();

   // Start the ResourceListFileReader by giving it the file name.
   // Do this after starting all the subscription client server tasks,
   // as otherwise it will fill their queues.
   mResourceListFileReader.setFileName(&mResourceListFile);

   // Start the SIP Subscribe Server after the ResourceListFileReader is
   // done loading the configuration.  This ensures that early subscribers
   // do not get NOTIFYs with incomplete information.
   mSubscribeServer.enableEventType(mEventType, NULL, NULL, NULL, NULL,
                                    SipSubscribeServer::standardVersionCallback,
                                    FALSE);
   mSubscribeServer.start();

   // Install a listener for MESSAGE requests into the server which queues
   // them for consideration by mResourceListTask.  The task will be triggered
   // to perform various debugging tasks.
   mServerUserAgent.addMessageObserver(*(mResourceListTask.getMessageQueue()),
         SIP_MESSAGE_METHOD, TRUE, // yes requests
         FALSE, // no responses
         TRUE, // incoming,
         FALSE // outgoing
   );

   OsSysLog::add(FAC_RLS, PRI_DEBUG, "ResourceListServer::_ Initialization done.");
}

// Shut down the server.
void ResourceListServer::shutdown()
{
   // Close down the call processing objects.

   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListServer::shutdown this = %p",
                 this);

   // Probably best not to delete the resources in the resource lists so that
   // we do not send NOTIFYs to the clients showing the lists as empty.
   // Instead, we force the subscribe server to just tell the subscribers
   // that their subscriptions are terminated.

   // Stop all the subscriptions so callbacks are no longer activated.
   mSubscribeClient.endAllSubscriptions();

   // Finalize ResourceListSet, so timers stop queueing messages to
   // ResourceList Task and there are no references to the
   // ResourceCached's.
   mResourceListSet.finalize();

   // Stop the SIP subscribe client.
   mSubscribeClient.requestShutdown();
   // Stop the refresh manager.
   mRefreshMgr.requestShutdown();
   // Stop the subscribe server.
   mSubscribeServer.requestShutdown();

   // Shut down SipUserAgent's.
   // Shut down the ResourceListTask.
   mServerUserAgent.shutdown(FALSE);
   mClientUserAgent.shutdown(FALSE);
   mResourceListTask.requestShutdown();
   while (!(mServerUserAgent.isShutdownDone() &&
            mClientUserAgent.isShutdownDone() &&
            mResourceListTask.isShutDown()))
   {
      OsTask::delay(100);
   }
}

/* ============================ ACCESSORS ================================= */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ INQUIRY =================================== */

// Dump the object's internal state.
void ResourceListServer::dumpState()
{
   // indented 0

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\tResourceListServer %p", this);
   mResourceListSet.dumpState();
   mSubscribeServer.dumpState();
   mSubscribeClient.dumpState();
}

/**
 * Get the ContainableType for a UtlContainable-derived class.
 */
UtlContainableType ResourceListServer::getContainableType() const
{
   return ResourceListServer::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
