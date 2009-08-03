//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

#include "tao/TaoTerminalConnectionAdaptor.h"
#include "tao/TaoTransportTask.h"
#include "tao/TaoString.h"
#include "cp/CpCallManager.h"

// TO_BE_REMOVED
#include "mp/MpStreamPlaylistPlayer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaoTerminalConnectionAdaptor::TaoTerminalConnectionAdaptor(TaoTransportTask*& rpSvrTransport,
                                                        CpCallManager *pCallMgr,
                                                        TaoMessage& rMsg,
                                                        const int maxRequestQMsgs)
        : TaoAdaptor("TaoTermConnAdaptor-%d", maxRequestQMsgs),
        mpCallMgrTask(pCallMgr)
{
        mpSvrTransport = rpSvrTransport;
        parseMessage(rMsg);

        mpObjectDb  = new TaoObjectMap();
        mpObjectCnt = new TaoReference();
        if (!isStarted())
        {
                start();
        }
}

TaoTerminalConnectionAdaptor::TaoTerminalConnectionAdaptor(TaoTransportTask*& rpSvrTransport,
                                                        CpCallManager *pCallMgr,
                                                        const int maxRequestQMsgs)
        : TaoAdaptor("TaoTermConnAdaptor-%d", maxRequestQMsgs),
        mpCallMgrTask(pCallMgr)
{
        mpSvrTransport = rpSvrTransport;
        mpObjectDb  = new TaoObjectMap();
        mpObjectCnt = new TaoReference();
        if (!isStarted())
        {
                start();
        }
}

TaoTerminalConnectionAdaptor::~TaoTerminalConnectionAdaptor()
{
        if (mpObjectDb)
        {
                delete mpObjectDb;
                mpObjectDb = 0;
        }

        if (mpObjectCnt)
        {
                delete mpObjectCnt;
                mpObjectCnt = 0;
        }
}


