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
#include <string.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

// APPLICATION INCLUDES
#include "cp/CpGatewayManager.h"
#include "os/OsStatus.h"
#include "ptapi/PtTerminal.h"
#include "ptapi/PtDefs.h"
#include "ptapi/PtProvider.h"
#include "ptapi/PtAddress.h"
#include "ptapi/PtCallListener.h"
#include "ptapi/PtPhoneButton.h"
#include "ptapi/PtPhoneDisplay.h"
#include "ptapi/PtPhoneGraphicDisplay.h"
#include "ptapi/PtPhoneHookswitch.h"
#include "ptapi/PtPhoneLamp.h"
#include "ptapi/PtPhoneMicrophone.h"
#include "ptapi/PtPhoneRinger.h"
#include "ptapi/PtPhoneSpeaker.h"
#include "ptapi/PtPhoneExtSpeaker.h"
#include "ptapi/PtTerminalListener.h"
#include "ptapi/PtComponent.h"
#include "ptapi/PtComponentGroup.h"
#include "ptapi/PtTerminalConnection.h"
#include "tao/TaoClientTask.h"
#include "tao/TaoServerTask.h"
#include "tao/TaoEvent.h"
#include "tao/TaoString.h"
#include "tao/TaoObjectMap.h"
#include "ps/PsTaoComponent.h"
#include "ps/PsTaoComponentGroup.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
OsBSem              PtTerminal::semInit(OsBSem::Q_PRIORITY, OsBSem::FULL) ;
TaoReference       *PtTerminal::mpTransactionCnt = 0;
TaoObjectMap       *PtTerminal::mpComponents = 0;
TaoObjectMap       *PtTerminal::mpComponentGroups = 0;
int                                PtTerminal::mRef = 0;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Default Constructor
PtTerminal::PtTerminal()
        : mpClient(NULL)
{
        initialize(NULL);
}

// Protected constructor.
PtTerminal::PtTerminal(const char* name, TaoClientTask *pClient)
{
        mpClient = pClient;
        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }

        initialize(name);
}

// Copy constructor
PtTerminal::PtTerminal(const PtTerminal& rPtTerminal)
        : mpClient(NULL)
{
    const char *name = NULL;
        if (rPtTerminal.mTerminalName[0])
        {
        name = rPtTerminal.mTerminalName;
        }

        mpClient = rPtTerminal.mpClient;
        initialize(name);
}

// Destructor
PtTerminal::~PtTerminal()
{
    semInit.acquire() ;
        mRef--;
        if (mRef < 1)
        {
                if (mpComponents)
                {
                        int num = mpComponents->numEntries();
                        TaoObjHandle *objs;
                        objs = new TaoObjHandle[num];

                        mpComponents->getActiveObjects(objs, num);
                        for (int i = 0; i < num; i++)
                                delete (PtComponent *) objs[i];

                        delete[] objs;
                        delete mpComponents;
                        mpComponents = 0;
                }

                if (mpComponentGroups)
                {
                        int num = mpComponentGroups->numEntries();
                        TaoObjHandle *objs;
                        objs = new TaoObjHandle[num];

                        mpComponentGroups->getActiveObjects(objs, num);
                        for (int i = 0; i < num; i++)
                                delete (PtComponentGroup *) objs[i];

                        delete[] objs;
                        delete mpComponentGroups;
                        mpComponentGroups = 0;
                }

                if (mpTransactionCnt)
                {
                        delete mpTransactionCnt;
                        mpTransactionCnt = 0;
                }
        }
    semInit.release() ;

}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtTerminal&
PtTerminal::operator=(const PtTerminal& rhs)
{
        if (this == &rhs)            // handle the assignment to self case
          return *this;

    setName(rhs.mTerminalName);

        mpClient = rhs.mpClient;

        mTimeOut = rhs.mTimeOut;

        return *this;
}

