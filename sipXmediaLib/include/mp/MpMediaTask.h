//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _MpMediaTask_h_
#define _MpMediaTask_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlHistogram.h"
#include "os/OsDefs.h"
#include "os/OsRWMutex.h"
#include "os/OsServerTask.h"
#include "os/OsMsgPool.h"
#include "mp/MpMediaTaskMsg.h"

// DEFINES

// #define _PROFILE

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class MpFlowGraphBase;
class MpCodecFactory;

//:Object responsible for coordinating the execution of media processing flow
//:graphs.
//
// <H3>Key Concepts</H3>
// Flow graphs are created outside of this class and are initially in the
// MpFlowGraphBase::STOPPED state.  Once a flow graph has been created and while
// it is still in the stopped state, the media processing task is informed of
// the new flow graph via the <i>manageFlowGraph()</i> method.<br>
// <br>
// A flow graph must be in the MpFlowGraphBase::STARTED state before it will
// process media streams.  The <i>startFlowGraph()</i> and
// <i>stopFlowGraph()</i> methods are used to start and stop flow graphs,
// respectively.<br>
// <br>
// The <i>unmanageFlowGraph()</i> method informs the media processing task
// that it should no longer execute the indicated flow graph.  If the flow
// graph is not in the "stopped" state, the media processing task stops the
// flow graph before unmanaging it.<br>
// <br>
// Changes to the flow graphs <i>startFlowGraph(), stopFlowGraph(),
// manageFlowGraph(), unmanageFlowGraph()</i> and <i>setFocus()</i> all take
// effect at frame processing boundaries.<br>
// <br>
// The media processing task expects to receive a notification every frame
// interval indicating that it is time to process the next frame's worth of
// media.  This signal is conveyed by calling the static
// <i>signalFrameStart()</i> method.
//
// <H3>Locking</H3>
// For the most part, this class relies on the atomicity of reads and writes
// to appropriately aligned 32-bit data to avoid the need for locks.  However,
// a single-writer/multiple-reader lock is used to protect the data structure
// that maintains the set of flow graphs currently being managed by the media
// processing task.  The media processing task takes the write lock while
// modifying the data structure.  The <i>getManagedFlowGraphs()</i> method
// acquires the read lock as part of getting the array of flow graphs.
//
// <H3>Messaging</H3>
// Several of the methods in this class post <i>MpMediaTask</i> messages to
// the media processing task. These messages are processed at the beginning
// of the next frame processing interval.  The methods that cause messages
// to be sent are: <i>manageFlowGraph(), unmanageFlowGraph(),
// startFlowGraph(), stopFlowGraph()</i> and <i>setFocus()</i>.<br>
// <br>
// At the beginning of the frame processing interval, the media processing
// task sends itself a "WAIT_FOR_SIGNAL" message.  When the task receives
// that message, it knows that it is time to finish the frame processing for
// the current interval and then wait for the "start" signal for the next
// frame before processing any more messages.
class MpMediaTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   enum { DEF_TIME_LIMIT_USECS    = 6000 };  // processing limit  = 6 msecs
   enum { DEF_SEM_WAIT_MSECS      =  500 };  // semaphore timeout = 0.5 secs

#if defined(_WIN32) || defined(__pingtel_on_posix__) /* [ */
   enum { MEDIA_TASK_PRIO_NORMAL = 0 };             // media task execution priority
#endif /* WIN32 ] */

