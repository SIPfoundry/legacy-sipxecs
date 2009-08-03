//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif //TEST

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

#include "tao/TaoCallAdaptor.h"
#include "tao/TaoTransportTask.h"
#include "tao/TaoString.h"
#include "ptapi/PtAddress.h"
#include "ptapi/PtCall.h"
#include "cp/CpCallManager.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaoCallAdaptor::TaoCallAdaptor(TaoTransportTask*& rpSvrTransport,
                                CpCallManager *pCallMgr,
                                TaoMessage& rMsg,
                                const UtlString& name,
                                const int maxRequestQMsgs)
        : TaoAdaptor("TaoCallAdaptor-%d", maxRequestQMsgs),
        mpCallMgrTask(pCallMgr)
{
        mpSvrTransport = rpSvrTransport;
        parseMessage(rMsg);

        if (!isStarted())
        {
                start();
        }
}

TaoCallAdaptor::TaoCallAdaptor(TaoTransportTask*& rpSvrTransport,
                                CpCallManager *pCallMgr,
                                const UtlString& name,
                                const int maxRequestQMsgs)
        : TaoAdaptor("TaoCallAdaptor-%d", maxRequestQMsgs),
        mpCallMgrTask(pCallMgr)
{
        mpSvrTransport = rpSvrTransport;
        if (!isStarted())
        {
                start();
        }
}

TaoCallAdaptor::~TaoCallAdaptor()
{

}