PtStatus PtTerminal::addCallListener(PtCallListener& rCallListener)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        UtlString arg;
        UtlString local;
        rCallListener.getLocation(&local);

        arg = local.data() + TAOMESSAGE_DELIMITER + mTerminalName;

        unsigned int transactionId = mpTransactionCnt->add();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::ADD_CALL_LISTENER,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t rc;
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getEventData(rc);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::ADD_CALL_LISTENER);
#endif
        mpEventMgr->release(pe);

        mpClient->addEventListener(&rCallListener);
        return PT_SUCCESS;
}

PtStatus PtTerminal::addTerminalListener(PtTerminalListener& rTerminalListener)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        int argCnt = 1;
        TaoObjHandle handle = (TaoObjHandle) &rTerminalListener;
    char buff[128];
        UtlString name;
        UtlString arg;

        if (PT_SUCCESS == rTerminalListener.getTerminalName(buff, 127)) // must have the terminal name
        {
                name.append(buff);
                argCnt = 2;
                sprintf(buff, "%ld", handle);
                arg = name + TAOMESSAGE_DELIMITER  + buff;
        }

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::ADD_TERM_LISTENER,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        argCnt,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t rc;
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getEventData(rc);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::ADD_TERM_LISTENER);
#endif
        mpEventMgr->release(pe);

        mpClient->addEventListener(&rTerminalListener);
        return PT_SUCCESS;
}


/**
* getAddresses() basically does the same thing as
* PtProvider:getAddresses() in this implementation.
* It returns the user lines as addresses.
*/
PtStatus PtTerminal::getAddresses(PtAddress arAddresses[],
                                                                   int size,
                                                                   int& nItems)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", size);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::GET_ADDRESSES,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        arg);
        mpClient->sendRequest(msg);

        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        intptr_t temp;
        pe->getEventData(temp);
        nItems = (int)temp;
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_ADDRESSES);
#endif
        mpEventMgr->release(pe);

        int actual = ((size < nItems) ? size : nItems);

        TaoString taoStr(arg, TAOMESSAGE_DELIMITER);

        for (int i = 0; i < actual; i++)
        {
                arAddresses[i] = PtAddress(mpClient, taoStr[i]);
        }


        return PT_SUCCESS;
}

PtStatus PtTerminal::getCallListeners(PtCallListener aCallListeners[],
                                                                           int size,
                                                                           int& nItems)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", size);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::GET_CALL_LISTENERS,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t rc;
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getEventData(rc);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_CALL_LISTENERS);
#endif
        mpEventMgr->release(pe);
        return PT_SUCCESS;
}

PtStatus PtTerminal::getComponent(const char* componentName, PtComponent& rComponent)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        if (!componentName)
                return PT_RESOURCE_UNAVAILABLE;

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::GET_COMPONENT,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        componentName);
        mpClient->sendRequest(msg);

        UtlString arg;
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_COMPONENT);
#endif
        mpEventMgr->release(pe);

        TaoString list(arg, TAOMESSAGE_DELIMITER);

        int nItems = atoi(list[0]);
        if (nItems == 1)
        {
                int type = atoi(list[1]);

                switch (type)
                {
                case PtComponent::BUTTON:
                        rComponent = PtPhoneButton(mpClient);
                        break;
                case PtComponent::DISPLAY:
                        rComponent = PtPhoneDisplay(mpClient);
                        break;
                case PtComponent::GRAPHIC_DISPLAY:
                        rComponent = PtPhoneGraphicDisplay(mpClient);
                        break;
                case PtComponent::HOOKSWITCH:
                        rComponent = PtPhoneHookswitch(mpClient);
                        break;
                case PtComponent::LAMP:
                        rComponent = PtPhoneLamp(mpClient);
                        break;
                case PtComponent::MICROPHONE:
                        rComponent = PtPhoneMicrophone(mpClient);
                        break;
                case PtComponent::RINGER:
                        rComponent = PtPhoneRinger(mpClient);
                        break;
                case PtComponent::SPEAKER:
                        rComponent = PtPhoneSpeaker(mpClient);
                        break;
                case PtComponent::EXTERNAL_SPEAKER:
                        rComponent = PtPhoneExtSpeaker(mpClient);
                        break;
                default:
                        break;
                }
        }

        return PT_SUCCESS;
}

