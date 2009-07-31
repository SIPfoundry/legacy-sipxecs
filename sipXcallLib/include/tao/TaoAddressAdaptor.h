//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoAddressAdaptor_h_
#define _TaoAddressAdaptor_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "tao/TaoAdaptor.h"
#include "tao/TaoObjectMap.h"
#include "tao/TaoReference.h"
#include "tao/TaoServerTask.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class TaoTransportTask;
class PtProvider;
class PtCall;
class CpCallManager;

class TaoAddressAdaptor : public TaoAdaptor
{
friend class TaoServerTask;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
        TaoAddressAdaptor(const TaoAddressAdaptor& rTaoAddressAdaptor);
     //:Copy constructor (not implemented for this class)

/* ============================ MANIPULATORS ============================== */

        virtual UtlBoolean handleMessage(OsMsg& rMsg);
         //:Handle an incoming message.
         // If the message is not one that the object is prepared to process,
         // the handleMessage() method in the derived class should return FALSE
         // which will cause the OsServerTask::handleMessage() method to be
         // invoked on the message.

//      virtual void parseMessage(TaoMessage& rMsg);
         //:Parse the incoming message.

        TaoStatus addressAddAddressListener(TaoMessage& rMsg);
        TaoStatus addressAddCallListener(TaoMessage& rMsg);
        TaoStatus addressCancelAllForward(TaoMessage& rMsg);
        TaoStatus addressCancelForward(TaoMessage& rMsg);
        TaoStatus addressGetAddrListeners(TaoMessage& rMsg);
        TaoStatus addressGetCallListeners(TaoMessage& rMsg);
        TaoStatus addressGetConnections(TaoMessage& rMsg);
        TaoStatus addressGetDoNotDisturb(TaoMessage& rMsg);
        TaoStatus addressGetForwarding(TaoMessage& rMsg);
        TaoStatus addressGetMsgWaiting(TaoMessage& rMsg);
        TaoStatus addressGetName(TaoMessage& rMsg);
        TaoStatus addressGetOfferedTimeout(TaoMessage& rMsg);
        TaoStatus addressGetTerminals(TaoMessage& rMsg);
        TaoStatus addressGetProvider(TaoMessage& rMsg);
        TaoStatus addressNumAddrListeners(TaoMessage& rMsg);
        TaoStatus addressNumCallListeners(TaoMessage& rMsg);
        TaoStatus addressNumConnections(TaoMessage& rMsg);
        TaoStatus addressNumForwards(TaoMessage& rMsg);
        TaoStatus addressNumTerminals(TaoMessage& rMsg);
        TaoStatus addressRemoveAddressListener(TaoMessage& rMsg);
        TaoStatus addressRemoveCallListener(TaoMessage& rMsg);
        TaoStatus addressSetDoNotDisturb(TaoMessage& rMsg);
        TaoStatus addressSetForwarding(TaoMessage& rMsg);
        TaoStatus addressSetMsgWaiting(TaoMessage& rMsg);
        TaoStatus addressSetOfferedTimeout(TaoMessage& rMsg);


private:
        TaoTransportTask*       mpSvrTransport;

        TaoObjectMap*   mpAddressListenerDb;
        TaoReference*   mpAddressListenerCnt;
        TaoObjectMap*   mpCallListenerDb;
        TaoReference*   mpCallListenerCnt;
        TaoObjectMap*   mpConnectionDb;
        TaoReference*   mpConnectionCnt;
        TaoObjectMap*   mpTerminalDb;
        TaoReference*   mpTerminalCnt;
        TaoObjectMap*   mpForwardDb;
        TaoReference*   mpForwardCnt;

        PtProvider*             mpProvider;
        PtCall                  *mpCall;
        char                    *mName;

        CpCallManager   *mpCallMgr;

        TaoAddressAdaptor(TaoTransportTask*& rpSvrTransport,
                                           CpCallManager *pCallMgr,
                                           TaoMessage& rMsg,
                                           const UtlString& phoneNumber,
                                           const UtlString& name = "TaoAddressAdaptor",
                                           const int maxRequestQMsgs=DEF_MAX_MSGS);
         //:Constructor

        virtual ~TaoAddressAdaptor();


};

#endif // _TaoAddressAdaptor_h_
