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
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#include <os/OsQueuedEvent.h>
#include <os/OsEventMsg.h>
#include "os/OsSysLog.h"
#include <os/OsDateTime.h>
#include <os/OsTimer.h>
#include <cp/CpCall.h>
#include <mi/CpMediaInterface.h>
#include <cp/CpMultiStringMessage.h>
#include <cp/CpIntMessage.h>
#include "ptapi/PtConnection.h"
#include "ptapi/PtCall.h"
#include "ptapi/PtTerminalConnection.h"
#include "tao/TaoProviderAdaptor.h"
#include "tao/TaoListenerEventMessage.h"

//#define TEST_PRINT 1

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define CALL_STACK_SIZE (24*1024)    // 24K stack for the call task
#       define LOCAL_ONLY 0
#       define LOCAL_AND_REMOTE 1
#define UI_TERMINAL_CONNECTION_STATE "TerminalConnectionState"
#define UI_CONNECTION_STATE "ConnectionState"

//#define debugNew(x)   osPrintf("new 0x%08x: %s/%d\n", (int)x, __FILE__, __LINE__);
//#define debugDelete(x)        osPrintf("delete 0x%08x: %s/%d\n", (int)x, __FILE__, __LINE__);

// STATIC VARIABLE INITIALIZATIONS
OsLockingList *CpCall::spCallTrackingList = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpCall::CpCall(CpCallManager* manager,
               CpMediaInterface* callMediaInterface,
               int callIndex,
               const char* callId,
               int holdType) :
OsServerTask("Call-%d", NULL, DEF_MAX_MSGS, DEF_PRIO, DEF_OPTIONS, CALL_STACK_SIZE),
mCallIdMutex(OsMutex::Q_FIFO),
mDtmfQMutex(OsMutex::Q_FIFO)
{
    // add the call task name to a list so we can track leaked calls.
    UtlString strCallTaskName = getName();
    addToCallTrackingList(strCallTaskName);

    mCallInFocus = FALSE;
    mRemoteDtmf = FALSE;
    mDtmfEnabled = FALSE;
    mCallMgrPostedDrop = FALSE;

    mpManager = manager;

    mDropping = FALSE;
    mLocalHeld = FALSE;

    mCallIndex = callIndex;
    if(callId && callId[0])
    {
        setCallId(callId);
    }
    mHoldType = holdType;
    if(mHoldType < CallManager::NEAR_END_HOLD ||
        mHoldType > CallManager::FAR_END_HOLD)
    {
        mHoldType = CallManager::NEAR_END_HOLD;
    }

    mDtmfQLen = 0;
    mListenerCnt = 0;
    mToneListenerCnt = 0;
    mMaxNumListeners = 20;
    mpListeners = (TaoListenerDb**) malloc(sizeof(TaoListenerDb *)*mMaxNumListeners);

    if (!mpListeners)
    {
        osPrintf("***** ERROR ALLOCATING LISTENERS IN CPCALL **** \n");
        return;
    }

    int i;

    for ( i = 0; i < mMaxNumListeners; i++)
        mpListeners[i] = 0;


    for (i = 0; i < MAX_NUM_TONE_LISTENERS; i++)
        mpToneListeners[i] = 0;

    // Create the media processing channel
    mpMediaInterface = callMediaInterface;

    mCallState = PtCall::IDLE;
    mLocalConnectionState = PtEvent::CONNECTION_IDLE;
    mLocalTermConnectionState = PtTerminalConnection::IDLE;

    // Meta event intitialization
    mMetaEventId = 0;
    mMetaEventType = PtEvent::META_EVENT_NONE;
    mNumMetaEventCalls = 0;
    mpMetaEventCallIds = NULL;
    mMessageEventCount = -1;

    UtlString name = getName();
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "%s Call constructed: %s",
                  name.data(), mCallId.data());
#endif
}

// Destructor
CpCall::~CpCall()
{
    if (isStarted())
    {
        waitUntilShutDown();
    }
    // remove the call task name from the list (for tracking leaked calls)
    UtlString strCallTaskName = getName();
    removeFromCallTrackingList(strCallTaskName);

    if(mpMediaInterface)
    {
        mpMediaInterface->release();
        mpMediaInterface = NULL;
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

    }
    if (mpListeners)
    {
        free(mpListeners);
        mpListeners = NULL;
    }

    if (mToneListenerCnt > 0)  // check if listener exists.
    {
        for (int i = 0; i < mToneListenerCnt; i++)
        {
            if (mpToneListeners[i])
            {
                OsQueuedEvent *pEv = (OsQueuedEvent *) mpToneListeners[i]->mIntData;
                if (pEv)
                    delete pEv;
                delete mpToneListeners[i];
                mpToneListeners[i] = 0;
            }
        }
    }

    if(mpMetaEventCallIds)
    {
        //for(int i = 0; i < mNumMetaEventCalls; i++)
        //{
        //    if(mpMetaEventCallIds[i]) delete mpMetaEventCallIds[i];
        //    mpMetaEventCallIds[1] = NULL;
        //}
        delete[] mpMetaEventCallIds;
        mpMetaEventCallIds = NULL;
    }

#ifdef TEST_PRINT
    UtlString name = getName();
    OsSysLog::add(FAC_CP, PRI_DEBUG, "%s destructed: %s\n", name.data(), mCallId.data());
    osPrintf("%s destructed: %s\n", name.data(), mCallId.data());
    name.remove(0);
#endif
    mCallId.remove(0);
    mIdOfOrigCall.remove(0);
    mTargetCallId.remove(0);
}

/* ============================ MANIPULATORS ============================== */

void CpCall::setDropState(UtlBoolean state)
{
    mDropping = state;
}

void CpCall::setCallState(int responseCode, UtlString responseText, int state, int cause)
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpCall::setCallState "
                  "mState %d, newState %d, event %d, cause %d",
                  mCallState, state,
                  (state == PtCall::INVALID ? PtEvent::CALL_INVALID : PtEvent::CALL_ACTIVE),
                  cause);
#endif
    if (state != mCallState)
    {
        switch(state)
        {
        case PtCall::INVALID:
            postTaoListenerMessage(responseCode, responseText, PtEvent::CALL_INVALID, CALL_STATE, cause);
            break;

        case PtCall::ACTIVE:
            postTaoListenerMessage(responseCode, responseText, PtEvent::CALL_ACTIVE, CALL_STATE, cause);
            break;

        default:
            break;
        }
    }

    mCallState = state;
}

