//
// Copyright (C) 2007, 2010 Avaya, Inc., certain elements licensed under a Contributor Agreement.
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
#if defined(_WIN32)
#       include "resparse/wnt/nterrno.h"
#elif defined(__pingtel_on_posix__)
#	include <sys/types.h>
#       include <sys/socket.h>
#       include <stdlib.h>
#endif

#include <utl/UtlHashBagIterator.h>
#include <utl/UtlSortedListIterator.h>
#include <net/SipSrvLookup.h>
#include <net/SipUserAgent.h>
#include <net/SipSession.h>
#include <net/SipMessageEvent.h>
#include <net/NameValueTokenizer.h>
#include <net/SipObserverCriteria.h>
#include <os/HostAdapterAddress.h>
#include <net/Url.h>
#ifdef SIP_TLS
#include <net/SipTlsServer.h>
#endif
#include <net/SipTcpServer.h>
#include <net/SipUdpServer.h>
#include <net/SipLineMgr.h>
#include <os/OsDateTime.h>
#include <os/OsEvent.h>
#include <os/OsQueuedEvent.h>
#include <os/OsTimer.h>
#include <os/OsTimerTask.h>
#include <os/OsEventMsg.h>
#include <os/OsRpcMsg.h>
#include <os/OsConfigDb.h>
#include <os/OsRWMutex.h>
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#ifndef _WIN32
// version.h is generated as part of the build by other platforms.  For
// windows, the sip stack version is defined under the project settings.
#include <net/version.h>
#endif
#include <os/OsSysLog.h>
#include <os/OsFS.h>
#include <utl/UtlTokenizer.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// Default value for mMaxTcpSocketIdleTime, which is how long a TCP socket
// can be idle before it is garbage collected.
// Default value is 5 minutes.
#define DEFAULT_TCP_SOCKET_IDLE_TIME 300

#define MAXIMUM_SIP_LOG_SIZE 100000
#define SIP_UA_LOG "sipuseragent.log"
#define CONFIG_LOG_DIR SIPX_LOGDIR

#ifndef  VENDOR
# define VENDOR "sipXecs"
#endif

#ifndef PLATFORM_UA_PARAM
#if defined(_WIN32)
#  define PLATFORM_UA_PARAM " (WinNT)"
#elif defined(_VXWORKS)
#  define PLATFORM_UA_PARAM " (VxWorks)"
#elif defined(__MACH__)
#  define PLATFORM_UA_PARAM " (Darwin)"
#elif defined(__linux__)
#  define PLATFORM_UA_PARAM " (Linux)"
#elif defined(sun)
#  define PLATFORM_UA_PARAM " (Solaris)"
#elif defined(__hpux)
#  define PLATFORM_UA_PARAM " (HP-UX)"
#elif defined(__FreeBSD__)
#  define PLATFORM_UA_PARAM " (FreeBSD)"
#endif
#endif /* PLATFORM_UA_PARAM */

//#define LOG_TIME
//#define TRANSACTION_MATCH_DEBUG // enable only for transaction match debugging - log is confusing otherwise

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipUserAgent::SipUserAgent(int sipTcpPort,
                           int sipUdpPort,
                           int sipTlsPort,
                           const char* publicAddress,
                           const char* defaultUser,
                           const char* defaultAddress,
                           const char* sipProxyServers,
                           const char* sipDirectoryServers,
                           const char* sipRegistryServers,
                           const char* authenticateRealm,
                           OsConfigDb* authenticateDb,
                           OsConfigDb* authorizeUserIds,
                           OsConfigDb* authorizePasswords,
                           SipLineMgr* lineMgr,
                           int sipUnreliableTransportTimeout,
                           UtlBoolean defaultToUaTransactions,
                           int readBufferSize,
                           int queueSize,
                           UtlBoolean bUseNextAvailablePort,
                           UtlBoolean doUaMessageChecks,
                           UtlBoolean bForceSymmetricSignaling,
                           OptionsRequestHandlePref howTohandleOptionsRequest
                           )
        : SipUserAgentBase(sipTcpPort, sipUdpPort, sipTlsPort, queueSize)
        , mSipTcpServer(NULL)
        , mSipUdpServer(NULL)
        , mSipTlsServer(NULL)
        , mMessageLogRMutex(OsRWMutex::Q_FIFO)
        , mMessageLogWMutex(OsRWMutex::Q_FIFO)
        , mOutputProcessorMutex(OsRWMutex::Q_FIFO)
        , mpLineMgr(NULL)
        , mIsUaTransactionByDefault(defaultToUaTransactions)
        , mbUseRport(FALSE)
        , mbIncludePlatformInUserAgentName(TRUE)
        , mDoUaMessageChecks(doUaMessageChecks)
        , mbForceSymmetricSignaling(bForceSymmetricSignaling)
        , mbShuttingDown(FALSE)
        , mbShutdownDone(FALSE)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipUserAgent[%s]::_ sipTcpPort = %d, sipUdpPort = %d, "
                 "sipTlsPort = %d",
                 getName().data(), sipTcpPort, sipUdpPort, sipTlsPort);

    // Get pointer to line manager
    mpLineMgr = lineMgr;

    mHandleOptionsRequests = howTohandleOptionsRequest;

   // Create and start the SIP TLS, TCP and UDP Servers
#ifdef SIP_TLS
    if (mTlsPort != PORT_NONE)
    {
        mSipTlsServer = new SipTlsServer(mTlsPort,
                                         this,
                                         bUseNextAvailablePort,
                                         defaultAddress);
        mSipTlsServer->startListener();
        mTlsPort = mSipTlsServer->getServerPort();

        if (mTlsPort == PORT_NONE || !mSipTlsServer->isOk())
        {
            OsSysLog::add(FAC_NET, PRI_EMERG, "Unable to bind on tls port %d (ok=%d)",
                    sipTlsPort, mSipTlsServer->isOk());
            mTlsPort = PORT_NONE;
        }
    }
#endif
    if (mTcpPort != PORT_NONE)
    {
        mSipTcpServer = new SipTcpServer(mTcpPort,
                                         this,
                                         "SipTcpServer-%d",
                                         bUseNextAvailablePort,
                                         defaultAddress);
        mSipTcpServer->startListener();
        mTcpPort = mSipTcpServer->getServerPort();

        if (mTcpPort == PORT_NONE || !mSipTcpServer->isOk())
        {
            OsSysLog::add(FAC_NET, PRI_EMERG, "Unable to bind on tcp port %d (ok=%d)",
                    sipTcpPort, mSipTcpServer->isOk());
            mTcpPort = PORT_NONE;
        }
    }

    if (mUdpPort != PORT_NONE)
    {
        mSipUdpServer = new SipUdpServer(mUdpPort,
                                         this,
                                         readBufferSize,
                                         bUseNextAvailablePort,
                                         defaultAddress);
        mSipUdpServer->startListener();
        // Get the UDP port that was obtained (in case mUdpPort was
        // PORT_DEFAULT).
        mUdpPort = mSipUdpServer->getServerPort();

        if (mUdpPort == PORT_NONE || !mSipUdpServer->isOk())
        {
            OsSysLog::add(FAC_NET, PRI_EMERG, "Unable to bind on udp port %d (ok=%d)",
                    sipUdpPort, mSipUdpServer->isOk());
            mUdpPort = PORT_NONE;
        }
    }
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipUserAgent[%s]::_ after creating mSip*Servers: "
                  "mTcpPort = %d, mUdpPort = %d, mTlsPort = %d",
                  getName().data(), mTcpPort, mUdpPort, mTlsPort);

    mMaxMessageLogSize = MAXIMUM_SIP_LOG_SIZE;
    mMaxForwards = SIP_DEFAULT_MAX_FORWARDS;

    // Set the idle time after which unused sockets are garbage collected.
    mMaxTcpSocketIdleTime = DEFAULT_TCP_SOCKET_IDLE_TIME;

    // INVITE transactions need to stick around for a minimum of
    // 3 minutes
    mMinInviteTransactionTimeout = DEFAULT_SIP_TRANSACTION_EXPIRES;

    mForkingEnabled = TRUE;
    mRecurseOnlyOne300Contact = FALSE;

    mMaxSrvRecords = 4;
    mDnsSrvTimeout = 4; // seconds

    if(authenticateRealm)
    {
        mAuthenticationRealm.append(authenticateRealm);
    }

    if(authenticateDb)
    {
        mpAuthenticationDb = authenticateDb;
    }
    else
    {
        mpAuthenticationDb = new OsConfigDb();
    }

    if(authorizeUserIds)
    {
        mpAuthorizationUserIds = authorizeUserIds;
    }
    else
    {
        mpAuthorizationUserIds = new OsConfigDb();
    }

    if(authorizePasswords)
    {
        mpAuthorizationPasswords = authorizePasswords;
    }
    else
    {
        mpAuthorizationPasswords = new OsConfigDb();
    }

    // SIP Server info
    if(sipProxyServers)
    {
        proxyServers.append(sipProxyServers);
    }
    if(sipDirectoryServers)
    {
        directoryServers.append(sipDirectoryServers);
    }
    if(defaultUser)
    {
        defaultSipUser.append(defaultUser);
        NameValueTokenizer::frontBackTrim(&defaultSipUser, " \t\n\r");
    }

    if (!defaultAddress || strcmp(defaultAddress, "0.0.0.0") == 0)
    {
        // get the first local address and
        // make it the default address
        const HostAdapterAddress* addresses[MAX_IP_ADDRESSES];
        int numAddresses = 0;
        memset(addresses, 0, sizeof(addresses));
        getAllLocalHostIps(addresses, numAddresses);
        if (numAddresses > 0)
        {
           // Bind to the first address in the list.
           defaultSipAddress = (char*)addresses[0]->mAddress.data();
           // Now free up the list.
           for (int i = 0; i < numAddresses; i++)
           {
              delete addresses[i];
           }
        }
        else
        {
           OsSysLog::add(FAC_SIP, PRI_WARNING,
                         "SipUserAgent::_ no IP addresses found.");
        }
    }
    else
    {
        defaultSipAddress.append(defaultAddress);
    }
    if(sipRegistryServers)
    {
        registryServers.append(sipRegistryServers);
    }

    if(publicAddress && *publicAddress)
    {
        sipIpAddress.append(publicAddress);
        mConfigPublicAddress = publicAddress;

        // make a config CONTACT entry
        char szAdapter[256];
        ContactAddress contact;
        contact.eContactType = ContactAddress::CONFIG;
        strcpy(contact.cIpAddress, publicAddress);

        if (getContactAdapterName(szAdapter, defaultSipAddress))
        {
           strcpy(contact.cInterface, szAdapter);
        }
        else
        {
           // If getContactAdapterName can't find an adapter.
           OsSysLog::add(FAC_SIP, PRI_WARNING,
                         "SipUserAgent::_ no adaptor found for address '%s'",
                         defaultSipAddress.data());
           strcpy(contact.cInterface, "(unknown)");
        }
        contact.iPort = mUdpPort; // what about the tcp port?
        mContactDb.addContact(contact);
    }
    else
    {
        OsSocket::getHostIp(&sipIpAddress);
    }

    mSipPort = PORT_NONE;

    UtlString hostIpAddress(sipIpAddress.data());

    // Set up timers.
    // First, calculate mUnreliableTransportTimeoutMs based on the
    // sipUnreliableTransportTimeout by applying appropriate limitations.
    // (SIP_DEFAULT_RTT default is thus 100 msec.)
    if (sipUnreliableTransportTimeout <= 0)
    {
        mUnreliableTransportTimeoutMs = SIP_DEFAULT_RTT;
    }
    else if (sipUnreliableTransportTimeout > 0 &&
             sipUnreliableTransportTimeout < SIP_MINIMUM_RTT)
    {
        mUnreliableTransportTimeoutMs = SIP_MINIMUM_RTT;
    }
    else
    {
        mUnreliableTransportTimeoutMs = sipUnreliableTransportTimeout;
    }

    // The other timers are scaled based on mUnreliableTransportTimeoutMs.
    // (Default sets resend to 800 msec.)
    mMaxResendTimeoutMs = 8 * mUnreliableTransportTimeoutMs;
    // Initial timeout for TCP is same as the UDP initial timeout.
    // (Default sets reliable to 100 msec.)
    mReliableTransportTimeoutMs = mUnreliableTransportTimeoutMs;

    // (Default sets state timeout to 8000 msec.)
    mTransactionStateTimeoutMs = 10 * mMaxResendTimeoutMs;
    // How long before we expire transactions by default
    mDefaultExpiresSeconds = DEFAULT_SIP_TRANSACTION_EXPIRES;
    mDefaultSerialExpiresSeconds = DEFAULT_SIP_SERIAL_EXPIRES;

    // Construct the default Contact header value.
    {
       int port;
       const char* protocol;
       if (portIsValid(mUdpPort) && mUdpPort == mTcpPort)
       {
          // Listening on both TCP and UDP on the same port.
          port = mUdpPort;
          protocol = NULL;
       }
       else if (portIsValid(mUdpPort))
       {
          // Listening on UDP.
          port = mUdpPort;
          protocol = "udp";
       }
       else if (portIsValid(mTcpPort))
       {
          // Listening on TCP.
          port = mTcpPort;
          protocol = "tcp";
       }
#ifdef SIP_TLS
       else if (portIsValid(mTlsPort))
       {
          // Listening on TLS
          port = mTlsPort;
          protocol = "tls";
       }
#endif // SIP_TLS
       else
       {
          // Unknown.
          OsSysLog::add(FAC_SIP, PRI_ERR,
                        "SipUserAgent:: neither TCP, UDP, nor TLS in use -- can't construct Contact");
          // Make a guess, it might work.
          port = PORT_NONE;
          protocol = NULL;
       }

       SipMessage::buildSipUri(&mContactURI,
                               !mConfigPublicAddress.isNull() ?
                               mConfigPublicAddress.data() :
                               sipIpAddress.data(),
                               port,
                               protocol,
                               defaultSipUser.data());
       OsSysLog::add(FAC_SIP, PRI_DEBUG,
                     "SipUserAgent[%s]::_ port = %d, protocol = %s, mContactURI = '%s'",
                     getName().data(), port,
                     protocol ? protocol : "NULL",
                     mContactURI.data());
    }

    // Initialize the transaction id seed
    BranchId::setSecret(mContactURI); // XECS-226 should be a configuration value

    // Allow the default SIP methods
    allowMethod(SIP_INVITE_METHOD);
    allowMethod(SIP_ACK_METHOD);
    allowMethod(SIP_CANCEL_METHOD);
    allowMethod(SIP_BYE_METHOD);
    allowMethod(SIP_REFER_METHOD);
    allowMethod(SIP_OPTIONS_METHOD);

    defaultUserAgentName.append(VENDOR "/" SIP_STACK_VERSION);

    OsMsgQ* incomingQ = getMessageQueue();
    mpTimer = new OsTimer(incomingQ, 0);
    // Convert from mSeconds to uSeconds
    OsTime lapseTime(0, mTransactionStateTimeoutMs * 1000);
    mpTimer->periodicEvery(lapseTime, lapseTime);

    OsTime time;
    OsDateTime::getCurTimeSinceBoot(time);
    mLastCleanUpTime = time.seconds();

    // Record the local address.
    cacheLocalAddress();
}

// Copy constructor NOT ALLOWED
//SipUserAgent::SipUserAgent(const SipUserAgent& rSipUserAgent)

