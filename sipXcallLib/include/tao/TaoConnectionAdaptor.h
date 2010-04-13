//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoConnectionAdaptor_h_
#define _TaoConnectionAdaptor_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "tao/TaoAdaptor.h"
#include "tao/TaoObjectMap.h"
#include "tao/TaoReference.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class CpCallManager;
class TaoTransportTask;
class PtProvider;
class PtCall;

class TaoConnectionAdaptor : public TaoAdaptor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
        TaoConnectionAdaptor(TaoTransportTask*& rpSvrTransport,
                                                CpCallManager *pCallMgr,
                                           TaoMessage& rMsg,
                                           const int maxRequestQMsgs=DEF_MAX_MSGS);

        TaoConnectionAdaptor(TaoTransportTask*& rpSvrTransport,
                                                CpCallManager *pCallMgr,
                                           const int maxRequestQMsgs=DEF_MAX_MSGS);
         //:Constructor

        TaoConnectionAdaptor(const TaoConnectionAdaptor& rTaoConnectionAdaptor);
     //:Copy constructor (not implemented for this class)

        virtual ~TaoConnectionAdaptor();

/* ============================ MANIPULATORS ============================== */

        virtual UtlBoolean handleMessage(OsMsg& rMsg);
         //:Handle an incoming message.
         // If the message is not one that the object is prepared to process,
         // the handleMessage() method in the derived class should return FALSE
         // which will cause the OsServerTask::handleMessage() method to be
         // invoked on the message.

//      virtual void parseMessage(TaoMessage& rMsg);
         //:Parse the incoming message.

        TaoStatus connectionAccept(TaoMessage& rMsg);
        TaoStatus connectionDisconnect(TaoMessage& rMsg);
        TaoStatus connectionGetAddress(TaoMessage& rMsg);
        TaoStatus connectionGetCall(TaoMessage& rMsg);
        TaoStatus connectionGetSessionInfo(TaoMessage& rMsg);
        TaoStatus connectionGetState(TaoMessage& rMsg);
        TaoStatus connectionGetTermConnections(TaoMessage& rMsg);
        TaoStatus connectionNumTermConnections(TaoMessage& rMsg);
        TaoStatus connectionPark(TaoMessage& rMsg);
        TaoStatus connectionRedirect(TaoMessage& rMsg);
        TaoStatus connectionReject(TaoMessage& rMsg);
        TaoStatus connectionGetFromField(TaoMessage& rMsg);
        TaoStatus connectionGetToField(TaoMessage& rMsg);

private:
        TaoTransportTask*       mpSvrTransport;
        CpCallManager*  mpCallMgrTask;

        TaoObjectMap*   mpTerminalConnectionDb;
        TaoReference*   mpTerminalConnectionCnt;

        PtProvider*             mpProvider;
        PtCall                  *mpCall;

        char                    mState;


};

#endif //_TaoConnectionAdaptor_h_
