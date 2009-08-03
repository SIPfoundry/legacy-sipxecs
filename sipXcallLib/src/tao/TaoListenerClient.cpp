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

#include <assert.h>
#include "tao/TaoListenerClient.h"
#include "tao/TaoString.h"
#include "tao/TaoClientTask.h"
#include "ptapi/PtTerminalConnectionEvent.h"
#include "ptapi/PtTerminalConnectionListener.h"
#include "ptapi/PtTerminalComponentListener.h"
#include "ptapi/PtTerminalComponentEvent.h"
#include "ptapi/PtConnectionEvent.h"

//#define TEST_PRINT 1


#ifdef WV_DEBUG
#ifdef _VXWORKS
#include <vxworks.h>
#include "wvLib.h"
#endif

typedef struct
{
    TaoEventId id;
    char name[16];
} USER_EVENT;

char name[16];
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaoListenerClientTask::TaoListenerClientTask(TaoClientTask *pClient,
                                                        const UtlString& name,
                                                        const int maxRequestQMsgs)
: OsServerTask(name, NULL, maxRequestQMsgs),
  mListenerSem(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
        mpClient = pClient;
        initInstance();
        if (!isStarted())
        {
                start();
        }
}

// Constructor
TaoListenerClientTask::TaoListenerClientTask(const int priority,
                const UtlString& name,
                void* pArg,
                const int maxRequestQMsgs,
                const int options,
                const int stackSize)
: OsServerTask(name, pArg, maxRequestQMsgs, priority, options, stackSize),
  mListenerSem(OsBSem::Q_PRIORITY, OsBSem::FULL)

{
        mpClient = NULL;
        initInstance();
        if (!isStarted())
        {
                start();
        }
}

TaoListenerClientTask::TaoListenerClientTask(const int maxIncomingQMsgs)
: OsServerTask("TaoListenerClient-%d", NULL, maxIncomingQMsgs),
  mListenerSem(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
        mpClient = NULL;
        initInstance();
        if (!isStarted())
        {
                start();
        }
}

// Destructor
TaoListenerClientTask::~TaoListenerClientTask()
{
        if (mListenerCnt > 0)
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

        if (mpCallEvent)
        {
                delete mpCallEvent;
                mpCallEvent = 0;
        }
        if (mpConnEvent)
        {
                delete mpConnEvent;
                mpConnEvent = 0;
        }

        if (mpTermConnEvent)
        {
                delete mpTermConnEvent;
                mpTermConnEvent = 0;
        }
}

//////////////////////////////////////////////////////////////////////
// MANIPULATORS
//////////////////////////////////////////////////////////////////////

// Initialization, called by constructor
TaoStatus TaoListenerClientTask::initInstance()
{
        mListenerCnt = 0;
    mMaxNumListeners = 20;

    mpListeners = (TaoListenerDb**) malloc(sizeof(TaoListenerDb *)*mMaxNumListeners);

    if (!mpListeners)
    {
        osPrintf("***** ERROR ALLOCATING LISTENERS IN TAOLISTENERCLIENT **** \n");
        return TAO_FAILURE;
    }

        for (int i = 0; i < mMaxNumListeners; i++)
                mpListeners[i] = 0;

        mpCallEvent = new PtCallEvent(mpClient);
        mpConnEvent = new PtConnectionEvent(mpClient);
        mpTermConnEvent = new PtTerminalConnectionEvent(mpClient);

        return TAO_SUCCESS;
}


UtlBoolean TaoListenerClientTask::handleMessage(OsMsg& rMsg)
{
        UtlBoolean handled = FALSE;

        switch (rMsg.getMsgSubType())
        {
        case TaoMessage::EVENT:
                handled = receiveEvent((TaoMessage&) rMsg);
                break;
        case TaoMessage::UNSPECIFIED:
        default:
//              handled = FALSE;
//              assert(FALSE);
                break;
        }

        return handled;
}

