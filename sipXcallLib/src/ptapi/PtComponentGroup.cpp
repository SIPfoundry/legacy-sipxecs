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

// APPLICATION INCLUDES
#include "ptapi/PtComponentGroup.h"
#include "ptapi/PtComponent.h"
#include "ptapi/PtPhoneSpeaker.h"
#include "ptapi/PtPhoneExtSpeaker.h"
#include "ptapi/PtPhoneRinger.h"
#include "tao/TaoEvent.h"
#include "tao/TaoString.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
OsBSem               PtComponentGroup::semInit(OsBSem::Q_PRIORITY, OsBSem::FULL) ;
TaoReference            *PtComponentGroup::mpTransactionCnt = 0;
int                                     PtComponentGroup::mRef = 0;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtComponentGroup::PtComponentGroup() :
mpComponents(0),
mGroupType(0),
mIsActivated(0),
mNumItems(0),
mpClient(0),
mComponentRef(0)
{
        initialize();
        mDescription.append("UNKNOWN");
}

PtComponentGroup::PtComponentGroup(int groupType, const UtlString& rDescription,
                                                                                 PtComponent* pComponents[], int nItems) :
mpComponents(0),
mGroupType(groupType),
mIsActivated(0),
mNumItems(nItems),
mpClient(0),
mComponentRef(0)
{

        initialize();

        mDescription.remove(0);

        switch (groupType)
        {
        case HEAD_SET:
                mDescription.append("HEAD_SET");
                break;
        case HAND_SET:
                mDescription.append("HAND_SET");
                break;
        case SPEAKER_PHONE:
                mDescription.append("SPEAKER_PHONE");
                break;
        case PHONE_SET:
                mDescription.append("PHONE_SET");
                break;
        case EXTERNAL_SPEAKER:
                mDescription.append("EXTERNAL_SPEAKER");
                break;
        case OTHER:
                mDescription.append("OTHER");
                break;
        default:
                mDescription.append("UNKNOWN");
                break;
        }

        if (pComponents && nItems > 0)
        {
                mComponentRef++;
                mpComponents = new PtComponent*[nItems];
                if (mpComponents)
                {
                        mNumItems = nItems;
                        for (int i = 0; i < nItems; i++)
                        {
                                mpComponents[i] = pComponents[i];

                        }
                }
        }
}

void PtComponentGroup::setTaoClient(TaoClientTask *pClient)
{
        mpClient = pClient;
        if (mpClient && !(mpClient->isStarted()))
        {
                mpClient->start();
        }
};

// Copy constructor
PtComponentGroup::PtComponentGroup(const PtComponentGroup& rPtComponentGroup) :
mGroupType(rPtComponentGroup.mGroupType),
mIsActivated(rPtComponentGroup.mIsActivated),
mNumItems(rPtComponentGroup.mNumItems),
mpClient(rPtComponentGroup.mpClient),
mComponentRef(rPtComponentGroup.mComponentRef)
{
        initialize();

        mDescription = rPtComponentGroup.mDescription;

        if (rPtComponentGroup.mpComponents)
        {
                mNumItems = rPtComponentGroup.mNumItems;

                if (mNumItems > 0)
                {
                        mpComponents = new PtComponent*[mNumItems + 1];
                        if (mpComponents)
                        {
                                for (int i = 0; i < mNumItems; i++)
                                {
                                        mpComponents[i] = rPtComponentGroup.mpComponents[i];
                                }
                        }
                }
        }
        else
        {
                mpComponents = 0;
                mNumItems = 0;
        }

}