UtlBoolean CpCall::handleMessage(OsMsg& eventMessage)
{
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();
    //int key;
    //int hookState;
    CpMultiStringMessage* multiStringMessage = (CpMultiStringMessage*)&eventMessage;

    UtlBoolean processedMessage = TRUE;
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpCall::handleMessage "
                  "message type: %d subtype %d\n",
                  msgType, msgSubType);

    switch(msgType)
    {

    case OsMsg::PHONE_APP:

        switch(msgSubType)
        {
            // If these cases need to be overrided,  they should
            // be broken out into virutal methods
        case CallManager::CP_START_TONE_TERM_CONNECTION:
            addHistoryEvent(msgSubType, multiStringMessage);
            {
                int toneId = ((CpMultiStringMessage&)eventMessage).getInt1Data();
                UtlBoolean local = ((CpMultiStringMessage&)eventMessage).getInt2Data();
                UtlBoolean remote = ((CpMultiStringMessage&)eventMessage).getInt3Data();

                if(mpMediaInterface)
                {
                    mpMediaInterface->startTone(toneId,
                    local, remote);
                }
            }
            break;

        case CallManager::CP_STOP_TONE_TERM_CONNECTION:
            addHistoryEvent(msgSubType, multiStringMessage);
            if(mpMediaInterface)
            {
                mpMediaInterface->stopTone();
            }
            break;

        case CallManager::CP_PLAY_AUDIO_TERM_CONNECTION:
            addHistoryEvent(msgSubType, multiStringMessage);
            {
                int repeat = ((CpMultiStringMessage&)eventMessage).getInt1Data();
                UtlBoolean local = ((CpMultiStringMessage&)eventMessage).getInt2Data();
                UtlBoolean remote = ((CpMultiStringMessage&)eventMessage).getInt3Data();
                UtlString url;
                ((CpMultiStringMessage&)eventMessage).getString2Data(url);

                if(mpMediaInterface)
                {
                    mpMediaInterface->playAudio(url.data(), repeat,
                        local, remote);
                }
                url.remove(0);
            }
            break;

        case CallManager::CP_PLAY_BUFFER_TERM_CONNECTION:
            addHistoryEvent(msgSubType, multiStringMessage);
            {
                int repeat = ((CpMultiStringMessage&)eventMessage).getInt2Data();
                UtlBoolean local = ((CpMultiStringMessage&)eventMessage).getInt3Data();
                UtlBoolean remote = ((CpMultiStringMessage&)eventMessage).getInt4Data();
                intptr_t buffer = ((CpMultiStringMessage&)eventMessage).getInt5Data();
                int bufSize = ((CpMultiStringMessage&)eventMessage).getInt6Data();
                int type = ((CpMultiStringMessage&)eventMessage).getInt7Data();
                OsProtectedEvent* ev = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();

                if(mpMediaInterface)
                {
                    mpMediaInterface->playBuffer((char*)buffer,
                    bufSize, type, repeat, local, remote, ev);
                }
            }
            break;

        case CallManager::CP_STOP_AUDIO_TERM_CONNECTION:
            addHistoryEvent(msgSubType, multiStringMessage);
            if(mpMediaInterface)
            {
                mpMediaInterface->stopAudio();
            }
            break;

        case CallManager::CP_CREATE_PLAYLIST_PLAYER:
            {
                UtlString callId;

                MpStreamPlaylistPlayer** ppPlayer = (MpStreamPlaylistPlayer **) ((CpMultiStringMessage&)eventMessage).getInt2Data();
                assert(ppPlayer != NULL);

                OsProtectedEvent* ev = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();
#ifdef TEST_PRINT
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                    "CpCall::handle creating MpStreamPlaylistPlayer ppPlayer 0x%08x ev 0x%08x",
                    (int)ppPlayer, (int)ev);
#endif

                addHistoryEvent(msgSubType, multiStringMessage);

                getCallId(callId);

                if (mpMediaInterface)
                {
                    mpMediaInterface->createPlaylistPlayer(ppPlayer, mpManager->getMessageQueue(), callId.data()) ;
                }

                if(OS_ALREADY_SIGNALED == ev->signal(0))
                {
                    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
                    eventMgr->release(ev);
                }
            }
            break;

        case CallManager::CP_CREATE_PLAYER:
            {
                UtlString callId;
                UtlString streamId ;

                MpStreamPlayer** ppPlayer = (MpStreamPlayer **) ((CpMultiStringMessage&)eventMessage).getInt2Data();

                assert(ppPlayer != NULL);

                OsProtectedEvent* ev = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();
                int flags = ((CpMultiStringMessage&)eventMessage).getInt3Data();
#ifdef TEST_PRINT
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                    "CpCall::handle creating MpStreamPlayer ppPlayer 0x%08x ev 0x%08x flags %d",
                    (int)ppPlayer, (int)ev, flags);
#endif

                addHistoryEvent(msgSubType, multiStringMessage);

                ((CpMultiStringMessage&)eventMessage).getString2Data(streamId);
                getCallId(callId);

                if (mpMediaInterface)
                {
                    mpMediaInterface->createPlayer((MpStreamPlayer**)ppPlayer, streamId, flags, mpManager->getMessageQueue(), callId.data()) ;
                }
                if(OS_ALREADY_SIGNALED == ev->signal(0))
                {
                    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
                    eventMgr->release(ev);
                }
            }
            break;

        case CallManager::CP_CREATE_QUEUE_PLAYER:
            {
                UtlString callId;

                MpStreamPlayer** ppPlayer = (MpStreamPlayer **) ((CpMultiStringMessage&)eventMessage).getInt2Data();

                assert(ppPlayer != NULL);

                OsProtectedEvent* ev = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();
#ifdef TEST_PRINT
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                    "CpCall::handle creating MpStreamQueuePlayer ppPlayer 0x%08x ev 0x%08x",
                    (int)ppPlayer, (int)ev);