UtlBoolean TaoListenerClientTask::receiveEvent(TaoMessage& rMsg)
{
    UtlBoolean bHandled ;
#ifdef TAO_TIME_DEBUG
        TaoEventId eventId = rMsg.getTaoObjHandle();
        OsTimeLog timeLog;
        char tmp[64];

        sprintf(tmp, "%ld ", eventId);
        UtlString stringData = UtlString("RECEIVE EVENT: ") + UtlString(tmp);
    timeLog.addEvent(stringData.data());
#else
        rMsg.getTaoObjHandle();
#endif
        {
                mListenerSem.acquire();
                TaoListenerDb **pHandles = mpListeners;
                int num = mListenerCnt;

#ifdef _VXWORKS
                char buf[128];
#endif
                if (num < 1)
                {
                        mListenerSem.release();
                        return TRUE;
                }

                if (num >= mMaxNumListeners)
                {
            //make more of em.
            mMaxNumListeners += 20;
            mpListeners = (TaoListenerDb **)realloc(mpListeners,sizeof(TaoListenerDb *)*mMaxNumListeners);
            for (int loop = mListenerCnt;loop < mMaxNumListeners;loop++)
                mpListeners[loop] = 0 ;

                        pHandles = (TaoListenerDb **)new TaoListenerDb[num+1];

                        if (num > 0)  // check if listener is already added.
                        {
                                for (int i = 0; i < num; i++)
                                {
                                        if (mpListeners[i])
                                        {
                                                pHandles[i] = mpListeners[i];
                                        }
                                }
                        }
                }
                mListenerSem.release();

                if (num > 0)
                {
                        void* listener;
                        for (int i = 0; i < num; i++)
                        {
                                if (pHandles[i])
                                        listener = pHandles[i]->mpListenerPtr;
                                else
                                        listener = 0;
                                if (listener)
                                {
                                        PtEventListener *pListener = (PtEventListener *)listener;

                                        // Note: the following if (w/o else)statements let the event
                                        // be sent to all parent classes of pListener.

                                        if ((pListener->isClass("PtTerminalConnectionListener")) ||
                                                (pListener->isClass("PtTerminalConnectionListenerWrap")))
                                        {
#ifdef TAO_TIME_DEBUG
                                                bHandled = receiveTerminalConnectionEvent(rMsg, (PtTerminalConnectionListener*) pListener, timeLog);
#else
                                                bHandled = receiveTerminalConnectionEvent(rMsg, (PtTerminalConnectionListener*) pListener);
#endif
                        if (bHandled)
                            continue;
                                        }

                                        if ((pListener->isClass("PtConnectionListener")) ||
                                                (pListener->isClass("PtConnectionListenerWrap")) ||
                                                (pListener->isClass("PtTerminalConnectionListenerWrap")))
                                        {
#ifdef TAO_TIME_DEBUG
                                                bHandled = receiveConnectionEvent(rMsg, (PtConnectionListener*) pListener, timeLog);
#else
                                                bHandled = receiveConnectionEvent(rMsg, (PtConnectionListener*) pListener);
#endif
                        if (bHandled)
                            continue;
                    }

                                        if ((pListener->isClass("PtCallListener")) ||
                                                (pListener->isClass("PtCallListenerWrap")) ||
                                                (pListener->isClass("PtConnectionListenerWrap")) ||
                                                (pListener->isClass("PtTerminalConnectionListenerWrap")))
                                        {
#ifdef TAO_TIME_DEBUG
                                                bHandled = receiveCallEvent(rMsg, (PtCallListener*) pListener, timeLog);
#else
                                                bHandled = receiveCallEvent(rMsg, (PtCallListener*) pListener);
#endif
                        if (bHandled)
                            continue;
                    }

                                        if ((pListener->isClass("PtTerminalComponentListener")) ||
                                                (pListener->isClass("PtTerminalComponentListenerContainer")))
                                        {
#ifdef TAO_TIME_DEBUG
                                                bHandled = receiveTerminalComponentEvent(rMsg, (PtTerminalComponentListener*) pListener, timeLog);
#else
                                                bHandled = receiveTerminalComponentEvent(rMsg, (PtTerminalComponentListener*) pListener);
#endif
                        if (bHandled)
                            continue;                                   }

                                        if ((pListener->isClass("PtTerminalListener")) ||
                                                (pListener->isClass("PtTerminalComponentListenerContainer")))
                                        {
#ifdef TAO_TIME_DEBUG
                                                bHandled = receiveTerminalEvent(rMsg, (PtTerminalListener*) pListener, timeLog);
#else
                                                bHandled = receiveTerminalEvent(rMsg, (PtTerminalListener*) pListener);
#endif
                        if (bHandled)
                            continue;
                                        }
                                }
                        }
                }

                if (num >= mMaxNumListeners)
                        delete[] pHandles;
        }

#ifdef TAO_TIME_DEBUG
        sprintf(tmp, "%ld ", eventId);
        stringData = UtlString("LEAVING EVENT: ") + UtlString(tmp);
    timeLog.addEvent(stringData.data());
    timeLog.dumpLog();
#endif
        return TRUE;
}

#ifdef TAO_TIME_DEBUG
UtlBoolean TaoListenerClientTask::receiveCallEvent(TaoMessage& rMsg,
                                                                                PtCallListener* pListener,
                                                                                OsTimeLog& timeLog)
#else
UtlBoolean TaoListenerClientTask::receiveCallEvent(TaoMessage& rMsg,
                                                                                PtCallListener* pListener)
