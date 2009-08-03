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

#include "tao/TaoConnectionAdaptor.h"
#include "tao/TaoTransportTask.h"
#include "tao/TaoString.h"
#include "cp/CpCallManager.h"
#include "net/SipSession.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaoConnectionAdaptor::TaoConnectionAdaptor(TaoTransportTask*& rpSvrTransport,
                                                        CpCallManager *pCallMgr,
                                                        TaoMessage& rMsg,
                                                        const int maxRequestQMsgs)
        : TaoAdaptor("TaoConnAdaptor-%d", maxRequestQMsgs),
        mpCallMgrTask(pCallMgr)
{
        mpSvrTransport = rpSvrTransport;
        parseMessage(rMsg);

        if (!isStarted())
        {
                start();
        }
}

TaoConnectionAdaptor::TaoConnectionAdaptor(TaoTransportTask*& rpSvrTransport,
                                                        CpCallManager *pCallMgr,
                                                        const int maxRequestQMsgs)
        : TaoAdaptor("TaoConnAdaptor-%d", maxRequestQMsgs),
        mpCallMgrTask(pCallMgr)
{
        mpSvrTransport = rpSvrTransport;
        if (!isStarted())
        {
                start();
        }
}

TaoConnectionAdaptor::~TaoConnectionAdaptor()
{

}