#endif

                addHistoryEvent(msgSubType, multiStringMessage);

                getCallId(callId);

                if (mpMediaInterface)
                {
                    mpMediaInterface->createQueuePlayer((MpStreamQueuePlayer**)ppPlayer, mpManager->getMessageQueue(), callId.data()) ;
                }

                if(OS_ALREADY_SIGNALED == ev->signal(0))
                {
                    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
                    eventMgr->release(ev);
                }
            }
            break;

        case CallManager::CP_DESTROY_PLAYLIST_PLAYER:
            {
                MpStreamPlaylistPlayer* pPlayer ;

                addHistoryEvent(msgSubType, multiStringMessage);

                // Redispatch Request to flowgraph
                if(mpMediaInterface)
                {
                    pPlayer = (MpStreamPlaylistPlayer*) ((CpMultiStringMessage&)eventMessage).getInt2Data();
                    // Note this only deletes the media structures for the
                    // player.  *pPlayer is not deleted, and should be
                    // deleted by our caller.
                    mpMediaInterface->destroyPlaylistPlayer(pPlayer) ;
                }

                // Signal Event so that the caller knows the work is done
                OsProtectedEvent* ev = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();
                if(OS_ALREADY_SIGNALED == ev->signal(0))
                {
                    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
                    eventMgr->release(ev);
                }
            }
            break;

        case CallManager::CP_DESTROY_PLAYER:
            {
                MpStreamPlayer* pPlayer ;

                addHistoryEvent(msgSubType, multiStringMessage);

                // Redispatch Request to flowgraph
                if(mpMediaInterface)
                {
                    pPlayer = (MpStreamPlayer*) ((CpMultiStringMessage&)eventMessage).getInt2Data();
                    mpMediaInterface->destroyPlayer(pPlayer) ;
                }

                // Signal Event so that the caller knows the work is done
                OsProtectedEvent* ev = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();
                if(OS_ALREADY_SIGNALED == ev->signal(0))
                {
                    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
                    eventMgr->release(ev);
                }
            }
            break;

        case CallManager::CP_DESTROY_QUEUE_PLAYER:
            {
                MpStreamPlayer* pPlayer ;

                addHistoryEvent(msgSubType, multiStringMessage);

                // Redispatch Request to flowgraph
                if(mpMediaInterface)
                {
                    pPlayer = (MpStreamPlayer*) ((CpMultiStringMessage&)eventMessage).getInt2Data();
                    mpMediaInterface->destroyQueuePlayer((MpStreamQueuePlayer*)pPlayer) ;
                }

                // Signal Event so that the caller knows the work is done
                OsProtectedEvent* ev = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();
                if(OS_ALREADY_SIGNALED == ev->signal(0))
                {
                    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
                    eventMgr->release(ev);
                }
            }
            break;

        case CallManager::CP_SET_PREMIUM_SOUND_CALL:
            addHistoryEvent(msgSubType, multiStringMessage);
            {
                UtlBoolean enabled = ((CpMultiStringMessage&)eventMessage).getInt1Data();
                mpMediaInterface->setPremiumSound(enabled);
            }
            break;

        case CallManager::CP_DROP:
            addHistoryEvent(msgSubType, multiStringMessage);
            {
                UtlString callId;
                int metaEventId = ((CpMultiStringMessage&)eventMessage).getInt1Data();
                ((CpMultiStringMessage&)eventMessage).getString1Data(callId);

                hangUp(callId, metaEventId);
            }
            break;

        case CallManager::CP_ENABLE_DTMF_EVENT:
            addHistoryEvent(msgSubType, multiStringMessage);
            {
                OsWriteLock lock(mDtmfQMutex);
                int ev = ((CpMultiStringMessage&)eventMessage).getInt1Data();

                assert(mDtmfQLen < MAX_NUM_TONE_LISTENERS);

                int found = dtmfEventExists(ev);
                if (found == -1)
                {
                    mDtmfEvents[mDtmfQLen].event = ev;
                    mDtmfEvents[mDtmfQLen].interdigitSecs = ((CpMultiStringMessage&)eventMessage).getInt2Data();
                    mDtmfEvents[mDtmfQLen].ignoreKeyUp = ((CpMultiStringMessage&)eventMessage).getInt3Data();
                    mDtmfEvents[mDtmfQLen].enabled = TRUE;
                    mDtmfQLen++;
                }
                else
                {
                    mDtmfEvents[found].enabled = TRUE;
                }

            }
            break;
        case CallManager::CP_DISABLE_DTMF_EVENT:
            addHistoryEvent(msgSubType, multiStringMessage);
            {
                OsWriteLock lock(mDtmfQMutex);

                // Temporarily remove the event from list, do not delete it.
                // removeDtmfEvent will/has to/ be called to delete it.
                int ev = ((CpMultiStringMessage&)eventMessage).getInt1Data();
                int found = dtmfEventExists(ev);
                if (found >= 0)
                    mDtmfEvents[found].enabled = FALSE;
            }
            break;

        case CallManager::CP_REMOVE_DTMF_EVENT:
            addHistoryEvent(msgSubType, multiStringMessage);
            {
                OsWriteLock lock(mDtmfQMutex);

                int ev = ((CpMultiStringMessage&)eventMessage).getInt1Data();
                removeFromDtmfEventList(ev);

                // 08/19/03 (rschaaf):
                // The entity requesting the CP_REMOVE_DTMF_EVENT can't delete
                // the event because it might still be in use.  Instead,
                // the recipient of the CP_REMOVE_DTMF_EVENT message must take
                // responsibility for deleting the event.
                OsQueuedEvent* pEvent = (OsQueuedEvent*) ev;
                delete pEvent;
            }
            break;

        case CallManager::CP_EZRECORD:
            addHistoryEvent(msgSubType, multiStringMessage);
            {
                UtlString fileName;
                ((CpMultiStringMessage&)eventMessage).getString2Data(fileName);
                OsProtectedEvent* ev = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();
                int ms = ((CpMultiStringMessage&)eventMessage).getInt2Data();
                int silenceLength = ((CpMultiStringMessage&)eventMessage).getInt3Data();
                int dtmfterm = ((CpMultiStringMessage&)eventMessage).getInt4Data();

                double duration;
                if (mpMediaInterface)
                {
                    mpMediaInterface->ezRecord(ms, silenceLength, fileName.data(), duration, dtmfterm, ev);
                }

            }
            break;

        case CallManager::CP_STOPRECORD:
            {
                stopRecord();

                OsProtectedEvent* ev = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();
                if (OS_ALREADY_SIGNALED == ev->signal(0))
                {
                    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
                    eventMgr->release(ev);
                }
            }
            break;
        default:
            processedMessage = handleCallMessage(eventMessage);
            break;
        }       // end PHONE_APP
        break;

    case OsMsg::OS_EVENT:
        {
            switch(msgSubType)
            {
            case OsEventMsg::NOTIFY:
                {
                    intptr_t eventData;
                    void* pListener;
                    ((OsEventMsg&)eventMessage).getEventData(eventData);
                    ((OsEventMsg&)eventMessage).getUserData(pListener);
                    if (pListener)
                    {
                        char    buf[128];
                        UtlString arg;
                        int argCnt = 2;
                        int i;

                        getCallId(arg);
                        arg.append(TAOMESSAGE_DELIMITER);
                        sprintf(buf, "%ld", (long)eventData);
                        arg.append(buf);

                        for (i = 0; i < mToneListenerCnt; i++)
                        {
                            if (mpToneListeners[i] && (mpToneListeners[i]->mpListenerPtr == pListener))
                            {
                                arg.append(TAOMESSAGE_DELIMITER);
                                arg.append(mpToneListeners[i]->mName);
                                argCnt = 3;

                                // post the dtmf event
                                TaoEventId eventId = TaoMessage::BUTTON_PRESS;

                                TaoMessage msg(TaoMessage::EVENT,
                                    0,
                                    0,
                                    eventId,
                                    0,
                                    argCnt,
                                    arg);

                                ((OsServerTask*)pListener)->postMessage((OsMsg&)msg);
                            }
                        }

                        // respond to the waitforDtmfTone event
                        {
                            OsWriteLock lock(mDtmfQMutex);

                            OsSysLog::add(FAC_CP, PRI_INFO, "CpCall %s - received dtmf event 0x%08lx QLen=%d\n",
                                mCallId.data(), (long)eventData, mDtmfQLen);

                            for (i = 0; i < mDtmfQLen; i++)
                            {
                                if (mDtmfEvents[i].enabled == FALSE)
                                {
                                    OsSysLog::add(FAC_CP, PRI_INFO, "CpCall %s - event %p is disabled\n",
                                        mCallId.data(), &mDtmfEvents[i]);
                                    continue;
                                }

                                if (mDtmfEvents[i].ignoreKeyUp && (eventData & 0x80000000))
                                {
                                    OsSysLog::add(FAC_CP, PRI_INFO, "CpCall %s - ignore KEYUP event 0x%08lx\n",
                                        mCallId.data(), (long)eventData);
                                    continue; // ignore keyup event
                                }

                                if ((eventData & 0x80000000) == 0 &&
                                    (eventData & 0x0000ffff))
                                {
                                    OsSysLog::add(FAC_CP, PRI_INFO, "CpCall %s - ignore KEYDOWN event 0x%08lx\n",
                                        mCallId.data(), (long)eventData);
                                    continue; // previous key still down, ignore long key event
                                }
                                OsQueuedEvent* dtmfEvent = (OsQueuedEvent*)(mDtmfEvents[i].event);
                                if (dtmfEvent)
                                {
                                    OsStatus res = dtmfEvent->signal((eventData & 0xfffffff0));
                                    // There could be a race condition in media server
                                    // where the receiving msgq can be processing an event from the
                                    // playerlistener and this event is signaled before the queue is reset,
                                    // so we'll try to send a few more times until success.
                                    int tries = 0;
                                    while ((tries++ < 10) && (res != OS_SUCCESS))
                                    {
                                        res = dtmfEvent->signal((eventData & 0xfffffff0));
                                        OsSysLog::add(FAC_CP, PRI_INFO, "CpCall %s - resend dtmfEvent event 0x%08lx to %p, res=%d\n",
                                            mCallId.data(), (long)eventData, dtmfEvent, res);
                                    }
                                    if (res != OS_SUCCESS && tries >= 10)
                                    {
                                        OsSysLog::add(FAC_CP, PRI_ERR, "CpCall %s - failed to notify DTMF event 0x%08ldx to %p, res=%d\n",
                                            mCallId.data(), (long)eventData, dtmfEvent, res);
                                    }
                                }
                            }
                        }
                    }

                }
                break;

            default:
                processedMessage = FALSE;
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpCall::handleMessage  "
                  "Unknown event message "
                  "TYPE: %d  subtype: %d\n",
                  msgType, msgSubType);
#endif
                break;
            }
        }       // end OS_EVENT
        break ;

    case OsMsg::STREAMING_MSG:
        if (mpMediaInterface)
        {
            mpMediaInterface->getMsgQ()->send(eventMessage) ;
        }
        break ;

    default:
        processedMessage = FALSE;
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpCall::handleMessage  "
                  "Unprocessed message "
                  "TYPE: %d  subtype: %d\n",
                  msgType, msgSubType);
