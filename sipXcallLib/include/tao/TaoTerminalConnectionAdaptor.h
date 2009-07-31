//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoTerminalConnectionAdaptor_h_
#define _TaoTerminalConnectionAdaptor_h_

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

class TaoTerminalConnectionAdaptor : public TaoAdaptor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
        TaoTerminalConnectionAdaptor(TaoTransportTask*& rpSvrTransport,
                                                CpCallManager *pCallMgr,
                                           TaoMessage& rMsg,
                                           const int maxRequestQMsgs=DEF_MAX_MSGS);

        TaoTerminalConnectionAdaptor(TaoTransportTask*& rpSvrTransport,
                                                CpCallManager *pCallMgr,
                                           const int maxRequestQMsgs=DEF_MAX_MSGS);
         //:Constructor

        TaoTerminalConnectionAdaptor(const TaoTerminalConnectionAdaptor& rTaoTerminalConnectionAdaptor);
     //:Copy constructor (not implemented for this class)

        virtual ~TaoTerminalConnectionAdaptor();

/* ============================ MANIPULATORS ============================== */

        virtual UtlBoolean handleMessage(OsMsg& rMsg);
         //:Handle an incoming message.
         // If the message is not one that the object is prepared to process,
         // the handleMessage() method in the derived class should return FALSE
         // which will cause the OsServerTask::handleMessage() method to be
         // invoked on the message.

        TaoStatus termConnectionAnswer(TaoMessage& rMsg);

        TaoStatus termConnectionGetConnection(TaoMessage& rMsg);

        TaoStatus termConnectionGetState(TaoMessage& rMsg);

        TaoStatus termConnectionGetTerminal(TaoMessage& rMsg);

        TaoStatus termConnectionHold(TaoMessage& rMsg);

        TaoStatus termConnectionUnhold(TaoMessage& rMsg);

        TaoStatus startTone(TaoMessage& rMsg);

        TaoStatus stopTone(TaoMessage& rMsg);

        TaoStatus playFileName(TaoMessage& rMsg);

        TaoStatus playFileURL(TaoMessage& rMsg);

        TaoStatus stopPlay(TaoMessage& rMsg);

   TaoStatus createPlayer(TaoMessage& rMsg);

   TaoStatus destroyPlayer(TaoMessage& rMsg);

   TaoStatus createPlaylistPlayer(TaoMessage& rMsg);

   TaoStatus destroyPlaylistPlayer(TaoMessage& rMsg);

        TaoStatus isLocal(TaoMessage& rMsg);


private:
        TaoTransportTask*       mpSvrTransport;
        CpCallManager*  mpCallMgrTask;
        TaoObjectMap*   mpObjectDb;
        TaoReference*   mpObjectCnt;

        PtProvider*             mpProvider;
        PtCall                  *mpCall;


};

#endif // _TaoTerminalConnectionAdaptor_h_
