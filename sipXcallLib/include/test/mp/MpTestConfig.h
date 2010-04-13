//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _MpTestConfig_h_
#define _MpTestConfig_h_

#include "mp/MpMediaTask.h"
#include "ps/PsPhoneTask.h"
#include "mp/MpCallFlowGraph.h"
#include "net/SipUserAgent.h"
#include "cp/CallManager.h"

// Base URL to find data sources for this test program.  This software expects
// to find the followings files:
//
// http://<BASE_URL/nums/0.raw
// http://<BASE_URL/nums/0.wav
// http://<BASE_URL/nums/1.raw
// http://<BASE_URL/nums/1.wav
// http://<BASE_URL/nums/2.raw
// http://<BASE_URL/nums/2.wav
// http://<BASE_URL/nums/3.raw
// http://<BASE_URL/nums/3.wav
// http://<BASE_URL/longstream.raw
// http://<BASE_URL/longstream.wav

#define BASE_URL         "sipxchange-test:8880"
#define TESTING_ATTEMPTS  4
#define MAX_STATES        32

#define DELAY_BEWTEEN_CALLS   2100

#define RUN_PLAYER_TESTS                  FALSE

// npowork
#define RUN_PLAYLIST_TESTS                FALSE     // Not used

#define RUN_QUEUE_TESTS                   TRUE
#define RUN_CALLMGR_TESTS                 TRUE
#define RUN_PLAYER_LOOP_TESTS             FALSE
#define RUN_CALLMGR_SIMPLE_PLAYER_TESTS   TRUE


// Common object between unittests in test/mp/*.cpp
class MpTestConfig
{
public:
    static MpTestConfig *getTestInstance(void);

    MpMediaTask *getMediaTask(void);

    PsPhoneTask *getPhoneTask(void);

    MpCallFlowGraph *getFlowGraph(void);

    SipUserAgent *getSipAgent(void);

    CallManager *getCallManager(void);

protected:
    MpTestConfig(void);

private:
    MpMediaTask*     mMediaTask;

    PsPhoneTask*     mPhoneTask;

    MpCallFlowGraph* mFlowGraph;

    SipUserAgent*    mUA;

    CallManager*     mCallManager;

    static MpTestConfig* spInstance;

    void config(void);

    void initializeCallManager(void);

    void initializeSipUA(void);

    void initializeMediaSystem(void);

};

#endif // _MpTestConfig_h_