#endif
        break;
    }

    //    osPrintf("exiting CpCall::handleMessage\n");
    return(processedMessage);
}

void CpCall::inFocus(int talking)
{

    mCallInFocus = TRUE;
    mLocalConnectionState = PtEvent::CONNECTION_ESTABLISHED;
    if (talking)
        mLocalTermConnectionState = PtTerminalConnection::TALKING;
    else
        mLocalTermConnectionState = PtTerminalConnection::IDLE;

    if(mpMediaInterface)
    {
        mpMediaInterface->giveFocus();
    }
}

void CpCall::outOfFocus()
{
    mCallInFocus = FALSE;
    //      mLocalConnectionState = PtEvent::CONNECTION_QUEUED;
    //      mLocalTermConnectionState = PtTerminalConnection::HELD;

    if(mpMediaInterface)
    {
        mpMediaInterface->defocus();
    }
}

void CpCall::localHold()
{
    if(!mLocalHeld)
    {
        mLocalHeld = TRUE;
        // Post a message to the callManager to change focus
        CpIntMessage localHoldMessage(CallManager::CP_YIELD_FOCUS,
            (intptr_t)this);
        mpManager->postMessage(localHoldMessage);
        mLocalTermConnectionState = PtTerminalConnection::HELD;
    }
}

void CpCall::hangUp(UtlString callId, int metaEventId)
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpCall::hangUp "
                  "enter mLCstate %d mLTCstate %d ",
                  mLocalConnectionState, mLocalTermConnectionState);
#endif
    mDropping = TRUE;
    mLocalConnectionState = PtEvent::CONNECTION_DISCONNECTED;
    mLocalTermConnectionState = PtTerminalConnection::DROPPED;

    if (metaEventId > 0)
        setMetaEvent(metaEventId, PtEvent::META_CALL_ENDING, 0, 0);
    else
        startMetaEvent(mpManager->getNewMetaEventId(), PtEvent::META_CALL_ENDING, 0, 0);

    onHook();
}


OsStatus CpCall::addTaoListener(OsServerTask* pListener,
                                char* callId,
                                int connectId,
                                int mask,
                                intptr_t pEv)
{
    return addListener(pListener,
        mpListeners,
        mListenerCnt,
        callId,
        connectId,
        mask,
        pEv);

}

void CpCall::setLocalConnectionState(int newState)
{
    mLocalConnectionState = newState;
}

OsStatus CpCall::stopRecord()
{
#ifdef TEST_PRINT
    osPrintf("Calling mpMediaInterface->stopRecording()\n");
    OsSysLog::add(FAC_CP, PRI_DEBUG, "Calling mpMediaInterface->stopRecording()");
#endif
    return mpMediaInterface->stopRecording();
}

OsStatus CpCall::ezRecord(int ms, int silenceLength, const char* fileName, double& duration, int& dtmfterm)
{
    return mpMediaInterface->ezRecord(ms, silenceLength, fileName, duration, dtmfterm);
}

void CpCall::addToneListenerToFlowGraph(intptr_t pListener, Connection* connection)
{
    OsQueuedEvent *pEv;
    pEv = new OsQueuedEvent(mIncomingQ, (void*)pListener);

    UtlString remoteAddress;
    connection->getRemoteAddress(&remoteAddress);

    addListener((OsServerTask*) pListener,
        mpToneListeners,
        mToneListenerCnt,
        (char*)remoteAddress.data(),
        connection->getConnectionId(),
        0,
        (intptr_t)pEv);

    mpMediaInterface->addToneListener(pEv, connection->getConnectionId());
}


void CpCall::removeToneListenerFromFlowGraph(intptr_t pListener, Connection* connection)
{
    mpMediaInterface->removeToneListener(connection->getConnectionId()) ;
}



/* ============================ ACCESSORS ================================= */

int CpCall::getCallIndex()
{
    return(mCallIndex);
}

int CpCall::getCallState()
{
    return(mCallState);
}

// For CallManager use ONLY.
int CpCall::getDropState()
{
    return(mCallMgrPostedDrop);
}

// For CallManager use ONLY.
void CpCall::setDropState()
{
    mCallMgrPostedDrop = TRUE;
}

void CpCall::printCall(int showHistory)
{
    UtlString callId;
    getCallId(callId);
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpCall::printCall "
                  "Call[%d] id: %s state: %d%s\n", mCallIndex,
                  callId.data(), getCallState(),
                  mDropping ? ", Dropping" : "");

    if (showHistory)
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CpCall::printCall "
                                         "Call message history:\n");
        for(int historyIndex = 0; historyIndex < CP_CALL_HISTORY_LENGTH; historyIndex++)
        {
            if(mMessageEventCount - historyIndex >= 0)
            {
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "CpCall::printCall "
                              "%d) %s\n",
                              mMessageEventCount - historyIndex,
                    (mCallHistory[(mMessageEventCount - historyIndex) % CP_CALL_HISTORY_LENGTH]).data());
            }
        }
    }
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CpCall::printCall "
                                     "Call message history done");
}

