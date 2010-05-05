//
//
// Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsFS.h>
#include <os/OsSysLog.h>
#include <os/OsConfigDb.h>

#include <cp/CallManager.h>
#include <cp/DialogEventPublisher.h>
#include <mi/CpMediaInterfaceFactoryFactory.h>
#include <mp/MpMediaTask.h>
#include <mp/NetInTask.h>
#include <net/NameValueTokenizer.h>
#include <net/SdpCodecFactory.h>
#include <net/SipPublishContentMgr.h>
#include <net/SipSubscribeServerEventHandler.h>
#include <net/SipSubscribeServer.h>
#include <net/SipLine.h>
#include <net/SipLineMgr.h>
#include <net/SipUserAgent.h>
#include <persist/SipPersistentSubscriptionMgr.h>
#include <ptapi/PtProvider.h>
#ifdef INCLUDE_RTCP
#include <rtcp/RTCManager.h>
#endif // INCLUDE_RTCP
#include "sipdb/CredentialDB.h"
#include "sipXecsService/SipXecsService.h"

#include "OrbitListener.h"
#include "ParkService.h"

// DEFINES
#define CONFIG_SETTING_AUTH_ID        "SIP_PARK_AUTHENTICATE_ID"
#define CONFIG_SETTING_AUTH_HA1       "SIP_PARK_AUTHENTICATE_HA1"
#define CONFIG_SETTING_UDP_PORT       "SIP_PARK_UDP_PORT"
#define CONFIG_SETTING_TCP_PORT       "SIP_PARK_TCP_PORT"
#define CONFIG_SETTING_BIND_IP        "SIP_PARK_BIND_IP"
#define CONFIG_SETTING_RTP_PORT       "SIP_PARK_RTP_PORT"
#define CONFIG_SETTING_CODECS         "SIP_PARK_CODEC_LIST"
#define CONFIG_SETTING_MAX_SESSIONS   "SIP_PARK_MAX_SESSIONS"
#define CONFIG_SETTING_LIFETIME       "SIP_PARK_LIFETIME"
#define CONFIG_SETTING_BLIND_WAIT     "SIP_PARK_BLIND_XFER_WAIT"
#define CONFIG_SETTING_ONE_BUTTON_BLF "SIP_PARK_ONE_BUTTON_BLF"
#define CONFIG_SETTING_KEEPALIVE_TIME "SIP_PARK_KEEPALIVE_TIME"

const char* PARK_SERVER_ID_TOKEN = "~~id~park"; // see sipXregistry/doc/service-tokens.txt

#define LOG_FACILITY                  FAC_ACD

#define PARK_DEFAULT_UDP_PORT         5120       // Default UDP port
#define PARK_DEFAULT_TCP_PORT         5120       // Default TCP port
#define DEFAULT_RTP_PORT              8000       // Starting RTP port
#define PARK_DEFAULT_BIND_IP          "0.0.0.0"  // Default bind ip; all interfaces

#define DEFAULT_CODEC_LIST            "pcmu pcma telephone-event"

#define DEFAULT_MAX_SESSIONS          50         // Max number of sim. conns
#define MP_SAMPLE_RATE                8000       // Sample rate (don't change)
#define MP_SAMPLES_PER_FRAME          80         // Frames per second (don't change)

#define DEFAULT_LIFETIME              86400      // Default max. lifetime for parked,
                                                 // call, 24 hours.
#define DEFAULT_BLIND_WAIT            60         // Default time to wait for a
                                                 // blind transfer to succeed or
                                                 // fail, 60 seconds.
#define DEFAULT_ONE_BUTTON_BLF        true
#define DEFAULT_KEEPALIVE_TIME        300        // Default time to send periodic
                                                 // keepalive signals, in order to
                                                 // check if the call is still connected.