// Destructor
PtComponentGroup::~PtComponentGroup()
{

        if (mpComponents)
        {
                delete[] mpComponents;
                mpComponents = 0;
        }

    semInit.acquire() ;
        mRef--;

        if (mRef < 1)
        {
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
PtComponentGroup&
PtComponentGroup::operator=(const PtComponentGroup& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        mComponentRef = rhs.mComponentRef;
        if (rhs.mpComponents)
        {
                mNumItems = rhs.mNumItems;
                if (mNumItems > 0)
                {
                        if (mpComponents)
                                delete[] mpComponents;

                        mpComponents = new PtComponent*[mNumItems + 1];
                        if (mpComponents)
                        {
                                for (int i = 0; i < mNumItems; i++)
                                {
                                        mpComponents[i] = rhs.mpComponents[i];
                                }
                        }
                }
        }
        else
        {
                mpComponents = 0;
                mNumItems = 0;
        }

        mDescription = rhs.mDescription;
        mGroupType = rhs.mGroupType;
        mIsActivated = rhs.mIsActivated;
        mNumItems = rhs.mNumItems;
        mpClient = rhs.mpClient;
        mTimeOut = rhs.mTimeOut;

        return *this;
}

PtStatus PtComponentGroup::setHandsetVolume(int level)
{
        EVENT_TRACE("Entering PtComponentGroup::setHandsetVolume\n") ;
        if (mGroupType == HAND_SET && mpComponents && mNumItems > 0)
        {
                PtComponent *pComponent;

                for (int i = 0; i < mNumItems; i++)
                {
                        pComponent = mpComponents[i];
                        int type;
                        if (PT_SUCCESS == pComponent->getType(type) && PtComponent::SPEAKER == type)
                        {
                                ((PtPhoneSpeaker *)pComponent)->setVolume(level);
                                return PT_SUCCESS;
                        }
                }
        }
        EVENT_TRACE("Exiting PtComponentGroup::setHandsetVolume\n") ;

        return PT_INVALID_ARGUMENT;
}

PtStatus PtComponentGroup::setSpeakerVolume(int level)
{
        EVENT_TRACE("Entering PtComponentGroup::setSpeakerVolume\n") ;
        if ((mGroupType == PHONE_SET || mGroupType == SPEAKER_PHONE) && mpComponents && mNumItems > 0)
        {
                PtComponent *pComponent;

                for (int i = 0; i < mNumItems; i++)
                {
                        pComponent = mpComponents[i];
                        int type;
                        if (PT_SUCCESS == pComponent->getType(type) && PtComponent::SPEAKER == type)
                        {
                                ((PtPhoneSpeaker *)pComponent)->setVolume(level);
                                return PT_SUCCESS;
                        }
                }
        }

        EVENT_TRACE("Exiting PtComponentGroup::setSpeakerVolume\n") ;
        return PT_INVALID_ARGUMENT;
}

PtStatus PtComponentGroup::setExtSpeakerVolume(int level)
{
        EVENT_TRACE("Entering PtComponentGroup::setExtSpeakerVolume\n") ;
        if (mGroupType == EXTERNAL_SPEAKER && mpComponents && mNumItems > 0)
        {
                PtComponent *pComponent;

                for (int i = 0; i < mNumItems; i++)
                {
                        pComponent = mpComponents[i];
                        int type;
                        if (PT_SUCCESS == pComponent->getType(type) && PtComponent::EXTERNAL_SPEAKER == type)
                        {
                                ((PtPhoneExtSpeaker *)pComponent)->setVolume(level);
                                return PT_SUCCESS;
                        }
                }
        }

        EVENT_TRACE("Exiting PtComponentGroup::setExtSpeakerVolume\n") ;
        return PT_INVALID_ARGUMENT;
}

PtStatus PtComponentGroup::setRingerVolume(int level)
{
        EVENT_TRACE("Entering PtComponentGroup::setRingerVolume\n") ;
        if ((mGroupType == PHONE_SET || mGroupType == SPEAKER_PHONE) && mpComponents && mNumItems > 0)
        {
                PtComponent *pComponent;

                for (int i = 0; i < mNumItems; i++)
                {
                        pComponent = mpComponents[i];
                        int type;
                        if (PT_SUCCESS == pComponent->getType(type) && PtComponent::RINGER == type)
                        {
                                ((PtPhoneRinger *)pComponent)->setRingerVolume(level);
                                return PT_SUCCESS;
                        }
                }
        }

        EVENT_TRACE("Exiting PtComponentGroup::setRingerVolume\n") ;
        return PT_INVALID_ARGUMENT;
}

UtlBoolean PtComponentGroup::activate(void)
{
        EVENT_TRACE("Entering PtComponentGroup::activate\n") ;
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", mGroupType);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                        TaoMessage::PHONEGROUP_ACTIVATE,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t rc;
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
                EVENT_TRACE("Exiting PtComponentGroup::activate, TIME OUT\n") ;
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getEventData(rc);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::PHONEGROUP_ACTIVATE);
#endif
        mpEventMgr->release(pe);
        mIsActivated = true;
        EVENT_TRACE("Exiting PtComponentGroup::activate\n") ;

        return true;
}

UtlBoolean PtComponentGroup::deactivate(void)
{
        EVENT_TRACE("Entering PtComponentGroup::deactivate\n") ;
        if (!mpClient)
        {
                return PT_NOT_FOUND;
        }

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", mGroupType);
        UtlString arg(buff);

        mpTransactionCnt->add();
        unsigned int transactionId = mpTransactionCnt->getRef();

        OsProtectedEvent *pe = mpEventMgr->alloc();
        TaoMessage      msg(TaoMessage::REQUEST_PHONECOMPONENT,
                                                                        TaoMessage::PHONEGROUP_DEACTIVATE,
                                                                        transactionId,
                                                                        0,
                                                                        (TaoObjHandle)pe,
                                                                        1,
                                                                        arg);
        mpClient->sendRequest(msg);

        intptr_t rc;
        if (OS_SUCCESS != pe->wait(msg.getCmd(), mTimeOut))
        {
                mpClient->resetConnectionSocket(msg.getMsgID());
                EVENT_TRACE("Exiting PtComponentGroup::deactivate, TIME OUT\n") ;
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pe->signal(0))
        {
            mpEventMgr->release(pe);
        }
                return PT_BUSY;
        }

        pe->getEventData(rc);
#ifdef PTAPI_TEST
        int cmd;
        pe->getIntData2(cmd);
        assert(cmd == TaoMessage::PHONEGROUP_DEACTIVATE);
#endif
        mpEventMgr->release(pe);
        mIsActivated = false;

        EVENT_TRACE("Exiting PtComponentGroup::deactivate\n") ;
        return true;
}

