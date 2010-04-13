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
#include "ps/PsTaoComponentGroup.h"
#include "ps/PsTaoComponent.h"
#include "ps/PsTaoSpeaker.h"
#include "ps/PsTaoRinger.h"
#include "ps/PsTaoMicrophone.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PsTaoComponentGroup::PsTaoComponentGroup() :
mpComponents(NULL),
mGroupType(0),
mIsActivated(0),
mNumItems(0)
{
        mHandsetVolume = -1;
        mHeadsetVolume = -1;
        mRingerVolume = -1;
        mSpeakerVolume = -1;
        mExtSpeakerVolume = -1;

        mLow = 0;
        mHigh = 100;
        mNominal = 50;
        mStepsize = 10;
        mMute = 0;

        mMicLow = 0;
        mMicHigh = 100;
        mMicNominal = 50;
        mMicStepsize = 10;
        mMicMute = 0;

        mMicGain = mMicNominal;
}

PsTaoComponentGroup::PsTaoComponentGroup(int groupType, const UtlString& rDescription,
                                                                                 PsTaoComponent* pComponents[], int nItems) :
mpComponents(NULL),
mGroupType(groupType),
mIsActivated(0),
mNumItems(nItems)
{

        if (pComponents && nItems > 0)
        {
                mpComponents = new PsTaoComponent*[nItems+1];
                if (mpComponents)
                {
                        for (int i = 0; i < nItems; i++)
                                mpComponents[i] = pComponents[i];
                }
        }
        mDescription = rDescription;

        mHandsetVolume = -1;
        mHeadsetVolume = -1;
        mRingerVolume = -1;
        mSpeakerVolume = -1;
        mExtSpeakerVolume = -1;

        mLow = 0;
        mHigh = 100;
        mNominal = 50;
        mStepsize = 10;
        mMute = 0;

        mMicLow = 0;
        mMicHigh = 100;
        mMicNominal = 50;
        mMicStepsize = 10;
        mMicMute = 0;

        mMicGain = mMicNominal;
}

// Copy constructor
PsTaoComponentGroup::PsTaoComponentGroup(const PsTaoComponentGroup& rPsTaoComponentGroup) :
mpComponents(NULL),
mGroupType(0),
mIsActivated(0),
mNumItems(0)
{
        mHandsetVolume = rPsTaoComponentGroup.mHandsetVolume;
        mHeadsetVolume = rPsTaoComponentGroup.mHeadsetVolume;
        mRingerVolume = rPsTaoComponentGroup.mRingerVolume;
        mSpeakerVolume = rPsTaoComponentGroup.mSpeakerVolume;
        mExtSpeakerVolume = rPsTaoComponentGroup.mExtSpeakerVolume;

        mLow = rPsTaoComponentGroup.mLow;
        mHigh = rPsTaoComponentGroup.mHigh;
        mNominal = rPsTaoComponentGroup.mNominal;
        mStepsize = rPsTaoComponentGroup.mStepsize;
        mMute = rPsTaoComponentGroup.mMute;

        mMicLow = rPsTaoComponentGroup.mMicLow;
        mMicHigh = rPsTaoComponentGroup.mMicHigh;
        mMicNominal = rPsTaoComponentGroup.mMicNominal;
        mMicStepsize = rPsTaoComponentGroup.mMicStepsize;
        mMicMute = rPsTaoComponentGroup.mMicMute;

        mMicGain = rPsTaoComponentGroup.mMicGain;
}