UtlBoolean TaoTerminalConnectionAdaptor::handleMessage(OsMsg& rMsg)
{
        UtlBoolean handled = FALSE;

        switch (((TaoMessage&)rMsg).getCmd())
        {
        case TaoMessage::ANSWER:
                if (TAO_SUCCESS == termConnectionAnswer((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_CONNECTION:
                if (TAO_SUCCESS == termConnectionGetConnection((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_STATE:
                if (TAO_SUCCESS == termConnectionGetState((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_TERMINAL:
                if (TAO_SUCCESS == termConnectionGetTerminal((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::HOLD:
                if (TAO_SUCCESS == termConnectionHold((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::UNHOLD:
                if (TAO_SUCCESS == termConnectionUnhold((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::PLAY_FILE_NAME:
                if (TAO_SUCCESS == playFileName((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::PLAY_FILE_URL:
                if (TAO_SUCCESS == playFileURL((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::START_TONE:
                if (TAO_SUCCESS == startTone((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::STOP_TONE:
                if (TAO_SUCCESS == stopTone((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::STOP_PLAY:
                if (TAO_SUCCESS == stopPlay((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;


   case TaoMessage::CREATE_PLAYER:
                if (TAO_SUCCESS == createPlayer((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::DESTROY_PLAYER:
                if (TAO_SUCCESS == destroyPlayer((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::CREATE_PLAYLIST_PLAYER:
                if (TAO_SUCCESS == createPlaylistPlayer((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::DESTROY_PLAYLIST_PLAYER:
                if (TAO_SUCCESS == destroyPlaylistPlayer((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;

        case TaoMessage::IS_LOCAL:
                if (TAO_SUCCESS == isLocal((TaoMessage&)rMsg))
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

/* ----------------------------- TERMCONNECTION --------------------------- */
TaoStatus TaoTerminalConnectionAdaptor::termConnectionAnswer(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString URL = arg[0];
        UtlString callId = arg[1];

        mpCallMgrTask->answerTerminalConnection(callId.data(), URL.data(), "");

        rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMCONNECTION);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;
        return TAO_FAILURE;
}

TaoStatus TaoTerminalConnectionAdaptor::termConnectionGetConnection(TaoMessage& rMsg)
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

                rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMCONNECTION);
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

TaoStatus TaoTerminalConnectionAdaptor::termConnectionGetState(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 3)
                return TAO_FAILURE;

        rMsg.getSocket();
        UtlString argList = rMsg.getArgList();
        TaoString arg(argList, TAOMESSAGE_DELIMITER);
        UtlString callId = arg[0];
        UtlString address = arg[1];
        UtlString terminal = arg[2];

        int state;
        if (mpCallMgrTask->getTermConnectionState(callId.data(), address.data(), terminal.data(), state))
        {
                char buff[MAXIMUM_INTEGER_STRING_LENGTH];

                sprintf(buff, "%d", state);
                argCnt = 1;
                argList = buff;
        }
        else
        {
                argCnt = 0;
                argList.remove(0);
        }


        rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMCONNECTION);
        rMsg.setArgCnt(argCnt);
        rMsg.setArgList(argList);

        if (mpSvrTransport->postMessage(rMsg))
        {
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoTerminalConnectionAdaptor::termConnectionGetTerminal(TaoMessage& rMsg)
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

                rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMCONNECTION);
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

TaoStatus TaoTerminalConnectionAdaptor::termConnectionHold(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 3)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString terminal = arg[0];
        UtlString address = arg[1];
        UtlString callId = arg[2];

        mpCallMgrTask->holdTerminalConnection(callId.data(), address.data(), terminal.data());
        osPrintf("  termConnectionHold: callId = %s, address = %s, terminal = %s", callId.data(), address.data(), terminal.data());

        rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMCONNECTION);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;
        return TAO_FAILURE;
}

TaoStatus TaoTerminalConnectionAdaptor::termConnectionUnhold(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 3)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString terminal = arg[0];
        UtlString address = arg[1];
        UtlString callId = arg[2];

        mpCallMgrTask->unholdTerminalConnection(callId.data(), address.data(), terminal.data());
        osPrintf("  termConnectionUnhold: callId = %s, address = %s, terminal = %s", callId.data(), address.data(), terminal.data());

        rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMCONNECTION);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;
        return TAO_FAILURE;
}

TaoStatus TaoTerminalConnectionAdaptor::startTone(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();
        UtlString locale;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        int toneId = atoi(arg[0]);
        int local = atoi(arg[1]);
        int remote = atoi(arg[2]);
        UtlString callId = arg[3];

        if (argCnt == 5)
        {
                locale.append(arg[4]);
        }

        mpCallMgrTask->toneStart(callId, toneId, local, remote);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMCONNECTION);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;
        return TAO_FAILURE;
}

TaoStatus TaoTerminalConnectionAdaptor::stopTone(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        mpCallMgrTask->toneStop(rMsg.getArgList());

        rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMCONNECTION);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;
        return TAO_FAILURE;
}

TaoStatus TaoTerminalConnectionAdaptor::playFileName(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 5)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString fileUrl = arg[0];
        int repeat = atoi(arg[1]);
        int local = atoi(arg[2]);
        int remote = atoi(arg[3]);
        UtlString callId = arg[4];

        mpCallMgrTask->audioPlay(callId, fileUrl, repeat, local, remote);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMCONNECTION);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;
        return TAO_FAILURE;
}

TaoStatus TaoTerminalConnectionAdaptor::playFileURL(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 5)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString audioUrl = arg[0];
        int repeat = atoi(arg[1]);
        int local = atoi(arg[2]);
        int remote = atoi(arg[3]);
        UtlString callId = arg[4];

        mpCallMgrTask->audioPlay(callId, audioUrl, repeat, local, remote);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMCONNECTION);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;
        return TAO_FAILURE;
}

TaoStatus TaoTerminalConnectionAdaptor::stopPlay(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        mpCallMgrTask->audioStop(rMsg.getArgList());

        rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMCONNECTION);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}


TaoStatus TaoTerminalConnectionAdaptor::createPlayer(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 4)
                return TAO_FAILURE;

   TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
   MpStreamPlayer** ppPlayer = (MpStreamPlayer**) atol(arg[0]) ;
   const char* szStream = arg[1] ;
   int flags = atoi(arg[2]) ;
   const char* szCallId = arg[3] ;


    // TO_BE_REMOVED
    mpCallMgrTask->createPlayer(MpPlayer::STREAM_PLAYER, szCallId, szStream, flags, ppPlayer) ;

    rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMCONNECTION);

    if (mpSvrTransport->postMessage(rMsg))
            return TAO_SUCCESS;

    return TAO_FAILURE;
}


TaoStatus TaoTerminalConnectionAdaptor::destroyPlayer(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

   TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
   MpStreamPlayer* pPlayer = (MpStreamPlayer*) atol(arg[0]) ;
   const char* szCallId = arg[1] ;

//   mpCallMgrTask->destroyPlayer(MpPlayer::STREAM_PLAYER, szCallId, pPlayer) ;

        rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMCONNECTION);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}


TaoStatus TaoTerminalConnectionAdaptor::createPlaylistPlayer(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

   TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
   MpStreamPlaylistPlayer** ppPlayer = (MpStreamPlaylistPlayer**) atol(arg[0]) ;
   const char* szCallId = arg[1] ;

    // TO_BE_REMOVED
    mpCallMgrTask->createPlayer(MpPlayer::STREAM_PLAYLIST_PLAYER,
        szCallId,
        NULL,
        0,
        (MpStreamPlayer **)ppPlayer) ;

        rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMCONNECTION);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}


TaoStatus TaoTerminalConnectionAdaptor::destroyPlaylistPlayer(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

   TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
   MpStreamPlaylistPlayer* pPlayer = (MpStreamPlaylistPlayer*) atol(arg[0]) ;
   const char* szCallId = arg[1] ;

    // TO_BE_REMOVED
    mpCallMgrTask->destroyPlayer(MpPlayer::STREAM_PLAYLIST_PLAYER, szCallId, (MpStreamPlayer *)pPlayer) ;

    rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMCONNECTION);

    if (mpSvrTransport->postMessage(rMsg))
            return TAO_SUCCESS;

    return TAO_FAILURE;
}


TaoStatus TaoTerminalConnectionAdaptor::isLocal(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 3)
                return TAO_FAILURE;

        UtlString argList = rMsg.getArgList();
        TaoString arg(argList, TAOMESSAGE_DELIMITER);
        UtlString callId = arg[0];
        UtlString address = arg[1];
        UtlString terminal = arg[2];

        UtlBoolean local = mpCallMgrTask->isTerminalConnectionLocal(callId.data(),
                                                                                                                        address.data(),
                                                                                                                        terminal.data());

        if (!local)
        {
                rMsg.setArgCnt(4);
                argList += TAOMESSAGE_DELIMITER + (UtlString) "FALSE";
                rMsg.setArgList(argList);
        }
        rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMCONNECTION);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;
        return TAO_FAILURE;
}
