//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpFlowGraphBase_h_
#define _MpFlowGraphBase_h_

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include <utl/UtlHashMap.h>

#include "os/OsDefs.h"
#include "os/OsMsgQ.h"
#include "os/OsStatus.h"
#include "os/OsRWMutex.h"
#include "os/OsTime.h"
#include "mp/MpResource.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class MpFlowGraphMsg;
class OsMsg;

//:Flow graph for coordinating the execution of media processing resources.
// The media processing for a call is modeled as a directed graph of media
// processing resources (e.g., dejitter, mixer, encode, decode).
//
// <H3>Frame Interval Processing</H3>
// The media processing subsystem works on blocks of data.  Every frame
// processing interval (typically 10 milliseconds), one frame interval's
// worth of media is processed.<br>
// <br>
// At the start of the interval, the media task calls the processNextFrame()
// method for the flow graph.  This method first calls processMessages() to
// handle any messages that have been posted to the flow graph.  Next, if
// any resources or links have been added or removed since the last
// frame processing interval, the computeOrder() method is invoked.
// computeOrder() performs a topological sort on the resources in the flow
// graph to determine the correct resource execution order.  This is done to
// ensure that resources producing output buffers are executed before other
// resources in the flow graph that expect to consume those buffers.  Once
// the execution order has been determined, the processFrame() method for
// each of the resources in the flow graph is executed.
//
// <H3>Stopped vs. Started States</H3>
// A flow graph must be in the <i>MpFlowGraphBase::STARTED</i> state in order to
// process media streams.  For safety, the methods that modify the state of
// the flow graph only take effect when the flow graph is in a quiescent
// state.  If the flow graph is in the STOPPED state, such methods take effect
// immediately.  However, when the flow graph is in the STARTED state, changes
// take effect at the start of the next frame processing interval.
//
// <H3>Synchronization</H3>
// A reader/writer lock is used to seialize access to the object's internal
// data.  A write lock is taken before executing any method that changes the
// object's internal state or posts a message to the flow graph's message
// queue.  A read lock is taken before invoking any method that reads the
// object's internal state information.  When reading internal state that
// is stored in 32 bits or less, the read lock is not acquired since such
// reads are assumed to be atomic.
class MpFlowGraphBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   friend OsStatus MpResource::postMessage(MpFlowGraphMsg& rMsg);

   enum FlowGraphState
   {
      STARTED,
      STOPPED
   };
     //!enumcode: STARTED  - flow graph has been started (is processing media streams)
     //!enumcode: STOPPED  - flow graph is stopped (not processing media streams)