PtStatus PtTerminal::getComponentGroups(PtComponentGroup componentGroup[],
                                                                                         int size,
                                                                                         int& nItems)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        if (mpComponentGroups)
        {
                int num = mpComponentGroups->numEntries();
                if (num > 0)
                {
                        TaoObjHandle *objs;
                        objs = new TaoObjHandle[num];

                        mpComponentGroups->getActiveObjects(objs, num);
                        osPrintf("PtTerminal::getComponentGroups found %d component groups.\n", num);
                        if (num > size)
                                num = size;
                        nItems = num;
                        for (int i = 0; i < num; i++)
                        {
                                componentGroup[i] = *((PtComponentGroup *) objs[i]);
                        }
                        delete[] objs;

                        return PT_SUCCESS;
                }
        }

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", size);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::GET_COMPONENTGROUPS,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        arg);

        mpClient->sendRequest(msg);

        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_COMPONENTGROUPS);
#endif
        mpEventMgr->release(pe);

        TaoString types(arg, TAOMESSAGE_DELIMITER);

        nItems = atoi(types[0]);
        if (nItems > 0)
        {
                PtComponent* components[9];

                int max = (size < nItems) ? size : nItems;
                for (int i = 0; i < max; i++)
                {
                        int type = atoi(types[i + 1]);
                        TaoObjHandle objValue;

                        switch (type)
                        {
                        case PsTaoComponentGroup::HEAD_SET:
                                if (TAO_NOT_FOUND == mpComponentGroups->findValue(HEAD_SET, objValue))
                                {
                                        PtPhoneMicrophone* pMicrophone = new PtPhoneMicrophone(mpClient);
                                        PtPhoneSpeaker* pSpeaker = new PtPhoneSpeaker(mpClient);
                                        pMicrophone->setGroupType(PtComponentGroup::HEAD_SET);
                                        pSpeaker->setGroupType(PtComponentGroup::HEAD_SET);
                                        components[0] = pMicrophone;
                                        components[1] = pSpeaker;

                                        PtComponentGroup *pGroup = new PtComponentGroup(PtComponentGroup::HEAD_SET,
                                                                                                        "HEAD_SET",
                                                                                                        components,
                                                                                                        2);
                                        pGroup->setTaoClient(mpClient);
                                        componentGroup[i] = *pGroup;
                                        mpComponentGroups->insert(HEAD_SET, (TaoObjHandle)pGroup);
                                }
                                else
                                {
                                        PtComponentGroup *pComponent = (PtComponentGroup *) objValue;
                                        componentGroup[i] = *pComponent;
                                }
                                break;
                        case PsTaoComponentGroup::HAND_SET:
                                if (TAO_NOT_FOUND == mpComponentGroups->findValue(HAND_SET, objValue))
                                {
                                        PtPhoneMicrophone* pMicrophone = new PtPhoneMicrophone(mpClient);
                                        PtPhoneSpeaker* pSpeaker = new PtPhoneSpeaker(mpClient);
                                        pMicrophone->setGroupType(PtComponentGroup::HAND_SET);
                                        pSpeaker->setGroupType(PtComponentGroup::HAND_SET);
                                        components[0] = pMicrophone;
                                        components[1] = pSpeaker;
                                        PtComponentGroup *pGroup = new PtComponentGroup(PtComponentGroup::HAND_SET,
                                                                                                        "HAND_SET",
                                                                                                        components,
                                                                                                        2);
                                        pGroup->setTaoClient(mpClient);
                                        componentGroup[i] = *pGroup;
                                        mpComponentGroups->insert(HAND_SET, (TaoObjHandle)pGroup);
                                }
                                else
                                {
                                        PtComponentGroup *pComponent = (PtComponentGroup *) objValue;
                                        componentGroup[i] = *pComponent;
                                }
                                break;
                        case PsTaoComponentGroup::SPEAKER_PHONE:
                                if (TAO_NOT_FOUND == mpComponentGroups->findValue(SPEAKER_PHONE, objValue))
                                {
                                        PtPhoneMicrophone* pMicrophone = new PtPhoneMicrophone(mpClient);
                                        PtPhoneSpeaker* pSpeaker = new PtPhoneSpeaker(mpClient);
                                        pMicrophone->setGroupType(PtComponentGroup::SPEAKER_PHONE);
                                        pSpeaker->setGroupType(PtComponentGroup::SPEAKER_PHONE);
                                        components[0] = pMicrophone;
                                        components[1] = pSpeaker;
                                        PtComponentGroup *pGroup = new PtComponentGroup(PtComponentGroup::SPEAKER_PHONE,
                                                                                                        "SPEAKER_PHONE",
                                                                                                        components,
                                                                                                        2);
                                        pGroup->setTaoClient(mpClient);
                                        componentGroup[i] = *pGroup;
                                        mpComponentGroups->insert(SPEAKER_PHONE, (TaoObjHandle)pGroup);
                                }
                                else
                                {
                                        PtComponentGroup *pComponent = (PtComponentGroup *) objValue;
                                        componentGroup[i] = *pComponent;
                                }
                                break;
                        case PsTaoComponentGroup::EXTERNAL_SPEAKER:
                                if (TAO_NOT_FOUND == mpComponentGroups->findValue(EXTERNAL_SPEAKER, objValue))
                                {
                                        PtPhoneExtSpeaker* pSpeaker = new PtPhoneExtSpeaker(mpClient);
                                        pSpeaker->setGroupType(PtComponentGroup::EXTERNAL_SPEAKER);
                                        components[0] = pSpeaker;
                                        PtComponentGroup *pGroup = new PtComponentGroup(PtComponentGroup::SPEAKER_PHONE,
                                                                                                        "SPEAKER_PHONE",
                                                                                                        components,
                                                                                                        1);
                                        pGroup->setTaoClient(mpClient);
                                        componentGroup[i] = *pGroup;
                                        mpComponentGroups->insert(EXTERNAL_SPEAKER, (TaoObjHandle)pGroup);
                                }
                                else
                                {
                                        PtComponentGroup *pComponent = (PtComponentGroup *) objValue;
                                        componentGroup[i] = *pComponent;
                                }
                                break;
                        case PsTaoComponentGroup::PHONE_SET:
                                if (TAO_NOT_FOUND == mpComponentGroups->findValue(PHONE_SET, objValue))
                                {
                                        PtPhoneMicrophone* pMicrophone = new PtPhoneMicrophone(mpClient);
                                        PtPhoneSpeaker* pSpeaker = new PtPhoneSpeaker(mpClient);
                                        PtPhoneButton* pButton = new PtPhoneButton(mpClient);
                                        PtPhoneDisplay* pDisplay = new PtPhoneDisplay(mpClient);
                                        PtPhoneHookswitch* pHookswitch = new PtPhoneHookswitch(mpClient);
                                        PtPhoneLamp* pLamp = new PtPhoneLamp(mpClient);
                                        PtPhoneRinger* pRinger = new PtPhoneRinger(mpClient);

                                        pMicrophone->setGroupType(PtComponentGroup::PHONE_SET);
                                        pSpeaker->setGroupType(PtComponentGroup::PHONE_SET);
                                        pButton->setGroupType(PtComponentGroup::PHONE_SET);
                                        pDisplay->setGroupType(PtComponentGroup::PHONE_SET);
                                        pHookswitch->setGroupType(PtComponentGroup::PHONE_SET);
                                        pLamp->setGroupType(PtComponentGroup::PHONE_SET);
                                        pRinger->setGroupType(PtComponentGroup::PHONE_SET);

                                        components[0] = pMicrophone;
                                        components[1] = pSpeaker;
                                        components[2] = pButton;
                                        components[3] = pDisplay;
                                        components[4] = pHookswitch;
                                        components[5] = pLamp;
                                        components[6] = pRinger;
                                        PtComponentGroup *pGroup = new PtComponentGroup(PtComponentGroup::PHONE_SET,
                                                                                                        "PHONE_SET",
                                                                                                        components,
                                                                                                        7);
                                        pGroup->setTaoClient(mpClient);
                                        componentGroup[i] = *pGroup;
                                        mpComponentGroups->insert(PHONE_SET, (TaoObjHandle)pGroup);
                                }
                                else
                                {
                                        PtComponentGroup *pComponent = (PtComponentGroup *) objValue;
                                        componentGroup[i] = *pComponent;
                                }
                                break;
                        case PsTaoComponentGroup::OTHER:
                                if (TAO_NOT_FOUND == mpComponentGroups->findValue(OTHER, objValue))
                                {
                                        PtPhoneMicrophone* pMicrophone = new PtPhoneMicrophone(mpClient);
                                        PtPhoneSpeaker* pSpeaker = new PtPhoneSpeaker(mpClient);
                                        pMicrophone->setGroupType(PtComponentGroup::OTHER);
                                        pSpeaker->setGroupType(PtComponentGroup::OTHER);
                                        components[0] = pMicrophone;
                                        components[1] = pSpeaker;
                                        PtComponentGroup *pGroup = new PtComponentGroup(PtComponentGroup::OTHER,
                                                                                                        "OTHER",
                                                                                                        components,
                                                                                                        2);
                                        pGroup->setTaoClient(mpClient);
                                        componentGroup[i] = *pGroup;
                                        mpComponentGroups->insert(OTHER, (TaoObjHandle)pGroup);
                                }
                                else
                                {
                                        PtComponentGroup *pComponent = (PtComponentGroup *) objValue;
                                        componentGroup[i] = *pComponent;
                                }
                                break;
                        default:
                                break;
                        }
                }
        }

        return PT_SUCCESS;
}