#endif
{
    UtlBoolean bHandledMsg = TRUE ;
#ifdef TAO_TIME_DEBUG
        char tmp[64];

        sprintf(tmp, "%ld %ld ", rMsg.getTaoObjHandle(), pListener);
        UtlString stringData = UtlString("RECEIVE CALL EVENT: ") + UtlString(tmp);
    timeLog.addEvent(stringData.data());
#endif
        TaoMessage msg(rMsg);

        TaoEventId eventId;
        if (!getCallEvent(rMsg, pListener, eventId))
                return FALSE;

#ifdef WV_DEBUG
        fireUserEvent(eventId, 3);
        osPrintf("after CALL_%s\n", name);
#endif
        switch(eventId)
        {
                case PtEvent::CALL_EVENT_TRANSMISSION_ENDED:
                        EVENT_TRACE("receiveCallEvent::CALL_EVENT_TRANSMISSION_ENDED:\n") ;
                        pListener->callEventTransmissionEnded((PtCallEvent&)(*mpCallEvent));
                        break;

                case PtEvent::CALL_ACTIVE:
                        EVENT_TRACE("receiveCallEvent::CALL_ACTIVE:\n") ;
                        pListener->callActive((PtCallEvent&)(*mpCallEvent));
                        break;

                case PtEvent::CALL_INVALID:
                        EVENT_TRACE("receiveCallEvent::CALL_INVALID:\n") ;
                        pListener->callInvalid((PtCallEvent&)(*mpCallEvent));
                        break;

                case PtEvent::CALL_META_CALL_STARTING_STARTED:
                        EVENT_TRACE("receiveCallEvent::CALL_META_CALL_STARTING_STARTED:\n") ;
                        pListener->callMetaCallStartStarted((PtCallEvent&)(*mpCallEvent));
                        break;

                case PtEvent::CALL_META_CALL_STARTING_ENDED:
                        EVENT_TRACE("receiveCallEvent::CALL_META_CALL_STARTING_ENDED:\n") ;
                        pListener->callMetaCallStartEnded((PtCallEvent&)(*mpCallEvent));
                        break;

                case PtEvent::CALL_META_CALL_ENDING_STARTED:
                        EVENT_TRACE("receiveCallEvent::CALL_META_CALL_ENDING_STARTED:\n") ;
                        pListener->callMetaCallEndStarted((PtCallEvent&)(*mpCallEvent));
                        break;

                case PtEvent::CALL_META_CALL_ENDING_ENDED:
                        EVENT_TRACE("receiveCallEvent::CALL_META_CALL_ENDING_ENDED:\n") ;
                        pListener->callMetaCallEndEnded((PtCallEvent&)(*mpCallEvent));
                        break;

                case PtEvent::SINGLECALL_META_PROGRESS_STARTED:
                        EVENT_TRACE("receiveCallEvent::SINGLECALL_META_PROGRESS_STARTED:\n") ;
                        pListener->callMetaProgressStarted((PtCallEvent&)(*mpCallEvent));
                        break;

                case PtEvent::SINGLECALL_META_PROGRESS_ENDED:
                        EVENT_TRACE("receiveCallEvent::SINGLECALL_META_PROGRESS_ENDED:\n") ;
                        pListener->callMetaProgressEnded((PtCallEvent&)(*mpCallEvent));
                        break;

                case PtEvent::SINGLECALL_META_SNAPSHOT_STARTED:
                        EVENT_TRACE("receiveCallEvent::SINGLECALL_META_SNAPSHOT_STARTED:\n") ;
                        pListener->callMetaSnapshotStarted((PtCallEvent&)(*mpCallEvent));
                        break;

                case PtEvent::SINGLECALL_META_SNAPSHOT_ENDED:
                        EVENT_TRACE("receiveCallEvent::SINGLECALL_META_SNAPSHOT_ENDED:\n") ;
                        pListener->callMetaSnapshotEnded((PtCallEvent&)(*mpCallEvent));
                        break;

                case PtEvent::CALL_META_ADD_PARTY_STARTED:
                        EVENT_TRACE("receiveCallEvent::CALL_META_ADD_PARTY_STARTED:\n") ;
                        pListener->callMetaAddPartyStarted((PtCallEvent&)(*mpCallEvent));
                        break;

                case PtEvent::CALL_META_ADD_PARTY_ENDED:
                        EVENT_TRACE("receiveCallEvent::CALL_META_ADD_PARTY_ENDED:\n") ;
                        pListener->callMetaAddPartyEnded((PtCallEvent&)(*mpCallEvent));
                        break;

                case PtEvent::CALL_META_REMOVE_PARTY_STARTED:
                        EVENT_TRACE("receiveCallEvent::CALL_META_REMOVE_PARTY_STARTED:\n") ;
                        pListener->callMetaRemovePartyStarted((PtCallEvent&)(*mpCallEvent));
                        break;

                case PtEvent::CALL_META_REMOVE_PARTY_ENDED:
                        EVENT_TRACE("receiveCallEvent::CALL_META_REMOVE_PARTY_ENDED:\n") ;
                        pListener->callMetaRemovePartyEnded((PtCallEvent&)(*mpCallEvent));
                        break;

                case PtEvent::MULTICALL_META_MERGE_STARTED:
                        EVENT_TRACE("receiveCallEvent::MULTICALL_META_MERGE_STARTED:\n") ;
                        pListener->multicallMetaMergeStarted((PtMultiCallMetaEvent &)*mpCallEvent);
                        break;

                case PtEvent::MULTICALL_META_MERGE_ENDED:
                        EVENT_TRACE("receiveCallEvent::MULTICALL_META_MERGE_ENDED:\n") ;
                        pListener->multicallMetaMergeEnded((PtMultiCallMetaEvent &)*mpCallEvent);
                        break;

                case PtEvent::MULTICALL_META_TRANSFER_STARTED:
                        EVENT_TRACE("receiveCallEvent::MULTICALL_META_TRANSFER_STARTED:\n") ;
                        pListener->multicallMetaTransferStarted((PtMultiCallMetaEvent &)*mpCallEvent);
                        break;

                case PtEvent::MULTICALL_META_TRANSFER_ENDED:
                        EVENT_TRACE("receiveCallEvent::MULTICALL_META_TRANSFER_ENDED:\n") ;
                        pListener->multicallMetaTransferEnded((PtMultiCallMetaEvent &)*mpCallEvent);
                        break;

                default:
            bHandledMsg = FALSE ;
                        EVENT_TRACE("receiveCallEvent::BOGUS EVENT:\n") ;
                        break;
        }

#ifdef WV_DEBUG
        fireUserEvent(eventId, 4);
        osPrintf("after CALL_%s\n", name);
#endif
        return bHandledMsg;
}


#ifdef TAO_TIME_DEBUG
UtlBoolean TaoListenerClientTask::receiveConnectionEvent(TaoMessage& rMsg,
                                                                                                PtConnectionListener* pListener,
                                                                                                OsTimeLog& timeLog)
#else
UtlBoolean TaoListenerClientTask::receiveConnectionEvent(TaoMessage& rMsg,
                                                                                                PtConnectionListener* pListener)
