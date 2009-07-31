//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoTerminalAdaptor_h_
#define _TaoTerminalAdaptor_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "tao/TaoAdaptor.h"
#include "tao/TaoObjectMap.h"
#include "tao/TaoReference.h"
#include "os/OsConfigDb.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class TaoTransportTask;
class TaoListenerManager;
class PtProvider;
class PtCall;
class CpCallManager;
class PsPhoneTask;

class TaoTerminalAdaptor : public TaoAdaptor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
        TaoTerminalAdaptor(CpCallManager *pCallMgr,
                                           PsPhoneTask *pPhoneTask,
                                           TaoTransportTask*& rpSvrTransport,
                                           TaoListenerManager* pListenerMgr,
                                           TaoMessage& rMsg,
                                           const UtlString& name = "TaoTerminalAdaptor-%d",
                                           const int maxRequestQMsgs=DEF_MAX_MSGS);

        TaoTerminalAdaptor(CpCallManager *pCallMgr,
                                           PsPhoneTask *pPhoneTask,
                                           TaoTransportTask*& rpSvrTransport,
                                           TaoListenerManager* pListenerMgr,
                                           const UtlString& name = "TaoTerminalAdaptor-%d",
                                           const int maxRequestQMsgs=DEF_MAX_MSGS);
         //:Constructor

        TaoTerminalAdaptor(const TaoTerminalAdaptor& rTaoTerminalAdaptor);
     //:Copy constructor (not implemented for this class)

        virtual ~TaoTerminalAdaptor();

/* ============================ MANIPULATORS ============================== */

        virtual UtlBoolean handleMessage(OsMsg& rMsg);
         //:Handle an incoming message.
         // If the message is not one that the object is prepared to process,
         // the handleMessage() method in the derived class should return FALSE
         // which will cause the OsServerTask::handleMessage() method to be
         // invoked on the message.

//      virtual void parseMessage(TaoMessage& rMsg);
         //:Parse the incoming message.

        TaoStatus terminalAddCallListener(TaoMessage& rMsg);
        TaoStatus terminalAddTermListener(TaoMessage& rMsg);
        TaoStatus terminalGetAddresses(TaoMessage& rMsg);
        TaoStatus terminalGetCallListeners(TaoMessage& rMsg);
        TaoStatus terminalGetComponent(TaoMessage& rMsg);

        TaoStatus terminalGetComponents(TaoMessage& rMsg);

        TaoStatus terminalGetComponentGroups(TaoMessage& rMsg);

        TaoStatus returnResult(TaoMessage& rMsg);

        TaoStatus terminalGetConfig(TaoMessage& rMsg);
        TaoStatus terminalGetDoNotDisturb(TaoMessage& rMsg);
        TaoStatus terminalGetName(TaoMessage& rMsg);
        TaoStatus terminalGetTermConnections(TaoMessage& rMsg);
        TaoStatus terminalGetTermListeners(TaoMessage& rMsg);
        TaoStatus terminalGetProvider(TaoMessage& rMsg);
        TaoStatus terminalNumAddresses(TaoMessage& rMsg);
        TaoStatus terminalNumCallListeners(TaoMessage& rMsg);
        TaoStatus terminalNumComponents(TaoMessage& rMsg);
        TaoStatus terminalNumTermListeners(TaoMessage& rMsg);
        TaoStatus terminalNumTermConnectionss(TaoMessage& rMsg);
        TaoStatus terminalPickup(TaoMessage& rMsg);
        TaoStatus terminalRemoveCallListener(TaoMessage& rMsg);
        TaoStatus terminalRemoveTermListener(TaoMessage& rMsg);
        TaoStatus terminalSetDoNotDisturb(TaoMessage& rMsg);
   TaoStatus terminalSetCodecCPULimit(TaoMessage& rMsg) ;

private:
        void initConfigFile(const char* configFileName);

        TaoTransportTask*       mpSvrTransport;
        TaoListenerManager* mpListenerMgr;

        TaoObjectMap*   mpObjectDb;
        TaoReference*   mpObjectCnt;

        PtProvider*             mpProvider;
        PtCall*                 mpCall;
        CpCallManager*  mpCallMgrTask; // call manager task
        PsPhoneTask*    mpPhoneTask;

    OsConfigDb          mAddresses;


};

#endif // _TaoTerminalAdaptor_h_