/* ============================ CREATORS ================================== */

   static MpMediaTask* getMediaTask(int maxFlowGraph);
     //:Return a pointer to the media processing task, creating it if
     //:necessary

   virtual
   ~MpMediaTask();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsStatus manageFlowGraph(MpFlowGraphBase& rFlowGraph);
     //:Directs the media processing task to add the flow graph to its
     //:set of managed flow graphs.  The flow graph must be in the
     //:MpFlowGraphBase::STOPPED state when this method is invoked.
     //!retcode: OS_SUCCESS - the flow graph will be added at the start of the next frame processing interval.
     //!retcode: OS_INVALID_ARGUMENT - flow graph is not in the STOPPED state

   OsStatus unmanageFlowGraph(MpFlowGraphBase& rFlowGraph);
     //:Directs the media processing task to remove the flow graph from its
     //:set of managed flow graphs.
     // If the flow graph is not already in the MpFlowGraphBase::STOPPED state,
     // then the flow graph will be stopped before it is removed from the set
     // of managed flow graphs.
     //!retcode: OS_SUCCESS - indicates that the media task will stop managing the indicated flow graph

   OsStatus setDebug(UtlBoolean enableFlag);
     //:When "debug" mode is enabled, the "time limit" checking is
     //:disabled and the wait for "frame start" timeout is set to "INFINITY".
     // For now, this method always returns OS_SUCCESS.

   OsStatus setFocus(MpFlowGraphBase* pFlowGraph);
     //:Changes the focus to the indicated flow graph.
     // At most one flow graph at a time can have focus.  Only the flow
     // graph that has focus is allowed to access the audio resources
     // (speaker and microphone) of the phone.
     // The affected flow graphs will be modified to reflect the change of
     // focus at the beginning of the next frame interval.
     // For now, this method always returns OS_SUCCESS.

   OsStatus setTimeLimit(int usecs);
     //:Sets the amount of time (in microseconds) allotted to the media
     //:processing task for processing a frame's worth of media.
     // If this time limit is exceeded, the media processing task increments
     // an internal statistic.  The value of this statistic can be retrieved
     // by calling the getLimitExceededCnt() method. For now, this method
     // always returns OS_SUCCESS.

   OsStatus setWaitTimeout(int msecs);
     //:Sets the maximum time (in milliseconds) that the media processing
     //:task will wait for a "frame start" signal. A value of -1 indicates
     //:that the task should wait "forever".
     // The new timeout will take effect at the beginning of the next frame
     // interval. For now, this method always returns OS_SUCCESS.

   static OsStatus signalFrameStart(void);
     //:Release the "frame start" semaphore.  This signals the media
     //:processing task that it should begin processing the next frame.
     // Returns the result of releasing the binary semaphore that is used
     // to send the signal.

   OsStatus startFlowGraph(MpFlowGraphBase& rFlowGraph);
     //:Directs the media processing task to start the specified flow
     //:graph.  A flow graph must be started in order for it to process
     //:the media stream.
     // The flow graph state change will take effect at the beginning of the
     // next frame interval. For now, this method always returns OS_SUCCESS.

   OsStatus stopFlowGraph(MpFlowGraphBase& rFlowGraph);
     //:Directs the media processing task to stop the specified flow
     //:graph.  When a flow graph is stopped it no longer processes the
     //:media stream.
     // The flow graph state change will take effect at the beginning of the
     // next frame interval. For now, this method always returns OS_SUCCESS.

/* ============================ ACCESSORS ================================= */

   int numHandledMsgErrs();
    //: Debug aid for tracking state. See MpMediaTaskTest

   UtlBoolean getDebugMode(void) const;
     //:Returns TRUE if debug mode is enabled, FALSE otherwise.

   MpFlowGraphBase* getFocus(void) const;
     //:Returns the flow graph that currently has focus (access to the audio
     //:apparatus) or NULL if there is no flow graph with focus.

   int getLimitExceededCnt(void) const;
     //:Returns the number of times that the frame processing time limit
     //:has been exceeded.

   OsStatus getManagedFlowGraphs(MpFlowGraphBase* flowGraphs[], const int size,
                                 int& numItems);
     //:Returns an array of MpFlowGraphBase pointers that are presently managed
     //:by the media processing task.
     // The caller is responsible for allocating the flowGraphs array
     // containing room for <i>size</i> pointers.  The number of items
     // actually filled in is passed back via the <i>nItems</i> argument.

   int getTimeLimit(void) const;
     //:Returns the amount of time (in microseconds) allotted to the media
     //:processing task for processing a frame's worth of media.

   int getWaitTimeout(void) const;
     //:Returns the maximum time (in milliseconds) that the media processing
     //:task will wait for the "frame start" signal. A value of -1 indicates
     //:that the task will wait "forever".

   int getWaitTimeoutCnt(void) const;
     //:Returns the number of times that the wait timeout associated with
     //:"frame start" signal has been exceeded.

   static MpFlowGraphBase* mediaInfo(void);
     //:Displays information on the console about the media processing task.

   int numManagedFlowGraphs(void) const;
     //:Returns the number of flow graphs currently being managed by the
     //:media processing task.

   static int maxNumManagedFlowGraphs(void);
     //:Returns the maximum number of flow graphs that can be managed by the
     //:media processing task.

   int numProcessedFrames(void) const;
     //:Returns the number of frames that the media processing task has
     //:processed. This count is maintained as an unsigned, 32-bit value.
     // Note: If the frame period is 10 msecs, then it will take
     // 2^32 / (100 * 3600 * 24 * 365) = 1.36 years before this count wraps.

   int numStartedFlowGraphs(void) const;
     //:Returns the number of flow graphs that have been started by the media
     //:processing task.
     // This value should always be <= the number of managed flow graphs.

   OsMsgPool* getBufferMsgPool(void) const;
     //:Returns pointer to pool of reusable buffer messages

   MpCodecFactory* getCodecFactory(void) const;
     //:Returns pointer to singleton codec factory