// Destructor
PsTaoComponentGroup::~PsTaoComponentGroup()
{
        if (mpComponents)
                delete[] mpComponents;

        mDescription.remove(0);
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PsTaoComponentGroup&
PsTaoComponentGroup::operator=(const PsTaoComponentGroup& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        mHandsetVolume = rhs.mHandsetVolume;
        mHeadsetVolume = rhs.mHeadsetVolume;
        mRingerVolume = rhs.mRingerVolume;
        mSpeakerVolume = rhs.mSpeakerVolume;
        mExtSpeakerVolume = rhs.mExtSpeakerVolume;

        mLow = rhs.mLow;
        mHigh = rhs.mHigh;
        mNominal = rhs.mNominal;
        mStepsize = rhs.mStepsize;
        mMute = rhs.mMute;

        mMicLow = rhs.mMicLow;
        mMicHigh = rhs.mMicHigh;
        mMicNominal = rhs.mMicNominal;
        mMicStepsize = rhs.mMicStepsize;
        mMicMute = rhs.mMicMute;

        mMicGain = rhs.mMicGain;

        return *this;
}

OsStatus PsTaoComponentGroup::setVolumeRange(int low,
                      int high,
                      int nominal,
                      int stepsize,
                      int mute)
{
        mLow = low;
        mHigh = high;
        mNominal = nominal;
        mStepsize = (high - low) / 10;
        mMute = mute;

        osPrintf("PsTaoComponentGroup::setVolumeRange: groupt type: %d, low:"
            " %d, high: %d,\n   nominal: %d, mute: %d, stepsize %d %d\n",
            mGroupType, mLow, mHigh, mNominal, mMute, mStepsize, stepsize);

        switch (mGroupType)
        {
        case HEAD_SET:
                mHeadsetVolume = mNominal;
                break;

        case HAND_SET:
                mHandsetVolume = mNominal;
                break;

        case SPEAKER_PHONE:
                mSpeakerVolume = mNominal;
                mRingerVolume = mNominal;
                break;

        case EXTERNAL_SPEAKER:
                mExtSpeakerVolume = mNominal;
                break;

        case PHONE_SET:
                mRingerVolume = mNominal;
                break;

        case OTHER:
        default:
                break;
        }

        return OS_SUCCESS;
}

OsStatus PsTaoComponentGroup::setGainRange(int low,
                      int high,
                      int nominal,
                      int stepsize,
                      int mute)
{
        mMicLow = low;
        mMicHigh = high;
        mMicNominal = nominal;
        mMicStepsize = (high - low) / 10;
        mMicMute = mute;

        mMicGain = mMicNominal;

        osPrintf("PsTaoComponentGroup::setGainRange: groupt type: %d, low:"
            " %d, high: %d,\n   nominal: %d, current: %d mute: %d, stepsize %d %d\n",
            mGroupType, mMicLow, mMicHigh, mMicNominal, mMicGain, mMicMute,
            mMicStepsize, stepsize);
        return OS_SUCCESS;
}

OsStatus PsTaoComponentGroup::setHandsetVolume(int& level) // input 0 <= level <= 10
{
        mHandsetVolume = normalize(level);

        return OS_SUCCESS;
}

OsStatus PsTaoComponentGroup::setHeadsetVolume(int& level) // input 0 <= level <= 10
{
        mHeadsetVolume = normalize(level);
        return OS_SUCCESS;
}

OsStatus PsTaoComponentGroup::setSpeakerVolume(int& level) // input 0 <= level <= 10
{
        mSpeakerVolume = normalize(level);
        return OS_SUCCESS;
}

OsStatus PsTaoComponentGroup::setExtSpeakerVolume(int& level) // input 0 <= level <= 10
{
        mExtSpeakerVolume = normalize(level);
        return OS_SUCCESS;
}

OsStatus PsTaoComponentGroup::setRingerVolume(int& level) // input 0 <= level <= 10
{
        mRingerVolume = normalize(level);
        return OS_SUCCESS;
}

OsStatus PsTaoComponentGroup::setMicGain(int group, int& level)
{
    if (mGroupType == group)
    {
        if (level == 5)
        {
            mMicGain = mMicNominal;
        }
        else
        {
            mMicGain = gainNormalize(level);
        }

        osPrintf("PsTaoComponentGroup::setMicGain: groupt type: %d, level %d mMicGain %d (is not set)\n", group, level, mMicGain);
        assert(false);
        return OS_SUCCESS;
    }

    return OS_INVALID_ARGUMENT;
}

UtlBoolean PsTaoComponentGroup::activate(void)
{
        mIsActivated = true;
        return true;
}

UtlBoolean PsTaoComponentGroup::deactivate(void)
{
        mIsActivated = false;
        return true;
}

/* ============================ ACCESSORS ================================= */
OsStatus PsTaoComponentGroup::getVolume(int groupType, int& level)   // mLow <= level <= mHigh
{


        switch (groupType)
        {
        case HEAD_SET:
                level = mHeadsetVolume;
                break;

        case HAND_SET:
                level = mHandsetVolume;
                break;

        case SPEAKER_PHONE:
                level = mSpeakerVolume;
                break;

        case EXTERNAL_SPEAKER:
                level = mExtSpeakerVolume;
                break;

        case PHONE_SET:
                level = mRingerVolume;
                break;

        case OTHER:
        default:
                level = mNominal;
                break;
        }

        return OS_SUCCESS;
}

OsStatus PsTaoComponentGroup::getHandsetVolume(int& level, int isNominal) // output 0 <= level <= 10
{
        if (isNominal)
        {
                level = (mNominal - mLow) / mStepsize;
                return OS_SUCCESS;
        }

        if (mHandsetVolume == mMute)
                level = 0;
        else
                level = (mHandsetVolume - mLow) / mStepsize;

        return OS_SUCCESS;
}

OsStatus PsTaoComponentGroup::getHeadsetVolume(int& level, int isNominal) // output 0 <= level <= 10
{
        if (isNominal)
        {
                level = (mNominal - mLow) / mStepsize;
                return OS_SUCCESS;
        }

        if (mHeadsetVolume == mMute)
                level = 0;
        else
                level = (mHeadsetVolume - mLow) / mStepsize;

        return OS_SUCCESS;
}

OsStatus PsTaoComponentGroup::getSpeakerVolume(int& level, int isNominal) // output 0 <= level <= 10
{
        if (isNominal)
        {
                level = (mNominal - mLow) / mStepsize;
                return OS_SUCCESS;
        }

        if (mSpeakerVolume == mMute)
                level = 0;
        else
                level = (mSpeakerVolume - mLow) / mStepsize;

        return OS_SUCCESS;
}

OsStatus PsTaoComponentGroup::getExtSpeakerVolume(int& level, int isNominal) // output 0 <= level <= 10
{
        if (isNominal)
        {
                level = (mNominal - mLow) / mStepsize;
                return OS_SUCCESS;
        }

        if (mExtSpeakerVolume == mMute)
                level = 0;
        else
                level = (mExtSpeakerVolume - mLow) / mStepsize;

        return OS_SUCCESS;
}

OsStatus PsTaoComponentGroup::getRingerVolume(int& level, int isNominal) // output 0 <= level <= 10
{
        if (isNominal)
        {
                level = (mNominal - mLow) / mStepsize;
                return OS_SUCCESS;
        }

        if (mRingerVolume == mMute)
                level = 0;
        else
                level = (mRingerVolume - mLow) / mStepsize;

        return OS_SUCCESS;
}

OsStatus PsTaoComponentGroup::getMicGain(int group, int& level)
{
        if (group == mGroupType)
        {
                if (mMicGain == mMicMute)
                        level = 0;
                else if (mMicGain == mMicNominal)
                        level = 5;
                else
                        level = (mMicGain - mMicLow) / mMicStepsize;

        osPrintf("PsTaoComponentGroup::getMicGain: groupt type: %d, level %d mMicGain %d, mMicLow %d mMicStepsize %d\n",
                                                                        group, level, mMicGain, mMicLow, mMicStepsize);
                return OS_SUCCESS;
        }

        return OS_INVALID_ARGUMENT;
}


OsStatus PsTaoComponentGroup::getMicGainValue(int group, int& value)
{
        if (group == mGroupType)
        {
                value = mMicGain;
                return OS_SUCCESS;
        }

        return OS_INVALID_ARGUMENT;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
int PsTaoComponentGroup::normalize(int& level)
{
        if (level < 0)
                level = mNominal;
        else if (level == 0)
                level = mMute;
        else
                level = mStepsize * level + mLow;

        if(level > mHigh)
                level = mHigh;

        return level;
}

int PsTaoComponentGroup::gainNormalize(int& level)
{
        if (level < 0)
                level = mMicNominal;
        else if (level == 0)
                level = mMicMute;
        else
                level = mMicStepsize * level + mMicLow;

        if(level > mMicHigh)
                level = mMicHigh;

        return level;
}

/* ============================ FUNCTIONS ================================= */
