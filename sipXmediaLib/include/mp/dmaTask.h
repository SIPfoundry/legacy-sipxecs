//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _INCLUDED_DMATASK_H /* [ */
#define _INCLUDED_DMATASK_H

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsTask.h"
#include "os/OsBSem.h"
#include "mp/MpMisc.h"

// DEFINES
/* the maximum number of buffers in input or output queue */
#define DMA_QLEN 10

#ifdef _WIN32
#  define N_BUFFERS           100
#  define USER_BUFFER_MASK    0xff   // if N_BUFFERS is greater than 256, must change USER_BUFFER_MASK!

#  define N_IN_BUFFERS        N_BUFFERS
#  define N_OUT_BUFFERS       N_BUFFERS

#  define N_IN_PRIME          32 // must not be more than N_IN_BUFFERS:
#  define N_OUT_PRIME         8 // must not be more than N_OUT_BUFFERS:

#  define N_SAMPLES           80

#  define SAMPLES_PER_SEC     8000
#  define MIN_SAMPLE_RATE     100
#  define BITS_PER_SAMPLE     16
#endif


// MACROS
// EXTERNAL FUNCTIONS
int showFrameCount(int silent);
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef void (*MuteListenerFuncPtr)(bool);

// FORWARD DECLARATIONS
extern OsStatus dmaStartup(int samplesPerFrame);/* initialize the DMA driver */
extern void dmaShutdown(void);                  /* release the DMA driver */

extern int unMuteSpkr(void);
extern int muteSpkr(void);
extern int unMuteMic(void);
extern int muteMic(void);

enum MpDmaMicChoice {
    MP_MIC_SELECT_NEITHER,
    MP_MIC_SELECT_HANDSET,
    MP_MIC_SELECT_BASE,
};

extern OsStatus MpDma_selectMic(MpDmaMicChoice choice);
extern MpDmaMicChoice MpDma_getMicMode(void);

extern unsigned short       gTableSize;
extern unsigned short       shpAttenTable[];

#ifdef WIN32 /* [ */
extern int DmaTask_setSpkrQPreload(int qlen);
extern int DmaTask_setMicQPreload(int qlen);
#endif /* WIN32 ] */

class DmaTask : public OsTask
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   static DmaTask* getDmaTask(int samplesPerFrame = 80);
     //:Return a pointer to the DMA task, creating it if necessary

   virtual
   ~DmaTask();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
   virtual int run(void* pArg);


/* ============================ ACCESSORS ================================= */

    static bool setRingerEnabled(bool enabled)
    {
        bool prevSetting = smIsRingerEnabled;
        smIsRingerEnabled = enabled;
        return prevSetting;
    } ;
    //: Maintain the "ringer" state for the device and return the previous setting.

    static bool isRingerEnabled(void) { return smIsRingerEnabled;} ;
    //: Test whether the device's ringer is currently enabled.

    static void setMuteListener(MuteListenerFuncPtr pFunc) { smpMuteListener = pFunc; } ;
    //: Set a listener for "mute" state changes.  To remove the listener, set the
    //: listener to NULL.

    static bool setMuteEnabled(bool enabled)
    {
        bool prevSetting = smIsMuted;
        smIsMuted = enabled;
        if (smpMuteListener)
        {
            smpMuteListener(enabled);
        }
        return prevSetting;
    };
    //: Set the "mute" state for the device and return the previous setting.

    static bool isMuteEnabled(void) { return smIsMuted; } ;
    //: Test whether the device's ringer is currently enabled.

    static void setRingDevice(const char* szDevice)
    {
        mRingDeviceName = szDevice ;
        mbOutputDevicesChanged = true ;

    };

    static void setCallDevice(const char* szDevice)
    {
        mCallDeviceName = szDevice ;
        mbOutputDevicesChanged = true ;

    };

    static void setInputDevice(const char* szDevice)
    {
        mMicDeviceName = szDevice ;
        mbInputDeviceChanged = true ;
    };

    static UtlString& getRingDevice() { return mRingDeviceName ;} ;
    static UtlString& getCallDevice() { return mCallDeviceName ;} ;
    static UtlString& getMicDevice() { return mMicDeviceName ;} ;
    static bool isOutputDeviceChanged() { return mbOutputDevicesChanged ;} ;
    static void clearOutputDeviceChanged() { mbOutputDevicesChanged = false ;} ;
    static bool isInputDeviceChanged() { return mbInputDeviceChanged ;} ;
    static void clearInputDeviceChanged() { mbInputDeviceChanged = false ;} ;

/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   DmaTask(MSG_Q_ID doneQ = NULL, int samplesPerFrame = 0,
      int prio    = DEF_DMA_TASK_PRIORITY,      // default task priority
      int options = DEF_DMA_TASK_OPTIONS,       // default task options
      int stack   = DEF_DMA_TASK_STACKSIZE);    // default task stacksize
     //:Default constructor

    static bool smIsRingerEnabled ;
    static bool smIsMuted ;
    static MuteListenerFuncPtr smpMuteListener ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   static const int DEF_DMA_TASK_PRIORITY;      // default task priority
   static const int DEF_DMA_TASK_OPTIONS;       // default task options
   static const int DEF_DMA_TASK_STACKSIZE;     // default task stacksize

   int       mFrameSamples; // Number of samples per frame
   MSG_Q_ID  mDoneQ;        // Message queue to wait on

   static UtlString mRingDeviceName;   // Name of "ring" device (speaker)
   static UtlString mCallDeviceName;   // Name of in-call device (speaker)
   static UtlString mMicDeviceName;    // Name of in-call input device (mic)
   static bool      mbOutputDevicesChanged;    // Has the output device changed?
   static bool      mbInputDeviceChanged;      // Has the output device changed?

   // Static data members used to enforce Singleton behavior
   static DmaTask* spInstance;  // pointer to the single instance of
                                    //  the MpDmaTask class
   static OsBSem       sLock;       // semaphore used to ensure that there
                                    //  is only one instance of this class

   DmaTask(const DmaTask& rDmaTask);
     //:Copy constructor (not implemented for this task)

   DmaTask& operator=(const DmaTask& rhs);
     //:Assignment operator (not implemented for this task)

};

/*************************************************************************/
#endif /* _INCLUDED_DMATASK_H ] */