/* ============================ ACCESSORS ================================= */
PtStatus PtComponentGroup::getComponents(PtComponent* pComponents[], int size,
                      int& nItems)
{
        EVENT_TRACE("Entering PtComponentGroup::getComponents\n") ;
        if (mpComponents && pComponents)
        {
                nItems = (size < mNumItems) ? size : mNumItems;

                for (int i = 0; i < nItems; i++)
                {
                        pComponents[i] = mpComponents[i];
                }
                return PT_SUCCESS;
        }

        EVENT_TRACE("Exiting PtComponentGroup::getComponents\n") ;
        return PT_INVALID_ARGUMENT;
}

PtStatus PtComponentGroup::getDescription(char* pDescription, int maxLen)
{
        EVENT_TRACE("Entering PtComponentGroup::getDescription\n") ;
        if (pDescription && maxLen > 0)
        {
                if (!mDescription.isNull())
                {
                        int bytes = strlen(mDescription.data());
                        bytes = (bytes > maxLen) ? maxLen : bytes;

                        memset(pDescription, 0, maxLen);
                        strncpy (pDescription, mDescription.data(), bytes);
                        return PT_SUCCESS;
                }
        }

        EVENT_TRACE("Exiting PtComponentGroup::getDescription\n") ;
        return PT_RESOURCE_UNAVAILABLE;
}

PtStatus PtComponentGroup::getHandsetVolume(int& level)
{
        EVENT_TRACE("Entering PtComponentGroup::getHandsetVolume\n") ;
        if (mGroupType == HAND_SET && mpComponents && mNumItems > 0)
        {
                PtComponent *pComponent;

                for (int i = 0; i < mNumItems; i++)
                {
                        pComponent = mpComponents[i];
                        int type;
                        if (PT_SUCCESS == pComponent->getType(type) && PtComponent::SPEAKER == type)
                        {
                                ((PtPhoneSpeaker *)pComponent)->getVolume(level);
                                return PT_SUCCESS;
                        }
                }
        }

        EVENT_TRACE("Exiting PtComponentGroup::getHandsetVolume\n") ;
        return PT_INVALID_ARGUMENT;
}

