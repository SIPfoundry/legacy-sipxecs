//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <os/OsDefs.h>
#include <cp/CallManager.h>
#include <os/OsSocket.h>
#include <net/SipUserAgent.h>
#include <net/SdpCodecFactory.h>
#include <cp/CpTestSupport.h>
#include <mi/CpMediaInterfaceFactoryFactory.h>


SipUserAgent *CpTestSupport::newSipUserAgent()
{
   SipUserAgent *ua = new SipUserAgent(
        SIP_PORT,       // TCP
                SIP_PORT,       // UDP
        SIP_PORT+1,     // TLS
                NULL, // public IP address (not used in proxy)
                NULL, // default user (not used in proxy)
                NULL, // default SIP address (not used in proxy)
                NULL, // outbound proxy
                NULL, // directory server
                NULL, // registry server
                NULL, // auth realm
                NULL, // auth DB
                NULL, // auth user IDs
                NULL, // auth passwords
                NULL, // line mgr
                SIP_DEFAULT_RTT, // first resend timeout
                TRUE, // default to UA transaction
                SIPUA_DEFAULT_SERVER_UDP_BUFFER_SIZE, // socket layer read buffer size
                SIPUA_DEFAULT_SERVER_OSMSG_QUEUE_SIZE // OsServerTask message queue size
                );

   return ua;
}

CallManager *CpTestSupport::newCallManager(SipUserAgent* sua)
{
        UtlString localAddress;
        OsSocket::getHostIp(&localAddress);

        // Enable PCMU, PCMA, Tones/RFC2833
        UtlString codecList("258 257 128");
        SdpCodecFactory* pCodecFactory = new SdpCodecFactory();
        UtlString oneCodec;
        pCodecFactory->buildSdpCodecFactory(codecList);

    CallManager *callManager =
       new CallManager(
          FALSE,
          NULL,
          TRUE, // early media in 180 ringing
          pCodecFactory,
          9000, //rtp start
          9999, //rtp end
          localAddress.data(),
          localAddress.data(),
          sua,
          0,//sipSessionReinviteTimer
          NULL, // mgcpStackTask
          "ivr@sip.pingtel.com", // defaultCallExtension
          Connection::RING, // availableBehavior
          NULL, // unconditionalForwardUrl
          -1, // forwardOnNoAnswerSeconds
          NULL, // forwardOnNoAnswerUrl
          Connection::BUSY, // busyBehavior
          NULL, // sipForwardOnBusyUrl
          NULL, // speedNums
          CallManager::SIP_CALL, // phonesetOutgoingCallProtocol
          4, // numDialPlanDigits
          CallManager::NEAR_END_HOLD, // holdType
          5000, // offeringDelay
          "", // pLocal
          CP_MAXIMUM_RINGING_EXPIRE_SECONDS, //inviteExpireSeconds
          QOS_LAYER3_LOW_DELAY_IP_TOS, // expeditedIpTos
          10, //maxCalls
          sipXmediaFactoryFactory(NULL)); //pMediaFactory

    return callManager;
}