PtStatus PtTerminal::getComponents(PtComponent* components[],
                                                                        int size,
                                                                        int& nItems)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        if (mpComponents)
        {
                int num = mpComponents->numEntries();
                if (num > 0)
                {
                        TaoObjHandle *objs;
                        objs = new TaoObjHandle[num];

                        mpComponents->getActiveObjects(objs, num);
                        if (num > size)
                                num = size;
                        nItems = num;
                        for (int i = 0; i < num; i++)
                        {
                                components[i] = (PtComponent *) objs[i];
                        }
                        delete[] objs;

                        return PT_SUCCESS;
                }
        }

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", size);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::GET_COMPONENTS,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        arg);
        mpClient->sendRequest(msg);

        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getStringData((UtlString &)arg);
        intptr_t temp;
        pe->getIntData(temp);
        nItems = (int)temp;
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_COMPONENTS);
#endif
        mpEventMgr->release(pe);

        TaoString types(arg, TAOMESSAGE_DELIMITER);

        if (nItems > 0)
        {
                PtPhoneButton *pButton;
                PtPhoneDisplay *pDisplay;
                PtPhoneGraphicDisplay *pGraphicDisplay;
                PtPhoneHookswitch *pHookswitch;
                PtPhoneLamp *pLamp;
                PtPhoneMicrophone* pMicrophone;
                PtPhoneRinger *pRinger;
                PtPhoneSpeaker *pSpeaker;

                int max = (size < nItems) ? size : nItems;
                for (int i = 0; i < max; i++)
                {
                        int type = atoi(types[i]);
                        TaoObjHandle objValue;

                        switch (type)
                        {
                        case PsTaoComponent::BUTTON:
                                {
                                        UtlString name;
                                        for (; i < nItems; i++)
                                        {
                                                name = types[i + 1];
                                                pButton = new PtPhoneButton(mpClient, name.data());
                                                components[i] = pButton;
                                                mpComponents->insert((TaoObjHandle)pButton, (TaoObjHandle)pButton);
                                        }
                                }
                                break;
                        case PsTaoComponent::DISPLAY:
                                if (TAO_NOT_FOUND == mpComponents->findValue((TaoObjHandle)DISPLAY, objValue))
                                {
                                        osPrintf("PtTerminal:: Creating PsTaoComponent::DISPLAY\n") ;
                                        pDisplay = new PtPhoneDisplay(mpClient);
                                        components[i] = pDisplay;
                                        mpComponents->insert((TaoObjHandle)DISPLAY, (TaoObjHandle)pDisplay);
                                }
                                else
                                {
                                        components[i] = (PtComponent *) objValue;
                                }
                                break;
                        case PsTaoComponent::GRAPHIC_DISPLAY:
                                if (TAO_NOT_FOUND == mpComponents->findValue((TaoObjHandle)GRAPHIC_DISPLAY, objValue))
                                {
                                        pGraphicDisplay = new PtPhoneGraphicDisplay(mpClient);
                                        components[i] = pGraphicDisplay;
                                        mpComponents->insert((TaoObjHandle)GRAPHIC_DISPLAY, (TaoObjHandle)pGraphicDisplay);
                                }
                                else
                                {
                                        components[i] = (PtComponent *) objValue;
                                }
                                break;
                        case PsTaoComponent::HOOKSWITCH:
                                if (TAO_NOT_FOUND == mpComponents->findValue((TaoObjHandle)HOOKSWITCH, objValue))
                                {
                                        pHookswitch = new PtPhoneHookswitch(mpClient);
                                        components[i] = pHookswitch;
                                        mpComponents->insert((TaoObjHandle)HOOKSWITCH, (TaoObjHandle)pHookswitch);
                                }
                                else
                                {
                                        components[i] = (PtComponent *) objValue;
                                }
                                break;
                        case PsTaoComponent::LAMP:
                                if (TAO_NOT_FOUND == mpComponents->findValue((TaoObjHandle)LAMP, objValue))
                                {
                                        pLamp = new PtPhoneLamp(mpClient);
                                        components[i] = pLamp;
                                        mpComponents->insert((TaoObjHandle)LAMP, (TaoObjHandle)pLamp);
                                }
                                else
                                {
                                        components[i] = (PtComponent *) objValue;
                                }
                                break;
                        case PsTaoComponent::MICROPHONE:
                                if (TAO_NOT_FOUND == mpComponents->findValue((TaoObjHandle)MICROPHONE, objValue))
                                {
                                        pMicrophone = new PtPhoneMicrophone(mpClient);
                                        components[i] = pMicrophone;
                                        mpComponents->insert((TaoObjHandle)MICROPHONE, (TaoObjHandle)pMicrophone);
                                }
                                else
                                {
                                        components[i] = (PtComponent *) objValue;
                                }
                                break;
                        case PsTaoComponent::RINGER:
                                if (TAO_NOT_FOUND == mpComponents->findValue((TaoObjHandle)RINGER, objValue))
                                {
                                        pRinger = new PtPhoneRinger(mpClient);
                                        components[i] = pRinger;
                                        mpComponents->insert((TaoObjHandle)RINGER, (TaoObjHandle)pRinger);
                                }
                                else
                                {
                                        components[i] = (PtComponent *) objValue;
                                }
                                break;
                        case PsTaoComponent::SPEAKER:
                                if (TAO_NOT_FOUND == mpComponents->findValue((TaoObjHandle)SPEAKER, objValue))
                                {
                                        pSpeaker = new PtPhoneSpeaker(mpClient);
                                        components[i] = pSpeaker;
                                        mpComponents->insert((TaoObjHandle)SPEAKER, (TaoObjHandle)pSpeaker);
                                }
                                else
                                {
                                        components[i] = (PtComponent *) objValue;
                                }
                                break;
                        default:
                                break;
                        }
                }
        }

        return PT_SUCCESS;
}