/* ============================ CREATORS ================================== */

   MpFlowGraphBase(int samplesPerFrame, int samplesPerSec);
     //:Constructor

   virtual
   ~MpFlowGraphBase();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsStatus addLink(MpResource& rFrom, int outPortIdx,
                    MpResource& rTo,   int inPortIdx);
     //:Creates a link between the <i>outPortIdx</i> port of the
     //:<i>rFrom</i> resource to the <i>inPortIdx</i> port of the <i>rTo</i>
     //:resource.
     // If the flow graph is not "started", this call takes effect
     // immediately.  Otherwise, the call takes effect at the start of the
     // next frame processing interval.
     //!retcode: OS_SUCCESS - successfully added the new link
     //!retcode: OS_INVALID_ARGUMENT - invalid port index
     //!retcode: OS_UNSPECIFIED - add link attempt failed

   OsStatus addResource(MpResource& rResource, UtlBoolean makeNameUnique=TRUE);
     //:Adds the indicated media processing object to the flow graph.  If
     //:<i>makeNameUnique</i> is TRUE, then if a resource with the same name
     //:already exists in the flow graph, the name for <i>rResource</i> will
     //:be changed (by adding a numeric suffix) to make it unique within the
     //:flow graph.
     // If the flow graph is not "started", this call takes effect
     // immediately.  Otherwise, the call takes effect at the start of the
     // next frame processing interval.
     //!retcode: OS_SUCCESS - success
     //!retcode: OS_UNSPECIFIED - add resource attempt failed

   OsStatus destroyResources(void);
     //:Stops the flow graph, removes all of the resources in the flow graph
     //:and destroys them.
     // If the flow graph is not "started", this call takes effect
     // immediately.  Otherwise, the call takes effect at the start of the
     // next frame processing interval.
     //!retcode: OS_SUCCESS - for now, this method always returns success

   OsStatus disable(void);
     //:Invokes the <i>disable()</i> method for each resource in the flow
     //:graph.
     // Resources must be enabled before they will perform any meaningful
     // processing on the media stream.
     // If the flow graph is not "started", this call takes effect
     // immediately.  Otherwise, the call takes effect at the start of the
     // next frame processing interval.
     //!retcode: OS_SUCCESS - for now, this method always returns success

   OsStatus enable(void);
     //:Invokes the <i>enable()</i> method for each resource in the flow graph.
     // Resources must be enabled before they will perform any meaningful
     // processing on the media stream.
     // If the flow graph is not "started", this call takes effect
     // immediately.  Otherwise, the call takes effect at the start of the
     // next frame processing interval.
     //!retcode: OS_SUCCESS - for now, this method always returns success

   virtual OsStatus gainFocus(void);
     //:Informs the flow graph that it now has the MpMediaTask focus.
     // Only the flow graph that has the focus is permitted to access
     // the audio hardware.
     //!retcode: OS_INVALID - only a MpBasicFlowGraph can have focus.

   OsStatus insertResourceAfter(MpResource& rResource,
                                MpResource& rUpstreamResource,
                                int outPortIdx);
     //:Inserts <i>rResource</i> into the flow graph downstream of the
     //:designated <i>rUpstreamResource</i> resource.
     // The new resource will be inserted on the <i>outPortIdx</i> output
     // link of <i>rUpstreamResource</i>.
     // If the flow graph is not "started", this call takes effect
     // immediately.  Otherwise, the call takes effect at the start of the
     // next frame processing interval.
     //!retcode: OS_SUCCESS - success
     //!retcode: OS_INVALID_ARGUMENT - invalid port index

   OsStatus insertResourceBefore(MpResource& rResource,
                                 MpResource& rDownstreamResource,
                                 int inPortIdx);
     //:Inserts <i>rResource</i> into the flow graph upstream of the
     //:designated <i>rDownstreamResource</i> resource.
     // The new resource will be inserted on the <i>inPortIdx</i> input
     // link of <i>rDownstreamResource</i>.
     // If the flow graph is not "started", this call takes effect
     // immediately.  Otherwise, the call takes effect at the start of the
     // next frame processing interval.
     //!retcode: OS_SUCCESS - success
     //!retcode: OS_INVALID_ARGUMENT - invalid port index

   virtual OsStatus loseFocus(void);
     //:Informs the flow graph that it has lost the MpMediaTask focus.
     // Only the flow graph that has the focus is permitted to access
     // the audio hardware.
     //!retcode: OS_INVALID - only a MpBasicFlowGraph can have focus.

   OsStatus processNextFrame(void);
     //:Processes the next frame interval's worth of media for the flow graph.
     //!retcode: OS_SUCCESS - for now, this method always returns success

   OsStatus removeLink(MpResource& rFrom, int outPortIdx);
     //:Removes the link between the <i>outPortIdx</i> port of the
     //:<i>rFrom</i> resource and its downstream counterpart.
     // If the flow graph is not "started", this call takes effect
     // immediately.  Otherwise, the call takes effect at the start of the
     // next frame processing interval.
     //!retcode: OS_SUCCESS - link has been removed
     //!retcode: OS_INVALID_ARGUMENT - invalid port index

   OsStatus removeResource(MpResource& rResource);
     //:Removes the indicated media processing object from the flow graph.
     // If the flow graph is not "started", this call takes effect
     // immediately.  Otherwise, the call takes effect at the start of the
     // next frame processing interval.
     //!retcode: OS_SUCCESS - success, resource has been removed
     //!retcode: OS_UNSPECIFIED - remove resource operation failed

   OsStatus setSamplesPerFrame(int samplesPerFrame);
     //:Sets the number of samples expected per frame.
     // If the flow graph is not "started", this call takes effect
     // immediately.  Otherwise, the call takes effect at the start of the
     // next frame processing interval.
     //!retcode: OS_SUCCESS - success
     //!retcode: OS_INVALID_ARGUMENT - specified duration is not supported

   OsStatus setSamplesPerSec(int samplesPerSec);
     //:Sets the number of samples expected per second.
     // If the flow graph is not "started", this call takes effect
     // immediately.  Otherwise, the call takes effect at the start of the
     // next frame processing interval.
     //!retcode: OS_SUCCESS - success
     //!retcode: OS_INVALID_ARGUMENT - specified duration is not supported

   OsStatus start(void);
     //:Start this flow graph.
     // A flow graph must be "started" before it will process media streams.
     // This call takes effect immediately.
     //!retcode: OS_SUCCESS - for now this method always returns success

   OsStatus stop(void);
     //:Stop this flow graph.
     // Stop processing media streams with this flow graph.  This call takes
     // effect at the start of the next frame processing interval.
     //!retcode: OS_SUCCESS - for now this method always returns success