UtlBoolean TaoConnectionAdaptor::handleMessage(OsMsg& rMsg)
{
        UtlBoolean handled = FALSE;

        switch (((TaoMessage&)rMsg).getCmd())
        {
        case TaoMessage::ACCEPT:
                if (TAO_SUCCESS == connectionAccept((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::DISCONNECT:
                if (TAO_SUCCESS == connectionDisconnect((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_ADDRESS:
                if (TAO_SUCCESS == connectionGetAddress((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_CALL:
                if (TAO_SUCCESS == connectionGetCall((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_SESSION_INFO:
                if (TAO_SUCCESS == connectionGetSessionInfo((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_STATE:
                if (TAO_SUCCESS == connectionGetState((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_TERM_CONNECTIONS:
                if (TAO_SUCCESS == connectionGetTermConnections((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::NUM_TERM_CONNECTIONS:
                if (TAO_SUCCESS == connectionNumTermConnections((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::PARK:
                if (TAO_SUCCESS == connectionPark((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::REDIRECT:
                if (TAO_SUCCESS == connectionRedirect((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::REJECT:
                if (TAO_SUCCESS == connectionReject((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_FROM_FIELD:
                if (TAO_SUCCESS == connectionGetFromField((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_TO_FIELD:
                if (TAO_SUCCESS == connectionGetToField((TaoMessage&)rMsg))
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

/* ----------------------------- CONNECTION ------------------------------- */
TaoStatus TaoConnectionAdaptor::connectionAccept(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString callId = arg[0];
        UtlString address = arg[1];


        mpCallMgrTask->acceptConnection(callId.data(), address.data());

        rMsg.setMsgSubType(TaoMessage::RESPONSE_CONNECTION);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;
        return TAO_FAILURE;
}

TaoStatus TaoConnectionAdaptor::connectionDisconnect(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString callId = arg[0];
        UtlString remoteAddress = arg[1];

        mpCallMgrTask->dropConnection(callId.data(), remoteAddress.data());

        rMsg.setMsgSubType(TaoMessage::RESPONSE_CONNECTION);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;
        return TAO_FAILURE;
}

TaoStatus TaoConnectionAdaptor::connectionGetAddress(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString callId = arg[0];
        UtlString address = arg[1];

        int numConnections = 0;
        mpCallMgrTask->getNumConnections(callId.data(), numConnections);

        UtlString *addresses;
        addresses = new UtlString[numConnections];

        UtlBoolean found = FALSE;
        if (addresses)
        {
                int maxConnections = numConnections;
                mpCallMgrTask->getConnections(callId.data(), maxConnections, numConnections, addresses);

                if (numConnections > maxConnections)
                        numConnections = maxConnections;

                for (int i = 0; i< numConnections; i++)
                {
                        if (!address.compareTo(addresses[i], UtlString::ignoreCase))
                        {
                                found = TRUE;
                                break;
                        }
                }

                rMsg.setMsgSubType(TaoMessage::RESPONSE_CONNECTION);
                if (!found)
                {
                        rMsg.setArgCnt(0);      // not found
                        rMsg.setArgList("");
                }

                delete[] addresses;

                if (mpSvrTransport->postMessage(rMsg))
                        return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoConnectionAdaptor::connectionGetCall(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString callId = arg[0];
        UtlString address = arg[1];

        int numConnections = 0;
        mpCallMgrTask->getNumConnections(callId.data(), numConnections);

        UtlString *addresses;
        addresses = new UtlString[numConnections];

        UtlBoolean found = FALSE;
        if (addresses)
        {
                int maxConnections = numConnections;
                mpCallMgrTask->getConnections(callId.data(), maxConnections, numConnections, addresses);

                if (numConnections > maxConnections)
                        numConnections = maxConnections;

                for (int i = 0; i< numConnections; i++)
                {
                        if (!address.compareTo(addresses[i], UtlString::ignoreCase))
                        {
                                found = TRUE;
                                break;
                        }
                }

                rMsg.setMsgSubType(TaoMessage::RESPONSE_CONNECTION);
                if (found)
                {
                        rMsg.setArgCnt(1);
                        rMsg.setArgList(callId);
                }
                else
                {
                        rMsg.setArgCnt(0);      // not found
                        rMsg.setArgList("");
                }

                delete[] addresses;

                if (mpSvrTransport->postMessage(rMsg))
                        return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoConnectionAdaptor::connectionGetFromField(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString callId = arg[0];
        UtlString address = arg[1];
        UtlString fromField;

        int ret = mpCallMgrTask->getFromField(callId.data(), address.data(), fromField);

        if (OS_SUCCESS == ret)
                ret = PT_SUCCESS;
        else
                ret = PT_NOT_FOUND;
        char buf[20];
        sprintf(buf, "%d", ret);
        callId = buf + TAOMESSAGE_DELIMITER + fromField;

        rMsg.setArgCnt(2);
        rMsg.setArgList(callId);
        rMsg.setMsgSubType(TaoMessage::RESPONSE_CONNECTION);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;
        return TAO_FAILURE;
}

TaoStatus TaoConnectionAdaptor::connectionGetToField(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString callId = arg[0];
        UtlString address = arg[1];
        UtlString toField;

        int ret = mpCallMgrTask->getToField(callId.data(), address.data(), toField);

        if (OS_SUCCESS == ret)
                ret = PT_SUCCESS;
        else
                ret = PT_NOT_FOUND;
        char buf[20];
        sprintf(buf, "%d", ret);
        callId = buf + TAOMESSAGE_DELIMITER + toField;

        rMsg.setArgCnt(2);
        rMsg.setArgList(callId);
        rMsg.setMsgSubType(TaoMessage::RESPONSE_CONNECTION);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;
        return TAO_FAILURE;
}

TaoStatus TaoConnectionAdaptor::connectionGetSessionInfo(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        SipSession session;
        UtlString callId  = arg[0];
        UtlString address = arg[1];
        UtlString sessionInfo;

        if (OS_SUCCESS == mpCallMgrTask->getSession(callId.data(),
                                                                  address.data(),
                                  session))
        {
                char buff[MAXIMUM_INTEGER_STRING_LENGTH];
                Url url;
                int     cseq;

                session.getToUrl(url);
                sessionInfo = url.toString() + TAOMESSAGE_DELIMITER;

                session.getFromUrl(url);
                sessionInfo += url.toString() + TAOMESSAGE_DELIMITER;

                session.getLocalContact(url);
                sessionInfo += url.toString() + TAOMESSAGE_DELIMITER;

                cseq = session.getNextFromCseq();
                sprintf(buff, "%d", cseq);
                sessionInfo += buff + TAOMESSAGE_DELIMITER;

                cseq = session.getLastFromCseq();
                sprintf(buff, "%d", cseq);
                sessionInfo += buff + TAOMESSAGE_DELIMITER;

                cseq = session.getLastToCseq();
                sprintf(buff, "%d", cseq);
                sessionInfo += buff + TAOMESSAGE_DELIMITER;

                cseq = session.getSessionState();
                sprintf(buff, "%d", cseq);
                sessionInfo += buff;

                argCnt = 6;
        }
        else    // timed out
        {
                argCnt = 0;
        }


        rMsg.setArgCnt(argCnt);
        rMsg.setArgList(sessionInfo);
        rMsg.setMsgSubType(TaoMessage::RESPONSE_CONNECTION);

        if (mpSvrTransport->postMessage(rMsg))
        {
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoConnectionAdaptor::connectionGetState(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        UtlString argList = rMsg.getArgList();
        TaoString arg(argList, TAOMESSAGE_DELIMITER);
        UtlString callId = arg[0];
        UtlString remoteAddress = arg[1];

        int state;
        if (mpCallMgrTask->getConnectionState(callId.data(), remoteAddress.data(), state))
        {
                char buff[MAXIMUM_INTEGER_STRING_LENGTH];

                sprintf(buff, "%d", state);
                argCnt = 1;
                argList = buff;
        }
        else
        {
                argCnt = 0;
        }

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CONNECTION,
                                                                        TaoMessage::GET_STATE,
                                                                        rMsg.getMsgID(),
                                                                        mState,
                                                                        clientSocket,
                                                                        argCnt,
                                                                        argList);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoConnectionAdaptor::connectionGetTermConnections(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        UtlString argList = rMsg.getArgList();
        TaoString arg(argList, TAOMESSAGE_DELIMITER);
        UtlString callId = arg[0];
        UtlString address = arg[1];

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

                for (int i = 0; i < numConnections; i++)
                {
                        argList += TAOMESSAGE_DELIMITER + terminalNames[i];
                        int isLocal = mpCallMgrTask->isTerminalConnectionLocal(callId.data(),
                                                                                        address.data(),
                                                                                        terminalNames[i].data());
                        char buff[128];
                        sprintf(buff, "%d", isLocal);
                        argList += TAOMESSAGE_DELIMITER + buff; // isLocal
                }

                rMsg.setMsgSubType(TaoMessage::RESPONSE_CONNECTION);
                rMsg.setArgCnt(2 * numConnections + 2);
                rMsg.setArgList(argList);

                delete[] terminalNames;

                if (mpSvrTransport->postMessage(rMsg))
                        return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoConnectionAdaptor::connectionNumTermConnections(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        UtlString argList = rMsg.getArgList();
        TaoString arg(argList, TAOMESSAGE_DELIMITER);
        UtlString callId = arg[0];
        UtlString address = arg[1];

        int numConnections = 0;
        mpCallMgrTask->getNumTerminalConnections(callId.data(),
                                                                                        address.data(),
                                                                                        numConnections);

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", numConnections);
        argList += TAOMESSAGE_DELIMITER + buff;

        rMsg.setMsgSubType(TaoMessage::RESPONSE_CONNECTION);
        rMsg.setArgCnt(3);
        rMsg.setArgList(argList);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

TaoStatus TaoConnectionAdaptor::connectionPark(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoObjHandle rTerminal = 0x00000099;    // fake

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_CONNECTION,
                                                                        TaoMessage::PARK,
                                                                        rMsg.getMsgID(),
                                                                        rTerminal,
                                                                        clientSocket,
                                                                        1,
                                                                        "9999");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoConnectionAdaptor::connectionRedirect(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 3)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString address = arg[0];
        UtlString forwardAddress = arg[1];
        UtlString callId = arg[2];

        int ret = mpCallMgrTask->redirectConnection(callId.data(), address.data(), forwardAddress.data());

        char buf[20];
        sprintf(buf, "%d", ret);

        UtlString args = address + TAOMESSAGE_DELIMITER
                                        + forwardAddress + TAOMESSAGE_DELIMITER
                                        + callId + TAOMESSAGE_DELIMITER
                                        + buf;

        rMsg.setMsgSubType(TaoMessage::RESPONSE_CONNECTION);
        rMsg.setArgCnt(4);
        rMsg.setArgList(args);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;
        return TAO_FAILURE;
}

TaoStatus TaoConnectionAdaptor::connectionReject(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString callId = arg[0];
        UtlString address = arg[1];


        mpCallMgrTask->rejectConnection(callId.data(), address.data(), SIP_BUSY_CODE, SIP_BUSY_TEXT);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_CONNECTION);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;
        return TAO_FAILURE;
}