void CpCall::getCallId(UtlString& callId)
{
    OsReadLock lock(mCallIdMutex);
    callId = mCallId;
}

void CpCall::setCallId(const char* callId)
{
    OsWriteLock lock(mCallIdMutex);
    mCallId.remove(0);
    if(callId) mCallId.append(callId);
}

void CpCall::enableDtmf()
{
    mDtmfEnabled = TRUE;
}

int CpCall::getLocalConnectionState(int state)
{
    int newState;

    switch(state)
    {
    case PtEvent::CONNECTION_CREATED:
    case PtEvent::CONNECTION_INITIATED:
        newState = Connection::CONNECTION_INITIATED;
        break;

    case PtEvent::CONNECTION_ALERTING:
        newState = Connection::CONNECTION_ALERTING;
        break;

    case PtEvent::CONNECTION_DISCONNECTED:
        newState = Connection::CONNECTION_DISCONNECTED;
        break;

    case PtEvent::CONNECTION_FAILED:
        newState = Connection::CONNECTION_FAILED;
        break;

    case PtEvent::CONNECTION_DIALING:
        newState = Connection::CONNECTION_DIALING;
        break;

    case PtEvent::CONNECTION_ESTABLISHED:
        newState = Connection::CONNECTION_ESTABLISHED;
        break;

    case PtEvent::CONNECTION_NETWORK_ALERTING:
        newState = Connection::CONNECTION_NETWORK_ALERTING;
        break;

    case PtEvent::CONNECTION_NETWORK_REACHED:
        newState = Connection::CONNECTION_NETWORK_REACHED;
        break;

    case PtEvent::CONNECTION_OFFERED:
        newState = Connection::CONNECTION_OFFERING;
        break;

    case PtEvent::CONNECTION_QUEUED:
        newState = Connection::CONNECTION_QUEUED;
        break;

    default:
        newState = Connection::CONNECTION_UNKNOWN;
        break;

    }

#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpCall::getLocalConnectionState: "
                  "state- new %d old %d ",
                  newState, state);
#endif
    return newState;
}

void CpCall::getStateString(int state, UtlString* stateLabel)
{
    stateLabel->remove(0);

    switch(state)
    {
    case PtEvent::CONNECTION_CREATED:
        stateLabel->append("CONNECTION_CREATED");
        break;

    case PtEvent::CONNECTION_ALERTING:
        stateLabel->append("CONNECTION_ALERTING");
        break;

    case PtEvent::CONNECTION_DISCONNECTED:
        stateLabel->append("CONNECTION_DISCONNECTED");
        break;

    case PtEvent::CONNECTION_FAILED:
        stateLabel->append("CONNECTION_FAILED");
        break;

    case PtEvent::CONNECTION_DIALING:
        stateLabel->append("CONNECTION_DIALING");
        break;

    case PtEvent::CONNECTION_ESTABLISHED:
        stateLabel->append("CONNECTION_ESTABLISHED");
        break;

    case PtEvent::CONNECTION_INITIATED:
        stateLabel->append("CONNECTION_INITIATED");
        break;

    case PtEvent::CONNECTION_NETWORK_ALERTING:
        stateLabel->append("CONNECTION_NETWORK_ALERTING");
        break;

    case PtEvent::CONNECTION_NETWORK_REACHED:
        stateLabel->append("CONNECTION_NETWORK_REACHED");
        break;

    case PtEvent::CONNECTION_OFFERED:
        stateLabel->append("CONNECTION_OFFERED");
        break;

    case PtEvent::CONNECTION_QUEUED:
        stateLabel->append("CONNECTION_QUEUED");
        break;

    case PtEvent::TERMINAL_CONNECTION_CREATED:
        stateLabel->append("TERMINAL_CONNECTION_CREATED");
        break;

    case PtEvent::TERMINAL_CONNECTION_RINGING:
        stateLabel->append("TERMINAL_CONNECTION_RINGING");
        break;

    case PtEvent::TERMINAL_CONNECTION_DROPPED:
        stateLabel->append("TERMINAL_CONNECTION_DROPPED");
        break;

    case PtEvent::TERMINAL_CONNECTION_UNKNOWN:
        stateLabel->append("TERMINAL_CONNECTION_UNKNOWN");
        break;

    case PtEvent::TERMINAL_CONNECTION_HELD:
        stateLabel->append("TERMINAL_CONNECTION_HELD");
        break;

    case PtEvent::TERMINAL_CONNECTION_IDLE:
        stateLabel->append("TERMINAL_CONNECTION_IDLE");
        break;

    case PtEvent::TERMINAL_CONNECTION_IN_USE:
        stateLabel->append("TERMINAL_CONNECTION_IN_USE");
        break;

    case PtEvent::TERMINAL_CONNECTION_TALKING:
        stateLabel->append("TERMINAL_CONNECTION_TALKING");
        break;

    case PtEvent::CALL_ACTIVE:
        stateLabel->append("CALL_ACTIVE");
        break;

    case PtEvent::CALL_INVALID:
        stateLabel->append("CALL_INVALID");
        break;

    case PtEvent::EVENT_INVALID:
        stateLabel->append("!! INVALID_STATE !!");
        break;

    case PtEvent::CALL_META_CALL_STARTING_STARTED:
        stateLabel->append("CALL_META_CALL_STARTING_STARTED");
        break;

    case PtEvent::CALL_META_CALL_STARTING_ENDED:
        stateLabel->append("CALL_META_CALL_STARTING_ENDED");
        break;

    case PtEvent::SINGLECALL_META_PROGRESS_STARTED:
        stateLabel->append("SINGLECALL_META_PROGRESS_STARTED");
        break;

    case PtEvent::SINGLECALL_META_PROGRESS_ENDED:
        stateLabel->append("SINGLECALL_META_PROGRESS_ENDED");
        break;

    case PtEvent::CALL_META_ADD_PARTY_STARTED:
        stateLabel->append("CALL_META_ADD_PARTY_STARTED");
        break;

    case PtEvent::CALL_META_ADD_PARTY_ENDED:
        stateLabel->append("CALL_META_ADD_PARTY_ENDED");
        break;

    case PtEvent::CALL_META_REMOVE_PARTY_STARTED:
        stateLabel->append("CALL_META_REMOVE_PARTY_STARTED");
        break;

    case PtEvent::CALL_META_REMOVE_PARTY_ENDED:
        stateLabel->append("CALL_META_REMOVE_PARTY_ENDED");
        break;

    case PtEvent::CALL_META_CALL_ENDING_STARTED:
        stateLabel->append("CALL_META_CALL_ENDING_STARTED");
        break;

    case PtEvent::CALL_META_CALL_ENDING_ENDED:
        stateLabel->append("CALL_META_CALL_ENDING_ENDED");
        break;

    case PtEvent::MULTICALL_META_MERGE_STARTED:
        stateLabel->append("MULTICALL_META_MERGE_STARTED");
        break;

    case PtEvent::MULTICALL_META_MERGE_ENDED:
        stateLabel->append("MULTICALL_META_MERGE_ENDED");
        break;

    case PtEvent::MULTICALL_META_TRANSFER_STARTED:
        stateLabel->append("MULTICALL_META_TRANSFER_STARTED");
        break;

    case PtEvent::MULTICALL_META_TRANSFER_ENDED:
        stateLabel->append("MULTICALL_META_TRANSFER_ENDED");
        break;

    case PtEvent::SINGLECALL_META_SNAPSHOT_STARTED:
        stateLabel->append("SINGLECALL_META_SNAPSHOT_STARTED");
        break;

    case PtEvent::SINGLECALL_META_SNAPSHOT_ENDED:
        stateLabel->append("SINGLECALL_META_SNAPSHOT_ENDED");
        break;

    default:
        stateLabel->append("STATE_UNKNOWN");
        break;

    }

}

