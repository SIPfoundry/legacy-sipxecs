//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef MP_AUDIO_ABSTRACT_H
#define MP_AUDIO_ABSTRACT_H

#include <os/iostream>
#include <stdio.h>
#include <stdint.h>

typedef int16_t AudioSample;
typedef unsigned char AudioByte;


//formats we support
#define AUDIO_FORMAT_UNKNOWN 0
#define AUDIO_FORMAT_WAV     1
#define AUDIO_FORMAT_AU      2

class MpAudioAbstract {

public:

/* ============================ CREATORS ================================== */
    MpAudioAbstract(void);
    //: Default Constructor.

    MpAudioAbstract(MpAudioAbstract *audio);
    //: Copy Constructor

    virtual ~MpAudioAbstract();
    //: Destructor

public: //: general non classfied
    virtual size_t getSamples(AudioSample *, size_t) = 0;
    // Returns number of samples actually read, 0 on error.
    virtual size_t readBytes(AudioByte * buff, size_t length);
    // read length of bytes
    virtual size_t getBytesSize();
    // get bytes size of the audio file
    virtual int getDecompressionType();
    // get decompression type of the audio file

public: //: MpAudioAbstract related operations
   MpAudioAbstract *getPreviousAudio(void);
   //: get previous audio
   void setPreviousAudio(MpAudioAbstract *a);
   //: set previous audio to a
   MpAudioAbstract *getNextAudio(void);
   //: get next audio
   void setNextAudio(MpAudioAbstract *a);
   //: set next audio to a

public: //: sample related functions
    virtual void setSamplingRate(long s);
    //: Set the sampling rate to s
    virtual void setSamplingRateRecursive(long s);
    //: Set sampling rate recursively
    virtual void minMaxSamplingRate(long *min, long *max, long *prefer);
    //: TODO: the meaning of this function, get the prefered Sampling rate
    virtual void negotiateSamplingRate(void);
    //: negotiate the sampling rate
    virtual long getSamplingRate(void);
    //: Return the sampling rate

public: //:Channel related functions
    virtual void setChannels(int ch);
    //:Set channel to ch
    virtual void setChannelsRecursive(int s);
    //:Set channel recusively
    virtual void minMaxChannels(int *min, int *max, int *preferred) ;
    //:Get prefered channel
    virtual void negotiateChannels(void);
    //: negotiate channel
    virtual int getChannels(void);
    //: Return the channels

    virtual void setAudioFormat(int type) { mDetectedFormat = type;}
    virtual int  getAudioFormat() { return mDetectedFormat;}
    //: Set and Get audio object format

    bool isOk() {return mbIsOk;}
    //: Return true if file loaded ok.
/* ============================ PRIVATE ================================== */
private:
    // pointers to MpAudioAbstract itself then
   MpAudioAbstract *mPrevious; // object to get data from
   MpAudioAbstract *mNext; // object pulling data from us
private:
   long mSamplingRate;
   bool mSamplingRateFrozen;
   //: sampling Rate related stuff
private:
   long mChannels;
   bool mChannelsFrozen;
   //: channel related stuff

   int mDetectedFormat;
   //:
protected:
   bool mbIsOk; //if file loaded ok.
};

#endif