// Destructor
SipUserAgent::~SipUserAgent()
{
    mpTimer->stop();
    delete mpTimer;
    mpTimer = NULL;

    // Wait until this OsServerTask has stopped or handleMessage
    // might access something we are about to delete here.
    waitUntilShutDown();

    if(mSipTcpServer)
    {
       // Destructor stops tasks, cleans up directly
       delete mSipTcpServer;
       mSipTcpServer = NULL;
    }
#ifdef SIP_TLS
    if(mSipTlsServer)
    {
       //mSipTlsServer->shutdownListener();
       mSipTlsServer->requestShutdown();
       delete mSipTlsServer;
       mSipTlsServer = NULL;
    }
#endif
    if(mSipUdpServer)
    {
       mSipUdpServer->shutdownListener();
       mSipUdpServer->requestShutdown();
       delete mSipUdpServer;
       mSipUdpServer = NULL;
    }

    if(mpAuthenticationDb)
    {
        delete mpAuthenticationDb;
        mpAuthenticationDb = NULL;
    }

    if(mpAuthorizationUserIds)
    {
        delete mpAuthorizationUserIds;
        mpAuthorizationUserIds = NULL;
    }

    if(mpAuthorizationPasswords)
    {
        delete mpAuthorizationPasswords;
        mpAuthorizationPasswords = NULL;
    }

    mMessageObservers.destroyAll();
    allowedSipExtensions.destroyAll();
    requiredSipExtensions.destroyAll();
    allowedSipMethods.destroyAll();
}

/* ============================ MANIPULATORS ============================== */


// Assignment operator
SipUserAgent&
SipUserAgent::operator=(const SipUserAgent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void SipUserAgent::shutdown(UtlBoolean blockingShutdown)
{
    mbShuttingDown = TRUE;
    mSipTransactions.stopTransactionTimers();

    if (blockingShutdown)
    {
        OsEvent shutdownEvent;
        OsStatus res;
        intptr_t rpcRetVal;

        OsRpcMsg shutdownMsg(OsMsg::PHONE_APP,
                             SipUserAgent::SHUTDOWN_MESSAGE_EVENT,
                             shutdownEvent);
        postMessage(shutdownMsg);

        res = shutdownEvent.wait();
        assert(res == OS_SUCCESS);

        res = shutdownEvent.getEventData(rpcRetVal);
        assert(res == OS_SUCCESS && rpcRetVal == OS_SUCCESS);

        // mbShutdownDone will have been set to TRUE by ::handleMessage.
    }
    else
    {
        OsMsg shutdownMsg(OsMsg::PHONE_APP,
                          SipUserAgent::SHUTDOWN_MESSAGE);
        postMessage(shutdownMsg);
    }
}

void SipUserAgent::enableStun(const char* szStunServer,
                              int refreshPeriodInSecs,
                              int stunOptions,
                              OsNotification* pNotification,
                              const char* szIp)
{
    if (mSipUdpServer)
    {
        mSipUdpServer->enableStun(szStunServer,
                                  szIp,
                                  refreshPeriodInSecs,
                                  stunOptions,
                                  pNotification);
    }
}

void SipUserAgent::addMessageConsumer(OsServerTask* messageEventListener)
{
        // Need to do the real thing by keeping a list of consumers
        // and putting a mutex around the add to list
        //if(messageListener)
        //{
        //      osPrintf("WARNING: message consumer is NOT a LIST\n");
        //}
        //messageListener = messageEventListener;
    if(messageEventListener)
    {
        addMessageObserver(*(messageEventListener->getMessageQueue()));
    }
}

void SipUserAgent::addMessageObserver(OsMsgQ& messageQueue,
                                      const char* sipMethod,
                                      UtlBoolean wantRequests,
                                      UtlBoolean wantResponses,
                                      UtlBoolean wantIncoming,
                                      UtlBoolean wantOutGoing,
                                      const char* eventName,
                                      SipSession* pSession,
                                      void* observerData)
{
   // Since watching outgoing messages does not work at all, the caller
   // must have wantIncoming true and wantOutGoing false.
   assert(wantIncoming);
   assert(!wantOutGoing);

   SipObserverCriteria* observer =
      new SipObserverCriteria(observerData,
                              &messageQueue,
                              sipMethod, wantRequests,
                              wantResponses, wantIncoming,
                              wantOutGoing, eventName, pSession);

   {
      // Add the observer and its filter criteria to the list lock scope
      OsWriteLock lock(mObserverMutex);
      mMessageObservers.insert(observer);

      // Allow the specified method
      if (sipMethod && *sipMethod && wantRequests)
      {
         allowMethod(sipMethod);
      }
   }
}


UtlBoolean SipUserAgent::removeMessageObserver(OsMsgQ& messageQueue, void* pObserverData /*=NULL*/)
{
    OsWriteLock lock(mObserverMutex);
    SipObserverCriteria* pObserver = NULL;
    UtlBoolean bRemovedObservers = FALSE;

    // Traverse all of the observers and remove any that match the
    // message queue/observer data.  If the pObserverData is null, all
    // matching message queue/observers will be removed.  Otherwise, only
    // those observers that match both the message queue and observer data
    // are removed.
    UtlHashBagIterator iterator(mMessageObservers);
    while ((pObserver = (SipObserverCriteria*) iterator()))
    {
        if (pObserver->getObserverQueue() == &messageQueue)
        {
            if ((pObserverData == NULL) ||
                    (pObserverData == pObserver->getObserverData()))
            {
                bRemovedObservers = true;
                UtlContainable* wasRemoved = mMessageObservers.removeReference(pObserver);

                if(wasRemoved)
                {
                   delete wasRemoved;
                }

            }
        }
    }

    return bRemovedObservers;
}

void SipUserAgent::addSipOutputProcessor( SipOutputProcessor *pProcessor )
{
   if( pProcessor )
   {
      OsWriteLock lock( mOutputProcessorMutex );
      mOutputProcessors.insert( pProcessor );
   }
}

UtlBoolean SipUserAgent::removeSipOutputProcessor( SipOutputProcessor *pProcessorToRemove )
{
   UtlBoolean bRemovedProcessor = FALSE;
   if( pProcessorToRemove )
   {
      OsWriteLock lock( mOutputProcessorMutex );
      bRemovedProcessor = ( mOutputProcessors.removeReference( pProcessorToRemove ) != NULL );
   }
   return bRemovedProcessor;
}

void SipUserAgent::executeAllSipOutputProcessors( SipMessage& message,
                                                  const char* address,
                                                  int port )
{
   OsWriteLock lock(mOutputProcessorMutex);
   SipOutputProcessor* pProcessor = NULL ;

   // Traverse all of the processors and call their handleOutputMessage() method
   UtlSortedListIterator iterator(mOutputProcessors);
   while ((pProcessor = (SipOutputProcessor*) iterator()))
   {
      pProcessor->handleOutputMessage( message, address, port );
   }
}

void SipUserAgent::allowMethod(const char* methodName, const bool bAllow)
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipUserAgent::allowMethod this = '%s', methodName = '%s', bAllow = %d",
                  mName.data(), methodName, bAllow);

    if(methodName)
    {
        UtlString matchName(methodName);
        // Do not add the name if it is already in there
        if(NULL == allowedSipMethods.find(&matchName))
        {
            if (bAllow)
            {
                allowedSipMethods.append(new UtlString(methodName));
            }
        }
        else
        {
            if (!bAllow)
            {
                allowedSipMethods.destroy(allowedSipMethods.find(&matchName));
            }
        }
    }
}


// Note: Jun 2010 - only SipConnection objects call with a responseListener queue
UtlBoolean SipUserAgent::send(SipMessage& message,
                              OsMsgQ* responseListener,
                              void* responseListenerData)
{
   if (mbShuttingDown || mbShutdownDone)
   {
      return FALSE;
   }

   UtlBoolean sendSucceeded = FALSE;

   // Extract information from the message.
   UtlBoolean isResponse = message.isResponse();
   UtlString method;
   int cseq = 0;
   // We can avoid extracting the method if this is a response message
   // and this SipUserAgent is a proxy.  Otherwise we will need it.
   if (!isResponse || mIsUaTransactionByDefault)
   {
      message.getCSeqField(&cseq, &method);
   }
   int responseCode = 0;
   if (isResponse)
   {
      responseCode = message.getResponseStatusCode();
   }

   // ===========================================

   // Do all the stuff that does not require transaction locking first

   // Make sure the date field is set
   long epochDate;
   if(!message.getDateField(&epochDate))
   {
      message.setDateField();
   }

   // Under appropriate circumstances, add a Contact header if one is
   // not provided.
   // The appropriate circumstances are:
   // - only for UA transactions (not proxy transactions)
   // - never for requests or responses whose methods are:
   //     CANCEL or ACK (which have special properties)
   //     REGISTER (which uses Contact in special ways)
   // - for responses, only for codes 101 to 299
   if (   mIsUaTransactionByDefault                             // is UA, not proxy
          && !(   method.compareTo(SIP_CANCEL_METHOD) == 0      // is not CANCEL, ACK, or REGISTER request or response
               || method.compareTo(SIP_ACK_METHOD) == 0
               || method.compareTo(SIP_REGISTER_METHOD) == 0)
       && (   !isResponse                                       // is either request or
           || (   SIP_1XX_CLASS_CODE < responseCode             //    response from 101 to 199, or 2xx
               && responseCode < SIP_3XX_CLASS_CODE))
       && !message.getHeaderValue(0, SIP_CONTACT_FIELD))        // does not have a Contact header
   {
      // Add a Contact header with the default Contact URI
      // for this SipUserAgent.
      message.setContactField(mContactURI.data());
   }

   if (!isResponse)
   {
      // Make sure that Max-Forwards is set.
      int maxForwards;
      if (!message.getMaxForwards(maxForwards))
      {
         message.setMaxForwards(mMaxForwards);
      }
   }

   if (!isResponse)
   {
      // This should always be true now:
      if (message.isFirstSend())
      {
         // Save the transaction listener info
         if (responseListener)
         {
            message.setResponseListenerQueue(responseListener);
         }
         if (responseListenerData)
         {
            message.setResponseListenerData(responseListenerData);
         }
      }

      // This is not the first time this message has been sent
      else
      {
         // Should not be getting here.
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "SipUserAgent::send message being resent");
      }
   }    // !isResponse

   // ===========================================

   // Find or create a transaction:
   UtlBoolean isUaTransaction = TRUE;
   enum SipTransaction::messageRelationship relationship;

   //mSipTransactions.lock();

   // verify that the transaction does not already exist
   SipTransaction* transaction = mSipTransactions.findTransactionFor(message,
                                                                     TRUE, // outgoing
                                                                     relationship);

#ifdef TRANSACTION_MATCH_DEBUG // enable only for transaction match debugging - log is confusing otherwise
   OsSysLog::add(FAC_SIP, PRI_DEBUG
                 ,"SipUserAgent[%s]::send "
                  "searched for existing transaction, relationship = %d",
                 getName().data(), relationship);
#endif

   // for forwarding 2xx ACK's, we need to ignore the Invite transaction
   SipTransaction* sav2xxAckTxValue = NULL;
   if (relationship == SipTransaction::MESSAGE_2XX_ACK_PROXY)
   {
       sav2xxAckTxValue = transaction;
       mSipTransactions.markAvailable(*transaction);
       transaction = NULL;
   }

   // Found a transaction for this message
   if (transaction)
   {
      isUaTransaction = transaction->isUaTransaction();

      // Response for which a transaction already exists
      if (isResponse)
      {
         if (isUaTransaction)
         {
            // It seems that the polite thing to do is to add the
            // allowed methods to all final responses
            UtlString allowedMethodsSet;
            if (message.getResponseStatusCode() >= SIP_OK_CODE &&
                !message.getAllowField(allowedMethodsSet))
            {
               UtlString allowedMethods;
               getAllowedMethods(&allowedMethods);
               message.setAllowField(allowedMethods);
            }
         }
      }

      // Request for which a transaction already exists
      else
      {
         // should not get here unless this is a CANCEL or ACK request
         if ((method.compareTo(SIP_CANCEL_METHOD) == 0) 
             || (method.compareTo(SIP_ACK_METHOD) == 0))
         {
            // no-op
         }

         // A request for which a transaction already exists
         // other than ACK and CANCEL
         else
         {
            // Should not be getting here
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "SipUserAgent::send %s request matches existing transaction",
                          method.data());

            // We pretend there is no match so this becomes a
            // new transaction branch.  Make sure we unlock the
            // transaction before we reset to NULL.
            mSipTransactions.markAvailable(*transaction);
            transaction = NULL;
         }
      }
   }

   // No existing transaction for this message
   if (transaction == NULL)
   {
      if (isResponse)
      {
         // Should not get here except possibly on a server
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "SipUserAgent::send response without an existing transaction"
                       );
      }
      else
      {
         // If there is already a via in the request this must
         // be a proxy transaction
         UtlString viaField;
         SipTransaction* parentTransaction = NULL;
         enum SipTransaction::messageRelationship parentRelationship;
         SipTransaction* sav2xxAckParentTxValue = NULL;
         if (message.getViaField(&viaField, 0))
         {
            isUaTransaction = FALSE;

            // See if there is a parent server proxy transaction
#ifdef TRANSACTION_MATCH_DEBUG // enable only for transaction match debugging - log is confusing otherwise
            OsSysLog::add(FAC_SIP, PRI_DEBUG
                          ,"SipUserAgent[%s]::send searching for parent transaction",
                          getName().data());
#           endif
            parentTransaction =
               mSipTransactions.findTransactionFor(message,
                                                   FALSE, // incoming
                                                   parentRelationship);
            // for forwarding 2xx ACK's, we need to ignore the Invite transaction
            if (parentRelationship == SipTransaction::MESSAGE_2XX_ACK_PROXY ||
                parentRelationship == SipTransaction::MESSAGE_2XX_ACK)
            {
                sav2xxAckParentTxValue = parentTransaction;
                // during debug transaction was stuck as unavailable, solved by marking it available here
                mSipTransactions.markAvailable(*parentTransaction);
                parentTransaction = NULL;
            }
         }

         // Create a new transactions
         // This should only be for requests
         // so it should not be a server transaction
         transaction = new SipTransaction(&message, TRUE /* outgoing */, isUaTransaction);
         transaction->markBusy();
         mSipTransactions.addTransaction(transaction);

         if (!isUaTransaction && parentTransaction)
         {
            if (parentRelationship ==
                SipTransaction::MESSAGE_DUPLICATE)
            {
               // Link the parent server transaction to the child client transaction
               parentTransaction->linkChild(*transaction);
               // The parent will be unlocked with the transaction
            }
            else
            {
               OsSysLog::add(FAC_SIP, PRI_WARNING,
                             "SipUserAgent::send proxied client transaction not "
                             "part of server transaction, parent relationship: %s",
                             SipTransaction::relationshipString(parentRelationship));

               if (parentTransaction)
               {
                  mSipTransactions.markAvailable(*parentTransaction);
               }
            }
         }
         else if (!isUaTransaction)
         {
            if (sav2xxAckParentTxValue)
            {
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                              "SipUserAgent[%s]::send proxied client transaction %p 2xx ACK has parent %p",
                              getName().data(), sav2xxAckTxValue, sav2xxAckParentTxValue);
            }
            // leave old comment but change back to warning, should only see the 2xx ACK message often
            // old comment-> this happens all the time in the authproxy, so log only at debug
            else
            {
                OsSysLog::add(FAC_SIP, PRI_WARNING,
                              "SipUserAgent[%s]::send proxied client transaction does not have parent",
                              getName().data());
            }

         }
         else if (parentTransaction)
         {
            mSipTransactions.markAvailable(*parentTransaction);
         }

         if (sav2xxAckTxValue == NULL)  // special case 2xx relationship, don't overwrite it here
         {
             relationship = SipTransaction::MESSAGE_UNKNOWN;
         }
      }
   }

   if (transaction)
   {
      // Make sure the User Agent field is set
      if (isUaTransaction)
      {
         setSelfHeader(message);

         // Make sure the accept language is set
         UtlString language;
         message.getAcceptLanguageField(&language);
         if (language.isNull())
         {
            // Beware that this value does not describe the desired media
            // sessions, but rather the preferred languages for reason
            // phrases, etc. (RFC 3261 sec. 20.3)  Thus, it is useful to
            // have a value for this header even in requests like
            // SUBSCRIBE/NOTIFY which are expected to not be seen by a human.
            // This value should be configurable, though.
            message.setAcceptLanguageField("en");
         }

         // Add Allow header to REFER and INVITE requests. It is
         // mandatory for the REFER method.
         UtlString allowedMethodsSet;
         if (   ! message.getAllowField(allowedMethodsSet)
             && (   method.compareTo(SIP_REFER_METHOD) == 0
                 || method.compareTo(SIP_INVITE_METHOD) == 0
                 )
             )
         {
            UtlString allowedMethods;
            getAllowedMethods(&allowedMethods);
            message.setAllowField(allowedMethods);
         }

         // Set the Supported field if this is not
         // an ACK request and the Supported field is not already set.
         if (   method.compareTo(SIP_ACK_METHOD) != 0
             && !message.getHeaderValue(0, SIP_SUPPORTED_FIELD)
             )
         {
            UtlString supportedExtensions;
            getSupportedExtensions(supportedExtensions);
            if (supportedExtensions.length() > 0)
            {
               message.setSupportedField(supportedExtensions.data());
               supportedExtensions.remove(0);
            }
         }

         // Set the Require field if this is neither a CANCEL
         // nor ACK request, and the Require field is not already set.
         if( method.compareTo(SIP_ACK_METHOD) != 0 &&
             method.compareTo(SIP_CANCEL_METHOD) != 0 &&
             !message.getHeaderValue(0, SIP_REQUIRE_FIELD)
           )
         {
            UtlString requiredExtensions;
            getRequiredExtensions(requiredExtensions);
            if( requiredExtensions.length() > 0 )
            {
               message.setRequireField(requiredExtensions.data());
            }
         }

         // If the caller provided no Contact, it will have been added above.
      }

      // If this is the top-most parent and it is a client transaction
      // There is no server transaction, so cancel all of the children
      if (   !isResponse
          && (method.compareTo(SIP_CANCEL_METHOD) == 0)
          && transaction->getTopMostParent() == NULL
          && !transaction->isServerTransaction()
          )
      {
         transaction->cancel(*this, mSipTransactions);
      }
      else
      {
         //  All other messages just get sent.
          OsSysLog::add(FAC_SIP, PRI_ERR,
                        "SipUserAgent::send "
                        "outgoing call 1");
         sendSucceeded = transaction->handleOutgoing(message,
                                                     *this,
                                                     mSipTransactions,
                                                     relationship);
      }

      mSipTransactions.markAvailable(*transaction);
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "SipUserAgent::send failed to construct new transaction");
   }

   if (!sendSucceeded)
   {
      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SipUserAgent::send returning false");
   }
   return (sendSucceeded);
}

