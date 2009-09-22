//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <utl/UtlHashBagIterator.h>
#include <os/OsSysLog.h>
#include <os/OsTimer.h>
#include <os/OsDateTime.h>
#include <net/SipPublishServerEventStateMgr.h>
#include <net/SipMessage.h>
#include <net/NetMd5Codec.h>


// Private class to contain callback for eventTypeKey
class PublishServerEventState : public UtlString
{
public:
    PublishServerEventState();

    virtual ~PublishServerEventState();

    // Parent UtlString contains the dialog handle
    UtlString mResourceId;
    UtlString mEventTypeKey;
    UtlString mEntityTagValue;
    long mExpirationDate; // epoch time
    OsTimer* mpExpirationTimer;

private:
    //! DISALLOWED accidental copying
    PublishServerEventState(const PublishServerEventState& rPublishServerEventState);
    PublishServerEventState& operator=(const PublishServerEventState& rhs);
};

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

PublishServerEventState::PublishServerEventState()
{
    mExpirationDate = -1;
    mEntityTagValue = NULL;
    mpExpirationTimer = NULL;
}
PublishServerEventState::~PublishServerEventState()
{
    if(mpExpirationTimer)
    {
        // Timer should have been stopped and the the task upon
        // which the fired timer queues its message need to have
        // synchronized to make sure it does not get touched after
        // it is deleted here.
        delete mpExpirationTimer;
        mpExpirationTimer = NULL;
    }
}


// Constructor
SipPublishServerEventStateMgr::SipPublishServerEventStateMgr()
   : mEventStateMgrMutex(OsMutex::Q_FIFO)
{
    mMinExpiration = 32;
    mDefaultExpiration = 3600;
    mMaxExpiration = 86400;
}


// Copy constructor NOT IMPLEMENTED
SipPublishServerEventStateMgr::SipPublishServerEventStateMgr(const SipPublishServerEventStateMgr& rSipPublishServerEventStateMgr)
   : mEventStateMgrMutex(OsMutex::Q_FIFO)
{
}


// Destructor
SipPublishServerEventStateMgr::~SipPublishServerEventStateMgr()
{
    // Iterate through and delete all the dialogs
    // TODO:
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipPublishServerEventStateMgr&
SipPublishServerEventStateMgr::operator=(const SipPublishServerEventStateMgr& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void
SipPublishServerEventStateMgr::generateETag(UtlString& entity)
{
}

void
SipPublishServerEventStateMgr::addPublish(UtlString& entity, UtlString& resourceId,
                                          UtlString& eventTypeKey, int expiration)
{
}

void
SipPublishServerEventStateMgr::updatePublish(UtlString& oldEntity,
                                             UtlString& entity,
                                             UtlString& resourceId,
                                             UtlString& eventTypeKey,
                                             int expiration)
{
}


void
SipPublishServerEventStateMgr::removePublish(UtlString& entity)
{
}


void SipPublishServerEventStateMgr::removeOldPublication(long oldEpochTimeSeconds)
{
    lock();
#if 0
    UtlHashBagIterator iterator(mSubscriptionStateResourceIndex);
    PublishServerEventStateIndex* stateIndex = NULL;
    while((stateIndex = (PublishServerEventStateIndex*) iterator()))
    {
        if(stateIndex->mpState)
        {
            if(stateIndex->mpState->mExpirationDate < oldEpochTimeSeconds)
            {
                mpDialogMgr->deleteDialog(*(stateIndex->mpState));
                mSubscriptionStatesByDialogHandle.removeReference(stateIndex->mpState);
                delete stateIndex->mpState;
                stateIndex->mpState = NULL;
                mSubscriptionStateResourceIndex.removeReference(stateIndex);
                delete stateIndex;
            }
        }

        else
        {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                "SipPublishServerEventStateMgr::removeOldSubscriptions PublishServerEventStateIndex with NULL mpState, deleting");
            mSubscriptionStateResourceIndex.removeReference(stateIndex);
            delete stateIndex;
        }
    }
#endif
    unlock();
}

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

UtlBoolean SipPublishServerEventStateMgr::publishExists(UtlString& entityTag)
{
    UtlBoolean publishFound = FALSE;

    lock();
    PublishServerEventState* state = (PublishServerEventState*)
        mEventStatesByEntityTag.find(&entityTag);
    if(state)
    {
        publishFound = TRUE;
    }
    unlock();

    return(publishFound);
}

UtlBoolean SipPublishServerEventStateMgr::isExpired(UtlString& entityTag)
{
    UtlBoolean publishExpired = TRUE;

    lock();
    PublishServerEventState* state = (PublishServerEventState*)
        mEventStatesByEntityTag.find(&entityTag);
    if(state)
    {
        long now = OsDateTime::getSecsSinceEpoch();

        if(now <= state->mExpirationDate)
        {
            publishExpired = FALSE;
        }
    }
    unlock();

    return(publishExpired);
}

UtlBoolean
SipPublishServerEventStateMgr::checkExpiration(int* expiration)
{
  return FALSE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


void SipPublishServerEventStateMgr::lock()
{
    mEventStateMgrMutex.acquire();
}

void SipPublishServerEventStateMgr::unlock()
{
    mEventStateMgrMutex.release();
}

/* ============================ FUNCTIONS ================================= */