/* ============================ INQUIRY =================================== */

   UtlBoolean isManagedFlowGraph(MpFlowGraphBase* pFlowGraph);
     //:Returns TRUE if the indicated flow graph is presently being managed
     //:by the media processing task, otherwise FALSE.

   void getQueueUsage(int& numMsgs, int& softLimit,
                                   int& hardLimit);
     //:Return usage information on the Media Task's message queue.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   MpMediaTask(int maxFlowGraph);
     //:Default constructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsMutex   mMutex;        // lock for synchronization
   UtlBoolean mDebugEnabled; // TRUE if debug mode is enabled, FALSE otherwise

   int       mTimeLimitCnt; // Number of frames where time limit was exceeded
   unsigned  mProcessedCnt; // Number of frames that have been processed
   int       mManagedCnt;   // Number of flow graphs presently managed
   int       mStartedCnt;   // Number of flow graphs presently started
   OsTime    mSemTimeout;   // Timeout value for acquiring the start semaphore
   int       mSemTimeoutCnt;// Number of times the mSemTimeOut was exceeded
   UtlBoolean mWaitForSignal;// If TRUE, then don't handle any incoming msgs
                            //  until a FrameStart signal has been received
   MpFlowGraphBase* mpFocus;    // FlowGraph that has the focus (may be NULL)
   MpFlowGraphBase** mManagedFGs;
   static int mMaxFlowGraph;
                            // The set of flow graphs presently managed
   int       mLimitUsecs;   // Frame processing time limit (in usecs)
   int       mHandleMsgErrs;// Number of message handling problems during the
                            //  last frame processing interval
   OsMsgPool* mpBufferMsgPool;// Pool of reusable buffer messages
   OsMsgPool* mpSignalMsgPool;// Pool of reusable frame signal messages

   MpCodecFactory* mpCodecFactory; // our codec factory

   // Static data members used to enforce Singleton behavior
   static MpMediaTask* spInstance;  // pointer to the single instance of
                                    //  the MpMediaTask class
   static OsMutex      sLock;       // semaphore used to ensure that there
                                    //  is only one instance of this class

#ifdef _PROFILE /* [ */
   // Start time (in microseconds) for the current frame processing interval
   long long mStartTicks;
   // Stop time (in microseconds) for the current frame processing interval
   long long mStopTicks;
   // Frame processing time limit (in nanoseconds)
   unsigned long long mLimitTicks;

   // Histograms for handleWaitForSignal() processing time.
   // Time from start of execution to end.
   UtlHistogram mStartToEndTime;
   // Time from start of execution to start of next execution.
   UtlHistogram mStartToStartTime;
   // Time from end of execution to start of next execution.
   UtlHistogram mEndToStartTime;
   // Time from one signal time to the next.
   UtlHistogram mSignalTime;
   // Time from signal time to the execution it starts.
   UtlHistogram mSignalToStartTime;
   // Time to process messages other than WAIT_FOR_SIGNAL.
   UtlHistogram mOtherMessages;
#endif /* _PROFILE ] */

   // int numQueuedMsgs;
   // MpMediaTaskMsg mpQueuedMsgs[100];
   //
   // void handleQueuedMessages(void);
   UtlBoolean handleMessage(OsMsg& rMsg);
     //:Handle an incoming message
     // Return TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleManage(MpFlowGraphBase* pFlowGraph);
     //:Handles the MANAGE message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleSetFocus(MpFlowGraphBase* pFlowGraph);
     //:Handles the SET_FOCUS message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleStart(MpFlowGraphBase* pFlowGraph);
     //:Handles the START message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleStop(MpFlowGraphBase* pFlowGraph);
     //:Handles the STOP message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleUnmanage(MpFlowGraphBase* pFlowGraph);
     //:Handles the UNMANAGE message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleWaitForSignal(MpMediaTaskMsg*);
     //:Handles the WAIT_FOR_SIGNAL message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   MpMediaTask(const MpMediaTask& rMpMediaTask);
     //:Copy constructor (not implemented for this task)

   MpMediaTask& operator=(const MpMediaTask& rhs);
     //:Assignment operator (not implemented for this task)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpMediaTask_h_