UtlBoolean SipUserAgent::sendUdp(SipMessage* message,
                                 const char* serverAddress,
                                 int port)
{
  if (mbShuttingDown || mbShutdownDone)
  {
     return FALSE;
  }

  assert(mSipUdpServer);
  UtlBoolean isResponse = message->isResponse();
  UtlString method;
  int seqNum;
  UtlString seqMethod;
  int responseCode = 0;
  UtlBoolean sentOk = FALSE;
  UtlString msgBytes;
  UtlString messageStatusString = "SipUserAgent::sendUdp ";
  int timesSent = message->getTimesSent();

  if(!isResponse)
    {
      message->getRequestMethod(&method);
    }
  else
    {
      message->getCSeqField(&seqNum, &seqMethod);
      responseCode = message->getResponseStatusCode();
    }

  if(timesSent == 0)
    {
      message->touchTransportTime();
    }
  // get the message if it was previously sent.
  else
    {
      char buffer[20];
      sprintf(buffer, "%d", timesSent);
      messageStatusString.append("resend ");
      messageStatusString.append(buffer);
      messageStatusString.append(" of UDP message\n");
    }

  // Send the message

  // Disallow an address begining with * as it gets broadcasted on NT
  if(! strchr(serverAddress, '*') && *serverAddress)
    {
      sentOk = mSipUdpServer->send(message, serverAddress, port);
    }
  else if(*serverAddress == '\0')
    {
      // Only bother processing if the logs are enabled
      if (    isMessageLoggingEnabled() ||
              OsSysLog::willLog(FAC_SIP_OUTGOING, PRI_INFO))
        {
          UtlString msgBytes;
          ssize_t msgLen;
          message->getBytes(&msgBytes, &msgLen);
          msgBytes.insert(0, "No send address\n");
          msgBytes.append("--------------------END--------------------\n");
          logMessage(msgBytes.data(), msgBytes.length());
          OsSysLog::add(FAC_SIP_OUTGOING, PRI_INFO, "%s", msgBytes.data());
        }
      sentOk = FALSE;
    }
  else
    {
      sentOk = FALSE;
    }

  // If we have not failed, schedule a resend.
  if (sentOk)
    {
      messageStatusString.append("UDP SIP User Agent sent message:\n");
      messageStatusString.append("----Local Host:");
      messageStatusString.append(mLocalHostAddress);
      messageStatusString.append("---- Port: ");
      messageStatusString.appendNumber(mLocalUdpHostPort);
      messageStatusString.append("----\n");
      messageStatusString.append("----Remote Host:");
      messageStatusString.append(serverAddress);
      messageStatusString.append("---- Port: ");
      char buff[10];
      sprintf(buff, "%d", !portIsValid(port) ? 5060 : port);
      messageStatusString.append(buff);
      messageStatusString.append("----\n");
    }
  else
    {
      messageStatusString.append("UDP SIP User Agent failed to send message:\n");
      messageStatusString.append("----Local Host:");
      messageStatusString.append(mLocalHostAddress);
      messageStatusString.append("---- Port: ");
      messageStatusString.appendNumber(mLocalUdpHostPort);
      messageStatusString.append("----\n");
      messageStatusString.append("----Remote Host:");
      messageStatusString.append(serverAddress);
      messageStatusString.append("---- Port: ");
      char buff[10];
      sprintf(buff, "%d", !portIsValid(port) ? 5060 : port);
      messageStatusString.append(buff);
      messageStatusString.append("----\n");
      message->logTimeEvent("FAILED");
    }

  // Only bother processing if the logs are enabled
  if (    isMessageLoggingEnabled() ||
          OsSysLog::willLog(FAC_SIP_OUTGOING, PRI_INFO))
    {
      ssize_t len;
      message->getBytes(&msgBytes, &len);
      msgBytes.insert(0, messageStatusString.data());
      msgBytes.append("--------------------END--------------------\n");
      logMessage(msgBytes.data(), msgBytes.length());
      if (msgBytes.length())
      {
        OsSysLog::add(FAC_SIP_OUTGOING, PRI_INFO, "%s", msgBytes.data());
      }
    }

  // if we failed to send it is the calling functions problem to deal with the error

  return(sentOk);
}

UtlBoolean SipUserAgent::sendSymmetricUdp(SipMessage& message,
                                          const char* serverAddress,
                                          int port)
{
    if (mbShuttingDown || mbShutdownDone)
    {
       return FALSE;
    }

    assert(mSipUdpServer);
    UtlBoolean sentOk = mSipUdpServer->sendTo(message,
                                             serverAddress,
                                             port);

    // :TODO: Relocate all the OUTGOING log messages into the SipClient*'s that
    // actually send the messages, so that the log messages record when/if
    // the message is sent, rather than when it is queued.
    // This will also consolidate the 6(?) different places where OUTGOING
    // messages are logged.

    // Don't bother processing unless the logs are enabled
    if (    isMessageLoggingEnabled() ||
            OsSysLog::willLog(FAC_SIP_OUTGOING, PRI_INFO))
    {
        UtlString msgBytes;
        ssize_t msgLen;
        message.getBytes(&msgBytes, &msgLen);
        UtlString outcomeMsg;
        char portString[20];
        sprintf(portString, "%d", !portIsValid(port) ? 5060 : port);

        if(sentOk)
        {
            outcomeMsg.append("UDP SIP User Agent sentTo message:\n");
            outcomeMsg.append("----Local Host:");
            outcomeMsg.append(mLocalHostAddress);
            outcomeMsg.append("---- Port: ");
            outcomeMsg.appendNumber(mLocalUdpHostPort);
            outcomeMsg.append("----\n");
            outcomeMsg.append("----Remote Host:");
            outcomeMsg.append(serverAddress);
            outcomeMsg.append("---- Port: ");
            outcomeMsg.append(portString);
            outcomeMsg.append("----\n");
            msgBytes.insert(0, outcomeMsg);
            msgBytes.append("--------------------END--------------------\n");
        }
        else
        {
            outcomeMsg.append("SIP User agent FAILED sendTo message:\n");
            outcomeMsg.append("----Local Host:");
            outcomeMsg.append(mLocalHostAddress);
            outcomeMsg.append("---- Port: ");
            outcomeMsg.appendNumber(mLocalUdpHostPort);
            outcomeMsg.append("----\n");
            outcomeMsg.append("----Remote Host:");
            outcomeMsg.append(serverAddress);
            outcomeMsg.append("---- Port: ");
            outcomeMsg.append(portString);
            outcomeMsg.append("----\n");
            msgBytes.insert(0, outcomeMsg);
            msgBytes.append("--------------------END--------------------\n");
        }

        logMessage(msgBytes.data(), msgBytes.length());
        OsSysLog::add(FAC_SIP_OUTGOING, PRI_INFO, "%s", msgBytes.data());
    }

    return(sentOk);
}

UtlBoolean SipUserAgent::sendStatelessResponse(SipMessage& rresponse)
{
    if (mbShuttingDown || mbShutdownDone)
    {
       return FALSE;
    }

    UtlBoolean sendSucceeded = FALSE;

    // Forward via the server tranaction
    SipMessage responseCopy(rresponse);
    responseCopy.removeTopVia();
    responseCopy.resetTransport();
    responseCopy.clearDNSField();

    UtlString sendProtocol;
    UtlString sendAddress;
    int sendPort;
    int receivedPort;
    UtlBoolean receivedSet;
    UtlBoolean maddrSet;
    UtlBoolean receivedPortSet;

    // use the via as the place to send the response
    responseCopy.getTopVia(&sendAddress, &sendPort, &sendProtocol,
                           &receivedPort, &receivedSet, &maddrSet,
                           &receivedPortSet);

    // If the sender of the request indicated support of
    // rport (i.e. received port) send this response back to
    // the same port it came from
    if(portIsValid(receivedPort) && receivedPortSet)
    {
        sendPort = receivedPort;
    }

    if(sendProtocol.compareTo(SIP_TRANSPORT_UDP, UtlString::ignoreCase) == 0)
    {
        sendSucceeded = sendUdp(&responseCopy, sendAddress.data(), sendPort);
    }
    else if(sendProtocol.compareTo(SIP_TRANSPORT_TCP, UtlString::ignoreCase) == 0)
    {
        sendSucceeded = sendTcp(&responseCopy, sendAddress.data(), sendPort);
    }
#ifdef SIP_TLS
    else if(sendProtocol.compareTo(SIP_TRANSPORT_TLS, UtlString::ignoreCase) == 0)
    {
        sendSucceeded = sendTls(&responseCopy, sendAddress.data(), sendPort);
    }
#endif

    return(sendSucceeded);
}

// !!!!!! SPECIAL CASE  !!!! SPECIAL CASE !!!!
UtlBoolean SipUserAgent::sendStatelessAck(SipMessage& ackRequest,
                                          UtlString& address,
                                          int port,
                                          OsSocket::IpProtocolSocketType protocol)
{
    if (mbShuttingDown || mbShutdownDone)
    {
       return FALSE;
    }

    UtlBoolean sendSucceeded = FALSE;
    UtlString method;

    ackRequest.getRequestMethod(&method);
    if(method.compareTo(SIP_ACK_METHOD,UtlString::ignoreCase) == 0)
    {
        BranchId* branchId = new BranchId(ackRequest);
        UtlString branchIdData(branchId->data());
        sendSucceeded = sendStatelessRequest(ackRequest,            // this will add via
                                             address,
                                             port,
                                             protocol,
                                             branchIdData);
    }
    return sendSucceeded;
}

UtlBoolean SipUserAgent::sendStatelessRequest(SipMessage& request,
                                              const UtlString& address,
                                              int port,
                                              OsSocket::IpProtocolSocketType protocol,
                                              const UtlString& branchId)
{
   if (mbShuttingDown || mbShutdownDone)
   {
      return FALSE;
   }

   // Convert the enum to a protocol string
   UtlString viaProtocolString;
   SipMessage::convertProtocolEnumToString(protocol,
                                           viaProtocolString);

   // Get via info
   UtlString viaAddress;
   int viaPort;
   getViaInfo(protocol,
              viaAddress,
              viaPort);

   // Add the via field data
   request.addVia(viaAddress.data(),
                  viaPort,
                  viaProtocolString,
                  branchId.data());


   // Send using the correct protocol
   UtlBoolean sendSucceeded = FALSE;
   if(protocol == OsSocket::UDP)
   {
      sendSucceeded = sendUdp(&request, address.data(), port);
   }
   else if(protocol == OsSocket::TCP)
   {
      sendSucceeded = sendTcp(&request, address.data(), port);
   }
#ifdef SIP_TLS
   else if(protocol == OsSocket::SSL_SOCKET)
   {
      sendSucceeded = sendTls(&request, address.data(), port);
   }
#endif

   return(sendSucceeded);
}

UtlBoolean SipUserAgent::sendTcp(SipMessage* message,
                                 const char* serverAddress,
                                 int port)
{
    if (mbShuttingDown || mbShutdownDone)
    {
       return FALSE;
    }

    int sendSucceeded = FALSE;
    ssize_t len;
    UtlString msgBytes;
    UtlString messageStatusString = "SipUserAgent::sendTcp ";

    // :TODO: Note code does not agree with comment.  Which is correct?
    // Disallow an address begining with * as it gets broadcasted on Windows NT.
    if(!strchr(serverAddress,'*') && *serverAddress)
    {
       if (mSipTcpServer)
       {
          sendSucceeded = mSipTcpServer->send(message, serverAddress, port);
       }
    }
    else if (*serverAddress == '\0')
    {
       if (isMessageLoggingEnabled() ||
           OsSysLog::willLog(FAC_SIP_OUTGOING, PRI_INFO))
       {
          message->getBytes(&msgBytes, &len);
          msgBytes.insert(0, "No send address\n");
          msgBytes.append("--------------------END--------------------\n");
          logMessage(msgBytes.data(), msgBytes.length());
          OsSysLog::add(FAC_SIP_OUTGOING, PRI_INFO, "%s", msgBytes.data());
       }
       sendSucceeded = FALSE;
    }
    else
    {
        sendSucceeded = FALSE;
    }

    if(sendSucceeded)
    {
        messageStatusString.append("TCP SIP User Agent sent message:\n");
    }
    else
    {
        messageStatusString.append("TCP SIP User Agent failed to send message:\n");
        message->logTimeEvent("FAILED");
    }

    if (   isMessageLoggingEnabled()
        || OsSysLog::willLog(FAC_SIP_OUTGOING, PRI_INFO)
        )
    {
       message->getBytes(&msgBytes, &len);
       messageStatusString.append("----Local Host:");
       messageStatusString.append(mLocalHostAddress);
       messageStatusString.append("---- Port: ");
       messageStatusString.appendNumber(mLocalTcpHostPort);
       messageStatusString.append("----\n");
       messageStatusString.append("----Remote Host:");
       messageStatusString.append(serverAddress);
       messageStatusString.append("---- Port: ");
       char buff[10];
       sprintf(buff, "%d", !portIsValid(port) ? 5060 : port);
       messageStatusString.append(buff);
       messageStatusString.append("----\n");

       msgBytes.insert(0, messageStatusString.data());
       msgBytes.append("--------------------END--------------------\n");

       logMessage(msgBytes.data(), msgBytes.length());
       OsSysLog::add(FAC_SIP_OUTGOING , PRI_INFO, "%s", msgBytes.data());
    }

    return (sendSucceeded);
}