/* ============================ ACCESSORS ================================= */

   static void flowGraphInfo(MpFlowGraphBase* pFlowGraph);
     //:Displays information on the console about the specified flow graph.

   int getSamplesPerFrame(void) const;
     //:Returns the number of samples expected per frame.

   int getSamplesPerSec(void) const;
     //:Returns the number of samples expected per second.

   int getState(void) const;
     //:Returns the current state of flow graph.

   OsStatus lookupResource(UtlString name,
                           MpResource*& rpResource);
     //:Sets <i>rpResource</i> to point to the resource that corresponds to
     //:<i>name</i> or to NULL if no matching resource is found.
     //!retcode: OS_SUCCESS - success
     //!retcode: OS_NOT_FOUND - no resource with the specified name

   int numLinks(void) const;
     //:Returns the number of links in the flow graph.

   int numFramesProcessed(void) const;
     //:Returns the number of frames this flow graph has processed.

   int numResources(void) const;
     //:Returns the number of resources in the flow graph.

   OsMsgQ* getMsgQ(void) ;
     //:Returns the message queue used by the flow graph.

/* ============================ INQUIRY =================================== */

   UtlBoolean isStarted(void) const;
     //:Returns TRUE if the flow graph has been started, otherwise FALSE.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   OsRWMutex        mRWMutex;      // reader/writer lock for synchronization

   OsStatus postMessage(const MpFlowGraphMsg& rMsg,
                        const OsTime& rTimeout=OsTime::NO_WAIT);
     //:Posts a message to this flow graph.
     // Returns the result of the message send operation.

   virtual UtlBoolean handleMessage(OsMsg& rMsg);
     //:Handles an incoming message for the flow graph.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleRemoveLink(MpResource* pFrom, int outPortIdx);
     //:Handle the FLOWGRAPH_REMOVE_LINK message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleRemoveResource(MpResource* pResource);
     //:Handle the FLOWGRAPH_REMOVE_RESOURCE message.
     // Returns TRUE if the message was handled, otherwise FALSE.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   enum { MAX_FLOWGRAPH_MESSAGES  = 150};
   enum { MAX_FLOWGRAPH_RESOURCES = 50};

   UtlHashMap mResourceDict; // resource dictionary
   MpResource* mExecOrder[MAX_FLOWGRAPH_RESOURCES]; // resource execution
                                                    //  order
   MpResource* mUnsorted[MAX_FLOWGRAPH_RESOURCES];  // unsorted resources
   int       mCurState;       // current flow graph state
   OsMsgQ    mMessages;       // message queue for this flow graph
   int       mPeriodCnt;      // number of frames processed by this flow graph
   int       mLinkCnt;        // number of links in this flow graph
   int       mResourceCnt;    // number of resources in this flow graph
   UtlBoolean mRecomputeOrder; // TRUE ==> the execution order needs computing
   int       mSamplesPerFrame;// number of samples per frame
   int       mSamplesPerSec;  // number of samples per second
   MpResource* mpResourceInProcess; // For debugging, keep track of what
                                    // resource we are working on in
                                    // processNextFrame().

   OsStatus computeOrder(void);
     //:Computes the execution order for the flow graph by performing a
     //:topological sort on the resource graph.
     //!retcode: OS_SUCCESS - successfully computed an execution order
     //!retcode: OS_LOOP_DETECTED - detected a loop in the flow graph

   UtlBoolean disconnectAllInputs(MpResource* pResource);
     //:Disconnects all inputs (and the corresponding upstream outputs) for
     //:the indicated resource.  Returns TRUE if successful, FALSE otherwise.

   UtlBoolean disconnectAllOutputs(MpResource* pResource);
     //:Disconnects all outputs (and the corresponding downstream inputs) for
     //:the indicated resource.  Returns TRUE if successful, FALSE otherwise.

   UtlBoolean handleAddLink(MpResource* pFrom, int outPortIdx,
                           MpResource* pTo,   int inPortIdx);
     //:Handle the FLOWGRAPH_ADD_LINK message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleAddResource(MpResource* pResource,
                               UtlBoolean makeNameUnique);
     //:Handle the FLOWGRAPH_ADD_RESOURCE message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleDestroyResources(void);
     //:Handle the FLOWGRAPH_DESTROY_RESOURCES message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleDisable(void);
     //:Handle the FLOWGRAPH_DISABLE message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleEnable(void);
     //:Handle the FLOWGRAPH_ENABLE message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleSetSamplesPerFrame(int samplesPerFrame);
     //:Handle the FLOWGRAPH_SET_SAMPLES_PER_FRAME message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleSetSamplesPerSec(int samplesPerSec);
     //:Handle the FLOWGRAPH_SET_SAMPLES_PER_SEC message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleStart(void);
     //:Handle the FLOWGRAPH_START message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleStop(void);
     //:Handle the FLOWGRAPH_STOP message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   OsStatus processMessages(void);
     //:Processes all of the messages currently queued for this flow graph.
     // For now, this method always returns OS_SUCCESS.

   MpFlowGraphBase(const MpFlowGraphBase& rMpFlowGraph);
     //:Copy constructor (not implemented for this task)

   MpFlowGraphBase& operator=(const MpFlowGraphBase& rhs);
     //:Assignment operator (not implemented for this task)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpFlowGraphBase_h_