void CpCall::setMetaEvent(int metaEventId, int metaEventType,
                          int numCalls, const char* metaEventCallIds[])
{
    if (mMetaEventId != 0 || mMetaEventType != PtEvent::META_EVENT_NONE)
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpCall::setMetaEvent "
                      "stopping event %d type %x",
                      mMetaEventId, mMetaEventType);
#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpCall::setMetaEvent "
                      "stopMetaEvent 5");
#endif
        stopMetaEvent();
    }

    mMetaEventId = metaEventId;
    mMetaEventType = metaEventType;

    if(mpMetaEventCallIds)
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpCall::setMetaEvent "
                      "deleting event call id(s)");
        delete[] mpMetaEventCallIds;
        mpMetaEventCallIds = NULL;
    }

    if (numCalls > 0)
    {
        mNumMetaEventCalls = numCalls;
        mpMetaEventCallIds = new UtlString[numCalls];
        for(int i = 0; i < numCalls; i++)
        {
            if (metaEventCallIds)
            {
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "CpCall::setMetaEvent "
                              "callids[%d] gets '%s'",
                              i, metaEventCallIds[i]);

                mpMetaEventCallIds[i] = metaEventCallIds[i];
            }
            else
            {
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                              "CpCall::setMetaEvent "
                              "callids[%d] gets '%s'",
                              i, mCallId.data());
                mpMetaEventCallIds[i] = mCallId.data();
            }
        }
    }
}

void CpCall::startMetaEvent(int metaEventId,
                            int metaEventType,
                            int numCalls,
                            const char* metaEventCallIds[],
                            int remoteIsCallee)
{
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpCall::startMetaEvent "
                  "m-event %d m-eventType 0x%x ",
                  metaEventId, metaEventType);
    setMetaEvent(metaEventId, metaEventType, numCalls, metaEventCallIds);
    postMetaEvent(METAEVENT_START, remoteIsCallee);
}

void CpCall::getMetaEvent(int& metaEventId, int& metaEventType,
                          int& numCalls, const UtlString* metaEventCallIds[]) const
{
    metaEventId = mMetaEventId;
    metaEventType = mMetaEventType;
    numCalls = mNumMetaEventCalls;
    *metaEventCallIds = mpMetaEventCallIds;
}

void CpCall::stopMetaEvent(int remoteIsCallee)
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpCall::stopMetaEvent "
                  "m-event %d m-eventType %x ",
                  mMetaEventId, mMetaEventType);
#endif
    postMetaEvent(METAEVENT_END, remoteIsCallee);

    // Clear the event info
    mMetaEventId = 0;
    mMetaEventType = PtEvent::META_EVENT_NONE;

    if(mpMetaEventCallIds)
    {
        delete[] mpMetaEventCallIds;
        mpMetaEventCallIds = NULL;
    }
}

void CpCall::setCallType(int callType)
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpCall::setCallType "
                  "change mCallType: %d to callType: %d\n",
                  mCallType, callType);
#endif
    mCallType = callType;
}

int CpCall::getCallType() const
{
    return(mCallType);
}

void CpCall::setTargetCallId(const char* targetCallId)
{
    if(targetCallId && * targetCallId) mTargetCallId = targetCallId;
}

void CpCall::getTargetCallId(UtlString& targetCallId) const
{
    targetCallId = mTargetCallId;
}

void CpCall::setIdOfOrigCall(const char* idOfOriginalCall)
{
    if(idOfOriginalCall && * idOfOriginalCall) mIdOfOrigCall = idOfOriginalCall;
}

void CpCall::getIdOfOrigCall(UtlString& idOfOriginalCall) const
{
    idOfOriginalCall = mIdOfOrigCall;
}
/* ============================ INQUIRY =================================== */
UtlBoolean CpCall::isQueued()
{
    return(FALSE);
}

void CpCall::getLocalAddress(char* address, int maxLen)
{
   // :TODO: Not yet implemented.
   assert(FALSE);
}

void CpCall::getLocalTerminalId(char* terminal, int maxLen)
{
   // :TODO: Not yet implemented.
   assert(FALSE);
}

UtlBoolean CpCall::isCallIdSet()
{
    OsReadLock lock(mCallIdMutex);
    return(!mCallId.isNull());
}