UtlBoolean SipUserAgent::sendTls(SipMessage* message,
                                 const char* serverAddress,
                                 int port)
{
   if (mbShuttingDown || mbShutdownDone)
   {
      return FALSE;
   }

#ifdef SIP_TLS
   int sendSucceeded = FALSE;
   int len;
   UtlString msgBytes;
   UtlString messageStatusString = "SipUserAgent::sendTls ";

   // Disallow an address begining with * as it gets broadcasted on NT
   if(!strchr(serverAddress,'*') && *serverAddress)
   {
      sendSucceeded = mSipTlsServer->send(message, serverAddress, port);
   }
   else if(*serverAddress == '\0')
   {
      if (    isMessageLoggingEnabled() ||
          OsSysLog::willLog(FAC_SIP_OUTGOING, PRI_INFO))
      {
         message->getBytes(&msgBytes, &len);
         msgBytes.insert(0, "No send address\n");
         msgBytes.append("--------------------END--------------------\n");
         logMessage(msgBytes.data(), msgBytes.length());
         OsSysLog::add(FAC_SIP_OUTGOING, PRI_INFO, "%s", msgBytes.data());
      }
      sendSucceeded = FALSE;
   }
   else
   {
      sendSucceeded = FALSE;
   }

   if(sendSucceeded)
   {
      messageStatusString.append("TLS SIP User Agent sent message:\n");
      //osPrintf("%s", messageStatusString.data());

   }
   else
   {
      messageStatusString.append("TLS SIP User Agent failed to send message:\n");
      //osPrintf("%s", messageStatusString.data());
      message->logTimeEvent("FAILED");
   }

   if (    isMessageLoggingEnabled() ||
       OsSysLog::willLog(FAC_SIP_OUTGOING, PRI_INFO))
   {
      message->getBytes(&msgBytes, &len);
      messageStatusString.append("----Local Host:");
      messageStatusString.append(mLocalHostAddress);
      messageStatusString.append("---- Port: ");
      messageStatusString.appendNumber(mLocalTlsHostPort);
      messageStatusString.append("----\n");
      messageStatusString.append("----Remote Host:");
      messageStatusString.append(serverAddress);
      messageStatusString.append("---- Port: ");
      char buff[10];
      sprintf(buff, "%d", !portIsValid(port) ? 5060 : port);
      messageStatusString.append(buff);
      messageStatusString.append("----\n");

      msgBytes.insert(0, messageStatusString.data());
      msgBytes.append("--------------------END--------------------\n");

      logMessage(msgBytes.data(), msgBytes.length());
      OsSysLog::add(FAC_SIP_OUTGOING , PRI_INFO, "%s", msgBytes.data());
   }

   return(sendSucceeded);
#else
   return FALSE;
#endif
}

void SipUserAgent::dispatch(SipMessage* message, int messageType)
{
   if (mbShuttingDown || mbShutdownDone)
   {
       delete message;
       return;
   }

   ssize_t len;
   UtlString msgBytes;
   UtlString messageStatusString;
   UtlBoolean resentWithAuth = FALSE;
   UtlBoolean isResponse = message->isResponse();
   UtlBoolean shouldDispatch = FALSE;
   SipMessage* delayedDispatchMessage = NULL;

#ifdef LOG_TIME
   OsTimeLog eventTimes;
   eventTimes.addEvent("start");
#endif

   // Get the message bytes for logging before the message is
   // potentially deleted or nulled out.
   if (   isMessageLoggingEnabled()
       || OsSysLog::willLog(FAC_SIP_INCOMING_PARSED, PRI_DEBUG)
       || OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
   {
      message->getBytes(&msgBytes, &len);
   }

   if(messageType == SipMessageEvent::APPLICATION)
   {
      // Strip out maddr/transport/non-default port according to RFC 3261 section 16.4
      if (doesMaddrMatchesUserAgent(*message))
      {
         UtlString uriStr;
         UtlString method;
         UtlString protocol;
         UtlString originalHeader;

         if (OsSysLog::willLog(FAC_SIP, PRI_NOTICE))
         {
             originalHeader = message->getFirstHeaderLine();
         }
         message->getRequestMethod(&method);
         message->getRequestProtocol(&protocol);
         message->getRequestUri(&uriStr);
         Url uri(uriStr, Url::AddrSpec, NULL);
         uri.removeUrlParameter("maddr");
         uri.removeUrlParameter("transport");
         uri.setHostPort(PORT_NONE);
         uri.getUri(uriStr);
         message->setFirstHeaderLine(method, uriStr, protocol);

         if (OsSysLog::willLog(FAC_SIP, PRI_NOTICE))
         {
            OsSysLog::add(FAC_SIP, PRI_NOTICE,
                  "SipUserAgent[%s]::dispatch updated first line header "
                  "per RFC 3261 Section 16.4:\n%s -> %s",
                  getName().data(),
                  originalHeader.data(),
                  message->getFirstHeaderLine());
         }
    }

      message->logTimeEvent("DISPATCHING");

      // Ensure that the incoming message does not contain
      // a sipX NAT Route header. If these are seen in
      // incoming messages, it could cause erratic routing
      // behavior.
      message->removeSipXNatRoute();

      UtlBoolean isUaTransaction = mIsUaTransactionByDefault;
      enum SipTransaction::messageRelationship relationship;
#ifdef TRANSACTION_MATCH_DEBUG // enable only for transaction match debugging - log is confusing otherwise
            OsSysLog::add(FAC_SIP, PRI_DEBUG
                          ,"SipUserAgent[%s]::dispatch(incoming) searching for transaction",
                          getName().data());
#           endif
      SipTransaction* transaction =
         mSipTransactions.findTransactionFor(*message,
                                             FALSE, // incoming
                                             relationship);
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipUserAgent[%s]::dispatch transaction = %p, relationship = %d",
                    getName().data(), transaction, relationship);

#ifdef LOG_TIME
      eventTimes.addEvent("found TX");
#endif
      // for some 2xx ACK's, we need to ignore the Invite transaction
      SipTransaction* sav2xxAckTxValue = NULL;
      if (relationship == SipTransaction::MESSAGE_2XX_ACK_PROXY)
      {
          sav2xxAckTxValue = transaction;
          transaction = NULL;
      }

      if(transaction == NULL)
      {
         if(isResponse)
         {
            OsSysLog::add(FAC_SIP, PRI_WARNING,"SipUserAgent::dispatch "
                          "received response without transaction");

         }      // end no transaction found for response message

         // New transaction for incoming request
         else
         {
             // Should create a server transaction
            transaction = new SipTransaction(message, FALSE /* incoming */, isUaTransaction);

            // Add the new transaction to the list
            transaction->markBusy();
            mSipTransactions.addTransaction(transaction);

            UtlString method;
            message->getRequestMethod(&method);

            if(method.compareTo(SIP_ACK_METHOD) == 0)
            {
               // Handle ACK messages without a related transaction.
               // This now includes messages that have an "ACK PROXY" relationship with an ignored transaction.

               // Only set relationship if it was really unrelated.
               // Log warning if this happens and it's not "ACK PROXY", otherwise post debug for now
               if (relationship != SipTransaction::MESSAGE_2XX_ACK_PROXY)   // don't overwrite this value
               {
                   relationship = SipTransaction::MESSAGE_ACK;
                   OsSysLog::add(FAC_SIP, PRI_WARNING,
                                 "SipUserAgent[%s]::dispatch received ACK without transaction",
                                 getName().data());
               }
               else
               {
                   // Should happen whenever the ACK is not traversing the same proxy as where the transaction was origniated.
                   // E.g. Call setup in the authproxy, because the original transaction was in the forking proxy.
                   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                 "SipUserAgent[%s]::dispatch received 2xx ACK to forward for transaction %p",
                                  getName().data(), sav2xxAckTxValue );
               }
            }   // end "unrelated" ACK
            else if(method.compareTo(SIP_CANCEL_METHOD) == 0)
            {
               relationship = SipTransaction::MESSAGE_CANCEL;
               OsSysLog::add(FAC_SIP, PRI_WARNING,
                             "SipUserAgent::dispatch received CANCEL without transaction");
            }
            else
            {
               relationship = SipTransaction::MESSAGE_REQUEST;
            }
         }      // end create new transaction
      }

#ifdef LOG_TIME
      eventTimes.addEvent("handling TX");
#endif
      // This is a message that was already recieved once
      if (   transaction
          && relationship == SipTransaction::MESSAGE_DUPLICATE
          )
      {
         // Resends of final INVITE responses need to be
         // passed through if they are 2xx class or the ACK
         // needs to be resent if it was a failure (i.e. 3xx,4xx,5xx,6xx)
         if(message->isResponse())
         {
            int responseCode = message->getResponseStatusCode();
            UtlString transactionMethod;
            int respCseq;
            message->getCSeqField(&respCseq, &transactionMethod);

            if (   responseCode >= SIP_2XX_CLASS_CODE
                && transactionMethod.compareTo(SIP_INVITE_METHOD) == 0
                )
            {
               transaction->handleIncoming(*message,
                                           *this,
                                           relationship,
                                           mSipTransactions,
                                           delayedDispatchMessage);

               // Should never dispatch a resend of a 2xx
               if(delayedDispatchMessage)
               {
                  delete delayedDispatchMessage;
                  delayedDispatchMessage = NULL;
               }
            }
         }
         // Resends of requests need to be passed to the transaction to
         // send the response again.
         else
         {
            transaction->handleIncoming(*message,
                                        *this,
                                        relationship,
                                        mSipTransactions,
                                        delayedDispatchMessage);
         }

         messageStatusString.append("Received duplicate message\n");
      }     // end DUPLICATE message

      // The first time we received this message
      else if (transaction)
      {
         switch (relationship)
         {
         case SipTransaction::MESSAGE_FINAL:
         case SipTransaction::MESSAGE_PROVISIONAL:
         case SipTransaction::MESSAGE_CANCEL_RESPONSE:
         {
            int delayedResponseCode = -1;
            SipMessage* request = transaction->getRequest();
            isUaTransaction = transaction->isUaTransaction();

            shouldDispatch =
               transaction->handleIncoming(*message,
                                           *this,
                                           relationship,
                                           mSipTransactions,
                                           delayedDispatchMessage);

            if(delayedDispatchMessage)
            {
               delayedResponseCode =
                  delayedDispatchMessage->getResponseStatusCode();
            }

            // Check for Authentication Error
            if(   request
               && delayedDispatchMessage
               && delayedResponseCode == HTTP_UNAUTHORIZED_CODE
               && isUaTransaction
               )
            {
               resentWithAuth =
                  resendWithAuthorization(delayedDispatchMessage,
                                          request,
                                          &messageType,
                                          HttpMessage::SERVER);
            }

            // Check for Proxy Authentication Error
            if(   request
               && delayedDispatchMessage
               && delayedResponseCode == HTTP_PROXY_UNAUTHORIZED_CODE
               && isUaTransaction
               )
            {
               resentWithAuth =
                  resendWithAuthorization(delayedDispatchMessage,
                                          request,
                                          &messageType,
                                          HttpMessage::PROXY);
            }

            // If we have a request for this incoming response
            // Forward it on to interested applications
            if (   request
                && (shouldDispatch || delayedDispatchMessage)
                )
            {
               UtlString method;
               request->getRequestMethod(&method);
               OsMsgQ* responseQ = NULL;
               responseQ =  request->getResponseListenerQueue();
               if (responseQ  && shouldDispatch)
               {
                  SipMessage * msg = new SipMessage(*message);      // post response to events
                  msg->setResponseListenerData(request->getResponseListenerData() );
                  SipMessageEvent eventMsg(msg);
                  eventMsg.setMessageStatus(messageType);
                  responseQ->send(eventMsg);
                  // The SipMessage gets freed with the SipMessageEvent
                  msg = NULL;
               }

               if(responseQ  && delayedDispatchMessage)
               {
                  SipMessage* tempDelayedDispatchMessage =
                     new SipMessage(*delayedDispatchMessage);       // post response to events

                  tempDelayedDispatchMessage->setResponseListenerData(
                     request->getResponseListenerData()
                                                                      );

                  SipMessageEvent eventMsg(tempDelayedDispatchMessage);
                  eventMsg.setMessageStatus(messageType);
                  responseQ->send(eventMsg);
                  // The SipMessage gets freed with the SipMessageEvent
                  tempDelayedDispatchMessage = NULL;
               }
            }
         }      // end final, provisional or cancel response case
         break;

         case SipTransaction::MESSAGE_REQUEST:
         {
            // if this is a request check if it is supported
            SipMessage* response = NULL;
            UtlString disallowedExtensions;
            UtlString method;
            UtlString allowedMethods;
            UtlString contentEncoding;
            UtlString toAddress;
            UtlString fromAddress;
            UtlString uriAddress;
            UtlString protocol;
            UtlString sipVersion;
            int port;
            int seqNumber;
            UtlString seqMethod;
            UtlString callIdField;
            int maxForwards;

            message->getRequestMethod(&method);
            if(isUaTransaction)
            {
               getAllowedMethods(&allowedMethods);
               whichExtensionsNotAllowed(message, &disallowedExtensions);
               message->getContentEncodingField(&contentEncoding);

               //delete leading and trailing white spaces
               disallowedExtensions = disallowedExtensions.strip(UtlString::both);
               allowedMethods = allowedMethods.strip(UtlString::both);
               contentEncoding = contentEncoding.strip(UtlString::both);
            }

            message->getToAddress(&toAddress, &port, &protocol);
            message->getFromAddress(&fromAddress, &port, &protocol);
            message->getUri(&uriAddress, &port, &protocol);
            message->getRequestProtocol(&sipVersion);
            sipVersion.toUpper();
            message->getCSeqField(&seqNumber, &seqMethod);
            seqMethod.toUpper();
            message->getCallIdField(&callIdField);

            // validate message, send error response if rejected
            // Check if the method is supported
            if(   isUaTransaction
               && !isMethodAllowed(method.data())
               )
            {
               response = new SipMessage();     // error reponse - not allowed

               // Since we are rejecting the request because its method is
               // not handled by this SipUserAgent, include an Allow: header
               // listing the methods we do handle.
               UtlString allowedMethods;
               getAllowedMethods(&allowedMethods);
               response->setRequestBadMethod(message, allowedMethods);
            }

            // Check if the extensions are supported
            else if(   mDoUaMessageChecks
                    && isUaTransaction
                    && !disallowedExtensions.isNull()
                    )
            {
               response = new SipMessage();     // error reponse - bad extension
               response->setRequestBadExtension(message,
                                                disallowedExtensions);
            }

            // Check if the encoding is supported
            // i.e. no encoding
            else if(   mDoUaMessageChecks
                    && isUaTransaction
                    && !contentEncoding.isNull()
                    )
            {
               response = new SipMessage();         // error reponse - content encoding
               response->setRequestBadContentEncoding(message,"");
            }

            // Check the addresses are present
            else if(toAddress.isNull() || fromAddress.isNull() ||
                    uriAddress.isNull())
            {
               response = new SipMessage();         // error reponse
               response->setDiagnosticSipFragResponse(*message,
                                                      SIP_BAD_ADDRESS_CODE, SIP_BAD_ADDRESS_TEXT,
                                                      SIP_WARN_MISC_CODE,
                                                      "Missing one or more of "
                                                      "To, From, or request URI",
                                                      sipIpAddress
                                                      );
               setSelfHeader(*response);
            }

            // Check SIP version
            else if(strcmp(sipVersion.data(), SIP_PROTOCOL_VERSION) != 0)
            {
               response = new SipMessage();         // error reponse protocol version
               response->setDiagnosticSipFragResponse(*message,
                                                      SIP_BAD_VERSION_CODE, SIP_BAD_VERSION_TEXT,
                                                      SIP_WARN_MISC_CODE,
                                                      "Expected " SIP_PROTOCOL_VERSION,
                                                      sipIpAddress
                                                      );
               setSelfHeader(*response);
            }

            // Check for missing CSeq or Call-Id
            else if(callIdField.isNull() || seqNumber < 0 ||
                    strcmp(seqMethod.data(), method.data()) != 0)
            {
               response = new SipMessage();     // error reponse
               response->setDiagnosticSipFragResponse(*message,
                                                      SIP_BAD_REQUEST_CODE, SIP_BAD_REQUEST_TEXT,
                                                      SIP_WARN_MISC_CODE,
                                                      "Missing one or more of "
                                                      "Call-Id, Method, or CSeq",
                                                      sipIpAddress
                                                      );
               setSelfHeader(*response);
           }

            // Process Options requests
            else if(isUaTransaction &&
                    !message->isResponse() &&
                    (method.compareTo(SIP_OPTIONS_METHOD) == 0) &&
                    mHandleOptionsRequests == HANDLE_OPTIONS_AUTOMATICALLY)
            {
               // Send an OK, the allowed field will get added to all final responces.
               response = new SipMessage();         // Options 200 response
               response->setResponseData(message,
                                         SIP_OK_CODE,
                                         SIP_OK_TEXT);
               setSelfHeader(*response);

               delete(message);
               message = NULL;
            }

            else if(message->getMaxForwards(maxForwards))
            {
               if(maxForwards <= 0)
               {
                  response = new SipMessage();      // error response
                  response->setDiagnosticSipFragResponse(*message,
                                                         SIP_TOO_MANY_HOPS_CODE,
                                                         SIP_TOO_MANY_HOPS_TEXT,
                                                         SIP_WARN_MISC_CODE,
                                                         SIP_TOO_MANY_HOPS_TEXT,
                                                         sipIpAddress
                                                         );
                  setSelfHeader(*response);

                  delete(message);
                  message = NULL;
               }
            }
            else
            {
               message->setMaxForwards(mMaxForwards);
            }

            // If the request is invalid
            if(response)
            {
               // Send the error response
                OsSysLog::add(FAC_SIP, PRI_ERR,
                              "SipUserAgent::send "
                              "outgoing call 2");
               transaction->handleOutgoing(*response,
                                           *this,
                                           mSipTransactions,
                                           SipTransaction::MESSAGE_FINAL);
               delete response;
               response = NULL;
               if(message) delete message;
               message = NULL;
            }
            else if(message)
            {
               shouldDispatch =
                  transaction->handleIncoming(*message,
                                              *this,
                                              relationship,
                                              mSipTransactions,
                                              delayedDispatchMessage);
            }
            else
            {
               OsSysLog::add(FAC_SIP, PRI_ERR, "SipUserAgent::dispatch NULL message to handle");
               //osPrintf("ERROR: SipUserAgent::dispatch NULL message to handle\n");
            }
         }      // end REQUEST case
         break;

         case SipTransaction::MESSAGE_ACK:
         case SipTransaction::MESSAGE_2XX_ACK:
         case SipTransaction::MESSAGE_2XX_ACK_PROXY:
         case SipTransaction::MESSAGE_CANCEL:
         {
            int maxForwards;

            // Check the ACK max-forwards has not gone too many hopes
            if(!isResponse &&
               (relationship == SipTransaction::MESSAGE_ACK ||
                relationship == SipTransaction::MESSAGE_2XX_ACK ||
                relationship == SipTransaction::MESSAGE_2XX_ACK_PROXY) &&
               message->getMaxForwards(maxForwards) &&
               maxForwards <= 0 )
            {

               // Drop ACK on the floor.
               if(message) delete(message);
               message = NULL;
            }

            else if(message)
            {
               shouldDispatch =
                  transaction->handleIncoming(*message,
                                              *this,
                                              relationship,
                                              mSipTransactions,
                                              delayedDispatchMessage);
            }
         }      // end various ACKs
         break;

         case SipTransaction::MESSAGE_NEW_FINAL:
         {
            // Forward it on to interested applications
            SipMessage* request = transaction->getRequest();
            shouldDispatch = TRUE;
            if( request)
            {
               UtlString method;
               request->getRequestMethod(&method);
               OsMsgQ* responseQ = NULL;
               responseQ =  request->getResponseListenerQueue();
               if (responseQ)
               {
                  SipMessage * msg = new SipMessage(*message);      // post in event
                  msg->setResponseListenerData(request->getResponseListenerData() );
                  SipMessageEvent eventMsg(msg);
                  eventMsg.setMessageStatus(messageType);
                  responseQ->send(eventMsg);
                  // The SipMessage gets freed with the SipMessageEvent
                  msg = NULL;
               }
            }
         }      // end new final?
         break;

         default:
         {
            if (OsSysLog::willLog(FAC_SIP, PRI_WARNING))
            {
               OsSysLog::add(FAC_SIP, PRI_WARNING,
                             "SipUserAgent::dispatch unhandled incoming message: %s",
                             SipTransaction::relationshipString(relationship));
            }
         }
         break;
         }
      }

      if(transaction)
      {
         mSipTransactions.markAvailable(*transaction);
      }
   }        // end SipMessageEvent::APPLICATION
   else if(messageType == SipMessageEvent::TRANSPORT_ERROR)
   {
       OsSysLog::add(FAC_SIP, PRI_DEBUG,
                     "SipUserAgent[%s]::dispatch transport error message received",
                     getName().data());

       if(message)
       {
          // WARNING: you cannot touch the contents of the transaction
          // attached to the message until the transaction has been
          // locked (via findTransactionFor, if no transaction is
          // returned, it either no longer exists or we could not get
          // a lock for it.

          if(message->getSipTransaction() == NULL)
          {
             OsSysLog::add(FAC_SIP, PRI_DEBUG,
                           "SipUserAgent[%s]::dispatch "
                           "transport error message with NULL transaction",
                           getName().data()
                           );
          }

          int nextTimeout = -1;
          enum SipTransaction::messageRelationship relationship;
          //mSipTransactions.lock();
#ifdef TRANSACTION_MATCH_DEBUG // enable only for transaction match debugging - log is confusing otherwise
       OsSysLog::add(FAC_SIP, PRI_DEBUG
                     ,"SipUserAgent[%s]::dispatch(transport error) searching for transaction",
                     getName().data());
#           endif
          SipTransaction* transaction =
             mSipTransactions.findTransactionFor(*message,
                                                 TRUE, // timers are only set for outgoing?
                                                 relationship);
          if(transaction)
          {
              // If we are shutting down, unlock the transaction
              // and set it to null.  We pretend that the transaction
              // does not exist (i.e. no-op).
              if (mbShuttingDown || mbShutdownDone)
              {
                  mSipTransactions.markAvailable(*transaction);
                  transaction = NULL;
              }
          }

          if(transaction)
          {
             SipMessage* delayedDispatchMessage = NULL;
             transaction->handleExpiresEvent(*message,
                                             *this,
                                             relationship,
                                             mSipTransactions,
                                             nextTimeout,
                                             delayedDispatchMessage);

             mSipTransactions.markAvailable(*transaction);

             if(delayedDispatchMessage)
             {
                // Only bother processing if the logs are enabled
                if (    isMessageLoggingEnabled() ||
                    OsSysLog::willLog(FAC_SIP_INCOMING_PARSED, PRI_DEBUG))
                {
                   UtlString delayMsgString;
                   ssize_t delayMsgLen;
                   delayedDispatchMessage->getBytes(&delayMsgString,
                                                    &delayMsgLen);
                   delayMsgString.insert(0, "SIP User agent delayed dispatch message:\n");
                   delayMsgString.append("++++++++++++++++++++END++++++++++++++++++++\n");

                   logMessage(delayMsgString.data(), delayMsgString.length());
                   OsSysLog::add(FAC_SIP_INCOMING_PARSED, PRI_DEBUG,"%s",
                                 delayMsgString.data());
                }

                queueMessageToObservers(delayedDispatchMessage,
                                        SipMessageEvent::TRANSPORT_ERROR
                                        );

                //delayedDispatchMessage gets freed in queueMessageToObservers
                delayedDispatchMessage = NULL;
             }
          }
          else // Could not find a transaction for this expired message
          {
             if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
             {
                UtlString noTxMsgString;
                ssize_t noTxMsgLen;
                message->getBytes(&noTxMsgString, &noTxMsgLen);

                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                              "SipUserAgent[%s]::dispatch "
                              "transport error with no matching transaction: %s",
                              getName().data(), noTxMsgString.data());
             }
          }
       }
   }    // End SipMessageEvent::TRANSPORT_ERROR
   else
   {
      shouldDispatch = TRUE;
      messageStatusString.append("SIP User agent FAILED to send message:\n");
   }

