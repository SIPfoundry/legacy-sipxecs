//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PsPhoneTask_h_
#define _PsPhoneTask_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
// #include "os/OsEventMsg.h"
// #include "os/OsMsg.h"
// #include "os/OsMsgQ.h"
// #include "os/OsQueuedEvent.h"
#include "os/OsRWMutex.h"
#include "os/OsServerTask.h"
// #include "os/OsTimer.h"
#include "ps/PsMsg.h"
#include "tao/TaoReference.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class PsButtonTask;
class PsHookswTask;
class PsLampTask;
class PsTaoButton;
class PsTaoHookswitch;
class PsTaoLamp;
class PsTaoMicrophone;
class PsTaoRinger;
class PsTaoSpeaker;
class PsTaoDisplay;
class PsTaoComponentGroup;
class TaoObjectMap;

//:Task responsible for managing the phone set.
class PsPhoneTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   enum SpkrMode
   {
      HANDSET_ENABLED      = 0x1,
      SPEAKERPHONE_ENABLED = 0x2,
      RINGER_ENABLED       = 0x4, // Logical ringer use of speaker
      SOUND_ENABLED        = 0x8,  // logigical speaker used for non-call sound
      HEADSET_ENABLED      = 0x10,
      EXTSPEAKER_ENABLED   = 0x20
   };

/* ============================ CREATORS ================================== */

   static PsPhoneTask* getPhoneTask(void);
     //:Return a pointer to the Phone task, creating it if necessary

   virtual
   ~PsPhoneTask();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus addListener(OsServerTask* pListener);
     //:Register as a listener for phone messages.
     // For now, the phone task allows at most one listener and forwards
     // all keypad and hookswitch messages to that listener.

   virtual OsStatus postEvent(const int msg, void* source,
                              const int param1, const int param2,
                              const OsTime& rTimeout=OsTime::OS_INFINITY);
     //:Create a phone message and post it to the Phone task
     // Return the result of the message send operation.

   virtual OsStatus setGain(int level);
     //:Set the microphone gain to a level between 0 (low) and 100 (high)

   virtual void speakerModeEnable(int mode);
   //: Add the speaker devices to what is already enabled

   virtual void speakerModeDisable(int mode);
   //: Remove the speaker devices to what is already enabled

   virtual void setSpeakerMode(int mode);
     //:Sets the speaker mode for the phone (which speakers are enabled).

   virtual OsStatus setVolume(int level);
     //:Set the speaker volume to a level between 0 (low) and 100 (high)

   void taoSetVolume(int volume, int type);

   OsStatus taoSetMicGain(int group, int level);

   void extSpeakerConnect(UtlBoolean connected);

/* ============================ ACCESSORS ================================= */

   UtlBoolean getComponent(PsMsg& rMsg);

   UtlBoolean numComponents(PsMsg& rMsg);

   UtlBoolean getComponents(PsMsg& rMsg);

   UtlBoolean getComponentGroups(PsMsg& rMsg);

   int activateGroup(PsMsg& rMsg);
        int activateGroup(int type);

   UtlBoolean deactivateGroup(PsMsg& rMsg);
   UtlBoolean deactivateGroup(int type);

   virtual int getGain(void) const;
     //:Return the current microphone gain setting (a value from 0..100)

   virtual int getSpeakerMode(void);
     //:Returns the speaker mode for the phone (which speakers are enabled).

   virtual int getVolume(void);
     //:Return the current speaker volume setting (a value from 0..100)

   void taoGetVolume(int& volume, int type);

   void taoGetNominalVolume(int& volume, int type);

   int taoGetMicGain(int group);

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   PsPhoneTask();
     //:Constructor (called only indirectly via getPhoneTask())
     // We identify this as a protected (rather than a private) method so
     // that gcc doesn't complain that the class only defines a private
     // constructor and has no friends.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   virtual UtlBoolean handleMessage(OsMsg& rMsg);
     //:Handle an incoming message
     // Return TRUE if the message was handled, otherwise FALSE.

   virtual UtlBoolean handlePhoneMessage(const PsMsg& rMsg);
     //:Handle an incoming phone message (class PsMsg or descendants)
     // Return TRUE if the message was handled, otherwise FALSE.
     // A write lock should be acquired before calling this method.

   void initComponentGroups();
     //:Initialize the platform-specific component group settings

   void initPlatformButtonSettings(PsButtonTask* pButtonTask);
     //:Initialize the platform-specific settings for the keyboard buttons

        void setGainValue(int value);

   OsRWMutex         mMutex;        // mutex for synchonizing access to data
   PsButtonTask*     mpButtonTask;
   PsHookswTask*     mpHookswTask;
   PsLampTask*       mpLampTask;
   int               mSpkrMode;     // current speaker mode

   TaoObjectMap*     mpListeners;
   TaoReference*     mpListenerCnt;
   TaoObjHandle*         mpActiveListeners;

   // Variables related to the interface between the phone task and the
   // TAO (Telephony Application Objects) layer.
   PsTaoHookswitch*     mpTaoHooksw;         // hookswitch object
   PsTaoButton*         mpTaoButton;         // button object
   PsTaoLamp*           mpTaoLamp;           // lamp object
   PsTaoRinger*         mpTaoRinger;         // ringer object
   PsTaoDisplay*        mpTaoDisplay;        // display object

   PsTaoComponentGroup* mpHeadSetGroup;      // HEAD_SET component group
   PsTaoMicrophone*     mpTaoHeadsetMic;     // headset microphone object
   PsTaoSpeaker*        mpTaoHeadsetSpeaker; // headset speaker object

   PsTaoComponentGroup* mpHandSetGroup;      // HAND_SET component group
   PsTaoMicrophone*     mpTaoHandsetMic;     // handset microphone object
   PsTaoSpeaker*        mpTaoHandsetSpeaker; // handset speaker object

   PsTaoComponentGroup* mpExtSpeakerGroup;   // EXTSPEAKER_PHONE component group
   PsTaoComponentGroup* mpSpeakerPhoneGroup; // SPEAKER_PHONE component group
   PsTaoComponentGroup* mpPhoneSetGroup;     // PHONE_SET component group
   PsTaoMicrophone*     mpTaoBaseMic;        // base microphone object
   PsTaoSpeaker*        mpTaoBaseSpeaker;    // handset speaker object
   PsTaoSpeaker*        mpTaoExtSpeaker;     // external speaker object

   PsTaoComponentGroup* mpOtherGroup;        // OTHER component group
   int mStepSize;                      // speaker volume adjust step size
   int mHigh;                          // speaker volume adjust high volume
   int mLow;                           // speaker volume adjust low volume
   int mNominal;                       // speaker volume nominal value
   int mSplash;                       // speaker volume for splash tone

   // Static data members used to enforce Singleton behavior
   static PsPhoneTask* spInstance;  // pointer to the single instance of
                                    //  the PsPhoneTask class
   static OsBSem       sLock;       // semaphore used to ensure that there
                                    //  is only one instance of this class

   void postListenerMessage(const PsMsg& rMsg);

   PsPhoneTask(const PsPhoneTask& rPsPhoneTask);
     //:Copy constructor (not implemented for this task)

   PsPhoneTask& operator=(const PsPhoneTask& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsPhoneTask_h_
