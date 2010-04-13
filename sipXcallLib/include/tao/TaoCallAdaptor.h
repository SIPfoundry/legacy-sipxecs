//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoCallAdaptor_h_
#define _TaoCallAdaptor_h_

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

class TaoTransportTask;
class PtProvider;
class PtCall;
class CpCallManager;

class TaoCallAdaptor : public TaoAdaptor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
        TaoCallAdaptor(TaoTransportTask*& rpSvrTransport,
                                        CpCallManager *pCallMgr,
                                        TaoMessage& rMsg,
                                        const UtlString& name = "TaoCallAdaptor",
                                        const int maxRequestQMsgs=DEF_MAX_MSGS);

        TaoCallAdaptor(TaoTransportTask*& rpSvrTransport,
                                        CpCallManager *pCallMgr,
                                        const UtlString& name = "TaoCallAdaptor",
                                        const int maxRequestQMsgs=DEF_MAX_MSGS);
         //:Constructor

        TaoCallAdaptor(const TaoCallAdaptor& rTaoCallAdaptor);
     //:Copy constructor (not implemented for this class)

        virtual ~TaoCallAdaptor();

/* ============================ MANIPULATORS ============================== */

        virtual UtlBoolean handleMessage(OsMsg& rMsg);
         //:Handle an incoming message.
         // If the message is not one that the object is prepared to process,
         // the handleMessage() method in the derived class should return FALSE
         // which will cause the OsServerTask::handleMessage() method to be
         // invoked on the message.

//      virtual void parseMessage(TaoMessage& rMsg);
         //:Parse the incoming message.

        TaoStatus callAddCallListener(TaoMessage& rMsg);
        TaoStatus callAddParty(TaoMessage& rMsg);
        TaoStatus callConference(TaoMessage& rMsg);

        TaoStatus getConnect(TaoMessage& rMsg);
        TaoStatus callConnect(TaoMessage& rMsg);
         //:Places a telephone call from an originating endpoint to a destination
         // address URL.
         // <br>
         // The <i>pSessionDesc</i> argument points to an object containing the
         // attributes requested for the connection.
         //!param: (in) hCall - a TaoObjHandle representing the PtCall object
         //!param: (in) rhTerminal - the TaoObjHandle for the originating terminal
         //!param: (in) rhAddress - the TaoObjHandle for the originating address
         //!param: (in) destinationURL - the intended destination for the call
         //!param: (in) phSessionDesc - TaoObjHandle for the pointer to the requested attributes for the new connection or NULL to use defaults
         //!retcode: TAO_SUCCESS - success
         //!retcode: TAO_INVALID_ARGUMENT - bad <i>rTerminal</i>, <i>rhAddress</i> or <i>phSessionDesc</i> argument
         //!retcode: TAO_INVALID_PARTY - invalid <i>destinationURL</i>
         //!retcode: TAO_RESOURCE_UNAVAILABLE - insufficient resources
         //!retcode: TAO_PROVIDER_UNAVAILABLE - the provider is not available

        TaoStatus callConsult(TaoMessage& rMsg);

        TaoStatus callDrop(TaoMessage& rMsg);
        TaoStatus getDrop(TaoMessage& rMsg);
   TaoStatus callHold(TaoMessage& rMsg);
   TaoStatus callUnhold(TaoMessage& rMsg);
   TaoStatus callSetCodecCPULimit(TaoMessage& rMsg);
   TaoStatus callGetCodecCPULimit(TaoMessage& rMsg);
   TaoStatus callCodecRenegotiate(TaoMessage& rMsg);
   TaoStatus callGetCodecCPUCost(TaoMessage& rMsg);
        TaoStatus callGetCallListeners(TaoMessage& rMsg);
        TaoStatus callGetCalledAddress(TaoMessage& rMsg);
        TaoStatus callGetCallingAddress(TaoMessage& rMsg);
        TaoStatus callGetCallingTerminal(TaoMessage& rMsg);
        TaoStatus callGetConfController(TaoMessage& rMsg);
        TaoStatus callGetConnections(TaoMessage& rMsg);
        TaoStatus callGetLastRedirectedAddress(TaoMessage& rMsg);
        TaoStatus callGetState(TaoMessage& rMsg);
        TaoStatus callGetTransferController(TaoMessage& rMsg);
        TaoStatus callGetProvider(TaoMessage& rMsg);
        TaoStatus callNumCallListeners(TaoMessage& rMsg);
        TaoStatus callNumConnections(TaoMessage& rMsg);
        TaoStatus callRemoveCallListener(TaoMessage& rMsg);
        TaoStatus callSetConfController(TaoMessage& rMsg);
        TaoStatus callSetTransferController(TaoMessage& rMsg);
        TaoStatus callTransferCon(TaoMessage& rMsg);
        TaoStatus callTransferSel(TaoMessage& rMsg);

private:
        TaoTransportTask*       mpSvrTransport;

        PtProvider*             mpProvider;
        PtCall*                 mpCall;
        CpCallManager*  mpCallMgrTask;



};

#endif // _TaoCallAdaptor_h_