#ifdef LOG_TIME
   eventTimes.addEvent("queuing");
#endif

   if (    isMessageLoggingEnabled()
       || OsSysLog::willLog(FAC_SIP_INCOMING_PARSED, PRI_DEBUG)
       )
   {
      msgBytes.insert(0, messageStatusString.data());
      msgBytes.append("++++++++++++++++++++END++++++++++++++++++++\n");

      logMessage(msgBytes.data(), msgBytes.length());
      OsSysLog::add(FAC_SIP_INCOMING_PARSED, PRI_DEBUG, "%s", msgBytes.data());
   }

   if(message && shouldDispatch)
   {
      queueMessageToObservers(message, messageType);
   }
   else
   {
      delete message;
      message = NULL;
   }

   if(delayedDispatchMessage)
   {
      if (   isMessageLoggingEnabled()
          || OsSysLog::willLog(FAC_SIP_INCOMING_PARSED, PRI_DEBUG)
          )
      {
         UtlString delayMsgString;
         ssize_t delayMsgLen;
         delayedDispatchMessage->getBytes(&delayMsgString,
                                          &delayMsgLen);
         delayMsgString.insert(0, "SIP User agent delayed dispatch message:\n");
         delayMsgString.append("++++++++++++++++++++END++++++++++++++++++++\n");

         logMessage(delayMsgString.data(), delayMsgString.length());
         OsSysLog::add(FAC_SIP_INCOMING_PARSED, PRI_DEBUG, "%s",
                       delayMsgString.data());
      }

      queueMessageToObservers(delayedDispatchMessage, messageType);
   }

#ifdef LOG_TIME
   eventTimes.addEvent("GC");
#endif

   // All garbage collection is now be done in the context of the
   // SipUserAgent (rather than here, which is executed by SipClient
   // threads) to prevent hiccups in the reading of SipMessages off
   // the sockets.

#ifdef LOG_TIME
   eventTimes.addEvent("finish");
   UtlString timeString;
   eventTimes.getLogString(timeString);
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipUserAgent[%s]::dispatch time log: %s",
                 getName().data(), timeString.data());
#endif
}

void SipUserAgent::queueMessageToObservers(SipMessage* message,
                                           int messageType)
{
   UtlString callId;
   message->getCallIdField(&callId);
   UtlString method;
   message->getRequestMethod(&method);

   // Create a new message event
   SipMessageEvent event(message);
   event.setMessageStatus(messageType);

   // Find all of the observers which are interested in
   // this method and post the message
   UtlBoolean isRsp = message->isResponse();
   if(isRsp)
   {
      int cseq;
      message->getCSeqField(&cseq, &method);
   }

   queueMessageToInterestedObservers(event, method);
   // send it to those with no method descrimination as well
   queueMessageToInterestedObservers(event, "");

   // Do not explicitly delete 'message', as it gets deleted when 'event'
   // is deleted at the end of this scope.
}

void SipUserAgent::queueMessageToInterestedObservers(SipMessageEvent& event,
                                                     const UtlString& method)
{
   const SipMessage* message;
   if((message = event.getMessage()))
   {
      // Find all of the observers which are interested in
      // this method and post the message
      UtlString messageEventName;
      message->getEventFieldParts(&messageEventName); // no parameters

      // do these constructors before taking the lock
      UtlString observerMatchingMethod(method);

      // lock the message observer list
      OsReadLock lock(mObserverMutex);

      UtlHashBagIterator observerIterator(mMessageObservers, &observerMatchingMethod);
      SipObserverCriteria* observerCriteria;
      while ((observerCriteria = (SipObserverCriteria*) observerIterator()))
      {
         // Check message direction and type
         if (   (  message->isResponse() && observerCriteria->wantsResponses())
             || (! message->isResponse() && observerCriteria->wantsRequests())
             )
         {
            // Decide if the event filter applies
            bool useEventFilter = false;
            bool matchedEvent = false;
            if (! message->isResponse()) // events apply only to requests
            {
               UtlString criteriaEventName;
               observerCriteria->getEventName(criteriaEventName);

               useEventFilter = ! criteriaEventName.isNull();
               if (useEventFilter)
               {
                  // see if the event type matches
                  matchedEvent = (   (   method.compareTo(SIP_SUBSCRIBE_METHOD,
                                                          UtlString::ignoreCase)
                                      == 0
                                      || method.compareTo(SIP_NOTIFY_METHOD,
                                                          UtlString::ignoreCase)
                                      == 0
                                      )
                                  && 0==messageEventName.compareTo(criteriaEventName,
                                                                   UtlString::ignoreCase
                                                                   )
                                  );
               }
            } // else - this is a response - event filter is not applicable

            // Check to see if the session criteria matters
            SipSession* pCriteriaSession = observerCriteria->getSession();
            bool useSessionFilter = (NULL != pCriteriaSession);
            UtlBoolean matchedSession = FALSE;
            if (useSessionFilter)
            {
               // it matters; see if it matches
               matchedSession = pCriteriaSession->isSameSession((SipMessage&) *message);
            }

            // We have a message type (req|rsp) the observer wants - apply filters
            if (   (! useSessionFilter || matchedSession)
                && (! useEventFilter   || matchedEvent)
                )
            {
               // This event is interesting, so send it up...
               OsMsgQ* observerQueue = observerCriteria->getObserverQueue();
               void* observerData = observerCriteria->getObserverData();

               // Cheat a little and set the observer data to be passed back
               ((SipMessage*) message)->setResponseListenerData(observerData);

               // Put the message in the observer's queue
               OsStatus r = observerQueue->send(event, OsTime::NO_WAIT);
               if (r != OS_SUCCESS)
               {
                  int numMsgs = observerQueue->numMsgs();
                  int maxMsgs = observerQueue->maxMsgs();
                  OsSysLog::add(FAC_SIP, PRI_CRIT,
                                "SipUserAgent::queueMessageToInterestedObservers "
                                "send failed with status %d "
                                "(numMsgs = %d, maxMsgs = %d)",
                                r, numMsgs, maxMsgs);
                  OsSysLog::add(FAC_SIP, PRI_CRIT,
                                "SipUserAgent::queueMessageToInterestedObservers "
                                "send failed to queue named '%s'",
                                observerQueue->getName()->data());
                  UtlString eventName;
                  observerCriteria->getEventName(eventName);
                  OsSysLog::add(FAC_SIP, PRI_CRIT,
                                "SipUserAgent::queueMessageToInterestedObservers "
                                "observerQueue %p, observerData %p, SIP method '%s', "
                                "wantsRequests %d, wantsResponses %d, wantsIncoming %d, "
                                "wantsOutGoing %d, eventName '%s', SipSession %p",
                                observerCriteria->getObserverQueue(),
                                observerCriteria->getObserverData(),
                                observerCriteria->getSipMethod(),
                                observerCriteria->wantsRequests(),
                                observerCriteria->wantsResponses(),
                                observerCriteria->wantsIncoming(),
                                observerCriteria->wantsOutGoing(),
                                eventName.data(),
                                observerCriteria->getSession());
                  UtlString messageContent;
                  ssize_t messageLength;
                  message->getBytes(&messageContent, &messageLength);
                  OsSysLog::add(FAC_SIP, PRI_CRIT,
                                "SipUserAgent::queueMessageToInterestedObservers failed message is: %s",
                                messageContent.data());
               }
            }
         }
         else
         {
            // either direction or req/rsp not a match
         }
      } // while observers
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT, "queueMessageToInterestedObservers - no message");
   }
}


