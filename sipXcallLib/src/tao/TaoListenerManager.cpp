//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <assert.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include "tao/TaoListenerManager.h"
#include <os/OsWriteLock.h>
#include <os/OsReadLock.h>
#include <os/OsConnectionSocket.h>
#include "cp/CpCallManager.h"
#include "ps/PsPhoneTask.h"
#include "ps/PsHookswTask.h"
#include "tao/TaoEventListener.h"
#include "tao/TaoListenerEventMessage.h"
#include "tao/TaoObjectMap.h"
#include "tao/TaoTransportTask.h"
#include "tao/TaoTransportAgent.h"
#include "tao/TaoMessage.h"
#include "tao/TaoString.h"
#include "tao/TaoListenerClient.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
TaoListenerManager::TaoListenerManager(CpCallManager *pCallMgr,
                PsPhoneTask *pPhoneTask,
                TaoTransportTask*& rpSvrTransport) :
OsServerTask("TaoLstnrMgr-%d"),
mListenerRWLock(OsRWMutex::Q_FIFO)
{
        mpCallMgr = pCallMgr;
        mpPhoneTask = pPhoneTask;
        mpHookswTask = PsHookswTask::getHookswTask();
        mpSvrTransport = rpSvrTransport;
        mListenerAdded = FALSE;
        mEventClient = 0;

        mpAgents                        = new TaoObjectMap();
        mpConnectionSockets = new TaoObjectMap();

        mListenerCnt = 0;
    mMaxNumListeners = 20;

    mpListeners = (TaoListenerDb**) malloc(sizeof(TaoListenerDb *)*mMaxNumListeners);

    if (!mpListeners)
    {
        osPrintf("***** ERROR ALLOCATING LISTENERS IN TAOLISTENERMANAGER **** \n");
        return;
    }

        for (int i = 0; i < mMaxNumListeners; i++)
                mpListeners[i] = 0;

   if (!isStarted())
           start();
}

// Copy constructor
TaoListenerManager::TaoListenerManager(const TaoListenerManager& rTaoListenerManager) :
OsServerTask("TaoLstnrMgr-%d"),
mListenerRWLock(OsRWMutex::Q_FIFO)
{
        mpCallMgr = rTaoListenerManager.mpCallMgr;
        mpPhoneTask = rTaoListenerManager.mpPhoneTask;
        mpHookswTask = PsHookswTask::getHookswTask();
        mpSvrTransport = rTaoListenerManager.mpSvrTransport;
        mListenerAdded = rTaoListenerManager.mListenerAdded;
        mEventClient = rTaoListenerManager.mEventClient;

        mListenerCnt = rTaoListenerManager.mListenerCnt;
        for (int i = 0; i < mMaxNumListeners; i++)
                mpListeners[i] = rTaoListenerManager.mpListeners[i];

}

