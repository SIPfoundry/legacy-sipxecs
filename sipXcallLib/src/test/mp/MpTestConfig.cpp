//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////


#include "test/mp/MpTestConfig.h"
#include "mp/MpMediaTask.h"
#include "ps/PsPhoneTask.h"
#include "ptapi/PtComponentGroup.h"
#include "mp/MpCallFlowGraph.h"
#include "net/SipUserAgent.h"
#include "cp/CallManager.h"
#include "net/SdpCodecFactory.h"
#include "os/OsConfigDb.h"
#include "mi/CpMediaInterfaceFactoryFactory.h"

//Default constructor (called only indirectly via getTestInstance())
MpTestConfig* MpTestConfig::spInstance = 0;

MpTestConfig::MpTestConfig()
{
}

MpTestConfig *MpTestConfig::getTestInstance()
{
    if (spInstance == NULL)
    {
        spInstance = new MpTestConfig();
        spInstance->config();
    }

    return spInstance;
}

void MpTestConfig::config(void)
{
    initializeMediaSystem();
    initializeSipUA();
    initializeCallManager();
}

MpMediaTask *MpTestConfig::getMediaTask()
{
    return mMediaTask;
}


PsPhoneTask *MpTestConfig::getPhoneTask()
{
    return mPhoneTask;
}


MpCallFlowGraph *MpTestConfig::getFlowGraph()
{
    return mFlowGraph;
}


SipUserAgent *MpTestConfig::getSipAgent()
{
    return mUA;
}


CallManager *MpTestConfig::getCallManager()
{
    return mCallManager;
}


// Initialize the media task, phone task, and a flow graph
void MpTestConfig::initializeMediaSystem()
{
   OsConfigDb configDb;

   mpStartUp(8000, 80, 64, &configDb);

   mMediaTask = MpMediaTask::getMediaTask(16); OsTask::delay(150) ;
   mPhoneTask = PsPhoneTask::getPhoneTask();   OsTask::delay(150) ;

   mpStartTasks() ;      OsTask::delay(150) ;

   mFlowGraph = new MpCallFlowGraph();

   mMediaTask->setFocus(getFlowGraph());
   mPhoneTask->activateGroup(PtComponentGroup::SOUND);

   OsTask::delay(1000) ;
}

void MpTestConfig::initializeSipUA()
{
   mUA = new SipUserAgent(5060, // TCP
                5060, // UDP
                5061, // TLS
                NULL, // public IP address (nopt used in proxy)
                NULL, // default user (not used in proxy)
                NULL, // default SIP address (not used in proxy)
                NULL, // outbound proxy
                NULL, // directory server
                NULL, // registry server
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

   mUA->start();
}

void MpTestConfig::initializeCallManager()
{
        UtlString localAddress;
        OsSocket::getHostIp(&localAddress);

        // Enable PCMU, PCMA, Tones/RFC2833
        UtlString codecList("258 257 128");
        SdpCodecFactory* pCodecFactory = new SdpCodecFactory();
        UtlString oneCodec;
        pCodecFactory->buildSdpCodecFactory(codecList);

         mCallManager = new CallManager(
            FALSE,
            NULL,
            TRUE,                              // early media in 180 ringing
            pCodecFactory,
            9000,                              // rtp start
            9999,                              // rtp end
            localAddress.data(),
            localAddress.data(),
            mUA,
            0,                                 // sipSessionReinviteTimer
            NULL,                              // mgcpStackTask
            "ivr@sip.pingtel.com",             // defaultCallExtension
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
            10,                                // maxCalls
            sipXmediaFactoryFactory(NULL));    // CpMediaInterfaceFactory

   mCallManager->start() ;
}