UtlBoolean checkMethods(SipMessage* message)
{
        return(TRUE);
}

UtlBoolean checkExtensions(SipMessage* message)
{
        return(TRUE);
}


UtlBoolean SipUserAgent::handleMessage(OsMsg& eventMessage)
{
   UtlBoolean messageProcessed = FALSE;
   //osPrintf("SipUserAgent: handling message\n");
   int msgType = eventMessage.getMsgType();
   int msgSubType = eventMessage.getMsgSubType();
   // Print message if input queue to SipUserAgent exceeds 100.
   if (getMessageQueue()->numMsgs() > 100)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipUserAgent[%s]::handleMessage "
                    "msgType = %d, msgSubType = %d, queue length = %d",
                    getName().data(),
                    msgType, msgSubType, getMessageQueue()->numMsgs());
   }

   if (msgType == OsMsg::PHONE_APP)
   {
      // Request to shutdown from SipUserAgent::shutdown -
      // All timers are stopped and are safe to delete
      if (msgSubType == SipUserAgent::SHUTDOWN_MESSAGE ||
          msgSubType == SipUserAgent::SHUTDOWN_MESSAGE_EVENT)
      {
         mSipTransactions.deleteTransactionTimers();

         // Record that the SipUserAgent is shut down to cause methods that are
         // requests for further work to return failure.
         mbShutdownDone = TRUE;

         // Cause our ::run() loop to finish.
         requestShutdown();

         if (msgSubType == SipUserAgent::SHUTDOWN_MESSAGE_EVENT)
         {
            OsEvent* pEvent =
               (dynamic_cast <OsRpcMsg&> (eventMessage)).getEvent();
            OsStatus res = pEvent->signal(OS_SUCCESS);
            assert(res == OS_SUCCESS);
         }
      }
      else
      {
         SipMessage* sipMsg = (SipMessage*)((SipMessageEvent&)eventMessage).getMessage();
         if(sipMsg)
         {
            //messages for which the UA is consumer will end up here.
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipUserAgent[%s]::handleMessage posting message",
                          getName().data());

            // I cannot remember what kind of message ends up here???
            if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
            {
               ssize_t len;
               UtlString msgBytes;
               sipMsg->getBytes(&msgBytes, &len);
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "??????????????????????????????????????\n"
                             "%s???????????????????????????????????\n",
                             msgBytes.data());
            }
         }
      }
      messageProcessed = TRUE;
   }

   // A timer expired
   else if(msgType == OsMsg::OS_EVENT &&
           msgSubType == OsEventMsg::NOTIFY)
   {
      OsTimer* timer;
      SipMessageEvent* sipEvent = NULL;
      intptr_t timerIntptr;
      void* sipEventVoid;

      OsEventMsg* osMsg = dynamic_cast<OsEventMsg*>(&eventMessage);
      // the following line will segfault iff someone creates some other msg class with these types
      osMsg->getUserData(sipEventVoid);
      osMsg->getEventData(timerIntptr);
      sipEvent = (SipMessageEvent*)sipEventVoid;
      timer = (OsTimer*)timerIntptr;

      if(sipEvent)
      {
         const SipMessage* sipMessage = sipEvent->getMessage();
         int msgEventType = sipEvent->getMessageStatus();

         // Resend timeout
         if(msgEventType == SipMessageEvent::TRANSACTION_RESEND)
         {
            if(sipMessage)
            {
               // WARNING: you cannot touch the contents of the transaction attached to the message until 
               // the transaction has been locked (via findTransactionFor).
               // if no transaction is returned, it either no longer exists or we could not get a lock for it.

               if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
               {
                  UtlString callId;
                  int protocolType = sipMessage->getSendProtocol();
                  sipMessage->getCallIdField(&callId);

                  if(sipMessage->getSipTransaction() == NULL)
                  {
                     OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                   "SipUserAgent[%s]::handleMessage "
                                   "resend Timeout message with NULL transaction",
                                   getName().data());
                  }
                  OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                "SipUserAgent[%s]::handleMessage "
                                "resend Timeout of message for protocol %d, callId: \"%s\"",
                                getName().data(), protocolType, callId.data());
               }

               int nextTimeout = -1;
               enum SipTransaction::messageRelationship relationship;
               //mSipTransactions.lock();
#ifdef TRANSACTION_MATCH_DEBUG // enable only for transaction match debugging - log is confusing otherwise
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipUserAgent[%s]::handleMessage(resend) "
                          "searching for transaction",
                          getName().data());
#           endif
               SipTransaction* transaction = mSipTransactions.findTransactionFor(*sipMessage,
                                                                                 TRUE, // timers are only set for outgoing messages I think
                                                                                 relationship);
               if (transaction)
               {
#ifdef TRANSACTION_MATCH_DEBUG // enable only for transaction match debugging - log is confusing otherwise
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipUserAgent[%s]::handleMessage "
                          "(resend) found transaction %p",
                          getName().data(), transaction);
#endif
                   // If we are in shutdown mode, unlock the transaction and set it to null.
                   // We pretend that the transaction does not exist (i.e. no-op).
                   if (mbShuttingDown || mbShutdownDone)
                   {
                       mSipTransactions.markAvailable(*transaction);
                       transaction = NULL;
                   }
               }


               // If we cannot lock it, it does not exist (or at least we  pretend it does not exist).
               // The transaction will be null if it has been deleted or we cannot get a lock on it.
               if (transaction)
               {
                  SipMessage* delayedDispatchMessage = NULL;
                  transaction->handleResendEvent(*sipMessage,
                                                 *this,
                                                 relationship,
                                                 mSipTransactions,
                                                 nextTimeout,
                                                 delayedDispatchMessage);

                  if(nextTimeout == 0)
                  {
                     if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
                     {
                        UtlString transactionString;
                        transaction->toString(transactionString, TRUE);
                        transactionString.insert(0,
                                                 "SipUserAgent::handleMessage "
                                                 "timeout send failed\n"
                                                 );
                        OsSysLog::add(FAC_SIP, PRI_DEBUG, "%s\n", transactionString.data());
                        //osPrintf("%s\n", transactionString.data());
                     }
                  }

                  if(delayedDispatchMessage)
                  {
                     // Only bother processing if the logs are enabled
                     if (    isMessageLoggingEnabled() ||
                         OsSysLog::willLog(FAC_SIP_INCOMING, PRI_DEBUG))
                     {
                        UtlString delayMsgString;
                        ssize_t delayMsgLen;
                        delayedDispatchMessage->getBytes(&delayMsgString,
                                                         &delayMsgLen);
                        delayMsgString.insert(0, "SIP User agent delayed dispatch message:\n");
                        delayMsgString.append("++++++++++++++++++++END++++++++++++++++++++\n");

                        logMessage(delayMsgString.data(), delayMsgString.length());
                        OsSysLog::add(FAC_SIP_INCOMING_PARSED, PRI_DEBUG,"%s",
                                      delayMsgString.data());
                     }

                     queueMessageToObservers(delayedDispatchMessage,
                                             SipMessageEvent::APPLICATION
                                             );

                     // delayedDispatchMessage gets freed in queueMessageToObservers
                     delayedDispatchMessage = NULL;
                  }
               }    // end transaction is not null

               // No transaction for this timeout
               else
               {
                  // If the user agent is shutting down, we don't intend
                  // to service this timeout anyway.
                  if (!(mbShuttingDown || mbShutdownDone))
                  {
                     // Somehow the transaction got deleted perhaps it timed
                     // out and there was a log jam that prevented the handling
                     // of the timeout ????? This should not happen.
                     OsSysLog::add(FAC_SIP, PRI_ERR, "SipUserAgent::handleMessage "
                                   "SIP message timeout expired with no matching transaction");
                  }
               }

               if(transaction)
               {
                  mSipTransactions.markAvailable(*transaction);
               }

               // Do this outside so that we do not get blocked
               // on locking or delete the transaction out
               // from under ouselves
               if(nextTimeout == 0)
               {
                  // Make a copy and dispatch it
                  dispatch(new SipMessage(*sipMessage),
                           SipMessageEvent::TRANSPORT_ERROR);
               }

               // The timer made its own copy of this message.
               // It is deleted by dispatch ?? if it is not
               // rescheduled.
            } // End if sipMessage

            // The timer that sent this event will be deleted by SipTransaction::~.
            // The attached SipMessageEvent will be deleted below.
         } // End SipMessageEvent::TRANSACTION_RESEND

         // Timeout for an transaction to expire
         else if(msgEventType == SipMessageEvent::TRANSACTION_EXPIRATION)
         {
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipUserAgent[%s]::handleMessage "
                          "transaction expiration message received",
                          getName().data());

            if(sipMessage)
            {
               // WARNING: you cannot touch the contents of the transaction
               // attached to the message until the transaction has been
               // locked (via findTransactionFor, if no transaction is
               // returned, it either no longer exists or we could not get
               // a lock for it.

               if(sipMessage->getSipTransaction() == NULL)
               {
                  OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                "SipUserAgent[%s]::handleMessage "
                                "expires Timeout message with NULL transaction",
                                getName().data()
                                );
               }

               int nextTimeout = -1;
               enum SipTransaction::messageRelationship relationship;
               //mSipTransactions.lock();
#ifdef TRANSACTION_MATCH_DEBUG // enable only for transaction match debugging - log is confusing otherwise
            OsSysLog::add(FAC_SIP, PRI_DEBUG
                          ,"SipUserAgent[%s]::handleMessage(expired) "
                           "searching for transaction",
                          getName().data());
#           endif
               SipTransaction* transaction =
                  mSipTransactions.findTransactionFor(*sipMessage,
                                                      TRUE, // timers are only set for outgoing?
                                                      relationship);
               if(transaction)
               {
#ifdef TRANSACTION_MATCH_DEBUG // enable only for transaction match debugging - log is confusing otherwise
            OsSysLog::add(FAC_SIP, PRI_DEBUG
                          ,"SipUserAgent[%s]::handleMessage(expired) "
                           "found transaction %p",
                          getName().data(), transaction);
#           endif
                   // If we are shutting down, unlock the transaction
                   // and set it to null.  We pretend that the transaction
                   // does not exist (i.e. no-op).
                   if (mbShuttingDown || mbShutdownDone)
                   {
                       mSipTransactions.markAvailable(*transaction);
                       transaction = NULL;
                   }
               }

               if(transaction)
               {
                  SipMessage* delayedDispatchMessage = NULL;
                  transaction->handleExpiresEvent(*sipMessage,
                                                  *this,
                                                  relationship,
                                                  mSipTransactions,
                                                  nextTimeout,
                                                  delayedDispatchMessage);

                  mSipTransactions.markAvailable(*transaction);

                  if(delayedDispatchMessage)
                  {
                     // Only bother processing if the logs are enabled
                     if (    isMessageLoggingEnabled() ||
                         OsSysLog::willLog(FAC_SIP_INCOMING_PARSED, PRI_DEBUG))
                     {
                        UtlString delayMsgString;
                        ssize_t delayMsgLen;
                        delayedDispatchMessage->getBytes(&delayMsgString,
                                                         &delayMsgLen);
                        delayMsgString.insert(0, "SIP User agent delayed dispatch message:\n");
                        delayMsgString.append("++++++++++++++++++++END++++++++++++++++++++\n");

                        logMessage(delayMsgString.data(), delayMsgString.length());
                        OsSysLog::add(FAC_SIP_INCOMING_PARSED, PRI_DEBUG,"%s",
                                      delayMsgString.data());
                     }

                     queueMessageToObservers(delayedDispatchMessage,
                                             SipMessageEvent::APPLICATION
                                             );

                     //delayedDispatchMessage gets freed in queueMessageToObservers
                     delayedDispatchMessage = NULL;
                  }
               }
               else // Could not find a transaction for this expired message
               {
                  if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
                  {
                     UtlString noTxMsgString;
                     ssize_t noTxMsgLen;
                     sipMessage->getBytes(&noTxMsgString, &noTxMsgLen);

                     OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                   "SipUserAgent[%s]::handleMessage "
                                   "event timeout with no matching transaction: %s",
                                   getName().data(), noTxMsgString.data());
                  }
               }
            }

            // The timer that sent this event will be deleted by SipTransaction::~.
            // The attached SipMessageEvent will be deleted below.
         } // End SipMessageEvent::TRANSACTION_EXPIRATION

         // Unknown timeout
         else
         {
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "SipUserAgent::handleMessage unknown timeout event: %d.", msgEventType);
         }

         // Since the timer has fired, it is our responsibility to
         // delete the SipMessageEvent.  The attached SipMessage will
         // be freed with the SipMessageEvent.
         delete sipEvent;
      } // end if sipEvent
      messageProcessed = TRUE;
   }

   else
   {
      messageProcessed = TRUE;
   }

   // Only GC if no messages are waiting -- othewise we may delete a timer
   // that is queued up for us.
   if (getMessageQueue()->isEmpty())
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipUserAgent[%s]::handleMessage calling garbageCollection()",
                    getName().data());
      garbageCollection();
   }
   return(messageProcessed);
}