PtStatus PtComponentGroup::getSpeakerVolume(int& level)
{
        EVENT_TRACE("Entering PtComponentGroup::getSpeakerVolume\n") ;
        if (mpComponents && (mNumItems > 0))
        {
                PtComponent *pComponent;

                for (int i = 0; i < mNumItems; i++)
                {
                        pComponent = mpComponents[i];
                        int type;
                        if (PT_SUCCESS == pComponent->getType(type) && PtComponent::SPEAKER == type)
                        {
                                ((PtPhoneSpeaker *)pComponent)->getVolume(level);
                                return PT_SUCCESS;
                        }
                }
        }

        EVENT_TRACE("Exiting PtComponentGroup::getSpeakerVolume\n") ;
        return PT_INVALID_ARGUMENT;
}

PtStatus PtComponentGroup::getSpeakerNominalVolume(int& level)
{
        EVENT_TRACE("Entering PtComponentGroup::getSpeakerNominalVolume\n") ;
        if (mpComponents && (mNumItems > 0))
        {
                PtComponent *pComponent;

                for (int i = 0; i < mNumItems; i++)
                {
                        pComponent = mpComponents[i];
                        int type;
                        if (PT_SUCCESS == pComponent->getType(type) && PtComponent::SPEAKER == type)
                        {
                                ((PtPhoneSpeaker *)pComponent)->getNominalVolume(level);
                                return PT_SUCCESS;
                        }
                }
        }

        EVENT_TRACE("Exiting PtComponentGroup::getSpeakerNominalVolume\n") ;
        return PT_INVALID_ARGUMENT;
}

PtStatus PtComponentGroup::getExtSpeakerVolume(int& level)
{
        EVENT_TRACE("Entering PtComponentGroup::getExtSpeakerVolume\n") ;
        if (mpComponents && (mNumItems > 0))
        {
                PtComponent *pComponent;

                for (int i = 0; i < mNumItems; i++)
                {
                        pComponent = mpComponents[i];
                        int type;
                        if (PT_SUCCESS == pComponent->getType(type) && PtComponent::EXTERNAL_SPEAKER == type)
                        {
                                ((PtPhoneExtSpeaker *)pComponent)->getVolume(level);
                                return PT_SUCCESS;
                        }
                }
        }

        EVENT_TRACE("Exiting PtComponentGroup::getExtSpeakerVolume\n") ;
        return PT_INVALID_ARGUMENT;
}

PtStatus PtComponentGroup::getExtSpeakerNominalVolume(int& level)
{
        EVENT_TRACE("Entering PtComponentGroup::getExtSpeakerNominalVolume\n") ;
        if (mpComponents && (mNumItems > 0))
        {
                PtComponent *pComponent;

                for (int i = 0; i < mNumItems; i++)
                {
                        pComponent = mpComponents[i];
                        int type;
                        if (PT_SUCCESS == pComponent->getType(type) && PtComponent::EXTERNAL_SPEAKER == type)
                        {
                                ((PtPhoneExtSpeaker *)pComponent)->getNominalVolume(level);
                                return PT_SUCCESS;
                        }
                }
        }

        EVENT_TRACE("Exiting PtComponentGroup::getExtSpeakerNominalVolume\n") ;
        return PT_INVALID_ARGUMENT;
}

PtStatus PtComponentGroup::getRingerVolume(int& level)
{
        EVENT_TRACE("Entering PtComponentGroup::getRingerVolume\n") ;
        if ((mGroupType == PHONE_SET || mGroupType == SPEAKER_PHONE) && mpComponents && mNumItems > 0)
        {
                PtComponent *pComponent;

                for (int i = 0; i < mNumItems; i++)
                {
                        pComponent = mpComponents[i];
                        int type;
                        if (PT_SUCCESS == pComponent->getType(type) && PtComponent::RINGER == type)
                        {
                                ((PtPhoneRinger *)pComponent)->getRingerVolume(level);
                                return PT_SUCCESS;
                        }
                }
        }

        EVENT_TRACE("Exiting PtComponentGroup::getRingerVolume\n") ;
        return PT_INVALID_ARGUMENT;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
void PtComponentGroup::initialize()
{
        mpEventMgr = OsProtectEventMgr::getEventMgr();
        mTimeOut = OsTime(PT_CONST_EVENT_WAIT_TIMEOUT, 0);
    semInit.acquire() ;

        mRef++;

        if (!mpTransactionCnt)
                mpTransactionCnt = new TaoReference();

    semInit.release() ;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