#define CONS_XFER_WAIT                1          // Time to wait for a cons. transfer
                                                 // to succeed or fail, 1 second.
                                                 // This is not configurable via
                                                 // sipxpark-config, as a cons. xfer.
                                                 // should succeed or fail immediately.

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS


/* ============================ FUNCTIONS ================================= */

//
// Create the service wrapper.  It doesn't do much until run is called.
//
ParkService::ParkService(
      const char* serviceName, const char* servicePrefix, const char* version
      ) :
   SipXecsService(serviceName, servicePrefix, version)
{
}

ParkService::~ParkService()
{
}


/**
 * Description:
 * Build an SdpCodecFactory based upon the codec list specified in the config file.
 */
void initCodecs(SdpCodecFactory* codecFactory, OsConfigDb* pConfig)
{
   UtlString codecList;
   UtlString oneCodec;
   int codecStringIndex = 0;
   SdpCodec::SdpCodecTypes internalCodecId;

   if ((pConfig->get(CONFIG_SETTING_CODECS, codecList) != OS_SUCCESS) ||
       codecList.isNull())
   {
      codecList = DEFAULT_CODEC_LIST;
   }

   NameValueTokenizer::getSubField(codecList, codecStringIndex, ", \n\r\t", &oneCodec);

   while(!oneCodec.isNull())
   {
      internalCodecId = SdpCodecFactory::getCodecType(oneCodec.data());
      if (internalCodecId == SdpCodec::SDP_CODEC_UNKNOWN)
      {
         OsSysLog::add(FAC_ACD, PRI_ERR, "initCodecs: Unknown codec ID: %s",
                       oneCodec.data());
      }
      else
      {
         codecFactory->buildSdpCodecFactory(1, &internalCodecId);
      }

      codecStringIndex++;
      NameValueTokenizer::getSubField(codecList, codecStringIndex, ", \n\r\t", &oneCodec);
   }
}