void SipUserAgent::garbageCollection()
{
    OsTime time;
    OsDateTime::getCurTimeSinceBoot(time);
    long bootime = time.seconds();

    long then = bootime - (mTransactionStateTimeoutMs / 1000);
    long tcpThen = bootime - mMaxTcpSocketIdleTime;
    long oldTransaction = then - (mTransactionStateTimeoutMs / 1000);
    long oldInviteTransaction = then - mMinInviteTransactionTimeout;

    // If the timeout is negative we never timeout or garbage collect
    // tcp connections
    if(mMaxTcpSocketIdleTime < 0)
    {
        tcpThen = -1;
    }

    if(mLastCleanUpTime < then)      // tx timeout could have happened
    {
       #ifdef LOG_TIME
          OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipUserAgent[%s]::garbageCollection"
                        " bootime: %ld then: %ld tcpThen: %ld"
                        " oldTransaction: %ld oldInviteTransaction: %ld",
                        getName().data(),
                        bootime, then, tcpThen, oldTransaction,
                        oldInviteTransaction);
          #endif
       mSipTransactions.removeOldTransactions(oldTransaction,
                                              oldInviteTransaction);
       if (mSipUdpServer)
       {
          #ifdef LOG_TIME
             OsSysLog::add(FAC_SIP, PRI_DEBUG,
                           "SipUserAgent[%s]::garbageCollection starting removeOldClients(udp)",
                           getName().data());
          #endif
          mSipUdpServer->removeOldClients(then);
       }
       if (mSipTcpServer)
       {
          #ifdef LOG_TIME
             OsSysLog::add(FAC_SIP, PRI_DEBUG,
                           "SipUserAgent[%s]::garbageCollection starting removeOldClients(tcp)",
                           getName().data());
          #endif
          mSipTcpServer->removeOldClients(tcpThen);
       }
       #ifdef SIP_TLS
          if (mSipTlsServer)
          {
             #ifdef LOG_TIME
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                              "SipUserAgent[%s]::garbageCollection starting removeOldClients(tls)",
                              getName().data());
             #endif
             mSipTlsServer->removeOldClients(tcpThen);
          }
       #endif // SIP_TLS
       #ifdef LOG_TIME
          OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipUserAgent[%s]::garbageCollection done",
                        getName().data());
       #endif
       mLastCleanUpTime = bootime;
    }
}

/* ============================ ACCESSORS ================================= */

// Enable or disable the outbound use of rport (send packet to actual
// port -- not advertised port).
UtlBoolean SipUserAgent::setUseRport(UtlBoolean bEnable)
{
    UtlBoolean bOld = mbUseRport;

    mbUseRport = bEnable;

    return bOld;
}

// Is use report set?
UtlBoolean SipUserAgent::getUseRport() const
{
    return mbUseRport;
}

void SipUserAgent::setUserAgentName(const UtlString& name)
{
    defaultUserAgentName = name;
    return;
}

const UtlString& SipUserAgent::getUserAgentName() const
{
    return defaultUserAgentName;
}


// Get the manually configured public address
UtlBoolean SipUserAgent::getConfiguredPublicAddress(UtlString* pIpAddress, int* pPort)
{
    UtlBoolean bSuccess = FALSE;

    if (mSipUdpServer &&
        mConfigPublicAddress.length())
    {
        if (pIpAddress)
        {
            *pIpAddress = mConfigPublicAddress;
        }

        if (pPort)
        {
            *pPort = mSipUdpServer->getServerPort();
        }

        bSuccess = TRUE;
    }

    return bSuccess;
}

// Get the local address and port.
UtlBoolean SipUserAgent::getLocalAddress(UtlString* pIpAddress, int* pPort)
{
   if (pIpAddress)
   {
      *pIpAddress = mLocalHostAddress;
   }

   if (pPort)
   {
      if (mLocalUdpHostPort != -1)
      {
         *pPort = mLocalUdpHostPort;
      }
      else if (mLocalTcpHostPort != -1)
      {
         *pPort = mLocalTcpHostPort;
      }
      else if (mLocalTlsHostPort != -1)
      {
         *pPort = mLocalTlsHostPort;
      }
   }

   return mLocalHostValid;
}

// Get the local address and port when SipUserAgent starts and save them.
void SipUserAgent::cacheLocalAddress()
{
   mLocalHostValid = false;
   mLocalUdpHostPort = -1;
   mLocalTcpHostPort = -1;
   mLocalTlsHostPort = -1;

   if (mSipUdpServer || mSipTcpServer || mSipTlsServer)
   {
      if (defaultSipAddress.length() > 0)
      {
         mLocalHostAddress = defaultSipAddress;
      }
      else
      {
         OsSocket::getHostIp(&mLocalHostAddress);
      }

      if (mSipUdpServer)
      {
         mLocalUdpHostPort = mSipUdpServer->getServerPort();
      }
      else if (mSipTcpServer)
      {
         mLocalTcpHostPort = mSipTcpServer->getServerPort();
      }
#ifdef SIP_TLS
      else if (mSipTlsServer)
      {
         mLocalTlsHostPort = mSipTlsServer->getServerPort();
      }
#endif

      mLocalHostValid = true;
   }
}

// Get the NAT mapped address and port
UtlBoolean SipUserAgent::getNatMappedAddress(UtlString* pIpAddress, int* pPort)
{
    UtlBoolean bRet(FALSE);

    if (mSipUdpServer)
    {
        bRet = mSipUdpServer->getStunAddress(pIpAddress, pPort);
    }
    else if (mSipTcpServer)
    {
        // TODO - a TCP server should also be able to return a stun address
        //bRet = mSipTcpServer->getStunAddress(pIpAddress, pPort);
    }
    return bRet;
}

// Get the Contact URI.
void SipUserAgent::getContactURI(UtlString& contact)
{
   contact = mContactURI;
}

void SipUserAgent::setIsUserAgent(UtlBoolean isUserAgent)
{
    mIsUaTransactionByDefault = isUserAgent;
}

/// Call either setServerHeader or setUserAgentHeader, as appropriate based on isUserAgent.
void SipUserAgent::setSelfHeader(SipMessage& message)
{
   if (mIsUaTransactionByDefault)
   {
      setUserAgentHeader(message);
   }
   else
   {
      setServerHeader(message);
   }
}


// setUserAgentHeaderProperty
//      provides a string to be appended to the standard User-Agent
//      header value between "<vendor>/<version>" and the platform (eg "(VxWorks)")
//      Value should be formated either as "token/token" or "(string)"
//      with no leading or trailing space.
void SipUserAgent::setUserAgentHeaderProperty( const char* property )
{
    if ( property )
    {
       mUserAgentHeaderProperties.append(" ");
       mUserAgentHeaderProperties.append( property );
    }
}


void SipUserAgent::setMaxForwards(int maxForwards)
{
    if(maxForwards > 0)
    {
        mMaxForwards = maxForwards;
    }
    else
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipUserAgent[%s]::setMaxForwards maxForwards <= 0: %d",
                      getName().data(),
                      maxForwards);
    }
}

int SipUserAgent::getMaxForwards()
{
    int maxForwards;
    if(mMaxForwards <= 0)
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipUserAgent[%s]::getMaxForwards maxForwards <= 0: %d",
                      getName().data(),
                      mMaxForwards);

        maxForwards = SIP_DEFAULT_MAX_FORWARDS;
    }
    else
    {
        maxForwards = mMaxForwards;
    }

    return(maxForwards);
}

int SipUserAgent::getMaxSrvRecords() const
{
    return(mMaxSrvRecords);
}

void SipUserAgent::setMaxSrvRecords(int maxSrvRecords)
{
    mMaxSrvRecords = maxSrvRecords;
}

int SipUserAgent::getDnsSrvTimeout()
{
    return(mDnsSrvTimeout);
}

void SipUserAgent::setDnsSrvTimeout(int timeout)
{
    mDnsSrvTimeout = timeout;
}

void SipUserAgent::setForking(UtlBoolean enabled)
{
    mForkingEnabled = enabled;
}

void SipUserAgent::getAllowedMethods(UtlString* allowedMethods)
{
   UtlDListIterator iterator(allowedSipMethods);
   allowedMethods->remove(0);
   UtlString* method;

   while ((method = (UtlString*) iterator()))
   {
      if(!method->isNull())
      {
         if(!allowedMethods->isNull())
         {
            allowedMethods->append(", ");
         }
         allowedMethods->append(method->data());
      }
   }
}

void SipUserAgent::getViaInfo(int protocol,
                              UtlString& address,
                              int& port)
{
    if(protocol == OsSocket::TCP)
    {
        port = mTcpPort == SIP_PORT ? PORT_NONE : mTcpPort;
    }
#ifdef SIP_TLS
    else if(protocol == OsSocket::SSL_SOCKET)
    {
        port = mTlsPort == SIP_TLS_PORT ? PORT_NONE : mTlsPort;
    }
#endif
    else
    {
        // Default to UDP and warning if the protocol type is not UDP
        if(protocol != OsSocket::UDP)
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "SipUserAgent::getViaInfo unknown protocol: %d",
                          protocol);
        }

        if(portIsValid(mSipPort))
        {
            port = mSipPort;
        }
        else if(mUdpPort == SIP_PORT)
        {
            port = PORT_NONE;
        }
        else
        {
            port = mUdpPort;
        }
    }

    address = sipIpAddress;
}

void SipUserAgent::getFromAddress(UtlString* address, int* port, UtlString* protocol)
{
   UtlTokenizer tokenizer(registryServers);
   UtlString regServer;

   tokenizer.next(regServer, ",");
   SipMessage::parseAddressFromUri(regServer.data(), address,
                                   port, protocol);

    if(address->isNull())
    {
            protocol->remove(0);
            // TCP only
            if(portIsValid(mTcpPort) && !portIsValid(mUdpPort))
            {
                    protocol->append(SIP_TRANSPORT_TCP);
                    *port = mTcpPort;
            }
            // UDP only
            else if(portIsValid(mUdpPort) && !portIsValid(mTcpPort))
            {
                    protocol->append(SIP_TRANSPORT_UDP);
                    *port = mUdpPort;
            }
            // TCP & UDP on non-standard port
            else if(mTcpPort != SIP_PORT)
            {
                    *port = mTcpPort;
            }
            // TCP & UDP on standard port
            else
            {
                    *port = PORT_NONE;
            }

            // If there is an address configured use it
            NameValueTokenizer::getSubField(defaultSipAddress.data(), 0,
                    ", \t", address);

            // else use the local host ip address
            if(address->isNull())
            {
            address->append(sipIpAddress);
                    //OsSocket::getHostIp(address);
            }
    }
}

void SipUserAgent::getDirectoryServer(int index, UtlString* address,
                                      int* port, UtlString* protocol)
{
        UtlString serverAddress;
        NameValueTokenizer::getSubField(directoryServers.data(), 0,
                SIP_MULTIFIELD_SEPARATOR, &serverAddress);

        address->remove(0);
        *port = PORT_NONE;
        protocol->remove(0);
        SipMessage::parseAddressFromUri(serverAddress.data(),
                address, port, protocol);
        serverAddress.remove(0);
}

void SipUserAgent::getProxyServer(int index, UtlString* address,
                                  int* port, UtlString* protocol)
{
        UtlString serverAddress;
        NameValueTokenizer::getSubField(proxyServers.data(), 0,
                SIP_MULTIFIELD_SEPARATOR, &serverAddress);

        address->remove(0);
        *port = PORT_NONE;
        protocol->remove(0);
        SipMessage::parseAddressFromUri(serverAddress.data(), address, port, protocol);
        serverAddress.remove(0);
}

void SipUserAgent::setProxyServers(const char* sipProxyServers)
{
    if (sipProxyServers)
    {
        proxyServers = sipProxyServers;
    }
    else
    {
        proxyServers.remove(0);
    }
}

int SipUserAgent::getSipStateTransactionTimeout()
{
    return mTransactionStateTimeoutMs;
}

int SipUserAgent::getReliableTransportTimeout()
{
    return(mReliableTransportTimeoutMs);
}

int SipUserAgent::getUnreliableTransportTimeout()
{
    return(mUnreliableTransportTimeoutMs);
}

int SipUserAgent::getMaxResendTimeout()
{
    return(mMaxResendTimeoutMs);
}

void SipUserAgent::setInviteTransactionTimeoutSeconds(int expiresSeconds)
{
    if(expiresSeconds > 0 )
    {
        mMinInviteTransactionTimeout = expiresSeconds;
        if(expiresSeconds > DEFAULT_SIP_TRANSACTION_EXPIRES)
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "SipUserAgent::setInviteTransactionTimeoutSeconds "
                          "large expiresSeconds value: %d NOT RECOMMENDED",
                          expiresSeconds);
        }
    }
    else
    {
        OsSysLog::add(FAC_SIP, PRI_ERR,
                      "SipUserAgent::setInviteTransactionTimeoutSeconds "
                      "illegal expiresSeconds value: %d IGNORED",
                      expiresSeconds);
    }
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipUserAgent::setInviteTransactionTimeoutSeconds "
                  "mMinInviteTransactionTimeout %d ",
                  mMinInviteTransactionTimeout);
}

int SipUserAgent::getDefaultExpiresSeconds() const
{
    return(mDefaultExpiresSeconds);
}

void SipUserAgent::setDefaultExpiresSeconds(int expiresSeconds)
{
    if(expiresSeconds > 0 )
    {
        mDefaultExpiresSeconds = expiresSeconds;
        if(expiresSeconds > DEFAULT_SIP_TRANSACTION_EXPIRES)
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "SipUserAgent::setDefaultExpiresSeconds "
                          "large expiresSeconds value: %d NOT RECOMMENDED",
                          expiresSeconds);
        }
        if(expiresSeconds > mMinInviteTransactionTimeout)
        {
            setInviteTransactionTimeoutSeconds(expiresSeconds);
        }
        if(expiresSeconds > mMaxTcpSocketIdleTime)
        {
            setMaxTcpSocketIdleTime(expiresSeconds);
        }
    }
    else
    {
        OsSysLog::add(FAC_SIP, PRI_ERR,
                      "SipUserAgent::setDefaultExpiresSeconds "
                      "illegal expiresSeconds value: %d IGNORED",
                      expiresSeconds);
    }
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipUserAgent::setDefaultExpiresSeconds "
                  "mDefaultExpiresSeconds %d ",
                  mDefaultExpiresSeconds);
}

int SipUserAgent::getDefaultSerialExpiresSeconds() const
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipUserAgent::getDefaultSerialExpiresSeconds time=%d",
                  mDefaultSerialExpiresSeconds);

    return(mDefaultSerialExpiresSeconds);
}

void SipUserAgent::setDefaultSerialExpiresSeconds(int expiresSeconds)
{
    if(expiresSeconds > 0 )
    {
        mDefaultSerialExpiresSeconds = expiresSeconds;
        if(expiresSeconds > DEFAULT_SIP_TRANSACTION_EXPIRES)
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "SipUserAgent::setDefaultSerialExpiresSeconds "
                          "large expiresSeconds value: %d NOT RECOMMENDED",
                          expiresSeconds);
        }
        if(expiresSeconds > mMinInviteTransactionTimeout)
        {
            setInviteTransactionTimeoutSeconds(expiresSeconds);
        }
        if(expiresSeconds > mMaxTcpSocketIdleTime)
        {
            setMaxTcpSocketIdleTime(expiresSeconds);
        }
    }
    else
    {
        OsSysLog::add(FAC_SIP, PRI_ERR,
                      "SipUserAgent::setDefaultSerialExpiresSeconds "
                      "illegal expiresSeconds value: %d IGNORED",
                      expiresSeconds);
    }
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipUserAgent::setDefaultSerialExpiresSeconds "
                  "mDefaultSerialExpiresSeconds %d ",
                  mDefaultSerialExpiresSeconds);
}

void SipUserAgent::setMaxTcpSocketIdleTime(int idleTimeSeconds)
{
    if(mMinInviteTransactionTimeout <= idleTimeSeconds)
    {
        mMaxTcpSocketIdleTime = idleTimeSeconds;
    }
    else
    {
        OsSysLog::add(FAC_SIP, PRI_ERR, "SipUserAgent::setMaxTcpSocketIdleTime "
                      "idleTimeSeconds: %d less than mMinInviteTransactionTimeout: %d, ignored",
                      idleTimeSeconds, mMinInviteTransactionTimeout);
    }
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipUserAgent::setMaxTcpSocketIdleTime "
                  "mMaxTcpSocketIdleTime value: %d ",
                  mMaxTcpSocketIdleTime);
}