#endif
{
    UtlBoolean bHandledMsg = TRUE ;
#ifdef TAO_TIME_DEBUG
        char tmp[64];

        sprintf(tmp, "%ld %ld ", rMsg.getTaoObjHandle(), pListener);
        UtlString stringData = UtlString("RECEIVE CONN EVENT: ") + UtlString(tmp);
    timeLog.addEvent(stringData.data());
#endif
        int addedToCall = 0;
        int remoteIsCallee = 0;
        TaoEventId     eventId;
        if (!getConnectionEvent(rMsg,
                                                        pListener,
                                                        eventId,
                                                        addedToCall,
                                                        remoteIsCallee))
        {
                return FALSE;
        }
#ifdef WV_DEBUG
        fireUserEvent(eventId, 3);
        osPrintf("before CONNECTION_%s\n", name);
#endif
        switch(eventId)
        {
                case PtEvent::CONNECTION_CREATED:
                        EVENT_TRACE("receiveConnectionEvent::CONNECTION_CREATED:\n") ;
                        pListener->connectionCreated((PtConnectionEvent&)(*mpConnEvent));
                        break;

                case PtEvent::CONNECTION_ALERTING:
                        if (addedToCall || !remoteIsCallee)
                        {
                                EVENT_TRACE("receiveConnectionEvent::CONNECTION_ALERTING:\n") ;
                                pListener->connectionAlerting((PtConnectionEvent&)(*mpConnEvent));
                        }
                        break;

                case PtEvent::CONNECTION_DIALING:
                        EVENT_TRACE("receiveConnectionEvent::CONNECTION_DIALING:\n") ;
                        pListener->connectionDialing((PtConnectionEvent&)(*mpConnEvent));
                        break;

                case PtEvent::CONNECTION_ESTABLISHED:
                        EVENT_TRACE("receiveConnectionEvent::CONNECTION_ESTABLISHED:\n") ;
                        pListener->connectionEstablished((PtConnectionEvent&)(*mpConnEvent));
                        break;

                case PtEvent::CONNECTION_INITIATED:
                        EVENT_TRACE("receiveConnectionEvent::CONNECTION_INITIATED:\n") ;
                        pListener->connectionCreated((PtConnectionEvent&)(*mpConnEvent));
                        break;

                case PtEvent::CONNECTION_DISCONNECTED:
                        EVENT_TRACE("1 receiveConnectionEvent::CONNECTION_DISCONNECTED:\n") ;
                        pListener->connectionDisconnected((PtConnectionEvent&)(*mpConnEvent));
                        EVENT_TRACE("2 receiveConnectionEvent::CONNECTION_DISCONNECTED:\n") ;
                        break;

                case PtEvent::CONNECTION_FAILED:
                        if (addedToCall || !remoteIsCallee)
                        {
                                EVENT_TRACE("receiveConnectionEvent::CONNECTION_FAILED:\n") ;
                                pListener->connectionFailed((PtConnectionEvent&)(*mpConnEvent));
                        }
                        break;

                case PtEvent::CONNECTION_NETWORK_ALERTING:
                        EVENT_TRACE("receiveConnectionEvent::CONNECTION_NETWORK_ALERTING:\n") ;
                        pListener->connectionNetworkAlerting((PtConnectionEvent&)(*mpConnEvent));
                        break;

                case PtEvent::CONNECTION_NETWORK_REACHED:
                        EVENT_TRACE("receiveConnectionEvent::CONNECTION_NETWORK_REACHED:\n") ;
                        pListener->connectionNetworkReached((PtConnectionEvent&)(*mpConnEvent));
                        break;

                case PtEvent::CONNECTION_OFFERED:
                        if (addedToCall || !remoteIsCallee)
                        {
#ifdef TEST_PRINT
            TaoString argList(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
            UtlString localAddr, remoteAddr;
            localAddr  = argList[1];
            remoteAddr = argList[2];
            osPrintf("\nTAO client sending OFFERED event to listener:\n\tlocal address %s\n\tremote address %s\n\n",
                     localAddr.data(), remoteAddr.data());
#endif
                                EVENT_TRACE("receiveConnectionEvent::CONNECTION_OFFERED:\n") ;
                                pListener->connectionOffered((PtConnectionEvent&)(*mpConnEvent));
                        }
                        break;

                case PtEvent::CONNECTION_QUEUED:
                        EVENT_TRACE("receiveConnectionEvent::CONNECTION_QUEUED:\n") ;
                        pListener->connectionQueued((PtConnectionEvent&)(*mpConnEvent));
                        break;

                case PtEvent::CONNECTION_UNKNOWN:
                        EVENT_TRACE("receiveConnectionEvent::CONNECTION_UNKNOWN:\n") ;
                        pListener->connectionUnknown((PtConnectionEvent&)(*mpConnEvent));
                        break;

                default:
            bHandledMsg = FALSE ;
                        EVENT_TRACE("receiveConnectionEvent::BOGUS EVENT:\n") ;
                        break;
        }

#ifdef WV_DEBUG
        fireUserEvent(eventId, 4);
        osPrintf("after CONNECTION_%s\n", name);
#endif
        return bHandledMsg;
}


#ifdef TAO_TIME_DEBUG
UtlBoolean TaoListenerClientTask::receiveTerminalEvent(TaoMessage& rMsg,
                                                                                                PtTerminalListener* pListener,
                                                                                                OsTimeLog& timeLog)
#else
UtlBoolean TaoListenerClientTask::receiveTerminalEvent(TaoMessage& rMsg,
                                                                                                PtTerminalListener* pListener)
#endif
{
    UtlBoolean bHandledMsg = TRUE ;
#ifdef TAO_TIME_DEBUG
        char tmp[64];

        sprintf(tmp, "%ld %ld ", rMsg.getTaoObjHandle(), pListener);
        UtlString stringData = UtlString("RECEIVE TERM EVENT: ") + UtlString(tmp);
    timeLog.addEvent(stringData.data());
#endif

        PtEvent::PtEventId      eventId = (PtEvent::PtEventId) rMsg.getTaoObjHandle();
        PtTerminalEvent event(eventId);

        switch(eventId)
        {
                case PtEvent::TERMINAL_EVENT_TRANSMISSION_ENDED:
                        EVENT_TRACE("receiveTerminalEvent::TERMINAL_EVENT_TRANSMISSION_ENDED:\n") ;
                        pListener->terminalEventTransmissionEnded(event);
                        break;

                default:
            bHandledMsg = FALSE ;
                        EVENT_TRACE("receiveTerminalEvent::BOGUS EVENT:\n") ;
                        break;
        }

        return bHandledMsg;
}


#ifdef TAO_TIME_DEBUG
UtlBoolean TaoListenerClientTask::receiveTerminalComponentEvent(TaoMessage& rMsg,
                                                                                                PtTerminalComponentListener* pListener,
                                                                                                OsTimeLog& timeLog)
#else
UtlBoolean TaoListenerClientTask::receiveTerminalComponentEvent(TaoMessage& rMsg,
                                                                                                PtTerminalComponentListener* pListener)
#endif
{
    UtlBoolean bHandledMsg = TRUE ;
#ifdef TAO_TIME_DEBUG
        char tmp[64];

        sprintf(tmp, "%ld %p ", rMsg.getTaoObjHandle(), pListener);
        UtlString stringData = UtlString("RECEIVE COMP EVENT: ") + UtlString(tmp);
    timeLog.addEvent(stringData.data());
#endif

        PtEvent::PtEventId      eventId = (PtEvent::PtEventId) rMsg.getTaoObjHandle();
        if (!PtEvent::isTerminalComponentEvent(eventId))
                return FALSE;

        TaoString        argList(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        int cnt = rMsg.getArgCnt();

        UtlString terminalName;

        terminalName.remove(0);
        if (cnt > 2)
        {
                terminalName = argList[2];
        }

        PtTerminalComponentEvent event(eventId, terminalName, mpClient);
        if (cnt > 2)
        {
                event.setIntData1(atoi(argList[0]));
                event.setIntData2(atoi(argList[1]));
                event.setStringData1(argList[3]);
        }

#ifdef WV_DEBUG
        fireUserEvent(eventId, 3);
        osPrintf("before TERMINAL_COMPONENT_%s\n", name);
#endif
        switch(eventId)
        {
                case PtEvent::PHONE_RINGER_VOLUME_CHANGED:
                        EVENT_TRACE("receiveTerminalComponentEvent::PHONE_RINGER_VOLUME_CHANGED:\n") ;
                        pListener->phoneRingerVolumeChanged((PtComponentIntChangeEvent&)event);
                        break;

                case PtEvent::PHONE_RINGER_PATTERN_CHANGED:
                        EVENT_TRACE("receiveTerminalComponentEvent::PHONE_RINGER_PATTERN_CHANGED:\n") ;
                        pListener->phoneRingerPatternChanged((PtComponentIntChangeEvent&)event);
                        break;
                case PtEvent::PHONE_RINGER_INFO_CHANGED:
                        EVENT_TRACE("receiveTerminalComponentEvent::PHONE_RINGER_INFO_CHANGED:\n") ;
                        pListener->phoneRingerInfoChanged((PtComponentStringChangeEvent&)event);
                        break;
                case PtEvent::PHONE_SPEAKER_VOLUME_CHANGED:
                        EVENT_TRACE("receiveTerminalComponentEvent::PHONE_SPEAKER_VOLUME_CHANGED:\n") ;
                        pListener->phoneSpeakerVolumeChanged((PtComponentIntChangeEvent&)event);
                        break;
                case PtEvent::PHONE_MICROPHONE_GAIN_CHANGED:
                        EVENT_TRACE("receiveTerminalComponentEvent::PHONE_MICROPHONE_GAIN_CHANGED:\n") ;
                        pListener->phoneMicrophoneGainChanged((PtComponentIntChangeEvent&)event);
                        break;
                case PtEvent::PHONE_LAMP_MODE_CHANGED:
                        EVENT_TRACE("receiveTerminalComponentEvent::PHONE_LAMP_MODE_CHANGED:\n") ;
                        pListener->phoneLampModeChanged((PtComponentIntChangeEvent&)event);
                        break;
                case PtEvent::PHONE_BUTTON_INFO_CHANGED:
                        EVENT_TRACE("receiveTerminalComponentEvent::PHONE_BUTTON_INFO_CHANGED:\n") ;
                        pListener->phoneButtonInfoChanged((PtComponentStringChangeEvent&)event);
                        break;
                case PtEvent::PHONE_DISPLAY_CHANGED:
                        EVENT_TRACE("receiveTerminalComponentEvent::PHONE_DISPLAY_CHANGED:\n") ;
                        pListener->phoneDisplayChanged(event);
                        break;
                case PtEvent::PHONE_BUTTON_DOWN:
                        EVENT_TRACE("receiveTerminalComponentEvent::PHONE_BUTTON_DOWN:\n") ;
                        pListener->phoneButtonDown(event);
                        break;
                case PtEvent::PHONE_BUTTON_UP:
                        EVENT_TRACE("receiveTerminalComponentEvent::PHONE_BUTTON_UP:\n") ;
                        pListener->phoneButtonUp(event);
                        break;
                case PtEvent::PHONE_BUTTON_REPEAT:
                        EVENT_TRACE("receiveTerminalComponentEvent::PHONE_BUTTON_REPEAT:\n") ;
                        pListener->phoneButtonRepeat(event);
                        break;
                case PtEvent::PHONE_HOOKSWITCH_OFFHOOK:
                        EVENT_TRACE("receiveTerminalComponentEvent::PHONE_HOOKSWITCH_OFFHOOK:\n") ;
                        pListener->phoneHookswitchOffhook(event);
                        break;
                case PtEvent::PHONE_HOOKSWITCH_ONHOOK:
                        EVENT_TRACE("receiveTerminalComponentEvent::PHONE_HOOKSWITCH_ONHOOK:\n") ;
                        pListener->phoneHookswitchOnhook(event);
                        break;

                case PtEvent::PHONE_EXTSPEAKER_CONNECTED:
                        EVENT_TRACE("receiveTerminalComponentEvent::PHONE_RINGER_VOLUME_CHANGED:\n") ;
                        pListener->phoneExtSpeakerConnected(event);
                        break;

                case PtEvent::PHONE_EXTSPEAKER_DISCONNECTED:
                        EVENT_TRACE("receiveTerminalComponentEvent::PHONE_RINGER_PATTERN_CHANGED:\n") ;
                        pListener->phoneExtSpeakerDisconnected(event);
                        break;
                default:
            bHandledMsg = FALSE ;
                        EVENT_TRACE("receiveTerminalComponentEvent::BOGUS EVENT:\n") ;
                        break;
        }

#ifdef WV_DEBUG
        fireUserEvent(eventId, 4);
        osPrintf("after TERMINAL_COMPONENT_%s\n", name);
#endif
        return bHandledMsg;
}


#ifdef TAO_TIME_DEBUG
UtlBoolean TaoListenerClientTask::receiveTerminalConnectionEvent(TaoMessage& rMsg,
                                                                                                PtTerminalConnectionListener* pListener,
                                                                                                OsTimeLog& timeLog)
#else
UtlBoolean TaoListenerClientTask::receiveTerminalConnectionEvent(TaoMessage& rMsg,
                                                                                                PtTerminalConnectionListener* pListener)
#endif
{
    UtlBoolean bHandledMsg = TRUE ;
#ifdef TAO_TIME_DEBUG
        char tmp[64];

        sprintf(tmp, "%ld %ld ", rMsg.getTaoObjHandle(), pListener);
        UtlString stringData = UtlString("RECEIVE  TC  EVENT: ") + UtlString(tmp);
    timeLog.addEvent(stringData.data());
#endif

        TaoEventId eventId;
        int addedToCall = 0;
        int remoteIsCallee = 0;
        if (!getTerminalConnectionEvent(rMsg,
                                                                        pListener,
                                                                        eventId,
                                                                        addedToCall,
                                                                        remoteIsCallee))
        {
                return FALSE;
        }

#ifdef WV_DEBUG
        fireUserEvent(eventId, 3);
        osPrintf("before TERMINAL_CONNECTION_%s\n", name);
#endif
        switch(eventId)
        {
                case PtEvent::TERMINAL_CONNECTION_CREATED:
                        EVENT_TRACE("receiveTerminalConnectionEvent::TERMINAL_CONNECTION_CREATED:\n") ;
                        pListener->terminalConnectionCreated((PtTerminalConnectionEvent&)(*mpTermConnEvent));
                        break;

                case PtEvent::TERMINAL_CONNECTION_RINGING:
                        if (addedToCall || !remoteIsCallee)
                        {
                                EVENT_TRACE("receiveTerminalConnectionEvent::TERMINAL_CONNECTION_RINGING:\n") ;
                                pListener->terminalConnectionRinging((PtTerminalConnectionEvent&)(*mpTermConnEvent));
                        }
                        break;

                case PtEvent::TERMINAL_CONNECTION_HELD:
                        EVENT_TRACE("receiveTerminalConnectionEvent::TERMINAL_CONNECTION_HELD:\n") ;
                        pListener->terminalConnectionHeld((PtTerminalConnectionEvent&)(*mpTermConnEvent));
                        break;

                case PtEvent::TERMINAL_CONNECTION_TALKING:
                        if (addedToCall || !remoteIsCallee)
                        {
                                EVENT_TRACE("receiveTerminalConnectionEvent::TERMINAL_CONNECTION_TALKING:\n") ;
                                pListener->terminalConnectionTalking((PtTerminalConnectionEvent&)(*mpTermConnEvent));
                        }
                        break;

                case PtEvent::TERMINAL_CONNECTION_DROPPED:
                        EVENT_TRACE("1 receiveTerminalConnectionEvent::TERMINAL_CONNECTION_DROPPED:\n") ;
                        pListener->terminalConnectionDropped((PtTerminalConnectionEvent&)(*mpTermConnEvent));
                        EVENT_TRACE("2 receiveTerminalConnectionEvent::TERMINAL_CONNECTION_DROPPED:\n") ;
                        break;

                case PtEvent::TERMINAL_CONNECTION_IDLE:
                        EVENT_TRACE("receiveTerminalConnectionEvent::TERMINAL_CONNECTION_IDLE:\n") ;
                        pListener->terminalConnectionIdle((PtTerminalConnectionEvent&)(*mpTermConnEvent));
                        break;

                case PtEvent::TERMINAL_CONNECTION_IN_USE:
                        EVENT_TRACE("receiveTerminalConnectionEvent::TERMINAL_CONNECTION_IN_USE:\n") ;
                        pListener->terminalConnectionInUse((PtTerminalConnectionEvent&)(*mpTermConnEvent));
                        break;

                case PtEvent::TERMINAL_CONNECTION_UNKNOWN:
                        EVENT_TRACE("receiveTerminalConnectionEvent::TERMINAL_CONNECTION_UNKNOWN:\n") ;
                        pListener->terminalConnectionUnknown((PtTerminalConnectionEvent&)(*mpTermConnEvent));
                        break;
                default:
            bHandledMsg = FALSE ;
                        EVENT_TRACE("receiveTerminalConnectionEvent::BOGUS STATE:\n") ;
                        break;
        }

#ifdef WV_DEBUG
        fireUserEvent(eventId, 4);
        osPrintf("after TERMINAL_CONNECTION_%s\n", name);
#endif
        return bHandledMsg;
}


void TaoListenerClientTask::addEventListener(PtEventListener* pListener, const char* callId)
{
        mListenerSem.acquire();
        if (mListenerCnt > 0)  // check if listener is already added.
        {
                for (int i = 0; i < mListenerCnt; i++)
                {
                        if (mpListeners[i] && mpListeners[i]->mpListenerPtr == pListener)
                        {
                                if (!callId || (callId && mpListeners[i]->mName.compareTo(callId) == 0))
                                {
                                        mpListeners[i]->mRef++;
                                        osPrintf("Listener already exists in TaoListenerClientTask: 0x%p\n", pListener);
                                        mListenerSem.release();
                                        return;
                                }
                        }
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

        TaoListenerDb *pListenerDb = new TaoListenerDb();
        if (pListenerDb)
        {
                pListenerDb->mRef++;
                pListenerDb->mpListenerPtr = pListener;
                if (callId)
                        pListenerDb->mName.append(callId);
                mpListeners[mListenerCnt++] = pListenerDb;
        }
        mListenerSem.release();
}

void TaoListenerClientTask::removeEventListener(PtEventListener& rListener)
{
        mListenerSem.acquire();
        if (mListenerCnt > 0)  // check if listener is already added.
        {
                for (int i = 0; i < mListenerCnt; i++)
                {
                        if (mpListeners[i] && mpListeners[i]->mpListenerPtr == &rListener)
                        {
                                mpListeners[i]->mRef--;
                                if (mpListeners[i]->mRef <= 0)
                                {
                                        delete mpListeners[i];
                                        for (int j = i; j < mListenerCnt; j++)
                                        {
                                                mpListeners[j] = mpListeners[j + 1];
                                        }
                                        mListenerCnt--;
                                        osPrintf("Remove listener in TaoListenerClientTask: 0x%p\n", &rListener);
                                        mListenerSem.release();
                                        return;
                                }
                        }
                }
        }

        mListenerSem.release();
}

UtlBoolean TaoListenerClientTask::getCallEvent(TaoMessage& rMsg,
                                               PtCallListener* pListener,
                                               TaoEventId& evId)
{
        int cnt = rMsg.getArgCnt();

        if (cnt < 1)
                return FALSE;

        PtEvent::PtEventId      eventId = (PtEvent::PtEventId) rMsg.getTaoObjHandle();
        if (!PtEvent::isCallEvent(eventId))
                return FALSE;

        TaoString argList(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString callId = argList[0];

        int addedToCall = 0;
        for (int i = 0; i < mListenerCnt; i++)
        {
                if (mpListeners[i] && mpListeners[i]->mpListenerPtr == pListener)
                {
                        if (!callId.isNull() && !(mpListeners[i]->mName.isNull()) && callId != mpListeners[i]->mName)
                        {
                                return FALSE;
                        }
                        addedToCall = 1;
                }
        }

        int metaCode = (cnt >= 10) ? atoi(argList[10]) : PtEvent::META_EVENT_NONE;
        int numOldCalls = (cnt > 10) ? (cnt - 11) : 0;

        UtlString* oldCallIds = 0;
        if (numOldCalls > 0)
        {       oldCallIds = new UtlString[numOldCalls];
                for (int i = 0; i < numOldCalls; i++)
                        oldCallIds[i] = argList[i + 11]; // 1st is the new call
        }

        int sipResponseCode = atoi(argList[7]);

        mpCallEvent->setEventId(eventId);
        mpCallEvent->setMetaCode((PtEvent::PtMetaCode)metaCode);
        mpCallEvent->setEventCallId(callId.data());                     // call id
        mpCallEvent->setEventSipResponseCode(sipResponseCode);          // SIP response code
        mpCallEvent->setEventSipResponseText(argList[8]);               // SIP response text
        mpCallEvent->setEventNewCallId(argList[10]);            // new call id
        mpCallEvent->setEventOldCallIds(numOldCalls, oldCallIds);

        if (oldCallIds) delete[] oldCallIds;

        evId = eventId;
        return TRUE;
}

UtlBoolean TaoListenerClientTask::getConnectionEvent(TaoMessage& rMsg,
                                                     PtConnectionListener* pListener,
                                                     TaoEventId& evId,
                                                     int& addedToCall,
                                                     int& remoteIsCallee)
{
        PtEvent::PtEventId      eventId = (PtEvent::PtEventId) rMsg.getTaoObjHandle();
        if (!PtEvent::isConnectionEvent(eventId))
                return FALSE;

        TaoString argList(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        int cnt = argList.getCnt();
        if (cnt < 5)
                return FALSE;

        UtlString callId = argList[0];

        for (int i = 0; i < mListenerCnt; i++)
        {
                if (mpListeners[i] && mpListeners[i]->mpListenerPtr == pListener)
                {
                        if (!callId.isNull() && !(mpListeners[i]->mName.isNull()) && callId != mpListeners[i]->mName)
                        {
                                return FALSE;
                        }
                        addedToCall = 1;
                }
        }

        PtEvent::PtEventCause cause = (PtEvent::PtEventCause) atoi(argList[4]);
        int metaCode = (cnt >= 10) ? atoi(argList[10]) : 0;
        int numOldCalls = (cnt > 10) ? (cnt - 11) : 0;

        UtlString* oldCallIds = 0;
        if (numOldCalls > 0)
        {       oldCallIds = new UtlString[numOldCalls];
                for (int i = 0; i < numOldCalls; i++)
                        oldCallIds[i] = argList[i + 11]; // 1st is the new call
        }

        int isLocal = atoi(argList[6]);
        UtlString addesss;
        if (isLocal)
                addesss = argList[1];           // local
        else
                addesss = argList[2];           // remote

        int sipResponseCode = atoi(argList[7]);

        mpConnEvent->setEventId(eventId);
        mpConnEvent->setMetaCode((PtEvent::PtMetaCode)metaCode);
        mpConnEvent->setEventAddress(addesss.data());                   // address
        mpConnEvent->setEventCallId(callId.data());                     // call id
        mpConnEvent->setEventSipResponseCode(sipResponseCode);          // SIP response code
        mpConnEvent->setEventSipResponseText(argList[8]);               // SIP response text
        mpConnEvent->setEventCause(cause);              // cause
        mpConnEvent->setEventNewCallId(argList[10]);            // new call id
        mpConnEvent->setEventOldCallIds(numOldCalls, oldCallIds);
        mpConnEvent->setEventLocal(isLocal);            // is local?

        if (oldCallIds) delete[] oldCallIds;

        evId = eventId;
        if (cnt > 3)
        {
                remoteIsCallee = atoi(argList[3]);
        }
        return TRUE;
}

UtlBoolean TaoListenerClientTask::getTerminalConnectionEvent(TaoMessage& rMsg,
                                                             PtConnectionListener* pListener,
                                                             TaoEventId& evId,
                                                             int& addedToCall,
                                                             int& remoteIsCallee)
{
        PtEvent::PtEventId      eventId = (PtEvent::PtEventId) rMsg.getTaoObjHandle();
        if (!PtEvent::isTerminalConnectionEvent(eventId))
                return FALSE;

        TaoString argList(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        int cnt = argList.getCnt();
        if (cnt < 7) {
                return FALSE;
        }

        UtlString callId = argList[0];

        for (int i = 0; i < mListenerCnt; i++)
        {
                if (mpListeners[i] && mpListeners[i]->mpListenerPtr == pListener)
                {
                        if (!callId.isNull() && !(mpListeners[i]->mName.isNull()) && callId != mpListeners[i]->mName)
                        {
                                return FALSE;
                        }
                        addedToCall = 1;
                }
        }

        PtEvent::PtEventCause cause = (PtEvent::PtEventCause) atoi(argList[4]);
        int metaCode = (cnt >= 10) ? atoi(argList[10]) : 0;
        int numOldCalls = (cnt > 10) ? (cnt - 11) : 0;

        UtlString* oldCallIds = 0;
        if (numOldCalls > 0)
        {       oldCallIds = new UtlString[numOldCalls];
                for (int i = 0; i < numOldCalls; i++)
                        oldCallIds[i] = argList[i + 11]; // 1st is the new call
        }

        int isLocal = atoi(argList[6]);
        UtlString addesss;
        if (isLocal)
                addesss = argList[1];           // local
        else
                addesss = argList[2];           // remote

        int sipResponseCode = atoi(argList[7]);

        mpTermConnEvent->setEventId(eventId);
        mpTermConnEvent->setMetaCode((PtEvent::PtMetaCode)metaCode);
        mpTermConnEvent->setEventAddress(addesss.data());                               // address
        mpTermConnEvent->setEventTerminal(argList[5]);                  // terminal name
        mpTermConnEvent->setEventCallId(callId.data());                                 // call id
        mpTermConnEvent->setEventSipResponseCode(sipResponseCode);              // SIP response code
        mpTermConnEvent->setEventSipResponseText(argList[8]);   // SIP response text
        mpTermConnEvent->setEventCause(cause);                                                  // cause
        mpTermConnEvent->setEventNewCallId(argList[10]);                        // new call id
        mpTermConnEvent->setEventOldCallIds(numOldCalls, oldCallIds);
        mpTermConnEvent->setEventTcLocal(isLocal);                                              // is local?
        mpTermConnEvent->setEventLocal(isLocal);                                                // is local?

        if (oldCallIds) delete[] oldCallIds;

        if (cnt > 3)
        {
                remoteIsCallee = atoi(argList[3]);
        }
        evId = eventId;

        return TRUE;
}

#ifdef WV_DEBUG
void TaoListenerClientTask::getEventName(int eventId, char *name)
{
        switch(eventId)
        {
                case PtEvent::CONNECTION_CREATED:
                        strcpy(name, (char*)"CREATED\0");
                        break;

                case PtEvent::CONNECTION_ALERTING:
                        strcpy(name, (char*)"ALERTING:\0");
                        break;

                case PtEvent::CONNECTION_DIALING:
                        strcpy(name, (char*)"DIALING\0");
                        break;

                case PtEvent::CONNECTION_ESTABLISHED:
                        strcpy(name, (char*)"ESTABLISHED\0");
                        break;

                case PtEvent::CONNECTION_INITIATED:
                        strcpy(name, (char*)"INITIATED\0");
                        break;

                case PtEvent::CONNECTION_DISCONNECTED:
                        strcpy(name, (char*)"DISCONNECTED\0");
                        break;

                case PtEvent::CONNECTION_FAILED:
                        strcpy(name, (char*)"FAILED\0");
                        break;

                case PtEvent::CONNECTION_NETWORK_ALERTING:
                        strcpy(name, (char*)"NETWORK_ALERTING\0");
                        break;

                case PtEvent::CONNECTION_NETWORK_REACHED:
                        strcpy(name, (char*)"NETWORK_REACHED\0");
                        break;

                case PtEvent::CONNECTION_OFFERED:
                        strcpy(name, (char*)"OFFERED\0");
                        break;

                case PtEvent::CONNECTION_QUEUED:
                        strcpy(name, (char*)"QUEUED\0");
                        break;

                case PtEvent::CONNECTION_UNKNOWN:
                        strcpy(name, (char*)"UNKNOWN\0");
                        break;

                case PtEvent::TERMINAL_CONNECTION_CREATED:
                        strcpy(name, (char*)"TC_CREATED\0");
                        break;

                case PtEvent::TERMINAL_CONNECTION_RINGING:
                        strcpy(name, (char*)"TC_RINGING\0");
                        break;

                case PtEvent::TERMINAL_CONNECTION_HELD:
                        strcpy(name, (char*)"TC_HELD\0");
                        break;

                case PtEvent::TERMINAL_CONNECTION_TALKING:
                        strcpy(name, (char*)"TC_TALKING\0");
                        break;

                case PtEvent::TERMINAL_CONNECTION_DROPPED:
                        strcpy(name, (char*)"TC_DROPPED\0");
                        break;

                case PtEvent::TERMINAL_CONNECTION_IDLE:
                        strcpy(name, (char*)"TC_IDLE\0");
                        break;

                case PtEvent::TERMINAL_CONNECTION_IN_USE:
                        strcpy(name, (char*)"TC_IN_USE\0");
                        break;

                case PtEvent::TERMINAL_CONNECTION_UNKNOWN:
                        strcpy(name, (char*)"TC_UNKNOWN\0");
                        break;

                case PtEvent::CALL_EVENT_TRANSMISSION_ENDED:
                        strcpy(name, (char*)"CL_TRANSMISSION_ENDED\0");
                        break;

                case PtEvent::CALL_ACTIVE:
                        strcpy(name, (char*)"CL_ACTIVE\0");
                        break;

                case PtEvent::CALL_INVALID:
                        strcpy(name, (char*)"CL_INVALID\0");
                        break;

                case PtEvent::CALL_META_CALL_STARTING_STARTED:
                        strcpy(name, (char*)"CL_META_STARTING_STARTED\0");
                        break;

                case PtEvent::CALL_META_CALL_STARTING_ENDED:
                        strcpy(name, (char*)"CL_META_STARTING_ENDED\0");
                        break;

                case PtEvent::CALL_META_CALL_ENDING_STARTED:
                        strcpy(name, (char*)"CL_META_ENDING_STARTED\0");
                        break;

                case PtEvent::CALL_META_CALL_ENDING_ENDED:
                        strcpy(name, (char*)"CL_META_ENDING_ENDED\0");
                        break;

                case PtEvent::SINGLECALL_META_PROGRESS_STARTED:
                        strcpy(name, (char*)"CL_META_PROGRESS_STARTED\0");
                        break;

                case PtEvent::SINGLECALL_META_PROGRESS_ENDED:
                        strcpy(name, (char*)"CL_META_PROGRESS_ENDED\0");
                        break;

                case PtEvent::SINGLECALL_META_SNAPSHOT_STARTED:
                        strcpy(name, (char*)"CL_META_SNAPSHOT_STARTED\0");
                        break;

                case PtEvent::SINGLECALL_META_SNAPSHOT_ENDED:
                        strcpy(name, (char*)"CL_META_SNAPSHOT_ENDED\0");
                        break;

                case PtEvent::CALL_META_ADD_PARTY_STARTED:
                        strcpy(name, (char*)"CL_META_ADD_PARTY_STARTED\0");
                        break;

                case PtEvent::CALL_META_ADD_PARTY_ENDED:
                        strcpy(name, (char*)"CL_META_ADD_PARTY_ENDED\0");
                        break;

                case PtEvent::CALL_META_REMOVE_PARTY_STARTED:
                        strcpy(name, (char*)"CL_META_REMOVE_PARTY_STARTED\0");
                        break;

                case PtEvent::CALL_META_REMOVE_PARTY_ENDED:
                        strcpy(name, (char*)"CL_META_REMOVE_PARTY_ENDED\0");
                        break;

                case PtEvent::MULTICALL_META_MERGE_STARTED:
                        strcpy(name, (char*)"CL_META_MERGE_STARTED\0");
                        break;

                case PtEvent::MULTICALL_META_MERGE_ENDED:
                        strcpy(name, (char*)"CL_META_MERGE_ENDED\0");
                        break;

                case PtEvent::MULTICALL_META_TRANSFER_STARTED:
                        strcpy(name, (char*)"CL_META_TRANSFER_STARTED\0");
                        break;

                case PtEvent::MULTICALL_META_TRANSFER_ENDED:
                        strcpy(name, (char*)"CL_META_TRANSFER_ENDED\0");
                        break;

                default:
                        strcpy(name, (char*)"BOGUS EVENT\0");
                        break;
        }
}

void TaoListenerClientTask::fireUserEvent(TaoEventId eventId, TaoEventId userEventId)
{
        getEventName(eventId, (char*)&name);

        USER_EVENT myEv;
        myEv.id = eventId;
        strncpy(myEv.name, name, 16);
#ifdef _VXWORKS
        wvEvent(userEventId, (char*)&myEv, sizeof(USER_EVENT));
#endif
}
#endif