//
// Pull out important parameters from the config DB.
// These parameters are set at runtime and cannot be changed without a restart.
//
UtlBoolean ParkService::loadConfig(
    int& UdpPort,
    int& TcpPort,
    int& RtpBase,
    UtlString& bindIp,
    int& MaxSessions,
    UtlBoolean& OneButtonBLF,
    UtlString&   domain,
    UtlString&   realm,
    UtlString&   user,
    SipLineMgr* lineMgr,
    int& Lifetime,
    int& BlindXferWait,
    int& KeepAliveTime
    )
{
   UtlBoolean ret = true;

   // SipXecsService owns the main configDb
   OsConfigDb& configDb = getConfigDb();

    // Read the user agent parameters from the config file.
    if (configDb.get(CONFIG_SETTING_UDP_PORT, UdpPort) != OS_SUCCESS)
        UdpPort = PARK_DEFAULT_UDP_PORT;

    if (configDb.get(CONFIG_SETTING_TCP_PORT, TcpPort) != OS_SUCCESS)
        TcpPort = PARK_DEFAULT_TCP_PORT;

    if (configDb.get(CONFIG_SETTING_RTP_PORT, RtpBase) != OS_SUCCESS)
        RtpBase = DEFAULT_RTP_PORT;

    if (configDb.get(CONFIG_SETTING_BIND_IP, bindIp) != OS_SUCCESS ||
            !OsSocket::isIp4Address(bindIp))
        bindIp = PARK_DEFAULT_BIND_IP;

    if (configDb.get(CONFIG_SETTING_MAX_SESSIONS, MaxSessions) != OS_SUCCESS)
    {
        MaxSessions = DEFAULT_MAX_SESSIONS;
    }

    OneButtonBLF =
       configDb.getBoolean(CONFIG_SETTING_ONE_BUTTON_BLF, DEFAULT_ONE_BUTTON_BLF);

    OsConfigDb  domainConfiguration;
    OsPath      domainConfigPath = SipXecsService::domainConfigPath();

    if (OS_SUCCESS == domainConfiguration.loadFromFile(domainConfigPath.data()))
    {
       domainConfiguration.get(SipXecsService::DomainDbKey::SIP_DOMAIN_NAME, domain);
       domainConfiguration.get(SipXecsService::DomainDbKey::SIP_REALM, realm);

       if (!domain.isNull() && !realm.isNull())
       {
          CredentialDB* credentialDb;
          if ((credentialDb = CredentialDB::getInstance()))
          {
             Url identity;

             identity.setUserId(PARK_SERVER_ID_TOKEN);
             identity.setHostAddress(domain);

             UtlString ha1_authenticator;
             UtlString authtype;

             if (credentialDb->getCredential(identity, realm, user, ha1_authenticator, authtype))
             {
                if ((lineMgr = new SipLineMgr()))
                {
                   SipLine line(identity // user entered url
                                ,identity // identity url
                                ,user     // user
                                ,TRUE     // visible
                                ,SipLine::LINE_STATE_PROVISIONED
                                ,TRUE     // auto enable
                                ,FALSE    // use call handling
                      );

                   if (lineMgr->addLine(line))
                   {
                      lineMgr->startLineMgr();
                      if (lineMgr->addCredentialForLine( identity, realm, user, ha1_authenticator
                                                        ,HTTP_DIGEST_AUTHENTICATION
                             )
                         )
                      {
                         OsSysLog::add(LOG_FACILITY, PRI_INFO,
                                       "Added identity '%s': user='%s' realm='%s'"
                                       ,identity.toString().data(), user.data(), realm.data()
                            );
                      }
                      else
                      {
                         OsSysLog::add(LOG_FACILITY, PRI_ERR,
                                       "Error adding identity '%s': user='%s' realm='%s'\n"
                                       "  escape and timeout from park may not work.",
                                       identity.toString().data(), user.data(), realm.data()
                            );
                      }

                      lineMgr->setDefaultOutboundLine(identity);
                   }     // end addLine
                   else
                   {
                      OsSysLog::add(LOG_FACILITY, PRI_ERR,
                                    "addLine failed: "
                                    "  escape and timeout from park may not work."
                         );
                   }
                }
                else
                {
                   OsSysLog::add(LOG_FACILITY, PRI_ERR,
                                 "Constructing SipLineMgr failed:  "
                                 "  escape and timeout from park may not work."
                      );
                }
             }  // end getCredential
             else
             {
                OsSysLog::add(LOG_FACILITY, PRI_ERR,
                              "No credential found for '%s@%s' in realm '%s'"
                              "; transfer functions will not work",
                              PARK_SERVER_ID_TOKEN, domain.data(), realm.data()
                              );
             }

             credentialDb->releaseInstance();
          }   // end credentialDB
          else
          {
             OsSysLog::add(LOG_FACILITY, PRI_ERR,
                           "Failed to open credentials database"
                           "; transfer functions will not work"
                           );
          }
       }    // end have domain and realm
       else
       {
          OsSysLog::add(LOG_FACILITY, PRI_ERR,
                        "Domain or Realm not configured:"
                        "\n  '%s' : '%s'\n  '%s' : '%s'"
                        "  transfer functions will not work.",
                        SipXecsService::DomainDbKey::SIP_DOMAIN_NAME, domain.data(),
                        SipXecsService::DomainDbKey::SIP_REALM, realm.data()
                        );
       }
    }       // end found domain config
    else
    {
       OsSysLog::add(LOG_FACILITY, PRI_ERR,
                     "main: failed to load domain configuration from '%s'",
                     domainConfigPath.data()
                     );
    }

    // Read Park Server parameters from the config file.
    if (configDb.get(CONFIG_SETTING_LIFETIME, Lifetime) != OS_SUCCESS)
    {
       Lifetime = DEFAULT_LIFETIME;
    }
    if (configDb.get(CONFIG_SETTING_BLIND_WAIT, BlindXferWait) != OS_SUCCESS)
    {
       BlindXferWait = DEFAULT_BLIND_WAIT;
    }
    if (configDb.get(CONFIG_SETTING_KEEPALIVE_TIME, KeepAliveTime) != OS_SUCCESS)
    {
       KeepAliveTime = DEFAULT_KEEPALIVE_TIME;
    }

   return ret;
}