void SipUserAgent::setHostAliases(const UtlString& aliases)
{
    UtlString aliasString;

    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipUserAgent[%s]::setHostAliases('%s')",
                  getName().data(),
                  aliases.data());

    for ( int aliasIndex = 0;
          NameValueTokenizer::getSubField(aliases.data(), aliasIndex,
                                          ", \t", &aliasString);
          aliasIndex++
         )
    {
        Url aliasUrl(aliasString);
        UtlString hostAlias;
        aliasUrl.getHostAddress(hostAlias);
        int port = aliasUrl.getHostPort();

        if(!portIsValid(port))
        {
            hostAlias.append(":5060");
        }
        else
        {
            char portString[20];
            sprintf(portString, ":%d", port);
            hostAlias.append(portString);
        }

        OsSysLog::add(FAC_SIP, PRI_INFO, "SipUserAgent::setHostAliases adding '%s'",
                      hostAlias.data());

        UtlString* newAlias = new UtlString(hostAlias);
        mMyHostAliases.insert(newAlias);
    }
}

#if 0
void SipUserAgent::printStatus()

{
    if(mSipUdpServer)
    {
        mSipUdpServer->printStatus();
    }
    if(mSipTcpServer)
    {
        mSipTcpServer->printStatus();
    }
#ifdef SIP_TLS
    if(mSipTlsServer)
    {
        mSipTlsServer->printStatus();
    }
#endif

    UtlString txString;
    mSipTransactions.toString(txString);

    osPrintf("Transactions:\n%s\n", txString.data());
}
#endif // 0

void SipUserAgent::startMessageLog(int newMaximumLogSize)
{
    if(newMaximumLogSize > 0) mMaxMessageLogSize = newMaximumLogSize;
    if(newMaximumLogSize == -1) mMaxMessageLogSize = -1;
    mMessageLogEnabled = TRUE;

    {
                OsWriteLock Writelock(mMessageLogWMutex);
                OsReadLock ReadLock(mMessageLogRMutex);
                if(mMaxMessageLogSize > 0)
                        mMessageLog.capacity(mMaxMessageLogSize);
        }
}

void SipUserAgent::stopMessageLog()
{
    mMessageLogEnabled = FALSE;
}

void SipUserAgent::clearMessageLog()
{
        OsWriteLock Writelock(mMessageLogWMutex);
        OsReadLock Readlock(mMessageLogRMutex);
        mMessageLog.remove(0);
}

void SipUserAgent::logMessage(const char* message, int messageLength)
{
    if(mMessageLogEnabled)
    {
       {// lock scope
          OsWriteLock Writelock(mMessageLogWMutex);
          // Do not allow the log go grow beyond the maximum
          if(mMaxMessageLogSize > 0 &&
             ((((int)mMessageLog.length()) + messageLength) > mMaxMessageLogSize))
          {
             mMessageLog.remove(0,
                                mMessageLog.length() + messageLength - mMaxMessageLogSize);
          }

          mMessageLog.append(message, messageLength);
       }//lock scope
    }
}

void SipUserAgent::getMessageLog(UtlString& logData)
{
        OsReadLock Readlock(mMessageLogRMutex);
        logData = mMessageLog;
}

void SipUserAgent::allowExtension(const char* extension)
{
   UtlString* extensionName = new UtlString(extension);
    allowedSipExtensions.append(extensionName);
}

void SipUserAgent::getSupportedExtensions(UtlString& extensionsString)
{
    extensionsString.remove(0);
    UtlString* extensionName = NULL;
    UtlDListIterator iterator(allowedSipExtensions);
    while ((extensionName = (UtlString*) iterator()))
    {
        if(!extensionsString.isNull()) extensionsString.append(", ");
        extensionsString.append(extensionName->data());
    }
}

void SipUserAgent::requireExtension(const char* extension)
{
    requiredSipExtensions.append(new UtlString(extension));
}

void SipUserAgent::getRequiredExtensions(UtlString& extensionsString)
{
    extensionsString.remove(0);
    UtlString* extensionName = NULL;
    UtlDListIterator iterator(requiredSipExtensions);

    while ((extensionName = (UtlString*) iterator()))
    {
        if(!extensionsString.isNull()) extensionsString.append(", ");
        extensionsString.append(extensionName->data());
    }
}

void SipUserAgent::setRecurseOnlyOne300Contact(UtlBoolean recurseOnlyOne)
{
    mRecurseOnlyOne300Contact = recurseOnlyOne;
}

SipMessage* SipUserAgent::getRequest(const SipMessage& response)
{
    // If the transaction exists and can be locked it
    // is returned.
    enum SipTransaction::messageRelationship relationship;
#ifdef TRANSACTION_MATCH_DEBUG // enable only for transaction match debugging - log is confusing otherwise
            OsSysLog::add(FAC_SIP, PRI_DEBUG
                          ,"SipUserAgent[%s]::getRequest searching for transaction",
                          getName().data());
#           endif
    SipTransaction* transaction =
        mSipTransactions.findTransactionFor(response,
                                             FALSE, // incoming
                                             relationship);
    SipMessage* request = NULL;

    if(transaction && transaction->getRequest())
    {
        // Make a copy to return
        request = new SipMessage(*(transaction->getRequest()));
    }

    // Need to unlock the transaction
    if(transaction)
        mSipTransactions.markAvailable(*transaction);

    return(request);
}

int SipUserAgent::getTcpPort() const
{
    int iPort = PORT_NONE;

    if (mSipTcpServer)
    {
        iPort = mSipTcpServer->getServerPort();
    }

    return iPort;
}

int SipUserAgent::getUdpPort() const
{
    int iPort = PORT_NONE;

    if (mSipUdpServer)
    {
        iPort = mSipUdpServer->getServerPort();
    }

    return iPort;
}

int SipUserAgent::getTlsPort() const
{
    int iPort = PORT_NONE;

#ifdef SIP_TLS
    if (mSipTlsServer)
    {
        iPort = mSipTlsServer->getServerPort();
    }
#endif

    return iPort;
}


/* ============================ INQUIRY =================================== */

UtlBoolean SipUserAgent::isMethodAllowed(const char* method)
{
        UtlString methodName(method);
        UtlBoolean isAllowed = (allowedSipMethods.occurrencesOf(&methodName) > 0);

        if (!isAllowed)
        {
           /* The method was not explicitly requested, but check for whether the
            * application has registered for the wildcard.  If so, the method is
            * allowed, but we do not advertise that fact in the Allow header.*/
           UtlString wildcardMethod;

           OsReadLock lock(mObserverMutex);
           isAllowed = mMessageObservers.contains(&wildcardMethod);
        }

        return(isAllowed);
}

UtlBoolean SipUserAgent::isExtensionAllowed(const char* extension) const
{
    UtlString extensionString;
    if(extension) extensionString.append(extension);
    extensionString.toLower();
        UtlString extensionName(extensionString);
        extensionString.remove(0);
        return(allowedSipExtensions.occurrencesOf(&extensionName) > 0);
}

UtlBoolean SipUserAgent::isExtensionRequired(const char* extension) const
{
    UtlString extensionString( extension );
    extensionString.toLower();
    return NULL != requiredSipExtensions.find( &extensionString );
}

void SipUserAgent::whichExtensionsNotAllowed(const SipMessage* message,
                                                           UtlString* disallowedExtensions) const
{
        int extensionIndex = 0;
        UtlString extension;

        disallowedExtensions->remove(0);
        while(message->getRequireExtension(extensionIndex, &extension))
        {
                if(!isExtensionAllowed(extension.data()))
                {
                        if(!disallowedExtensions->isNull())
                        {
                                disallowedExtensions->append(SIP_MULTIFIELD_SEPARATOR);
                                disallowedExtensions->append(SIP_SINGLE_SPACE);
                        }
                        disallowedExtensions->append(extension.data());
                }
                extensionIndex++;
        }
        extension.remove(0);
}

UtlBoolean SipUserAgent::isMessageLoggingEnabled()
{
    return(mMessageLogEnabled);
}

UtlBoolean SipUserAgent::isForkingEnabled()
{
    return(mForkingEnabled);
}

UtlBoolean SipUserAgent::isMyHostAlias(const Url& route) const
{
    UtlString hostAlias;
    route.getHostAddress(hostAlias);
    int port = route.getHostPort();

    if(port == PORT_NONE)
    {
        hostAlias.append(":5060");
    }
    else
    {
        char portString[20];
        sprintf(portString, ":%d", port);
        hostAlias.append(portString);
    }

    UtlString aliasMatch(hostAlias);
    UtlContainable* found = mMyHostAliases.find(&aliasMatch);

    return(found != NULL);
}

UtlBoolean SipUserAgent::recurseOnlyOne300Contact()
{
    return(mRecurseOnlyOne300Contact);
}


UtlBoolean SipUserAgent::isOk(OsSocket::IpProtocolSocketType socketType)
{
   UtlBoolean retval = FALSE;
   switch(socketType)
   {
   case OsSocket::TCP :
      if (mSipTcpServer)
      {
         retval = mSipTcpServer->isOk();
      }
      break;
   case OsSocket::UDP :
      if (mSipUdpServer)
      {
         retval = mSipUdpServer->isOk();
      }
      break;
#ifdef SIP_TLS
   case OsSocket::SSL_SOCKET :
      if (mSipTlsServer)
      {
         retval = mSipTlsServer->isOk();
      }
      break;
#endif
   default :
      OsSysLog::add(FAC_SIP, PRI_ERR, "SipUserAgent::isOK - invalid socket type %d",
                    socketType);
      break;
   }

   return retval;
}

UtlBoolean SipUserAgent::isOk()
{
    UtlBoolean retval = TRUE;

    if (mSipTcpServer)
    {
        retval = retval && mSipTcpServer->isOk();
    }
    if (mSipUdpServer && retval)
    {
        retval = retval && mSipUdpServer->isOk();
    }
#ifdef SIP_TLS
    if (mSipTlsServer && retval)
    {
        retval = retval && mSipTlsServer->isOk();
    }
#endif

    // The SipUserAgent is NOT OK if no protocol servers are available
    // This could happen if PORT_NONE is specified for all ports
    if (retval
            && (mSipTcpServer == NULL)
            && (mSipUdpServer == NULL)
#ifdef SIP_TLS
            && (mSipTlsServer == NULL)
#endif
            )
    {
        retval = FALSE;
    }

    return retval;
}

UtlBoolean SipUserAgent::isSymmetricSignalingImposed()
{
    return mbForceSymmetricSignaling;
}

UtlBoolean SipUserAgent::isShutdownDone()
{
    return mbShutdownDone;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
UtlBoolean SipUserAgent::resendWithAuthorization(SipMessage* response,
                                                 SipMessage* request,
                                                 int* messageType,
                                                 int authorizationEntity)
{
        UtlBoolean requestResent =FALSE;
        int sequenceNum;
        UtlString method;
        response->getCSeqField(&sequenceNum, &method);

        SipMessage* authorizedRequest = new SipMessage();

        OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                      "SipUserAgent::resendWithAuthorization ");
        if ( mpLineMgr && mpLineMgr->buildAuthorizationRequest(response, request,authorizedRequest))
        {
           requestResent = send(*authorizedRequest);
           // Send the response back to the application
           // to notify it of the CSeq change for the response
           *messageType = SipMessageEvent::AUTHENTICATION_RETRY;
        }
        else
        {
           // error was logged in SipLineMgr
        }

    delete authorizedRequest;

    return(requestResent);
}

/// Apply the recorded User-Agent/Server header value as a Server header.
void SipUserAgent::setServerHeader(SipMessage& message)
{
   UtlString existing;
   message.getServerField(&existing);

   if(existing.isNull())
   {
      UtlString headerValue;
      selfHeaderValue(headerValue);

      message.setServerField(headerValue.data());
   }
}

// Apply the recorded User-Agent/Server header value as a User-Agent header.
void SipUserAgent::setUserAgentHeader(SipMessage& message)
{
   UtlString uaName;
   message.getUserAgentField(&uaName);

   if(uaName.isNull())
   {
      selfHeaderValue(uaName);
      message.setUserAgentField(uaName.data());
   }
}

// Compose the full User-Agent/Server header value.
void SipUserAgent::selfHeaderValue(UtlString& self)
{
   self = defaultUserAgentName;

   if ( !mUserAgentHeaderProperties.isNull() )
   {
      self.append(mUserAgentHeaderProperties);
   }

   if (mbIncludePlatformInUserAgentName)
   {
      self.append(PLATFORM_UA_PARAM);
   }
}

void SipUserAgent::setIncludePlatformInUserAgentName(const bool bInclude)
{
    mbIncludePlatformInUserAgentName = bInclude;
}

const bool SipUserAgent::addContactAddress(ContactAddress& contactAddress)
{
    return mContactDb.addContact(contactAddress);
}

void SipUserAgent::getContactAddresses(ContactAddress* pContacts[], int &numContacts)
{
    mContactDb.getAll(pContacts, numContacts);
}

UtlBoolean SipUserAgent::doesMaddrMatchesUserAgent(SipMessage& message)
{
   UtlBoolean bMatch = false;

   // "If the Request-URI contains a maddr parameter, the proxy MUST check
   // to see if its value is in the set of addresses or domains the proxy
   // is configured to be responsible for.  If the Request-URI has a maddr
   // parameter with a value the proxy is responsible for, and the request
   // was received using the port and transport indicated (explicitly or by
   // default) in the Request-URI, the proxy MUST strip the maddr and any
   // non-default port or transport parameter and continue processing as if
   // those values had not been present in the request."
   //

   // Must be a response
   if (!message.isResponse())
   {
      UtlString uriStr;
      UtlString maddr;
      message.getRequestUri(&uriStr);
      Url uri(uriStr, Url::AddrSpec, NULL);
      if (uri.getUrlParameter("maddr", maddr) && !maddr.isNull())
      {
         // Normalize Port
         int uriPort = uri.getHostPort();
         if (uriPort == PORT_NONE)
             uriPort = SIP_PORT;

         // IP/Port must Match -- TODO:: Host check
         if ((message.getInterfaceIp().compareTo(maddr) == 0) &&
                 (uriPort == message.getInterfacePort()))
         {
            OsSocket::IpProtocolSocketType socketType = message.getSendProtocol();
            UtlString transport;

            switch (socketType)
            {
            case OsSocket::UDP:
               uri.getUrlParameter("transport", transport);
               if (transport.isNull() || transport.compareTo("UDP", UtlString::ignoreCase) == 0)
               {
                  bMatch = TRUE;
               }
               break;
            case OsSocket::TCP:
               uri.getUrlParameter("transport", transport);
               if (transport.compareTo("TCP", UtlString::ignoreCase) == 0)
               {
                  bMatch = TRUE;
               }
               break;
            case OsSocket::SSL_SOCKET:
               uri.getUrlParameter("transport", transport);
               // Note: checking for transport=tls for RFC 2543 support
               if ((transport.compareTo("TLS", UtlString::ignoreCase) == 0) ||
                     uri.getScheme() == Url::SipsUrlScheme)
               {
                  bMatch = TRUE;
               }
               break;
            default:
               // If we don't have or understand the transport -- allow it.
               break;
            }
         }
      }
   }

   return bMatch;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