UtlBoolean CpCall::isLocalHeld()
{
    return mLocalHeld;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void CpCall::addHistoryEvent(const char* messageLogString)
{
    mMessageEventCount++;
    mCallHistory[mMessageEventCount % CP_CALL_HISTORY_LENGTH] =
        messageLogString;
}

void CpCall::addHistoryEvent(const int msgSubType,
                             const CpMultiStringMessage* multiStringMessage)
{
    char eventDescription[100];
    UtlString subTypeString;
    CpCallManager::getEventSubTypeString((enum CpCallManager::EventSubTypes)msgSubType,
        subTypeString);
    UtlString msgDump;
    if(multiStringMessage) multiStringMessage->toString(msgDump, ", ");
    sprintf(eventDescription, " (%d) \n\t", msgSubType);
    addHistoryEvent(subTypeString + eventDescription + msgDump);
}

OsStatus CpCall::addListener(OsServerTask* pListener,
                             TaoListenerDb** pListeners,
                             int& listenerCnt,
                             char* callId,
                             int connectId,
                             int mask,
                             intptr_t pEv)
{
    for (int i = 0; i < listenerCnt; i++)
    {
        if (pListeners[i] &&
            pListeners[i]->mpListenerPtr == pListener &&
            (!callId || pListeners[i]->mName.compareTo(callId) == 0) &&
            (pListeners[i]->mId == connectId))
        {
            pListeners[i]->mRef++;
            return OS_SUCCESS;
        }
    }

    if (mListenerCnt == mMaxNumListeners)
    {
        //make more of em.
        mMaxNumListeners += 20;
        mpListeners = (TaoListenerDb **)realloc(mpListeners,sizeof(TaoListenerDb *)*mMaxNumListeners);
        for (int loop = mListenerCnt;loop < mMaxNumListeners;loop++)
            mpListeners[loop] = 0 ;
    }

    // add to listenerDb
    TaoListenerDb *pListenerDb = new TaoListenerDb();
    if (callId)
        pListenerDb->mName.append(callId);
    pListenerDb->mpListenerPtr = pListener;
    pListenerDb->mRef = 1;
    pListenerDb->mId = connectId;
    pListenerDb->mIntData = pEv;
    pListeners[listenerCnt++] = pListenerDb;

    return OS_SUCCESS;
}

void CpCall::postMetaEvent(int state, int remoteIsCallee)
{
    OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpCall::postMetaEvent "
                  "enter m-state %d m-event 0x%x numListeners=%d",
                  state, mMetaEventType, mListenerCnt);

    if (mMetaEventType != PtEvent::META_EVENT_NONE
        && mListenerCnt > 0)
    {
        int eventId = PtEvent::META_UNKNOWN;

        switch (mMetaEventType)
        {
        case PtEvent::META_CALL_STARTING:
            if (state == METAEVENT_START)
                eventId = PtEvent::CALL_META_CALL_STARTING_STARTED;
            else if (state == METAEVENT_END)
                eventId = PtEvent::CALL_META_CALL_STARTING_ENDED;
            break;

        case PtEvent::META_CALL_PROGRESS:
            if (state == METAEVENT_START)
                eventId = PtEvent::SINGLECALL_META_PROGRESS_STARTED;
            else if (state == METAEVENT_END)
                eventId = PtEvent::SINGLECALL_META_PROGRESS_ENDED;
            break;

        case PtEvent::META_CALL_ADDITIONAL_PARTY:
            if (state == METAEVENT_START)
                eventId = PtEvent::CALL_META_ADD_PARTY_STARTED;
            else if (state == METAEVENT_END)
                eventId = PtEvent::CALL_META_ADD_PARTY_ENDED;
            break;

        case PtEvent::META_CALL_REMOVING_PARTY:
            if (state == METAEVENT_START)
                eventId = PtEvent::CALL_META_REMOVE_PARTY_STARTED;
            else if (state == METAEVENT_END)
                eventId = PtEvent::CALL_META_REMOVE_PARTY_ENDED;
            break;

        case PtEvent::META_CALL_ENDING:
            if (state == METAEVENT_START)
                eventId = PtEvent::CALL_META_CALL_ENDING_STARTED;
            else if (state == METAEVENT_END)
                eventId = PtEvent::CALL_META_CALL_ENDING_ENDED;
            break;

        case PtEvent::META_CALL_MERGING:
            if (state == METAEVENT_START)
                eventId = PtEvent::MULTICALL_META_MERGE_STARTED;
            else if (state == METAEVENT_END)
                eventId = PtEvent::MULTICALL_META_MERGE_ENDED;
            break;

        case PtEvent::META_CALL_TRANSFERRING:
            if (state == METAEVENT_START)
                eventId = PtEvent::MULTICALL_META_TRANSFER_STARTED;
            else if (state == METAEVENT_END)
                eventId = PtEvent::MULTICALL_META_TRANSFER_ENDED;
            break;

        case PtEvent::META_SNAPSHOT:
            if (state == METAEVENT_START)
                eventId = PtEvent::SINGLECALL_META_SNAPSHOT_STARTED;
            else if (state == METAEVENT_END)
                eventId = PtEvent::SINGLECALL_META_SNAPSHOT_ENDED;
            break;

        case PtEvent::META_CALL_REPLACING:
            if (state == METAEVENT_START)
                eventId = PtEvent::MULTICALL_META_REPLACE_STARTED;
            else if (state == METAEVENT_END)
                eventId = PtEvent::MULTICALL_META_REPLACE_ENDED;
            break;

        default:
        case PtEvent::META_UNKNOWN:
            osPrintf("CpCall::postMetaEvent - UNKNOWN_EVENT 0x%3x\n", mMetaEventId);
            eventId = PtEvent::META_UNKNOWN;
            break;
        }

        if (remoteIsCallee != -1)
        {
            postTaoListenerMessage(0, "", eventId, CALL_STATE, PtEvent::CAUSE_UNKNOWN, remoteIsCallee);
        }
        else
        {
            postTaoListenerMessage(0, "", eventId, CALL_STATE, PtEvent::CAUSE_UNKNOWN);
        }

        OsSysLog::add(FAC_CP, PRI_DEBUG,
                      "CpCall::postMetaEvent "
                      "leave eventId %d",
                      eventId);
    }

}

void CpCall::postTaoListenerMessage(int responseCode,
                                    UtlString responseText,
                                    TaoEventId eventId,
                                    int type,
                                    int cause,
                                    int remoteIsCallee,
                                    UtlString remoteAddress,
                                    int isRemote,
                                    UtlString targetCallId)
{
    if (type == CONNECTION_STATE && !PtEvent::isStateTransitionAllowed(eventId, mLocalConnectionState))
    {
        osPrintf("Connection state change from %d to %d is illegal\n", mLocalConnectionState, (int)eventId);
        return;
    }

    if (type == CONNECTION_STATE)
    {
        mLocalConnectionState = eventId;
    }
    else if (type == TERMINAL_CONNECTION_STATE)
    {
        mLocalTermConnectionState = tcStateFromEventId(eventId);
    }

    if (mListenerCnt > 0 && eventId != PtEvent::EVENT_INVALID)
    {
        char    buf[128];
        UtlString arg;
        UtlString callId;
        if (targetCallId == OsUtil::NULL_OS_STRING)
        {
            getCallId(callId);
        }
        else
        {
            callId.append(targetCallId.data());
        }

        getLocalAddress(buf, 127);
        arg = callId;                                                                   // arg[0]
        arg.append(TAOMESSAGE_DELIMITER);
        arg.append(buf); // call id, local address              // arg[1]
        if (remoteAddress.isNull())
        {
            arg.append(TAOMESSAGE_DELIMITER);                       // arg[2]
            arg.append("UNKNOWN");  // remote address not available yet
        }
        else
        {
            arg.append(TAOMESSAGE_DELIMITER);                       // arg[2]
            arg.append(remoteAddress);      // remote address
        }

        arg.append(TAOMESSAGE_DELIMITER);                               // arg[3]
        if (remoteIsCallee)                             // remoteiscallee?
        {
            arg.append("1");
        }
        else
        {
            arg.append("0");
        }

        sprintf(buf, "%d", cause);
        arg.append(TAOMESSAGE_DELIMITER);
        arg.append(buf);        // cause                                        // arg[4]

        int argCnt = 9;
        //              if (type == TERMINAL_CONNECTION_STATE)  // local terminal name
        //              {
        getLocalTerminalId(buf, 127);
        arg.append(TAOMESSAGE_DELIMITER);
        arg.append(buf);                                                        // arg[5]
        //                      argCnt = 7;
        //              }

        arg.append(TAOMESSAGE_DELIMITER);                               // arg[6]
        if (isRemote)
        {
            arg.append("0");
        }
        else                        // isLocal
        {
            arg.append("1");
        }

        sprintf(buf, "%d", responseCode);                               // arg[7]
        arg += TAOMESSAGE_DELIMITER + UtlString(buf);   // SIP response code

        arg += TAOMESSAGE_DELIMITER + responseText;             // arg[8] SIP response text

        if (mMetaEventId > 0)
        {
            arg.append(TAOMESSAGE_DELIMITER);
            sprintf(buf, "%d", mMetaEventId);                       // arg[9]
            arg.append(buf);

            arg.append(TAOMESSAGE_DELIMITER);
            sprintf(buf, "%d", mMetaEventType);                     // arg[10]
            arg.append(buf);

            if(mpMetaEventCallIds)
            {
                for (int i = 0; i < mNumMetaEventCalls; i++)
                {
                    arg.append(TAOMESSAGE_DELIMITER);
                    arg.append(mpMetaEventCallIds[i]);
                }
            }
            argCnt += (mNumMetaEventCalls + 2);
        }

        TaoMessage msg(TaoMessage::EVENT,
                       0, 0,
                       eventId,
                       0,
                       argCnt, arg);

        for (int i = 0; i < mListenerCnt; i++)
        {
            if (mpListeners[i] && mpListeners[i]->mpListenerPtr)
            {
                ((OsServerTask*) (mpListeners[i]->mpListenerPtr))->postMessage((OsMsg&)msg);
            }
        }

        UtlString eventLog;
        getStateString(eventId, &eventLog);
        UtlString causeStr;

        switch(cause)
        {
        case PtEvent::CAUSE_UNHOLD:
            causeStr.append("CAUSE_UNHOLD");
            break;
        case PtEvent::CAUSE_UNKNOWN:
            causeStr.append("CAUSE_UNKNOWN");
            break;
        case PtEvent::CAUSE_REDIRECTED:
            causeStr.append("CAUSE_REDIRECTED");
            break ;

        case PtEvent::CAUSE_NETWORK_CONGESTION:
            causeStr.append("CAUSE_NETWORK_CONGESTION");
            break;

        case PtEvent::CAUSE_NETWORK_NOT_OBTAINABLE:
            causeStr.append("CAUSE_NETWORK_NOT_OBTAINABLE");
            break;

        case PtEvent::CAUSE_DESTINATION_NOT_OBTAINABLE:
            causeStr.append("CAUSE_DESTINATION_NOT_OBTAINABLE");
            break;

        case PtEvent::CAUSE_INCOMPATIBLE_DESTINATION:
            causeStr.append("CAUSE_INCOMPATIBLE_DESTINATION");
            break;

        case PtEvent::CAUSE_NOT_ALLOWED:
            causeStr.append("CAUSE_NOT_ALLOWED");
            break;

        case PtEvent::CAUSE_NETWORK_NOT_ALLOWED:
            causeStr.append("CAUSE_NETWORK_NOT_ALLOWED");
            break;

        case PtEvent::CAUSE_BUSY:
            causeStr.append("CAUSE_BUSY");
            break ;

        case PtEvent::CAUSE_CALL_CANCELLED:
            causeStr.append("CAUSE_CALL_CANCELLED");
            break ;

        case PtEvent::CAUSE_TRANSFER:
            causeStr.append("CAUSE_TRANSFER");
            break;

        case PtEvent::CAUSE_NEW_CALL:
            causeStr.append("CAUSE_NEW_CALL");
            break;

        default:
        case PtEvent::CAUSE_NORMAL:
            causeStr.append("CAUSE_NORMAL");
            break;
        }
        mpManager->logCallState(arg.data(), eventLog.data(), causeStr.data());

        arg.remove(0);
        callId.remove(0);
        eventLog.remove(0);
        causeStr.remove(0);
    }
}