PtStatus PtTerminal::getConfiguration(PtConfigDb& rpConfigDb)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::GET_CONFIG,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        0,
                                                                        "");
        mpClient->sendRequest(msg);

        intptr_t rc;
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getEventData(rc);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_CONFIG);
#endif
        mpEventMgr->release(pe);
        return PT_SUCCESS;
}

PtStatus PtTerminal::getDoNotDisturb(PtBoolean& rFlag)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::GET_DONOT_DISTURB,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        0,
                                                                        "");
        mpClient->sendRequest(msg);

        intptr_t rc;
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getEventData(rc);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_DONOT_DISTURB);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtTerminal::getTerminalConnections(PtTerminalConnection termConnections[],
                                                                                         int size,
                                                                                         int& nItems)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        UtlString arg(mTerminalName);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::GET_TERM_CONNECTIONS,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        arg);
        mpClient->sendRequest(msg);

        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        arg.remove(0);
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_TERM_CONNECTIONS);
#endif
        mpEventMgr->release(pe);

        TaoString argList(arg, TAOMESSAGE_DELIMITER);
        nItems = (argList.getCnt() - 2)/2;

        if (nItems < 1)
                return PT_NO_MORE_DATA;

        int cnt = (nItems > size) ? size : nItems;

        nItems = 0;
        for (int i = 0; i < cnt; i++)
        {
                UtlString name = argList[2 * i + 2];
                int isLocal = atoi(argList[2 * i + 3]);
                PtTerminalConnection tc = PtTerminalConnection(mpClient,
                                                                                                        mTerminalName,
                                                                                                        name.data(),
                                                                                                        mTerminalName,
                                                                                                        isLocal);
                termConnections[i] = tc;
                nItems++;
        }

        return PT_SUCCESS;
}

