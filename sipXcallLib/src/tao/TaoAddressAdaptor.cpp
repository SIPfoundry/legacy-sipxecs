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
#endif

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

#include "tao/TaoAddressAdaptor.h"
#include "tao/TaoTransportTask.h"
#include "tao/TaoString.h"
#include "ptapi/PtAddress.h"
#include "ptapi/PtAddressForwarding.h"
#include "cp/CpCallManager.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaoAddressAdaptor::TaoAddressAdaptor(TaoTransportTask*& rpSvrTransport,
                                           CpCallManager *pCallMgr,
                                           TaoMessage& rMsg,
                                           const UtlString& phoneNumber,
                                           const UtlString& name,
                                           const int maxRequestQMsgs)
        : TaoAdaptor("TaoAddressAdaptor-%d", maxRequestQMsgs)
{
        mpCallMgr = pCallMgr;
        mpSvrTransport = rpSvrTransport;
        parseMessage(rMsg);

        mName = new char[strlen(phoneNumber) + 1];
        strcpy(mName, phoneNumber);

        if (!isStarted())
        {
                start();
        }
}

TaoAddressAdaptor::~TaoAddressAdaptor()
{
        if (mName)
        {
                delete[] mName;
                mName = 0;
        }
}

UtlBoolean TaoAddressAdaptor::handleMessage(OsMsg& rMsg)
{
        UtlBoolean handled = FALSE;

        switch (((TaoMessage&)rMsg).getCmd())
        {
        case TaoMessage::ADD_ADDRESS_LISTENER:
                if (TAO_SUCCESS == addressAddAddressListener((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::ADD_CALL_LISTENER:
                if (TAO_SUCCESS == addressAddCallListener((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::CANCEL_ALL_FORWARDING:
                if (TAO_SUCCESS == addressCancelAllForward((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::CANCEL_FORWARDING:
                if (TAO_SUCCESS == addressCancelForward((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_ADDRESS_LISTENERS:
                if (TAO_SUCCESS == addressGetAddrListeners((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_CALL_LISTENERS:
                if (TAO_SUCCESS == addressGetCallListeners((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_CONNECTIONS:
                if (TAO_SUCCESS == addressGetConnections((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_DONOT_DISTURB:
                if (TAO_SUCCESS == addressGetDoNotDisturb((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_FORWARDING:
                if (TAO_SUCCESS == addressGetForwarding((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_MESSAGE_WAITING:
                if (TAO_SUCCESS == addressGetMsgWaiting((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_NAME:
                if (TAO_SUCCESS == addressGetName((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_OFFERED_TIMEOUT:
                if (TAO_SUCCESS == addressGetOfferedTimeout((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_PROVIDER:
                if (TAO_SUCCESS == addressGetProvider((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_TERMINALS:
                if (TAO_SUCCESS == addressGetTerminals((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::NUM_ADDRESS_LISTENERS:
                if (TAO_SUCCESS == addressNumAddrListeners((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::NUM_CALL_LISTENERS:
                if (TAO_SUCCESS == addressNumCallListeners((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::NUM_CONNECTIONS:
                if (TAO_SUCCESS == addressNumConnections((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::NUM_FORWARDS:
                if (TAO_SUCCESS == addressNumForwards((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::NUM_TERMINALS:
                if (TAO_SUCCESS == addressNumTerminals((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::REMOVE_ADDRESS_LISTENER:
                if (TAO_SUCCESS == addressRemoveAddressListener((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::REMOVE_CALL_LISTENER:
                if (TAO_SUCCESS == addressRemoveCallListener((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::SET_DONOT_DISTURB:
                if (TAO_SUCCESS == addressSetDoNotDisturb((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::SET_FORWARDING:
                if (TAO_SUCCESS == addressSetForwarding((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::SET_MESSAGE_WAITING:
                if (TAO_SUCCESS == addressSetMsgWaiting((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::SET_OFFERED_TIMEOUT:
                if (TAO_SUCCESS == addressSetOfferedTimeout((TaoMessage&)rMsg))
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

/* ----------------------------- ADDRESS ---------------------------------- */
TaoStatus TaoAddressAdaptor::addressAddAddressListener(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle    clientSocket = rMsg.getSocket();
        TaoObjHandle    msgId = rMsg.getMsgID();
        TaoObjHandle    objHdl = rMsg.getTaoObjHandle();
//      UtlString               listener = rMsg.getArgList();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::ADD_ADDRESS_LISTENER,
                                                                        msgId,
                                                                        objHdl,
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

TaoStatus TaoAddressAdaptor::addressAddCallListener(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        UtlString arg = "0";
        if (mpListenerMgr)
        {
                argCnt = 0;
                mpListenerMgr->addEventListener(rMsg);
        }
        else
        {
                argCnt = 1;
                arg = "-1";     // indicating dailure
        }


        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoObjHandle msgId = rMsg.getMsgID();
        TaoObjHandle objId = rMsg.getTaoObjHandle();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
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

TaoStatus TaoAddressAdaptor::addressCancelAllForward(TaoMessage& rMsg)
{
        mpCallMgr->cancelAddressForwarding(0, 0);

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoObjHandle objId = rMsg.getTaoObjHandle();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::CANCEL_ALL_FORWARDING,
                                                                        rMsg.getMsgID(),
                                                                        objId,
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

TaoStatus TaoAddressAdaptor::addressCancelForward(TaoMessage& rMsg)
{
        TaoString        argList(rMsg.getArgList(), TAOMESSAGE_DELIMITER);

        int size = atoi(argList[0]);

        PtAddressForwarding *pAddressForwards = new PtAddressForwarding[size];

        for (int i = 0, j = 1; i < size; i++)
        {
                int type = atoi(argList[j]);
                int filterType = atoi(argList[j + 1]);
                int noAnswerTimeout = atoi(argList[j + 4]);
                pAddressForwards[i] = PtAddressForwarding(argList[j + 2],
                                                                                                        type,
                                                                                                        filterType,
                                                                                                        argList[j + 3],
                                                                                                        noAnswerTimeout);
                j += 5;
        }

        mpCallMgr->cancelAddressForwarding(size, pAddressForwards);

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoObjHandle objId = rMsg.getTaoObjHandle();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::CANCEL_FORWARDING,
                                                                        rMsg.getMsgID(),
                                                                        objId,
                                                                        clientSocket,
                                                                        0,
                                                                        "");

        delete[] pAddressForwards;
        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoAddressAdaptor::addressGetAddrListeners(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        int nItems = mpAddressListenerDb->numEntries();
        int     size   = atoi(rMsg.getArgList());
        int actual = ((size < nItems) ? size : nItems);

        TaoObjHandle    *pAddressListeners;
        pAddressListeners = new TaoObjHandle[nItems];

        mpAddressListenerDb->getActiveObjects(pAddressListeners, nItems);

        UtlString       argList;
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        for (int i = 0; i < actual; i++)
        {
                sprintf(buff, "%ld", pAddressListeners[i]);
                argList += (UtlString) buff;

                if (i < (actual - 1))
                {
                        argList += TAOMESSAGE_DELIMITER;
                }
        }

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::GET_ADDRESS_LISTENERS,
                                                                        rMsg.getMsgID(),
                                                                        nItems,
                                                                        clientSocket,
                                                                        actual,
                                                                        argList);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoAddressAdaptor::addressGetCallListeners(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        int nItems = mpCallListenerDb->numEntries();
        int     size   = atoi(rMsg.getArgList());
        int actual = ((size < nItems) ? size : nItems);

        TaoObjHandle    *pCallListeners;
        pCallListeners = new TaoObjHandle[nItems];

        mpCallListenerDb->getActiveObjects(pCallListeners, nItems);

        UtlString       argList;
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        for (int i = 0; i < actual; i++)
        {
                sprintf(buff, "%ld", pCallListeners[i]);
                argList += (UtlString) buff;

                if (i < (actual - 1))
                {
                        argList += TAOMESSAGE_DELIMITER;
                }
        }

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::GET_CALL_LISTENERS,
                                                                        rMsg.getMsgID(),
                                                                        nItems,
                                                                        clientSocket,
                                                                        actual,
                                                                        argList);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoAddressAdaptor::addressGetConnections(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        int nItems = mpConnectionDb->numEntries();
        int     size   = atoi(rMsg.getArgList());
        int actual = ((size < nItems) ? size : nItems);

        TaoObjHandle    *pConnections;
        pConnections = new TaoObjHandle[nItems];

        mpConnectionDb->getActiveObjects(pConnections, nItems);

        UtlString       argList;
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        for (int i = 0; i < actual; i++)
        {
                sprintf(buff, "%ld", pConnections[i]);
                argList += (UtlString) buff;

                if (i < (actual - 1))
                {
                        argList += TAOMESSAGE_DELIMITER;
                }
        }

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::GET_CONNECTIONS,
                                                                        rMsg.getMsgID(),
                                                                        nItems,
                                                                        clientSocket,
                                                                        actual,
                                                                        argList);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoAddressAdaptor::addressGetDoNotDisturb(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoObjHandle rTerminal = 0x00000099;    // fake

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::GET_DONOT_DISTURB,
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

TaoStatus TaoAddressAdaptor::addressGetForwarding(TaoMessage& rMsg)
{
        rMsg.setMsgSubType(TaoMessage::RESPONSE_ADDRESS);

        if (mpSvrTransport->postMessage(rMsg))
        {
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoAddressAdaptor::addressGetMsgWaiting(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoObjHandle rTerminal = 0x00000099;    // fake

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::GET_MESSAGE_WAITING,
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

TaoStatus TaoAddressAdaptor::addressGetName(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoObjHandle objId = rMsg.getTaoObjHandle();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::GET_NAME,
                                                                        rMsg.getMsgID(),
                                                                        objId,
                                                                        clientSocket,
                                                                        1,
                                                                        mName);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoAddressAdaptor::addressGetOfferedTimeout(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoObjHandle rTerminal = 0x00000099;    // fake

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::GET_OFFERED_TIMEOUT,
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

TaoStatus TaoAddressAdaptor::addressGetTerminals(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        int nItems = mpTerminalDb->numEntries();
        int     size   = atoi(rMsg.getArgList());
        int actual = ((size < nItems) ? size : nItems);

        TaoObjHandle    *pTerminals;
        pTerminals = new TaoObjHandle[nItems];

        mpTerminalDb->getActiveObjects(pTerminals, nItems);

        UtlString       argList;
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        for (int i = 0; i < actual; i++)
        {
                sprintf(buff, "%ld", pTerminals[i]);
                argList += (UtlString) buff;

                if (i < (actual - 1))
                {
                        argList += TAOMESSAGE_DELIMITER;
                }
        }

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::GET_TERMINALS,
                                                                        rMsg.getMsgID(),
                                                                        nItems,
                                                                        clientSocket,
                                                                        actual,
                                                                        argList);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoAddressAdaptor::addressGetProvider(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::GET_PROVIDER,
                                                                        rMsg.getMsgID(),
                                                                        (TaoObjHandle)mpProvider,
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

TaoStatus TaoAddressAdaptor::addressNumAddrListeners(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::NUM_ADDRESS_LISTENERS,
                                                                        rMsg.getMsgID(),
                                                                        (TaoObjHandle)mpAddressListenerDb->numEntries(),
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

TaoStatus TaoAddressAdaptor::addressNumCallListeners(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::NUM_CALL_LISTENERS,
                                                                        rMsg.getMsgID(),
                                                                        (TaoObjHandle)mpCallListenerDb->numEntries(),
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

TaoStatus TaoAddressAdaptor::addressNumConnections(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::NUM_CONNECTIONS,
                                                                        rMsg.getMsgID(),
                                                                        (TaoObjHandle)mpConnectionDb->numEntries(),
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

TaoStatus TaoAddressAdaptor::addressNumForwards(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::NUM_FORWARDS,
                                                                        rMsg.getMsgID(),
                                                                        (TaoObjHandle)mpForwardDb->numEntries(),
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

TaoStatus TaoAddressAdaptor::addressNumTerminals(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::NUM_TERMINALS,
                                                                        rMsg.getMsgID(),
                                                                        (TaoObjHandle)mpTerminalDb->numEntries(),
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

TaoStatus TaoAddressAdaptor::addressRemoveAddressListener(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoObjHandle rTerminal = 0x00000099;    // fake

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::REMOVE_ADDRESS_LISTENER,
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

TaoStatus TaoAddressAdaptor::addressRemoveCallListener(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        // UtlString    name = rMsg.getArgList();

        TaoObjHandle rTerminal = 0x00000099;    // fake

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::REMOVE_CALL_LISTENER,
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

TaoStatus TaoAddressAdaptor::addressSetDoNotDisturb(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        int flag = atoi(rMsg.getArgList());

        mpCallMgr->setDoNotDisturb(flag);

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoObjHandle objId = rMsg.getTaoObjHandle();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::SET_DONOT_DISTURB,
                                                                        rMsg.getMsgID(),
                                                                        objId,
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

TaoStatus TaoAddressAdaptor::addressSetForwarding(TaoMessage& rMsg)
{
        TaoString        argList(rMsg.getArgList(), TAOMESSAGE_DELIMITER);

        int size = atoi(argList[0]);

        PtAddressForwarding *pAddressForwards = new PtAddressForwarding[size];

        for (int i = 0, j = 1; i < size; i++)
        {
                int type = atoi(argList[j]);
                int filterType = atoi(argList[j + 1]);
                int noAnswerTimeout = atoi(argList[j + 4]);
                pAddressForwards[i] = PtAddressForwarding(argList[j + 2],
                                                                                                        type,
                                                                                                        filterType,
                                                                                                        argList[j + 3],
                                                                                                        noAnswerTimeout);
                j += 5;
        }

        mpCallMgr->setAddressForwarding(size, pAddressForwards);

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoObjHandle objId = rMsg.getTaoObjHandle();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::SET_FORWARDING,
                                                                        rMsg.getMsgID(),
                                                                        objId,
                                                                        clientSocket,
                                                                        0,
                                                                        "");

        delete[] pAddressForwards;
        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoAddressAdaptor::addressSetMsgWaiting(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoObjHandle objId = rMsg.getTaoObjHandle();

        int flag = atoi(rMsg.getArgList());

        mpCallMgr->setMessageWaiting(flag);

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::SET_MESSAGE_WAITING,
                                                                        rMsg.getMsgID(),
                                                                        objId,
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

TaoStatus TaoAddressAdaptor::addressSetOfferedTimeout(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoObjHandle objId = rMsg.getTaoObjHandle();

        int milisec = atoi(rMsg.getArgList());

        mpCallMgr->setOfferedTimeout(milisec);

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_ADDRESS,
                                                                        TaoMessage::SET_OFFERED_TIMEOUT,
                                                                        rMsg.getMsgID(),
                                                                        objId,
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