int CpCall::tcStateFromEventId(int eventId)
{
    int state;

    switch(eventId)
    {
    case PtEvent::TERMINAL_CONNECTION_CREATED:
    case PtEvent::TERMINAL_CONNECTION_IDLE:
        state = PtTerminalConnection::IDLE;
        break;

    case PtEvent::TERMINAL_CONNECTION_HELD:
        state = PtTerminalConnection::HELD;
        break;

    case PtEvent::TERMINAL_CONNECTION_RINGING:
        state = PtTerminalConnection::RINGING;
        break;

    case PtEvent::TERMINAL_CONNECTION_TALKING:
        state = PtTerminalConnection::TALKING;
        break;

    case PtEvent::TERMINAL_CONNECTION_IN_USE:
        state = PtTerminalConnection::IN_USE;
        break;

    case PtEvent::TERMINAL_CONNECTION_DROPPED:
        state = PtTerminalConnection::DROPPED;
        break;

    default:
        state = PtTerminalConnection::UNKNOWN;
        break;
    }

    return state;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void CpCall::removeFromDtmfEventList(int ev)
{
    for (int i = 0; i < mDtmfQLen; i++)
    {
        if (mDtmfEvents[i].event == ev)
        {
            for (int j = i; j < (mDtmfQLen - 1); j++)
            {
                mDtmfEvents[j].event = mDtmfEvents[j + 1].event;
                mDtmfEvents[j].interdigitSecs = mDtmfEvents[j + 1].interdigitSecs;
                mDtmfEvents[j].timeoutSecs = mDtmfEvents[j + 1].timeoutSecs;
                mDtmfEvents[j].ignoreKeyUp = mDtmfEvents[j + 1].ignoreKeyUp;
            }
            mDtmfQLen--;
        }
    }
}

int CpCall::dtmfEventExists(int ev)
{
    int found = -1;
    for (int i = 0; i < mDtmfQLen; i++)
    {
        if (mDtmfEvents[i].event == ev)
        {
            found = i;
            break;
        }
    }
    return found;
}

/* ============================ FUNCTIONS ================================= */

OsStatus CpCall::addToCallTrackingList(UtlString &rCallTaskName)
{
    OsStatus retval = OS_FAILED;

    if (!spCallTrackingList)
        spCallTrackingList = new OsLockingList();

    UtlString *tmpTaskName = new UtlString(rCallTaskName);
    spCallTrackingList->push((void *)tmpTaskName);
    retval = OS_SUCCESS; //push doesn't have a return value

    return retval;
}



//Removes a call task from the tracking list, then deletes it.
//
OsStatus CpCall::removeFromCallTrackingList(UtlString &rCallTaskName)
{
    OsStatus retval = OS_FAILED;


    UtlString *pStrFoundTaskName;

    //get an iterator handle for safe traversal
    int iteratorHandle = spCallTrackingList->getIteratorHandle();

    pStrFoundTaskName = (UtlString *)spCallTrackingList->next(iteratorHandle);
    while (pStrFoundTaskName)
    {
        // we found a Call task name that matched.  Lets remove it
        if (*pStrFoundTaskName == rCallTaskName)
        {
            spCallTrackingList->remove(iteratorHandle);
            delete pStrFoundTaskName;
            retval = OS_SUCCESS;
        }

        pStrFoundTaskName = (UtlString *)spCallTrackingList->next(iteratorHandle);
    }

    spCallTrackingList->releaseIteratorHandle(iteratorHandle);

    return retval;
}

//returns number of call tasks being tracked
int CpCall::getCallTrackingListCount()
{
    int numCalls = 0;

    if (spCallTrackingList)
        numCalls = spCallTrackingList->getCount();

    return numCalls;
}

// xecs-1698 hack
unsigned long CpCall::getElapsedTime(void)
{
   unsigned long idleSeconds;
   OsTime     nowSeconds;
   OsDateTime nowTime;

   OsDateTime::getCurTime(nowTime);
   nowTime.cvtToTimeSinceEpoch(nowSeconds);

   idleSeconds = nowSeconds.seconds() - mCallTimeStart.seconds();

   return idleSeconds;
}