// Destructor
TaoListenerManager::~TaoListenerManager()
{
        if (mpAgents)
        {
                delete mpAgents;
                mpAgents = 0;
        }
        if (mpConnectionSockets)
        {
                int num = mpConnectionSockets->numEntries();

                if (num > 0)
                {
                        TaoObjHandle *pHandles;

                        pHandles = new TaoObjHandle[num+1];

                        if (pHandles)
                        {
                                if (mpConnectionSockets->getActiveObjects(pHandles, num))
                                {
                                        for (int i = 0; i < num; i++)
                                        {
                                                OsConnectionSocket* pSock = (OsConnectionSocket*)pHandles[i];
                                                delete pSock;
                                        }
                                }
                                delete[] pHandles;
                        }
                }
                delete mpConnectionSockets;
                mpConnectionSockets = 0;
        }

        if (mListenerCnt > 0)  // check if listener exists.
        {
                for (int i = 0; i < mListenerCnt; i++)
                {
                        if (mpListeners[i])
                        {
                                delete mpListeners[i];
                                mpListeners[i] = 0;
                        }
                }
        free(mpListeners);

        }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
TaoListenerManager&
TaoListenerManager::operator=(const TaoListenerManager& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        mpCallMgr = rhs.mpCallMgr;
        mpPhoneTask = rhs.mpPhoneTask;
        mpHookswTask = PsHookswTask::getHookswTask();
        mpSvrTransport = rhs.mpSvrTransport;
        mListenerAdded = rhs.mListenerAdded;
        mEventClient = rhs.mEventClient;

   return *this;
}

UtlBoolean TaoListenerManager::handleMessage(OsMsg& eventMessage)
{
        PtEvent::PtEventId eventId1 = (PtEvent::PtEventId)((TaoListenerEventMessage&)eventMessage).getEventId();

        OsReadLock lock(mListenerRWLock);

        if (mListenerCnt > 0)
        {
                for (int i = 0; i < mListenerCnt; i++)
                {
                        if (!mpListeners[i] || mpListeners[i]->mpListenerPtr == 0)
                                continue;
                        TaoEventListener* pListener = (TaoEventListener*)mpListeners[i]->mpListenerPtr;

                        if (pListener && pListener->isEventEnabled(eventId1))
                        {
                                UtlString rTerminalName;
                                char buf[128];

                                if (TAO_SUCCESS == pListener->getTerminalName(buf, 127))
                                {
                                        TaoObjHandle hSocket;

                                        rTerminalName.append(buf);
                                        if (TAO_NOT_FOUND == mpAgents->findValue(rTerminalName.data(), hSocket))
                                        {
                                                // OsConnectionSocket* pConnectionSocket;
                                                // pConnectionSocket = new OsConnectionSocket(DEF_TAO_EVENT_PORT, rTerminalName.data());

                                                // mpConnectionSockets->insert((const char *)rTerminalName.data(), (TaoObjHandle)pConnectionSocket);
                                                // TaoTransportAgent *pAgent = new TaoTransportAgent(pConnectionSocket, this);
                                                // mpAgents->insert((const char *)rTerminalName.data(), (TaoObjHandle)pAgent);

                                                // hSocket = (TaoObjHandle) pAgent;
                        return TRUE;
                                        }

                                        int msgType = eventMessage.getMsgType();
                                        int msgSubType = eventMessage.getMsgSubType();
                                        PtEvent::PtEventId eventId = PtEvent::EVENT_INVALID;
                                        TaoMessage*     pMsg = NULL;
                                        UtlBoolean      bValidEvent = TRUE;

                                        switch(msgType)
                                        {

                                        case OsMsg::TAO_EVENT:
                                                pMsg = new TaoMessage((TaoListenerEventMessage&) eventMessage, hSocket);
                                                break;
                                        case OsMsg::TAO_MSG:
                                                pMsg = new TaoMessage((TaoMessage&)eventMessage);
                                                pMsg->setSocket(hSocket);
                                                break;
                                        case OsMsg::TAO_LISTENER_EVENT_MSG:
                                                {
                                                        TaoListenerEventMessage *pEventMsg = (TaoListenerEventMessage*) &eventMessage;

                                                        UtlString callId;
                                                        UtlString remoteAddress;
                                                        pEventMsg->getStringData1(callId);
                                                        pEventMsg->getStringData2(remoteAddress);

                                                        UtlString arg = callId + TAOMESSAGE_DELIMITER + remoteAddress;

                                                        char buf[MAXIMUM_INTEGER_STRING_LENGTH];

                                                        sprintf(buf, "%d", msgSubType); // actual event id
                                                        arg +=  TAOMESSAGE_DELIMITER + buf;

                                                        pMsg = new TaoMessage(TaoMessage::EVENT,
                                                                                                0,
                                                                                                0,
                                                                                                0,
                                                                                                hSocket,
                                                                                                3,
                                                                                                arg);
                                                }

                                                break;
                                        case OsMsg::PS_MSG:
                                                {
                                                        PsMsg msg = (PsMsg&)eventMessage;
                                                        int hookType = msg.getParam1();
                                                        int key = msg.getParam2();
                                                        UtlString param;
                                                        msg.getStringParam1(param);

                                                        switch(msgSubType)
                                                        {
                                                        case PsMsg::EXTSPEAKER_CONNECT:
                                                                eventId = PtEvent::PHONE_EXTSPEAKER_CONNECTED;
                                                                break;
                                                        case PsMsg::EXTSPEAKER_DISCONNECT:
                                                                eventId = PtEvent::PHONE_EXTSPEAKER_DISCONNECTED;
                                                                break;
                                                        case PsMsg::BUTTON_DOWN:
                                                                eventId = PtEvent::PHONE_BUTTON_DOWN;
                                                                break;
                                                        case PsMsg::BUTTON_UP:
                                                                eventId = PtEvent::PHONE_BUTTON_UP;
                                                                break;
                                                        case PsMsg::BUTTON_SET_INFO:
                                                                eventId = PtEvent::PHONE_BUTTON_INFO_CHANGED;
                                                                break;
                                                        case PsMsg::BUTTON_REPEAT:
                                                                eventId = PtEvent::PHONE_BUTTON_REPEAT;
                                                                break;
                                                        case PsMsg::HOOKSW_STATE:
                                                        case PsMsg::HOOKSW_SET_STATE:
                                                                if(hookType == PsHookswTask::OFF_HOOK)
                                                                {
                                                                        eventId = PtEvent::PHONE_HOOKSWITCH_OFFHOOK;
                                                                }
                                                                else if( hookType == PsHookswTask::ON_HOOK)
                                                                {
                                                                        eventId = PtEvent::PHONE_HOOKSWITCH_ONHOOK;
                                                                }
                                                                break;
                                                        case PsMsg::HANDSET_SET_VOLUME:
                                                                eventId = PtEvent::PHONE_HANDSET_VOLUME_CHANGED;
                                                                break;
                                                        case PsMsg::RINGER_SET_VOLUME:
                                                                eventId = PtEvent::PHONE_RINGER_VOLUME_CHANGED;
                                                                break;
                                                        case PsMsg::SPEAKER_SET_VOLUME:
                                                                eventId = PtEvent::PHONE_SPEAKER_VOLUME_CHANGED;
                                                                break;
                                                        default:
                                                                bValidEvent = FALSE;
                                                                break;
                                                        }

                     if (bValidEvent)
                                                        {
                                                            UtlString arg;
                                                            char buf[MAXIMUM_INTEGER_STRING_LENGTH];

                                                            sprintf(buf, "%d", hookType);
                                                            arg = buf + TAOMESSAGE_DELIMITER;

                                                            sprintf(buf, "%d", key);
                                                            arg += buf + TAOMESSAGE_DELIMITER;

                                                            arg += rTerminalName + TAOMESSAGE_DELIMITER + param;

                                                            pMsg = new TaoMessage(TaoMessage::EVENT,
                                                                                                    0,
                                                                                                    0,
                                                                                                    eventId,
                                                                                                    hSocket,
                                                                                                    4,
                                                                                            arg);
                     }
                                                }
                                                break;
                                        default:
                                                bValidEvent = FALSE;
                                                break;
                                        }

                                        if (bValidEvent)
                                        {
                                                pMsg->setMsgQueueHandle(mEventClient);
                                                if (mEventClient)
                                                        ((TaoListenerClientTask*) mEventClient)->postMessage((TaoMessage&)(*pMsg));
                                                else
                                                {
                                                        assert(FALSE);
                                                        mpSvrTransport->postMessage((TaoMessage&)(*pMsg));
                                                }
                                                delete pMsg;
                                        }
                                }
                                rTerminalName.remove(0);
                        }
                }
        }

        return(TRUE);
}


TaoStatus TaoListenerManager::addEventListener(const char* terminalName,
                                                                   UtlBoolean callListener)
{
        if (terminalName)
        {
                OsWriteLock lock(mListenerRWLock);
                if (mListenerCnt > 0)  // check if listener is already added.
                {
                        for (int i = 0; i < mListenerCnt; i++)
                        {
                                if (mpListeners[i] && mpListeners[i]->mName.compareTo(terminalName) == 0)
                                {
                                        mpListeners[i]->mRef++;
                                        return TAO_SUCCESS;
                                }
                        }
                }
                //create local event listener with proper terminal name
                TaoEventListener* pListener = new TaoEventListener(terminalName);
                if (pListener)
                {
                        TaoObjHandle hSocket;
                        if (TAO_NOT_FOUND == mpAgents->findValue(terminalName, hSocket))
                        {
                                OsConnectionSocket* pConnectionSocket;
                                pConnectionSocket = new OsConnectionSocket(DEF_TAO_EVENT_PORT, terminalName);

                                mpConnectionSockets->insert(terminalName, (TaoObjHandle)pConnectionSocket);
                                TaoTransportAgent *pAgent = new TaoTransportAgent(pConnectionSocket, this);
                                mpAgents->insert(terminalName, (TaoObjHandle)pAgent);

                return TAO_SUCCESS;
                        }

                        // add to listenerDb
                        TaoListenerDb *pListenerDb = new TaoListenerDb();
                        pListenerDb->mName = terminalName;
                        pListenerDb->mpListenerPtr = pListener;
                        pListenerDb->mRef = 1;
            if (mListenerCnt == mMaxNumListeners)
            {
                osPrintf("***** INCREASING LISTENER COUNT in TaoListenerManager!\n");
                //make more of em.
                mMaxNumListeners += 20;
                mpListeners = (TaoListenerDb **)realloc(mpListeners,sizeof(TaoListenerDb *)*mMaxNumListeners);
                for (int loop = mListenerCnt;loop < mMaxNumListeners;loop++)
                    mpListeners[loop] = 0 ;
            }

                        mpListeners[mListenerCnt++] = pListenerDb;

                        if (!mListenerAdded && !callListener)
                        {
                                mpPhoneTask->addListener(this);
                                mListenerAdded = true;
                        }

                        return(TAO_SUCCESS);
                }
        }

        return(TAO_FAILURE);
}

TaoStatus TaoListenerManager::addEventListener(TaoMessage& rMsg)
{
        UtlString terminalName;

        TaoString str(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        terminalName = str[0];

        if (terminalName.isNull() || 0 == terminalName.compareTo("0.0.0.0"))
        {
                if (mListenerCnt <= 0)
                {
                        terminalName = "127.0.0.1";
                        osPrintf("WARNING - TaoListenerManager::addEventListener: using invalid host, replaced with %s\n", terminalName.data());
                }
                else
                {
                        osPrintf("WARNING - TaoListenerManager::addEventListener: using invalid host %s, listener not added.\n", terminalName.data());
                        return TAO_FAILURE;
                }
        }

        return addEventListener(terminalName.data(), FALSE);
}


TaoStatus TaoListenerManager::addCallListener(TaoMessage& rMsg)
{
        TaoStatus ret = TAO_SUCCESS;
        UtlString terminalName;

        TaoString str(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        terminalName = str[0];

        if (terminalName.isNull() || 0 == terminalName.compareTo("0.0.0.0"))
        {
                if (mListenerCnt <= 0)
                {
                        terminalName = "127.0.0.1";
                        osPrintf("WARNING - TaoListenerManager::addCallListener: using invalid host, replaced with %s\n", terminalName.data());
                }
                else
                {
                        osPrintf("WARNING - TaoListenerManager::addCallListener: using invalid host %s, listener not added.\n", terminalName.data());
                        return TAO_FAILURE;
                }
        }

        mpCallMgr->addTaoListener(this);

        ret = addEventListener(terminalName.data(), TRUE);

        return ret;
}

TaoStatus TaoListenerManager::removeEventListener(TaoMessage& rMsg)
{

        UtlString terminalName;
        TaoString str(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        terminalName = str[0];

        if (terminalName.isNull() || 0 == terminalName.compareTo("0.0.0.0"))
                terminalName = "127.0.0.1";

//    OsWriteLock lock(mListenerRWLock);

        if (mListenerCnt > 0)  // check if listener exists.
        {
                for (int i = 0; i < mListenerCnt; i++)
                {
                        if (mpListeners[i] && mpListeners[i]->mName.compareTo(terminalName) == 0)
                        {
                                mpListeners[i]->mRef--;
                                if (mpListeners[i]->mRef <= 0)
                                {
                                        osPrintf("*** TaoListenerManager::removeEventListener %s 0x%p %d\n", terminalName.data(), mpListeners[i], mpListeners[i]->mRef);
                                        if (mpListeners[i]->mpListenerPtr)
                                        {
                                                TaoEventListener* pListener = (TaoEventListener*) mpListeners[i]->mpListenerPtr;
                                                delete pListener;
                                        }
                                        delete mpListeners[i];
                                        mpListeners[i] = 0;
                                        mListenerCnt--;

                                        // find and delete the TaoTransportAgent
                                        TaoObjHandle object;
                                        if (TAO_SUCCESS == mpAgents->findValue(terminalName.data(), object))
                                        {
                                                if (object)
                                                {
                                                        osPrintf("TaoListenerManager removeEventListener TaoTransportAgent = %d\n", (int)object);
                                                        TaoTransportAgent *pAgent = (TaoTransportAgent *)object;
                                                        delete pAgent;
                                                }
                                        }
                                        else
                                                osPrintf("TaoListenerManager removeEventListener Failure! did not find socket %s\n", terminalName.data());

                                        if (TAO_SUCCESS == mpAgents->remove(terminalName.data()))
                                        {
                                                osPrintf(" **** TaoListenerManager removeEventListener socket removed %s ****\n", terminalName.data());
                                        }
                                        else
                                                osPrintf("TaoListenerManager removeEventListener Failure! did not remove socket %s\n", terminalName.data());
                                }
                        }
                }
        }

        return TAO_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