//
// Create the real Server, passing in the configured parameters
//
void ParkService::run()
{
    int UdpPort;
    int TcpPort;
    int RtpBase;
    UtlString bindIp;
    int MaxSessions;
    UtlBoolean OneButtonBLF;
    UtlString   domain;
    UtlString   realm;
    UtlString   user;
    SipLineMgr* lineMgr = NULL;
    int Lifetime;
    int BlindXferWait;
    int KeepAliveTime;

   loadConfig(
       UdpPort,
       TcpPort,
       RtpBase,
       bindIp,
       MaxSessions,
       OneButtonBLF,
       domain,
       realm,
       user,
       lineMgr,
       Lifetime,
       BlindXferWait,
       KeepAliveTime
       );

   // initialize other pieces

    // This is not configurable, as consultative transfers should
    // succeed or fail immediately.
    int ConsXferWait;
    ConsXferWait = CONS_XFER_WAIT;

    // Bind the SIP user agent to a port and start it up
    SipUserAgent* userAgent = new SipUserAgent(TcpPort,
                                               UdpPort,
                                               TcpPort+1,
                                               NULL, // publicAddress
                                               user.isNull() ? NULL : user.data(), // default user
                                               bindIp,
                                               domain.isNull() ? NULL : domain.data(), // outbound proxy
                                               NULL, // sipDirectoryServers (deprecated)
                                               NULL, // sipRegistryServers (deprecated)
                                               NULL, // authenicateRealm
                                               NULL, // authenticateDb
                                               NULL, // authorizeUserIds (deprecated)
                                               NULL, // authorizePasswords (deprecated)
                                               lineMgr
                                               );
    userAgent->setUserAgentHeaderProperty("sipXecs/park");
    userAgent->start();

    if (!userAgent->isOk())
    {
       OsSysLog::add(LOG_FACILITY, PRI_EMERG, "SipUserAgent failed to initialize, requesting shutdown");
       setShutdownFlag(true);
    }

    // Read the list of codecs from the configuration file.
    SdpCodecFactory codecFactory;
    initCodecs(&codecFactory, &getConfigDb());

    // Initialize and start up the media subsystem
    mpStartUp(MP_SAMPLE_RATE, MP_SAMPLES_PER_FRAME, 6 * MaxSessions, &getConfigDb());
    MpMediaTask::getMediaTask(MaxSessions);

#ifdef INCLUDE_RTCP
    CRTCManager::getRTCPControl();
#endif //INCLUDE_RTCP

    mpStartTasks();

    // Instantiate the call processing subsystem
    UtlString localAddress;
    int localPort ;
    userAgent->getLocalAddress(&localAddress, &localPort) ;
    if (localAddress.isNull())
        OsSocket::getHostIp(&localAddress);

    // Construct an address to be used in outgoing requests (primarily
    // INVITEs stimulated by REFERs).
    UtlString outgoingAddress;
    {
       char buffer[100];
       sprintf(buffer, "sip:sipXpark@%s:%d", localAddress.data(),
               portIsValid(UdpPort) ? UdpPort : TcpPort);
       outgoingAddress = buffer;
    }
    CallManager callManager(FALSE,
                           NULL,
                           TRUE,                              // early media in 180 ringing
                           &codecFactory,
                           RtpBase,                           // rtp start
                           RtpBase + (2 * MaxSessions),       // rtp end
                           localAddress,
                           localAddress,
                           userAgent,
                           0,                                 // sipSessionReinviteTimer
                           NULL,                              // mgcpStackTask
                           outgoingAddress,                   // defaultCallExtension
                           Connection::RING,                  // availableBehavior
                           NULL,                              // unconditionalForwardUrl
                           -1,                                // forwardOnNoAnswerSeconds
                           NULL,                              // forwardOnNoAnswerUrl
                           Connection::BUSY,                  // busyBehavior
                           NULL,                              // sipForwardOnBusyUrl
                           NULL,                              // speedNums
                           CallManager::SIP_CALL,             // phonesetOutgoingCallProtocol
                           4,                                 // numDialPlanDigits
                           CallManager::NEAR_END_HOLD,        // holdType
                           5000,                              // offeringDelay
                           "",                                // pLocal
                           CP_MAXIMUM_RINGING_EXPIRE_SECONDS, // inviteExpiresSeconds
                           QOS_LAYER3_LOW_DELAY_IP_TOS,       // expeditedIpTos
                           MaxSessions,                       // maxCalls
                           sipXmediaFactoryFactory(NULL));    // CpMediaInterfaceFactory



    // Create a listener (application) to deal with call
    // processing events (e.g. incoming call and hang ups)
    OrbitListener listener(&callManager, Lifetime, BlindXferWait, KeepAliveTime, ConsXferWait);

    callManager.addTaoListener(&listener);
    listener.start();

    // Create the SIP Subscribe Server
    SipPersistentSubscriptionMgr
       subscriptionMgr(SUBSCRIPTION_COMPONENT_PARK,
                       domain,
                       "subscription"); // Component for holding the subscription data
    SipSubscribeServerEventHandler policyHolder; // Component for granting the subscription rights
    SipPublishContentMgr publisher; // Component for publishing the event contents

    SipSubscribeServer subscribeServer(SipSubscribeServer::terminationReasonSilent,
                                       *userAgent, publisher,
                                       subscriptionMgr, policyHolder);
    subscribeServer.enableEventType(DIALOG_EVENT_TYPE, NULL, NULL, NULL,
                                    SipSubscribeServer::standardVersionCallback,
                                    TRUE // dialogEvents only produces full content.
       );
    subscribeServer.start();

    // Create the DialogEventPublisher.
    // Use the sipX domain as the hostport of resource-IDs of the
    // published events, as that will be the request-URIs of SUBSCRIBEs.
    DialogEventPublisher dialogEvents(&callManager, &publisher, domain, PORT_NONE, OneButtonBLF);
    callManager.addTaoListener(&dialogEvents);
    dialogEvents.start();

    // Start up the call processing system
    callManager.start();

    // Loop forever until signaled to shut down

    int numTwoSecIntervals = 0;
    int CleanLoopWaitTimeSecs = 10;

    while (!getShutdownFlag())
    {
       OsTask::delay(2000);

       if (2*numTwoSecIntervals >= CleanLoopWaitTimeSecs)
       {
           numTwoSecIntervals = 0;
           if (OsSysLog::willLog(FAC_PARK, PRI_DEBUG))
           {
               OsSysLog::add(LOG_FACILITY, PRI_DEBUG,
                             "park main "
                             "logging call status"
                             );
               callManager.printCalls(0) ;
               listener.dumpCallsAndTransfers();
           }
       }
       else
       {
           numTwoSecIntervals += 1;
       }

    }

}

void ParkService::configDbChanged(UtlString& configDbFile)
{
   // call the super's configDbChanged, which loads the new DB and adjusts the log level
   SipXecsService::configDbChanged(configDbFile);
   // now load changes specific to this service
}

// Note that this method is called in the StdinListener thread, so the implementation must be threadsafe.
void ParkService::resourceChanged(UtlString& fileType, UtlString& configFile)
{
   // load changes specific to this service
   OsSysLog::add(LOG_FACILITY, PRI_INFO,
                 "ParkService::resourceChanged: %s (type %s)",
                 configFile.data(), fileType.data() );

   //TODO: make any run-time config changes
}