PtStatus PtTerminal::getTerminalListeners(PtTerminalListener aTermListeners[],
                                                                                   int size,
                                                                                   int& nItems)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", size);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::GET_TERM_LISTENERS,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t rc;
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getEventData(rc);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_TERM_LISTENERS);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtTerminal::getProvider(PtProvider& rProvider)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::GET_PROVIDER,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        0,
                                                                        "");
        mpClient->sendRequest(msg);

        intptr_t rc;
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getEventData(rc);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::GET_PROVIDER);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtTerminal::numAddresses(int& count)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        UtlString arg(mTerminalName);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::NUM_ADDRESSES,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        arg);
        mpClient->sendRequest(msg);

        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        arg.remove(0);
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::NUM_ADDRESSES);
#endif
        mpEventMgr->release(pe);

        if (!arg.isNull())
                count = atoi(arg);
        else
                count = 0;

        return PT_SUCCESS;
}

PtStatus PtTerminal::numCallListeners(int& count)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        UtlString arg(mTerminalName);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::NUM_CALL_LISTENERS,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        arg);

        mpClient->sendRequest(msg);

        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        arg.remove(0);
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::NUM_CALL_LISTENERS);
#endif
        mpEventMgr->release(pe);

        if (!arg.isNull())
                count = atoi(arg);
        else
                count = 0;

        return PT_SUCCESS;
}

