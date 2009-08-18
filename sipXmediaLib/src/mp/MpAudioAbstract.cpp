//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include "os/OsDefs.h"
#include "mp/MpAudioAbstract.h"
#include "mp/MpAudioFileUtils.h"

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
// Default Constructor
MpAudioAbstract::MpAudioAbstract() {
      mPrevious = 0;
      mNext = 0;
      mSamplingRate = 0;  mSamplingRateFrozen = false;
      mChannels = 0;      mChannelsFrozen = false;
      mDetectedFormat = AUDIO_FORMAT_UNKNOWN;
}

// Construct a Audio from an existed Audio
MpAudioAbstract::MpAudioAbstract(MpAudioAbstract *audio) {
      mPrevious = audio;
      mNext = 0;
      audio->setNextAudio(this);
      mSamplingRate = 0;  mSamplingRateFrozen = false;
      mChannels = 0;      mChannelsFrozen = false;
      mDetectedFormat = getAudioFormat();
}

// Destructor
MpAudioAbstract::~MpAudioAbstract()
{
    // do nothing right now
}

/* ============================ General Unclassfied ================ */
//  getSamples is virtual function
// readBytes: read from previous resource of length bytes
size_t MpAudioAbstract::readBytes(AudioByte * buff, size_t length)
{
    return getPreviousAudio()->readBytes(buff,length);
}

size_t MpAudioAbstract::getBytesSize()
{
    return getPreviousAudio()->getBytesSize();
}

int MpAudioAbstract::getDecompressionType()
{
    return 0;
    // we might just use pure virtual here
}
/* ============================ MpAudioAbstract Operations ================ */
// get previouse audio
MpAudioAbstract* MpAudioAbstract::getPreviousAudio(void)
{
    return mPrevious;
}

// previous from existing audio
void MpAudioAbstract::setPreviousAudio(MpAudioAbstract *a)
{
    mPrevious = a;
}

// get next audioAbstract
MpAudioAbstract* MpAudioAbstract::getNextAudio(void)
{
    return mNext;
}

// assign next Audio to a
void MpAudioAbstract::setNextAudio(MpAudioAbstract *a)
{
    mNext = a;
}

/* ============================ Sampling related functions ================ */
// set samplingRate to s if it's not frozen
void MpAudioAbstract::setSamplingRate(long s)
{ // Set the sampling rate
      if (mSamplingRateFrozen) {
         osPrintf("Can't change sampling rate.\n");
      }
      mSamplingRate = s;
}

// Set sampling rate recursively
void MpAudioAbstract::setSamplingRateRecursive(long s) {
   if (getPreviousAudio()) // Set towards the right first
      getPreviousAudio()->setSamplingRateRecursive(s);
   setSamplingRate(s); // Set it
   mSamplingRateFrozen = true; // Yes, we've negotiated
}


void MpAudioAbstract::minMaxSamplingRate(long *min, long *max, long *preferred)
{
   if (getPreviousAudio()) getPreviousAudio()->minMaxSamplingRate(min,max,preferred);
   if (mSamplingRate) *preferred = mSamplingRate;
   if (*preferred < *min) *preferred = *min;
   if (*preferred > *max) *preferred = *max;
}

// negotiate the sampling rate
void MpAudioAbstract::negotiateSamplingRate(void) {
   if (getNextAudio())
      getNextAudio()->negotiateSamplingRate();
   else {
      long min = 8000, max = 44100, preferred = 44100;
      minMaxSamplingRate(&min,&max,&preferred);
      if (min > max) {
         osPrintf("Couldn't negotiate sampling rate.\n");
      }
      setSamplingRateRecursive(preferred); // Set them everywhere
   }
}

// get sampling rate
long MpAudioAbstract::getSamplingRate(void)
{
      if (!mSamplingRateFrozen)  // Not frozen?
         negotiateSamplingRate(); // Go figure it out
      return mSamplingRate; // Return it
}

/* ============================ Channel related functions ================ */
// Set channel to ch
void MpAudioAbstract::setChannels(int ch)
{
      if (mChannelsFrozen) {
         osPrintf("Can't change number of channels.\n");
      }
      mChannels = ch;
}

// Set channel recursively to ch
void MpAudioAbstract::setChannelsRecursive(int ch)
{
   if (getPreviousAudio()) getPreviousAudio()->setChannelsRecursive(ch);
   setChannels(ch);
   mChannelsFrozen = true;
}

// Get prefered channel
void MpAudioAbstract::minMaxChannels(int *min, int *max, int *preferred)
{
   if (getPreviousAudio())  getPreviousAudio()->minMaxChannels(min,max,preferred);
   if (mChannels) *preferred = mChannels;
   if (*preferred < *min) *preferred = *min;
   if (*preferred > *max) *preferred = *max;
}

// negotiate channels
void MpAudioAbstract::negotiateChannels(void)
{
   if (getNextAudio())
      getNextAudio()->negotiateChannels();
   else {
      int min=1, max=2, preferred=1; // Some reasonable default
      minMaxChannels(&min,&max,&preferred);
      if (min > max) {
         osPrintf("Couldn't negotiate sampling rate.\n");
      }
      setChannelsRecursive(preferred);
   }
}

// Get channels
int MpAudioAbstract::getChannels(void)
{
      if (!mChannelsFrozen) negotiateChannels();
      return mChannels;
}