UtlBoolean TaoCallAdaptor::handleMessage(OsMsg& rMsg)
{
        UtlBoolean handled = FALSE;

        switch (((TaoMessage&)rMsg).getCmd())
        {
        case TaoMessage::ADD_CALL_LISTENER:
                if (TAO_SUCCESS == callAddCallListener((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::ADD_PARTY:
                if (TAO_SUCCESS == callAddParty((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::CONFERENCE:
                if (TAO_SUCCESS == callConference((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::CONNECT:
                if (TAO_SUCCESS == callConnect((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::CONSULT:
                if (TAO_SUCCESS == callConsult((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::DROP:
                if (TAO_SUCCESS == callDrop((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_CALL_LISTENERS:
                if (TAO_SUCCESS == callGetCallListeners((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_CALLED_ADDRESS:
                if (TAO_SUCCESS == callGetCalledAddress((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_CALLING_ADDRESS:
                if (TAO_SUCCESS == callGetCallingAddress((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_CALLING_TERMINAL:
                if (TAO_SUCCESS == callGetCallingTerminal((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_CONF_CONTROLLER:
                if (TAO_SUCCESS == callGetConfController((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_CONNECTIONS:
                if (TAO_SUCCESS == callGetConnections((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_LAST_REDIRECTED_ADDRESS:
                if (TAO_SUCCESS == callGetLastRedirectedAddress((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_STATE:
                if (TAO_SUCCESS == callGetState((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_TRANSFER_CONTROLLER:
                if (TAO_SUCCESS == callGetTransferController((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_PROVIDER:
                if (TAO_SUCCESS == callGetProvider((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::NUM_CALL_LISTENERS:
                if (TAO_SUCCESS == callNumCallListeners((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::NUM_CONNECTIONS:
                if (TAO_SUCCESS == callNumConnections((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::REMOVE_CALL_LISTENER:
                if (TAO_SUCCESS == callRemoveCallListener((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::SET_CONF_CONTROLLER:
                if (TAO_SUCCESS == callSetConfController((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::SET_TRANSFER_CONTROLLER:
                if (TAO_SUCCESS == callSetTransferController((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::TRANSFER_CON:
                if (TAO_SUCCESS == callTransferCon((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::TRANSFER_SEL:
                if (TAO_SUCCESS == callTransferSel((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::CALL_HOLD:
                if (TAO_SUCCESS == callHold((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::CALL_UNHOLD:
                if (TAO_SUCCESS == callUnhold((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;

        case TaoMessage::CODEC_RENEGOTIATE:
                if (TAO_SUCCESS == callCodecRenegotiate((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;

        case TaoMessage::GET_CODEC_CPU_COST:
                if (TAO_SUCCESS == callGetCodecCPUCost((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;

        case TaoMessage::GET_CODEC_CPU_LIMIT:
                if (TAO_SUCCESS == callGetCodecCPULimit((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;

        case TaoMessage::SET_CODEC_CPU_LIMIT:
                if (TAO_SUCCESS == callSetCodecCPULimit((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;

        case TaoMessage::CONNECT_RESULT:
                if (TAO_SUCCESS == getConnect((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::DROP_RESULT:
                if (TAO_SUCCESS == getDrop((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        default:
          break;
        }

        if (!handled)
        {
                TaoMessage*     pMsg = (TaoMessage*) &rMsg;

                pMsg->setArgCnt(1);
                pMsg->setArgList("-1");

                if (mpSvrTransport->postMessage(*pMsg))
                        handled = TRUE;
        }

        return handled;
}

/* ----------------------------- CALL ------------------------------------ */
TaoStatus TaoCallAdaptor::callAddCallListener(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        UtlString arg = "0";
        if (mpListenerMgr)
        {
                argCnt = 0;
                mpListenerMgr->addCallListener(rMsg);
        }
        else
        {
                argCnt = 1;
                arg = "-1";     // indicating dailure
        }


        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoObjHandle msgId = rMsg.getMsgID();
        TaoObjHandle objId = rMsg.getTaoObjHandle();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::ADD_CALL_LISTENER,
                                                                        msgId,
                                                                        objId,
                                                                        clientSocket,
                                                                        argCnt,
                                                                        arg);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoCallAdaptor::callAddParty(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 3)
                return TAO_FAILURE;

        TaoObjHandle    clientSocket = rMsg.getSocket();
        TaoString               args(rMsg.getArgList(), TAOMESSAGE_DELIMITER);

        UtlString               destinationURL;
        UtlString               callId;

        destinationURL = args[0];
        callId = args[2];

        int ret = mpCallMgrTask->connect(callId.data(), destinationURL.data());

        char buf[20];
        sprintf(buf, "%d", ret);

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::ADD_PARTY,
                                                                        rMsg.getMsgID(),
                                                                        0,
                                                                        clientSocket,
                                                                        1,
                                                                        buf);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoCallAdaptor::callConference(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoString       args(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString       otherCallId = args[0];
        UtlString       thisCallId = args[1];

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::CONFERENCE,
                                                                        rMsg.getMsgID(),
                                                                        0,
                                                                        clientSocket,
                                                                        0,
                                                                        "");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoCallAdaptor::callConnect(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 5)
                return TAO_FAILURE;

        TaoObjHandle    clientSocket = rMsg.getSocket();
        TaoObjHandle    objId = rMsg.getTaoObjHandle();
        TaoString               args(rMsg.getArgList(), TAOMESSAGE_DELIMITER);

        PtTerminal              *pTerminal;
        PtAddress               *pAddress;
        UtlString               destinationURL;
        PtSessionDesc   *pSession;
        UtlString               callId;

        pTerminal = (PtTerminal *) atol(args[0]);
        pAddress =  (PtAddress *) atol(args[1]);
        destinationURL = args[2];
        pSession = (PtSessionDesc *) atol(args[3]);
        callId = args[4];

        int ret = mpCallMgrTask->connect(callId.data(), destinationURL.data());

        char buf[20];
        sprintf(buf, "%d", ret);

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::CREATE_CALL,
                                                                        rMsg.getMsgID(),
                                                                        objId,
                                                                        clientSocket,
                                                                        1,
                                                                        buf);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoCallAdaptor::getConnect(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle  clientSocket = rMsg.getSocket();
        UtlString callId;
        TaoObjHandle connection = rMsg.getTaoObjHandle();

//      mpCallDb->insert(mpCallCnt->add(), objId);
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::CONNECT,
                                                                        rMsg.getMsgID(),
                                                                        connection,
                                                                        clientSocket,
                                                                        0,
                                                                        "");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }


        return TAO_FAILURE;
}

TaoStatus TaoCallAdaptor::callConsult(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 5)
                return TAO_FAILURE;

   UtlString argList = rMsg.getArgList();
        TaoString       args(argList, TAOMESSAGE_DELIMITER);
        UtlString consultAddressUrl;
        UtlString activeOriginalCallId;
        UtlString originalCallControllerAddress;
        UtlString originalCallControllerTerminalId;
        UtlString idleTargetCallId;

        UtlString targetCallControllerAddress;
        UtlString targetCallConsultAddress;

        originalCallControllerTerminalId = args[0];
        originalCallControllerAddress = args[1];
        activeOriginalCallId = args[2];
        idleTargetCallId = args[3];
        consultAddressUrl = args[4];

    int ret = mpCallMgrTask->consult(idleTargetCallId.data(),
                                                                        activeOriginalCallId.data(),
                                                                        originalCallControllerAddress.data(),
                                                                        originalCallControllerTerminalId.data(),
                                                                        consultAddressUrl.data(),
                                                                        targetCallControllerAddress,
                                                                        targetCallConsultAddress);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_CALL);
        argList = targetCallControllerAddress + TAOMESSAGE_DELIMITER + targetCallConsultAddress;
        char buf[20];
        sprintf(buf, "%d", ret);

        argList += TAOMESSAGE_DELIMITER + buf;
        rMsg.setMsgSubType(TaoMessage::RESPONSE_CALL);
        rMsg.setArgCnt(3);
        rMsg.setArgList(argList);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

TaoStatus TaoCallAdaptor::callDrop(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        UtlString callId = rMsg.getArgList();

        mpCallMgrTask->drop(callId);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_CALL);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

TaoStatus TaoCallAdaptor::getDrop(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle  clientSocket = rMsg.getSocket();
        UtlString callId;
        TaoObjHandle call = rMsg.getTaoObjHandle();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::DROP,
                                                                        rMsg.getMsgID(),
                                                                        call,
                                                                        clientSocket,
                                                                        0,
                                                                        "");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}


TaoStatus TaoCallAdaptor::callSetCodecCPULimit(TaoMessage& rMsg)
{
   int  argCnt = rMsg.getArgCnt();

        if (argCnt != 3)
                return TAO_FAILURE;

        TaoObjHandle  clientSocket = rMsg.getSocket();
        UtlString  argString = rMsg.getArgList();
   		TaoString args(argString, TAOMESSAGE_DELIMITER);
        TaoObjHandle call = rMsg.getTaoObjHandle();

   UtlString  callId = args[0] ;
   int       iLevel = atoi(args[1]) ;
   UtlBoolean bRenegotiate = atoi(args[2]) ;

   mpCallMgrTask->setCodecCPULimitCall(callId, iLevel, bRenegotiate) ;
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::SET_CODEC_CPU_LIMIT,
                                                                        rMsg.getMsgID(),
                                                                        call,
                                                                        clientSocket,
                                                                        0,
                                                                        "");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}


TaoStatus TaoCallAdaptor::callCodecRenegotiate(TaoMessage& rMsg)
{
   int  argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle  clientSocket = rMsg.getSocket();
        UtlString callId = rMsg.getArgList();
        TaoObjHandle call = rMsg.getTaoObjHandle();

   mpCallMgrTask->renegotiateCodecsAllTerminalConnections(callId.data()) ;

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::GET_CODEC_CPU_COST,
                                                                        rMsg.getMsgID(),
                                                                        call,
                                                                        clientSocket,
                                                                        0,
                                                                        "");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}





TaoStatus TaoCallAdaptor::callGetCodecCPUCost(TaoMessage& rMsg)
{
        int argCnt = rMsg.getArgCnt();
   int iCodecCPUCodec = 1 ;

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle  clientSocket = rMsg.getSocket();
        UtlString callId = rMsg.getArgList();
        TaoObjHandle call = rMsg.getTaoObjHandle();

   mpCallMgrTask->getCodecCPUCostCall(callId, iCodecCPUCodec) ;


   char buff[MAXIMUM_INTEGER_STRING_LENGTH];
        sprintf(buff, "%d", iCodecCPUCodec);
   UtlString arg(buff) ;

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::GET_CODEC_CPU_COST,
                                                                        rMsg.getMsgID(),
                                                                        call,
                                                                        clientSocket,
                                                                        1,
                                                                        buff);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}


TaoStatus TaoCallAdaptor::callGetCodecCPULimit(TaoMessage& rMsg)
{
        int argCnt = rMsg.getArgCnt();
   int iCodecCPULimit = 1 ;

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle  clientSocket = rMsg.getSocket();
        UtlString callId = rMsg.getArgList();
        TaoObjHandle call = rMsg.getTaoObjHandle();

   mpCallMgrTask->getCodecCPULimitCall(callId, iCodecCPULimit) ;


   char buff[MAXIMUM_INTEGER_STRING_LENGTH];
        sprintf(buff, "%d", iCodecCPULimit);
   UtlString arg(buff) ;

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::GET_CODEC_CPU_LIMIT,
                                                                        rMsg.getMsgID(),
                                                                        call,
                                                                        clientSocket,
                                                                        1,
                                                                        buff);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}


TaoStatus TaoCallAdaptor::callGetCalledAddress(TaoMessage& rMsg)
{
        // for now, return all addresses on this call

        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        UtlString callId = rMsg.getArgList();
        UtlString argList;

        int numConnections = 0;
        mpCallMgrTask->getNumConnections(callId.data(), numConnections);

        if (numConnections)
        {
                numConnections = 2*numConnections + 1;  // make sure it's big enough
                UtlString *addresses;
                addresses = new UtlString[numConnections];

                if (addresses)
                {
                        int maxConnections = numConnections;
                        mpCallMgrTask->getCalledAddresses(callId.data(),
                                                                                        maxConnections,
                                                                                        numConnections,
                                                                                        addresses);

                        if (numConnections > maxConnections)
                                numConnections = maxConnections;

                        for (int i = 0; i < numConnections; i++)
                        {
                                callId += TAOMESSAGE_DELIMITER + addresses[i];
                                argCnt++;
                        }

                        delete[] addresses;
                }
        }

        argCnt++;
        rMsg.setMsgSubType(TaoMessage::RESPONSE_CALL);
        rMsg.setArgCnt(argCnt);
        char buf[20];
        sprintf(buf, "%d", argCnt);
        argList = buf + TAOMESSAGE_DELIMITER + callId;
        rMsg.setArgList(argList);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;

}

TaoStatus TaoCallAdaptor::callGetCallingAddress(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        UtlString callId = rMsg.getArgList();
        UtlString argList;

        int numConnections = 0;
        mpCallMgrTask->getNumConnections(callId.data(), numConnections);

        if (numConnections)
        {
                numConnections = 2*numConnections + 1;  // make sure it's big enough
                UtlString *addresses;
                addresses = new UtlString[numConnections];

                if (addresses)
                {
                        int maxConnections = numConnections;
                        mpCallMgrTask->getCallingAddresses(callId.data(),
                                                                                        maxConnections,
                                                                                        numConnections,
                                                                                        addresses);

                        if (numConnections > maxConnections)
                                numConnections = maxConnections;

                        for (int i = 0; i < numConnections; i++)
                        {
                                callId += TAOMESSAGE_DELIMITER + addresses[i];
                                argCnt++;
                        }

                        delete[] addresses;
                }
        }

        argCnt++;
        rMsg.setMsgSubType(TaoMessage::RESPONSE_CALL);
        rMsg.setArgCnt(argCnt);
        char buf[20];
        sprintf(buf, "%d", argCnt);
        argList = buf + TAOMESSAGE_DELIMITER + callId;
        rMsg.setArgList(argList);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;


        return TAO_FAILURE;

}

TaoStatus TaoCallAdaptor::callGetCallingTerminal(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString address = arg[0];
        UtlString callId  = arg[1];

        int numConnections = 0;
        mpCallMgrTask->getNumTerminalConnections(callId.data(),
                                                                                        address.data(),
                                                                                        numConnections);

        UtlString *terminalNames;
        terminalNames = new UtlString[numConnections];

        if (terminalNames)
        {
                int maxConnections = numConnections;
                mpCallMgrTask->getTerminalConnections(callId.data(),
                                                                                        address.data(),
                                                                                        maxConnections,
                                                                                        numConnections,
                                                                                        terminalNames);

                if (numConnections > maxConnections)
                        numConnections = maxConnections;

                callId += TAOMESSAGE_DELIMITER + address;
                for (int i = 0; i< numConnections; i++)
                {
                        callId += TAOMESSAGE_DELIMITER + terminalNames[i];
                }

                rMsg.setMsgSubType(TaoMessage::RESPONSE_CALL);
                rMsg.setArgCnt(numConnections + 2);
                rMsg.setArgList(callId);

                delete[] terminalNames;

                if (mpSvrTransport->postMessage(rMsg))
                        return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoCallAdaptor::callGetCallListeners(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();
        UtlString arg = rMsg.getArgList();

        TaoObjHandle rTerminal = 0x00000099;    // fake
        arg = "1" + TAOMESSAGE_DELIMITER + "101";

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::GET_CALL_LISTENERS,
                                                                        rMsg.getMsgID(),
                                                                        rTerminal,
                                                                        clientSocket,
                                                                        2,
                                                                        arg);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoCallAdaptor::callGetConfController(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();
        UtlString arg = rMsg.getArgList();

        TaoObjHandle rTerminal = 0x00000099;    // fake

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::GET_CONF_CONTROLLER,
                                                                        rMsg.getMsgID(),
                                                                        rTerminal,
                                                                        clientSocket,
                                                                        1,
                                                                        "102");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoCallAdaptor::callGetConnections(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        UtlString argList = rMsg.getArgList();
        TaoString arg(argList, TAOMESSAGE_DELIMITER);
        int     numConnections   = atoi(arg[0]);
        UtlString callId = arg[1];
        UtlString *addresses;

        if (numConnections > 0 && (addresses = new UtlString[numConnections]))
        {
                int maxConnections = numConnections;
                mpCallMgrTask->getConnections(callId.data(), maxConnections, numConnections, addresses);

                if (numConnections > maxConnections)
                        numConnections = maxConnections;

                for (int i = 0; i< numConnections; i++)
                {
                        argList += TAOMESSAGE_DELIMITER + addresses[i];
                }

                rMsg.setMsgSubType(TaoMessage::RESPONSE_CALL);
                rMsg.setArgCnt(numConnections + 2);
                rMsg.setArgList(argList);

                delete[] addresses;

                if (mpSvrTransport->postMessage(rMsg))
                        return TAO_SUCCESS;
        }


        return TAO_FAILURE;
}

TaoStatus TaoCallAdaptor::callGetLastRedirectedAddress(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoObjHandle rTerminal = 0x00000099;    // fake

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::GET_LAST_REDIRECTED_ADDRESS,
                                                                        rMsg.getMsgID(),
                                                                        rTerminal,
                                                                        clientSocket,
                                                                        1,
                                                                        "9991010");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoCallAdaptor::callGetProvider(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoObjHandle rTerminal = 0x00000099;    // fake

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::GET_PROVIDER,
                                                                        rMsg.getMsgID(),
                                                                        rTerminal,
                                                                        clientSocket,
                                                                        1,
                                                                        "9");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoCallAdaptor::callGetState(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        UtlString arg = rMsg.getArgList();              // callId

        int state;
        if (mpCallMgrTask->getCallState(arg.data(), state))
        {
                char buff[MAXIMUM_INTEGER_STRING_LENGTH];

                sprintf(buff, "%d", state);
                argCnt = 1;
                arg = buff;
        }
        else
        {
                argCnt = 0;
        }


        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::GET_STATE,
                                                                        rMsg.getMsgID(),
                                                                        0,
                                                                        clientSocket,
                                                                        argCnt,
                                                                        arg);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoCallAdaptor::callGetTransferController(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoObjHandle rTerminal = 0x00000099;    // fake

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::GET_TRANSFER_CONTROLLER,
                                                                        rMsg.getMsgID(),
                                                                        rTerminal,
                                                                        clientSocket,
                                                                        1,
                                                                        "901");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoCallAdaptor::callNumCallListeners(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoObjHandle rTerminal = 0x00000099;    // fake
        UtlString arg = "1" ;
   arg.append(TAOMESSAGE_DELIMITER);
   arg.append("101");

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::NUM_CALL_LISTENERS,
                                                                        rMsg.getMsgID(),
                                                                        rTerminal,
                                                                        clientSocket,
                                                                        2,
                                                                        arg);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoCallAdaptor::callNumConnections(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        UtlString callId = rMsg.getArgList();

        int numConnections = 0;
        mpCallMgrTask->getNumConnections(callId.data(), numConnections);

   char buff[MAXIMUM_INTEGER_STRING_LENGTH];

   sprintf(buff, "%d", numConnections);
        callId.append(TAOMESSAGE_DELIMITER);
   callId.append(buff);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_CALL);
        rMsg.setArgCnt(2);
        rMsg.setArgList(callId);

        if (mpSvrTransport->postMessage(rMsg))
        {
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;

}

TaoStatus TaoCallAdaptor::callRemoveCallListener(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        UtlString arg = "0";
        if (mpListenerMgr)
        {
                argCnt = 0;
                mpListenerMgr->removeEventListener(rMsg);
        }
        else
        {
                argCnt = 1;
                arg = "-1";     // indicating dailure
        }

        TaoMessage*     pMsg = (TaoMessage*) &rMsg;

        pMsg->setMsgSubType(TaoMessage::RESPONSE_CALL);
        pMsg->setArgCnt(argCnt);
        pMsg->setArgList(arg);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoCallAdaptor::callSetConfController(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();
        UtlString arg = rMsg.getArgList();

        TaoObjHandle rTerminal = 0x00000099;    // fake

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::SET_CONF_CONTROLLER,
                                                                        rMsg.getMsgID(),
                                                                        rTerminal,
                                                                        clientSocket,
                                                                        0,
                                                                        "");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoCallAdaptor::callSetTransferController(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();
        UtlString arg = rMsg.getArgList();

        TaoObjHandle rTerminal = 0x00000099;    // fake

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CALL,
                                                                        TaoMessage::SET_TRANSFER_CONTROLLER,
                                                                        rMsg.getMsgID(),
                                                                        rTerminal,
                                                                        clientSocket,
                                                                        0,
                                                                        "");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

// Trasfer this call to another address
TaoStatus TaoCallAdaptor::callTransferCon(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 4)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString destination = arg[1];
        UtlString callId = arg[2];
        int transferType = atoi(arg[3]);

        UtlString targetConnectionCallId;
        UtlString targetConnectionAddress;

        mpCallMgrTask->setTransferType(transferType);
        int ret = mpCallMgrTask->transfer_blind(callId.data(),
                                                                        destination.data(),
                                                                        &targetConnectionCallId,
                                                                        &targetConnectionAddress);

        char buf[20];
        sprintf(buf, "%d", ret);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_CALL);
        TaoMessage*     pMsg = (TaoMessage*) &rMsg;

        callId = targetConnectionCallId + TAOMESSAGE_DELIMITER + targetConnectionAddress+
                         TAOMESSAGE_DELIMITER + buf;

        pMsg->setMsgSubType(TaoMessage::RESPONSE_CALL);
        pMsg->setArgCnt(3);
        pMsg->setArgList(callId);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

// trasfer all participants in call (with callId) to this call (with selfCallId)
TaoStatus TaoCallAdaptor::callTransferSel(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString selfCallId = arg[0];
        UtlString callId = arg[1];

        mpCallMgrTask->transfer(selfCallId.data(), callId.data());

        rMsg.setMsgSubType(TaoMessage::RESPONSE_CALL);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;
        return TAO_FAILURE;
}


// places a call on hold
TaoStatus TaoCallAdaptor::callHold(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlBoolean bBridgeParticipants = atoi(arg[1]);

   if (bBridgeParticipants)
   {
      mpCallMgrTask->holdLocalTerminalConnection(arg[0]);
   }
   else
   {
      mpCallMgrTask->holdAllTerminalConnections(arg[0]);
   }

        rMsg.setMsgSubType(TaoMessage::RESPONSE_CALL);
        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;
        return TAO_FAILURE;
}



// releases a call from hold
TaoStatus TaoCallAdaptor::callUnhold(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
   UtlBoolean bRemoteParticipants = atoi(arg[1]);

   if (bRemoteParticipants)
   {
      mpCallMgrTask->unholdAllTerminalConnections(arg[0]) ;
   }
   else
   {
      mpCallMgrTask->unholdLocalTerminalConnection(arg[0]) ;
   }

        rMsg.setMsgSubType(TaoMessage::RESPONSE_CALL);
        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;
        return TAO_FAILURE;
}