PtStatus PtTerminal::numComponents(int& count)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        UtlString arg(mTerminalName);

        // record this transaction in the transaction db
        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::NUM_COMPONENTS,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        arg);
        mpClient->sendRequest(msg);

        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        arg.remove(0);
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::NUM_COMPONENTS);
#endif
        mpEventMgr->release(pe);

        if (!arg.isNull())
                count = atoi(arg);
        else
                count = 0;

        return PT_SUCCESS;
}

PtStatus PtTerminal::numTerminalListeners(int& count)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        UtlString arg(mTerminalName);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::NUM_TERM_LISTENERS,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        arg);
        mpClient->sendRequest(msg);

        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        arg.remove(0);
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::NUM_TERM_LISTENERS);
#endif
        mpEventMgr->release(pe);

        if (!arg.isNull())
                count = atoi(arg);
        else
                count = 0;

        return PT_SUCCESS;
}

PtStatus PtTerminal::numTerminalConnections(int& count)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        UtlString arg(mTerminalName);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::NUM_TERM_CONNECTIONS,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        arg);
        mpClient->sendRequest(msg);

        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        arg.remove(0);
        pe->getStringData((UtlString &)arg);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::NUM_TERM_CONNECTIONS);
#endif
        mpEventMgr->release(pe);

        if (!arg.isNull())
                count = atoi(arg);
        else
                count = 0;

        return PT_SUCCESS;
}

PtStatus PtTerminal::pickup(PtAddress& rPickupAddress,
                                                         PtAddress& rTerminalAddress,
                                                         PtTerminalConnection*& rpNewTermConnection)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        UtlString addr = "9385306"; //fake
        UtlString term = "12.1.1.1"; //fake
        UtlString arg = addr + TAOMESSAGE_DELIMITER + term;


        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::PICKUP,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t rc;
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getEventData(rc);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::PICKUP);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtTerminal::removeCallListener(PtCallListener& rCallListener)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        UtlString arg;
        UtlString local;
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%" PRIdPTR, (intptr_t)&rCallListener);

        rCallListener.getLocation(&local);

        arg = local + TAOMESSAGE_DELIMITER + buff;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::REMOVE_CALL_LISTENER,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t rc;
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getEventData(rc);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::REMOVE_CALL_LISTENER);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

PtStatus PtTerminal::removeTerminalListener(PtTerminalListener& rTerminalListener)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

        TaoObjHandle handle = (TaoObjHandle) &rTerminalListener;
    char buff[128];
        UtlString name;
        UtlString arg;

        if (PT_SUCCESS == rTerminalListener.getTerminalName(buff, 127)) // must have the terminal name
        {
                name.append(buff);
                sprintf(buff, "%ld", handle);
                arg = name + TAOMESSAGE_DELIMITER  + buff;
        }
        else
                return PT_INVALID_ARGUMENT;

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::REMOVE_TERM_LISTENER,
                                                                        transactionId,
                                                                        (TaoObjHandle)NULL,
                                                                        (TaoObjHandle)pe,
                                                                        2,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t rc;
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getEventData(rc);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::REMOVE_TERM_LISTENER);
#endif
        mpEventMgr->release(pe);

        mpClient->removeEventListener(rTerminalListener);
        return PT_SUCCESS;
}

PtStatus PtTerminal::setDoNotDisturb(PtBoolean flag)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", flag);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
                                                                        TaoMessage::SET_DONOT_DISTURB,
                                                                        transactionId,
                                                                        0, //NULL
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t rc;
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getEventData(rc);
#ifdef PTAPI_TEST
        intptr_t cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::SET_DONOT_DISTURB);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}


// Sets the codec CPU limit level for inbound calls.
PtStatus PtTerminal::setCodecCPULimit(int limit)
{
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

   char buff[MAXIMUM_INTEGER_STRING_LENGTH];

   sprintf(buff, "%d", limit);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_TERMINAL,
         TaoMessage::SET_INBOUND_CODEC_CPU_LIMIT,
                        transactionId,
                        0, //NULL
                        (TaoObjHandle)pe,
                   1,
                        arg);
        mpClient->sendRequest(msg);

        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::SET_INBOUND_CODEC_CPU_LIMIT);
#endif
        mpEventMgr->release(pe);

        return PT_SUCCESS;
}

/* ============================ ACCESSORS ================================= */
// Return name associated with the terminal.
PtStatus PtTerminal::getName(char* rpName, int maxLen)
{
        if (rpName && maxLen > 0)
        {
                if (mTerminalName)
                {
                        int bytes = strlen(mTerminalName);
                        bytes = (bytes > maxLen) ? maxLen : bytes;
                        memset(rpName, 0, maxLen);
                        strncpy (rpName, mTerminalName, bytes);
                        return PT_SUCCESS;
                }
        }

        return PT_FAILED ;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void PtTerminal::initialize(const char *name)
{
    setName(name);

        mpEventMgr = OsProtectEventMgr::getEventMgr();
        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
    semInit.acquire() ;
        mRef++;

        // Initialize static variables
        if(!mpComponents)
                mpComponents = new TaoObjectMap();

        if(!mpComponentGroups)
                mpComponentGroups = new TaoObjectMap();

        if (!mpTransactionCnt)
                mpTransactionCnt = new TaoReference();

    semInit.release() ;
}
/* //////////////////////////// PRIVATE /////////////////////////////////// */

void PtTerminal::setName(const char *name)
{
        memset(mTerminalName, 0, PTTERMINAL_MAX_NAME_LENGTH + 1);
        if (name)
        {
                int len = strlen(name);

                if (len > PTTERMINAL_MAX_NAME_LENGTH)
        {
            len = PTTERMINAL_MAX_NAME_LENGTH;
        }
                strncpy(mTerminalName, name, len);
                mTerminalName[len] = 0;
        }
}


/* ============================ FUNCTIONS ================================= */
