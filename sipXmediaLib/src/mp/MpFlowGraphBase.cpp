//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>


// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsTask.h"
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "mp/MpFlowGraphBase.h"
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpResourceSortAlg.h"
#include "mp/MpMediaTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpFlowGraphBase::MpFlowGraphBase(int samplesPerFrame, int samplesPerSec)
: mRWMutex(OsRWMutex::Q_PRIORITY),
  mResourceDict(),
  mCurState(STOPPED),
  mMessages(MAX_FLOWGRAPH_MESSAGES),
  mPeriodCnt(0),
  mLinkCnt(0),
  mResourceCnt(0),
  mRecomputeOrder(FALSE),
  mSamplesPerFrame(samplesPerFrame),
  mSamplesPerSec(samplesPerSec),
  mpResourceInProcess(NULL)
{
   int i;

   for (i=0; i < MAX_FLOWGRAPH_RESOURCES; i++)
   {
      mUnsorted[i] = NULL;
      mExecOrder[i] = NULL;
   }
}

// Destructor
MpFlowGraphBase::~MpFlowGraphBase()
{
   int      msecsPerFrame;
   OsStatus res;

   // release the flow graph and any resources it contains
   res = destroyResources();
   assert(res == OS_SUCCESS);

   // since the destroyResources() call may not take effect until the start
   // of the next frame processing interval, we loop until this flow graph is
   // stopped and contains no resources
   msecsPerFrame = (mSamplesPerFrame * 1000) / mSamplesPerSec;
   while (mCurState != STOPPED || mResourceCnt != 0)
   {
      res = OsTask::delay(msecsPerFrame);
      assert(res == OS_SUCCESS);
   }
}

/* ============================ MANIPULATORS ============================== */

// Creates a link between the "outPortIdx" port of the "rFrom" resource
// to the "inPortIdx" port of the "rTo" resource.
// If the flow graph is not "started", this call takes effect immediately.
// Otherwise, the call takes effect at the start of the next frame processing
// interval.
// Returns OS_SUCCESS if the link was successfully added. Returns
// OS_INVALID_ARGUMENT if the caller specified an invalid port index.
// Returns OS_UNSPECIFIED if the addLink attempt failed for some other
// reason.
OsStatus MpFlowGraphBase::addLink(MpResource& rFrom, int outPortIdx,
                              MpResource& rTo,   int inPortIdx)
{
   OsWriteLock    lock(mRWMutex);

   UtlBoolean      handled;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_ADD_LINK, NULL,
                      &rFrom, &rTo, outPortIdx, inPortIdx);

   if (outPortIdx < 0 || outPortIdx >= rFrom.maxOutputs() ||
       inPortIdx  < 0 || inPortIdx  >= rTo.maxInputs())
      return OS_INVALID_ARGUMENT;

   if (mCurState == STARTED)
      return postMessage(msg);

   handled = handleMessage(msg);
   if (handled)
      return OS_SUCCESS;
   else
      return OS_UNSPECIFIED;
}

// Adds the indicated media processing object to the flow graph.  If
// "makeNameUnique" is TRUE, then if a resource with the same name already
// exists in the flow graph, the name for "rResource" will be changed (by
// adding a numeric suffix) to make it unique within the flow graph.
// If the flow graph is not "started", this call takes effect immediately.
// Otherwise, the call takes effect at the start of the next frame processing
// interval.
// Returns OS_SUCCESS if the resource was successfully added.  Otherwise
// returns OS_UNSPECIFIED.
OsStatus MpFlowGraphBase::addResource(MpResource& rResource,
                                  UtlBoolean makeNameUnique)
{
   OsWriteLock    lock(mRWMutex);

   UtlBoolean      handled;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_ADD_RESOURCE, NULL,
                      &rResource, NULL, makeNameUnique);

   if (mCurState == STARTED)
      return postMessage(msg);

   handled = handleMessage(msg);
   if (handled)
      return OS_SUCCESS;
   else
      return OS_UNSPECIFIED;
}

// Stops the flow graph, removes all of the resources in the flow graph
// and destroys them.  If the flow graph is not "started", this call takes
// effect immediately.  Otherwise, the call takes effect at the start of
// the next frame processing interval.  For now, this method always returns
// success.
OsStatus MpFlowGraphBase::destroyResources(void)
{
   OsWriteLock    lock(mRWMutex);

   UtlBoolean      handled;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_DESTROY_RESOURCES, NULL);

   if (mCurState == STARTED)
      return postMessage(msg);

   handled = handleMessage(msg);
   if (handled)
      return OS_SUCCESS;
   else
      return OS_UNSPECIFIED;
}

// Invokes the disable() method for each resource in the flow graph.
// Resources must be enabled before they will perform any meaningful
// processing on the media stream.
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.  For now, this method always returns
// success.
OsStatus MpFlowGraphBase::disable(void)
{
   OsWriteLock    lock(mRWMutex);

   UtlBoolean      handled;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_DISABLE, NULL);

   if (mCurState == STARTED)
      return postMessage(msg);

   handled = handleMessage(msg);
   if (handled)
      return OS_SUCCESS;
   else
      return OS_UNSPECIFIED;
}

// Invokes the enable() method for each resource in the flow graph.
// Resources must be enabled before they will perform any meaningful
// processing on the media stream.
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.  For now, this method always returns
// success.
OsStatus MpFlowGraphBase::enable(void)
{
   OsWriteLock    lock(mRWMutex);

   UtlBoolean      handled;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_ENABLE, NULL);

   if (mCurState == STARTED)
      return postMessage(msg);

   handled = handleMessage(msg);
   if (handled)
      return OS_SUCCESS;
   else
      return OS_UNSPECIFIED;
}

// Notification that this flow graph has just been granted the focus.
// However, we do not want it.
OsStatus MpFlowGraphBase::gainFocus(void)
{
   Nprintf("MpFG::gainFocus(0x%p), not supported!\n", this, 0,0,0,0,0);
   return OS_INVALID_ARGUMENT;
}

// Inserts "rResource" into the flow graph downstream of the
// designated "rUpstreamResource" resource.
// The new resource will be inserted on the "outPortIdx" output
// link of "rUpstreamResource".
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.
// Returns OS_SUCCESS if the resource was successfully inserted. Returns
// OS_INVALID_ARGUMENT if the caller specified an invalid port index.
OsStatus MpFlowGraphBase::insertResourceAfter(MpResource& rResource,
                                 MpResource& rUpstreamResource,
                                 int outPortIdx)
{
   MpResource *pDownstreamResource;
   int         downstreamInPortIdx;
   OsStatus    res;

   // Get information about the downstream end of the link
   rUpstreamResource.getOutputInfo(outPortIdx, pDownstreamResource,
                                   downstreamInPortIdx);

   // Add the new resource to the flow graph
   res = addResource(rResource);
   if (res != OS_SUCCESS)
      return res;

   if (pDownstreamResource != NULL)
   {
      // Remove the link between the upstream and downstream resources
      res = removeLink(rUpstreamResource, outPortIdx);
      if (res != OS_SUCCESS)
      {                              // recover from remove link failure
         removeResource(rResource);
         return res;
      }

      // Add the link between output port 0 the new resource and the
      // downstream resource
      res = addLink(rResource, 0, *pDownstreamResource, downstreamInPortIdx);
      if (res != OS_SUCCESS)
      {                              // recover from add link failure
         removeResource(rResource);
         addLink(rUpstreamResource, outPortIdx,
                 *pDownstreamResource, downstreamInPortIdx);
         return res;
      }
   }

   // Add the link between the upstream resource and input port 0 of
   // the new resource
   res = addLink(rUpstreamResource, outPortIdx, rResource, 0);
   if (res != OS_SUCCESS)
   {                              // recover from add link failure
      removeResource(rResource);
      if (pDownstreamResource != NULL)
      {
         addLink(rUpstreamResource, outPortIdx,
                 *pDownstreamResource, downstreamInPortIdx);
      }
   }

   return res;
}

//:Inserts "rResource" into the flow graph upstream of the
//:designated "rDownstreamResource" resource.
// The new resource will be inserted on the "inPortIdx" input
// link of "rDownstreamResource".
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.
// Returns OS_SUCCESS if the resource was successfully inserted. Returns
// OS_INVALID_ARGUMENT if the caller specified an invalid port index.
OsStatus MpFlowGraphBase::insertResourceBefore(MpResource& rResource,
                                 MpResource& rDownstreamResource,
                                 int inPortIdx)
{
   MpResource *pUpstreamResource;
   int         upstreamOutPortIdx;
   OsStatus    res;

   // Get information about the downstream end of the link
   rDownstreamResource.getInputInfo(inPortIdx, pUpstreamResource,
                                    upstreamOutPortIdx);

   // Add the new resource to the flow graph
   res = addResource(rResource);
   if (res != OS_SUCCESS)
      return res;

   if (pUpstreamResource != NULL)
   {
      // Remove the link between the upstream and downstream resources
      res = removeLink(*pUpstreamResource, upstreamOutPortIdx);
      if (res != OS_SUCCESS)
      {                              // recover from remove link failure
         removeResource(rResource);
         return res;
      }

      // Add the link between output port 0 the new resource and the
      // downstream resource
      res = addLink(rResource, 0, rDownstreamResource, inPortIdx);
      if (res != OS_SUCCESS)
      {                              // recover from add link failure
         removeResource(rResource);
         addLink(*pUpstreamResource, upstreamOutPortIdx,
                 rDownstreamResource, inPortIdx);
         return res;
      }
   }

   // Add the link between the upstream resource and input port 0 of
   // the new resource
   res = addLink(*pUpstreamResource, upstreamOutPortIdx, rResource, 0);
   if (res != OS_SUCCESS)
   {                              // recover from add link failure
      removeResource(rResource);
      if (pUpstreamResource != NULL)
      {
         addLink(*pUpstreamResource, upstreamOutPortIdx,
                 rDownstreamResource, inPortIdx);
      }
   }

   return res;
}

// Notification that this flow graph has just lost the focus.
// However, we did not want it.
OsStatus MpFlowGraphBase::loseFocus(void)
{
   Nprintf("MpFG::loseFocus(0x%p), not supported!\n", this, 0,0,0,0,0);
   return OS_INVALID_ARGUMENT;
}

// Processes the next frame interval's worth of media for the flow graph.
// For now, this method always returns success.
OsStatus MpFlowGraphBase::processNextFrame(void)
{
   UtlBoolean boolRes;
   int       i;
   OsStatus  res;

   // Call processMessages() to handle any messages that have been posted
   // to either resources in the flow graph or to the flow graph itself.
   res = processMessages();
   assert(res == OS_SUCCESS);

   // If resources or links have been added/removed from the flow graph,
   // then we need to recompute the execution order for resources in the
   // flow graph.  This is done to ensure that resources producing output
   // buffers are executed before other resources in the flow graph that
   // expect to consume those buffers.
   if (mRecomputeOrder)
   {
      res = computeOrder();
      assert(res == OS_SUCCESS);
   }

   // If the flow graph is "STOPPED" then there is no further processing
   // required for this frame interval.  However, if the flow graph is
   // "STARTED", we invoke the processFrame() method for each of the
   // resources in the flow graph.
   if (getState() == STARTED)
   {

      for (i=0; i < mResourceCnt; i++)
      {
         mpResourceInProcess = mExecOrder[i];
         boolRes = mExecOrder[i]->processFrame();
         if (!boolRes) {
            osPrintf("MpMedia: called %s, which indicated failure\n",
               mpResourceInProcess->mName.data());
         }
      }
   }

   mpResourceInProcess = NULL;
   mPeriodCnt++;

   return OS_SUCCESS;
}

// Removes the link between the "outPortIdx" port of the "rFrom"
// resource and its downstream counterpart. If the flow graph is not
// "started", this call takes effect immediately.  Otherwise, the call
// takes effect at the start of the next frame processing interval.
// Returns OS_SUCCESS if the link is removed.  Returns
// OS_INVALID_ARGUMENT if the caller specified an invalid port index.
OsStatus MpFlowGraphBase::removeLink(MpResource& rFrom, int outPortIdx)
{
   OsWriteLock    lock(mRWMutex);

   UtlBoolean      handled;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_REMOVE_LINK, NULL,
                      &rFrom, NULL, outPortIdx);

   if (outPortIdx < 0 || outPortIdx >= rFrom.maxOutputs())
      return OS_INVALID_ARGUMENT;

   if (mCurState == STARTED)
      return postMessage(msg);

   handled = handleMessage(msg);
   if (handled)
      return OS_SUCCESS;
   else
      return OS_UNSPECIFIED;
}

// Removes the indicated media processing object from the flow graph.
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.  Returns OS_SUCCESS to indicate that
// the resource has been removed or OS_UNSPECIFIED if the removeResource
// operation failed.
OsStatus MpFlowGraphBase::removeResource(MpResource& rResource)
{
   OsWriteLock    lock(mRWMutex);

   UtlBoolean      handled;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_REMOVE_RESOURCE, NULL,
                      &rResource);

   if (mCurState == STARTED)
      return postMessage(msg);

   handled = handleMessage(msg);
   if (handled)
      return OS_SUCCESS;
   else
      return OS_UNSPECIFIED;
}

// Sets the number of samples expected per frame.
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.
// Returns OS_SUCCESS to indicate success or OS_INVALID_ARGUMENT if the
// indicated "samples per frame" rate is not supported.
OsStatus MpFlowGraphBase::setSamplesPerFrame(int samplesPerFrame)
{
   OsWriteLock    lock(mRWMutex);

   UtlBoolean      handled;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_SET_SAMPLES_PER_FRAME, NULL,
                      NULL, NULL, samplesPerFrame);

   if (mCurState == STARTED)
      return postMessage(msg);

   handled = handleMessage(msg);
   if (handled)
      return OS_SUCCESS;
   else
      return OS_UNSPECIFIED;
}

// Sets the number of samples expected per second.
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.
// Returns OS_SUCCESS to indicate success or OS_INVALID_ARGUMENT if the
// indicated "samples per second" rate is not supported.
OsStatus MpFlowGraphBase::setSamplesPerSec(int samplesPerSec)
{
   OsWriteLock    lock(mRWMutex);

   UtlBoolean      handled;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_SET_SAMPLES_PER_SEC, NULL,
                      NULL, NULL, samplesPerSec);

   if (mCurState == STARTED)
      return postMessage(msg);

   handled = handleMessage(msg);
   if (handled)
      return OS_SUCCESS;
   else
      return OS_UNSPECIFIED;
}

// Start this flow graph.
// A flow graph must be "started" before it will process media streams.
// This call takes effect immediately.  For now, this method always
// returns success.
OsStatus MpFlowGraphBase::start(void)
{
   OsWriteLock lock(mRWMutex);
   UtlBoolean   handled;

   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_START);

   handled = handleMessage(msg);
   if (handled)
      return OS_SUCCESS;
   else
      return OS_UNSPECIFIED;
}

// Stop this flow graph.
// Stop processing media streams with this flow graph.  This call takes
// effect at the start of the next frame processing interval.  For now,
// this method always returns success.
OsStatus MpFlowGraphBase::stop(void)
{
   OsWriteLock    lock(mRWMutex);

   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_STOP);

   return postMessage(msg);
}

/* ============================ ACCESSORS ================================= */

// (static) Displays information on the console about the specified flow
// graph.
void MpFlowGraphBase::flowGraphInfo(MpFlowGraphBase* pFlowGraph)
{
   int         i;
   MpResource* pResource;

   if (NULL == pFlowGraph) {
      MpMediaTask* pMediaTask = MpMediaTask::getMediaTask(0);
      pFlowGraph = pMediaTask->getFocus();
      if (NULL == pFlowGraph) {
         pMediaTask->getManagedFlowGraphs(&pFlowGraph, 1, i);
         if (0 == i) pFlowGraph = NULL;
      }
   }
   if (NULL == pFlowGraph) {
      printf("No flowGraph to display!\n");
      return;
   }
   printf("\nFlow graph information for %p\n", pFlowGraph);
   printf("  State:                    %s\n",
             pFlowGraph->isStarted() ? "STARTED" : "STOPPED");

   printf("  Processed Frame Count:    %d\n",
             pFlowGraph->numFramesProcessed());

   printf("  Samples Per Frame:        %d\n",
             pFlowGraph->getSamplesPerFrame());

   printf("  Samples Per Second:       %d\n",
             pFlowGraph->getSamplesPerSec());

   pResource = pFlowGraph->mpResourceInProcess;
   if (pResource == NULL)
      printf("  Resource Being Processed: NULL\n");
   else
      printf("  Resource Being Processed: %p\n", pResource);

   printf("\n  Resource Information\n");
   printf("    Resources:   %d\n", pFlowGraph->numResources());
   printf("    Links: %d\n", pFlowGraph->numLinks());
   for (i=0; i < pFlowGraph->mResourceCnt; i++)
   {
      pResource = pFlowGraph->mUnsorted[i];
      pResource->resourceInfo(pResource, i);
   }
}

int flowGI(int x) {
   MpFlowGraphBase::flowGraphInfo((MpFlowGraphBase*) x);
   return 0;
}

// Returns the number of samples expected per frame.
int MpFlowGraphBase::getSamplesPerFrame(void) const
{
   return mSamplesPerFrame;
}

// Returns the number of samples expected per second.
int MpFlowGraphBase::getSamplesPerSec(void) const
{
   return mSamplesPerSec;
}

// Returns the current state of flow graph.
int MpFlowGraphBase::getState(void) const
{
   return mCurState;
}

// Sets rpResource to point to the resource that corresponds to
// name  or to NULL if no matching resource is found.
// Returns OS_SUCCESS if there is a match, otherwise returns OS_NOT_FOUND.
OsStatus MpFlowGraphBase::lookupResource(UtlString name,
                                     MpResource*& rpResource)
{
   OsReadLock          lock(mRWMutex);
   UtlString key(name);

   rpResource = (MpResource*) mResourceDict.findValue(&key);

   if (rpResource != NULL)
      return OS_SUCCESS;
   else
      return OS_NOT_FOUND;
}

// Returns the number of links in the flow graph.
int MpFlowGraphBase::numLinks(void) const
{
   return mLinkCnt;
}

// Returns the number of frames this flow graph has processed.
int MpFlowGraphBase::numFramesProcessed(void) const
{
   return mPeriodCnt;
}

// Returns the number of resources in the flow graph.
int MpFlowGraphBase::numResources(void) const
{
   return mResourceCnt;
}

// Returns the message queue used by the flow graph.
OsMsgQ* MpFlowGraphBase::getMsgQ(void)
{
   return &mMessages ;
}

/* ============================ INQUIRY =================================== */

// Returns TRUE if the flow graph has been started, otherwise FALSE.
UtlBoolean MpFlowGraphBase::isStarted(void) const
{
   return (mCurState == STARTED);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Computes the execution order for the flow graph by performing a
// topological sort on the resource graph.
// Returns OS_SUCCESS if an execution order was successfully computed.
// Returns OS_LOOP_DETECTED is a loop was detected in the flow graph.
OsStatus MpFlowGraphBase::computeOrder(void)
{
   OsWriteLock       lock(mRWMutex);

   OsStatus          res;
   MpResourceSortAlg topoSort;

   res = topoSort.doSort(mUnsorted, mExecOrder, mResourceCnt);
   if (res == OS_SUCCESS)
      mRecomputeOrder = FALSE;

   return res;
}

// Disconnects all inputs (and the corresponding upstream outputs) for
// the indicated resource.  Returns TRUE if successful, FALSE otherwise.
UtlBoolean MpFlowGraphBase::disconnectAllInputs(MpResource* pResource)
{
   int         i;
   MpResource* pUpstreamResource;
   int         upstreamPortIdx;

   if (pResource->numInputs() == 0)
      return TRUE;

   for (i = 0; i < pResource->maxInputs(); i++)
   {
      pResource->getInputInfo(i, pUpstreamResource, upstreamPortIdx);
      if (pUpstreamResource != NULL)
      {
         if (!handleRemoveLink(pUpstreamResource, upstreamPortIdx))
         {
             assert(FALSE);
             return FALSE;
         }
      }
   }

   return TRUE;
}

// Disconnects all outputs (and the corresponding downstream inputs) for
// the indicated resource.  Returns TRUE if successful, FALSE otherwise.
UtlBoolean MpFlowGraphBase::disconnectAllOutputs(MpResource* pResource)
{
   int i;

   if (pResource->numOutputs() == 0)
      return TRUE;

   for (i = 0; i < pResource->maxOutputs(); i++)
   {
      if (pResource->isOutputConnected(i))
      {
         if (!handleRemoveLink(pResource, i))
         {
            assert(FALSE);
            return FALSE;
         }
      }
   }

   return TRUE;
}

// Handles an incoming message for the flow graph.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpFlowGraphBase::handleMessage(OsMsg& rMsg)
{
   MpFlowGraphMsg* pMsg = (MpFlowGraphMsg*) &rMsg ;
   UtlBoolean retCode;
   MpResource* ptr1;
   MpResource* ptr2;
   int         int1;
   int         int2;

   retCode = FALSE;

   ptr1 = (MpResource*) pMsg->getPtr1();    // get the parameters out of
   ptr2 = (MpResource*) pMsg->getPtr2();    // the message
   int1 = pMsg->getInt1();
   int2 = pMsg->getInt2();

   switch (pMsg->getMsg())
   {
   case MpFlowGraphMsg::FLOWGRAPH_ADD_LINK:
      retCode = handleAddLink(ptr1, int1, ptr2, int2);
      break;
   case MpFlowGraphMsg::FLOWGRAPH_ADD_RESOURCE:
      retCode = handleAddResource(ptr1, int1);
      break;
   case MpFlowGraphMsg::FLOWGRAPH_DESTROY_RESOURCES:
      retCode = handleDestroyResources();
      break;
   case MpFlowGraphMsg::FLOWGRAPH_DISABLE:
      retCode = handleDisable();
      break;
   case MpFlowGraphMsg::FLOWGRAPH_ENABLE:
      retCode = handleEnable();
      break;
   case MpFlowGraphMsg::FLOWGRAPH_REMOVE_LINK:
      retCode = handleRemoveLink(ptr1, int1);
      break;
   case MpFlowGraphMsg::FLOWGRAPH_REMOVE_RESOURCE:
      retCode = handleRemoveResource(ptr1);
      break;
   case MpFlowGraphMsg::FLOWGRAPH_SET_SAMPLES_PER_FRAME:
      retCode = handleSetSamplesPerFrame(int1);
      break;
   case MpFlowGraphMsg::FLOWGRAPH_SET_SAMPLES_PER_SEC:
      retCode = handleSetSamplesPerSec(int1);
      break;
   case MpFlowGraphMsg::FLOWGRAPH_START:
      retCode = handleStart();
      break;
   case MpFlowGraphMsg::FLOWGRAPH_STOP:
      retCode = handleStop();
      break;
   default:
      break;
   }

   return retCode;
}

static void complainAdd(const char *n1, int p1, const char *n2,
   int p2, const char *n3, int p3)
{
      Zprintf("MpFlowGraphBase::handleAddLink(%s:%d, %s:%d)\n"
         " %s:%d is already connected!\n",
         n1, p1, n2, p2, n3, p3);
}

// Handle the FLOWGRAPH_ADD_LINK message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpFlowGraphBase::handleAddLink(MpResource* pFrom, int outPortIdx,
                                     MpResource* pTo,   int inPortIdx)
{
   // make sure that both resources are part of this flow graph
   if ((pFrom->getFlowGraph() != this) || (pTo->getFlowGraph() != this))
   {
      assert(FALSE);
      return FALSE;
   }

   // make sure both ports are free
   if (pFrom->isOutputConnected(outPortIdx))
   {
         complainAdd(
         pFrom->getName(), outPortIdx,
         pTo->getName(), inPortIdx,
         pFrom->getName(), outPortIdx);
      // assert(FALSE);
      return FALSE;
   }

   if (pTo->isInputConnected(inPortIdx))
   {
         complainAdd(
         pFrom->getName(), outPortIdx,
         pTo->getName(), inPortIdx,
         pTo->getName(), inPortIdx);
      // assert(FALSE);
      return FALSE;
   }

   // build the downstream end of the link
   if (pTo->connectInput(*pFrom, outPortIdx, inPortIdx) == FALSE)
   {
      assert(FALSE);
      return FALSE;
   }

   // build the upstream end of the link
   if (pFrom->connectOutput(*pTo, inPortIdx, outPortIdx) == FALSE)
   {                  // should not happen, but if it does we remove the
      assert(FALSE);  //  downstream end of the link
      pTo->disconnectInput(inPortIdx);
      return FALSE;
   }

   mLinkCnt++;
   mRecomputeOrder = TRUE;

   return TRUE;
}

// Handle the FLOWGRAPH_ADD_RESOURCE message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpFlowGraphBase::handleAddResource(MpResource* pResource,
                                         UtlBoolean makeNameUnique)
{
   UtlString* pInsertedKey;
   UtlString* pKey;

   // make sure that we won't exceed the MAX_FLOWGRAPH_RESOURCES limit
   if (mResourceCnt >= MAX_FLOWGRAPH_RESOURCES)
   {
      assert(FALSE);
      return FALSE;
   }

   // make sure this resource isn't part of another flow graph
   if (pResource->getFlowGraph() != NULL)
   {
      assert(FALSE);
      return FALSE;
   }

   // add the resource to the dictionary
   // $$$ For now we aren't handling the makeNameUnique option, if the name
   // $$$ is not unique, the current code will trigger an assert failure
   OsSysLog::add(FAC_MP, PRI_DEBUG, "MpFlowGraphBase::handleAddResource %s\n",
         					pResource->getName().data());

   pKey = new UtlString(pResource->getName());
   pInsertedKey = (UtlString*)
                  mResourceDict.insertKeyAndValue(pKey, pResource);

   if (pInsertedKey == NULL)
   {                             // insert failed because of non-unique name
      delete pKey;               // clean up the key object
      assert(FALSE);             // $$$ for now, trigger an assert failure
      return FALSE;
   }

   // add the resource to the unsorted array of resources for this flow graph
   mUnsorted[mResourceCnt] = pResource;

   pResource->setFlowGraph(this);

   mResourceCnt++;
   mRecomputeOrder = TRUE;

   return TRUE;
}

// Handle the FLOWGRAPH_DESTROY_RESOURCES message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpFlowGraphBase::handleDestroyResources(void)
{
   int         i;
   int         numResources;
   MpResource* pResource;

   // if not already stopped, then stop the flow graph
   if (mCurState == STARTED)
      if (handleStop() == FALSE)
      {
         assert(FALSE);
         return FALSE;
      }

   // iterate over all resources

   // BE VERY CAREFUL HERE.  The handleRemoveResource() operation
   // SHUFFLES the array we are using to tell us what resources need
   // to be removed.

   // You have been warned.

   numResources = mResourceCnt;
   for (i=numResources-1; i >= 0; i--)
   {
      pResource = mUnsorted[i];

      // disconnect all inputs and outputs
      if ((disconnectAllInputs(pResource) == FALSE) ||
          (disconnectAllOutputs(pResource) == FALSE))
      {
         assert(FALSE);
         return FALSE;
      }

      // remove the resource from the flow graph
      if (handleRemoveResource(pResource) == FALSE)
      {
         assert(FALSE);
         return FALSE;
      }

      // destroy the resource
      delete pResource;
   }

   return TRUE;
}

// Handle the FLOWGRAPH_DISABLE message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpFlowGraphBase::handleDisable(void)
{
   int            i;
   MpFlowGraphMsg msg(MpFlowGraphMsg::RESOURCE_DISABLE);
   MpResource*    pResource;

   // iterate over all resources
   // invoke the disable() method for each resource in the flow graph
   for (i=0; i < mResourceCnt; i++)
   {
      // iterate through the resources
      pResource = mUnsorted[i];

      // make each resource handle a RESOURCE_DISABLE message
      msg.setMsgDest(pResource);
      if (!pResource->handleMessage(msg))
      {
         assert(FALSE);
         return FALSE;
      }
   }

   return TRUE;
}

// Handle the FLOWGRAPH_ENABLE message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpFlowGraphBase::handleEnable(void)
{
   int            i;
   MpFlowGraphMsg msg(MpFlowGraphMsg::RESOURCE_ENABLE);
   MpResource*    pResource;

   // iterate over all resources
   // invoke the enable() method for each resource in the flow graph
   for (i=0; i < mResourceCnt; i++)
   {
      // iterate through the resources
      pResource = mUnsorted[i];

      // make each resource handle a RESOURCE_ENABLE message
      msg.setMsgDest(pResource);
      if (!pResource->handleMessage(msg))
      {
         assert(FALSE);
         return FALSE;
      }
   }

   return TRUE;
}

// Handle the FLOWGRAPH_REMOVE_LINK message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpFlowGraphBase::handleRemoveLink(MpResource* pFrom, int outPortIdx)
{
   int         connectedPort;
   MpResource* pConnectedResource;

   // make sure the resource is part of this flow graph
   if (pFrom->getFlowGraph() != this)
   {
      Zprintf("handleRemoveLink: pFrom->getFlowGraph() != this: 0x%p != 0x%p\n",
         (pFrom->getFlowGraph()), this, 0,0,0,0);
      assert(FALSE);
      return FALSE;
   }

   // get information about the downstream end of the link
   pFrom->getOutputInfo(outPortIdx, pConnectedResource, connectedPort);

   // disconnect the upstream end of the link
   if (pFrom->disconnectOutput(outPortIdx) == FALSE)
   {
      Zprintf("handleRemoveLink: disconnectOutput(0x%p, %d) failed\n",
         pFrom, outPortIdx, 0,0,0,0);
      assert(FALSE);    // couldn't disconnect
      return FALSE;
   }

   // disconnect the downstream end of the link
   if (pConnectedResource->disconnectInput(connectedPort) == FALSE)
   {
      Zprintf("handleRemoveLink: disconnectInput(0x%p, %d) failed\n",
         pConnectedResource, connectedPort, 0,0,0,0);
      assert(FALSE);    // couldn't disconnect
      return FALSE;
   }

   mLinkCnt--;
   mRecomputeOrder = TRUE;

   return TRUE;
}

// Handle the FLOWGRAPH_REMOVE_RESOURCE message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpFlowGraphBase::handleRemoveResource(MpResource* pResource)
{
   UtlBoolean            found;
   int                  i;
   UtlString* pDictKey;
   UtlString* pKey;

   // make sure this resource is part of this flow graph
   if (pResource->getFlowGraph() != this)
   {
      Zprintf("handleRemoveResource:\n"
         "  pResource=0x%p, pResource->getFlowGraph()=0x%p, this=0x%p\n",
         pResource, (pResource->getFlowGraph()), this, 0,0,0);
      assert(FALSE);
      return FALSE;
   }

   // remove all input links from this resource
   if (disconnectAllInputs(pResource) == FALSE)
   {
      assert(FALSE);
      return FALSE;
   }

   // remove all output links from this resource
   if (disconnectAllOutputs(pResource) == FALSE)
   {
      assert(FALSE);
      return FALSE;
   }

   // remove the entry from the dictionary for this resource
   OsSysLog::add(FAC_MP, PRI_DEBUG, "MpFlowGraphBase::handleRemoveResource %s\n",
         						pResource->getName().data());
   pKey = new UtlString(pResource->getName());
   pDictKey = (UtlString*) mResourceDict.remove(pKey);
   delete pKey;

   if (pDictKey == NULL)
   {
      assert(FALSE);         // no entry found for the resource
      return FALSE;
   }
   delete pDictKey;          // get rid of the dictionary key for the entry

   // remove the resource from the unsorted array of resources for this graph
   found = FALSE;
   for (i=0; i < mResourceCnt; i++)
   {
      if (found)
      {                                   // shift entries following the
         mUnsorted[i-1] = mUnsorted[i];   //  deleted resource down by one
      }
      else if (mUnsorted[i] == pResource)
      {
         found = TRUE;
         mUnsorted[i] = NULL;      // clear the entry
      }
   }

   if (!found)
   {
      assert(FALSE);               // didn't find the entry
      return FALSE;
   }

   pResource->setFlowGraph(NULL);  // remove the reference to this flow graph

   mResourceCnt--;
   mUnsorted[mResourceCnt] = NULL;
   mRecomputeOrder = TRUE;

   return TRUE;
}

// Handle the FLOWGRAPH_SET_SAMPLES_PER_FRAME message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpFlowGraphBase::handleSetSamplesPerFrame(int samplesPerFrame)
{
   int            i;
   MpFlowGraphMsg msg(MpFlowGraphMsg::RESOURCE_SET_SAMPLES_PER_FRAME,
                      NULL, NULL, NULL, samplesPerFrame);
   MpResource*    pResource;

   // iterate over all resources
   for (i=0; i < mResourceCnt; i++)
   {
      pResource = mUnsorted[i];

      // make each resource handle a SET_SAMPLES_PER_FRAME message
      msg.setMsgDest(pResource);
      if (!pResource->handleMessage(msg))
      {
         assert(FALSE);
         return FALSE;
      }
   }

   mSamplesPerFrame = samplesPerFrame;
   return TRUE;
}

// Handle the FLOWGRAPH_SET_SAMPLES_PER_SEC message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpFlowGraphBase::handleSetSamplesPerSec(int samplesPerSec)
{
   int            i;
   MpFlowGraphMsg msg(MpFlowGraphMsg::RESOURCE_SET_SAMPLES_PER_SEC,
                      NULL, NULL, NULL, samplesPerSec);
   MpResource*    pResource;

   // iterate over all resources
   for (i=0; i < mResourceCnt; i++)
   {
      pResource = mUnsorted[i];

      // make each resource handle a SET_SAMPLES_PER_SEC message
      msg.setMsgDest(pResource);
      if (!pResource->handleMessage(msg))
      {
         assert(FALSE);
         return FALSE;
      }
   }

   mSamplesPerSec = samplesPerSec;
   return TRUE;
}

// Handle the FLOWGRAPH_START message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpFlowGraphBase::handleStart(void)
{
   mCurState  = STARTED;

   return TRUE;
}

// Handle the FLOWGRAPH_STOP message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpFlowGraphBase::handleStop(void)
{
   mCurState  = STOPPED;

   return TRUE;
}

// Posts a message to this flow graph.
// Returns the result of the message send operation.
OsStatus MpFlowGraphBase::postMessage(const MpFlowGraphMsg& rMsg,
                                  const OsTime& rTimeout)
{
   OsStatus res;

   res = mMessages.send(rMsg, rTimeout);
   return res;
}

// Processes all of the messages currently queued for this flow graph.
// For now, this method always returns OS_SUCCESS.
OsStatus MpFlowGraphBase::processMessages(void)
{
   OsWriteLock     lock(mRWMutex);

   UtlBoolean       done;
   UtlBoolean       handled;
   static MpFlowGraphMsg* pStopMsg = NULL;
   MpResource*     pMsgDest;

   OsStatus        res;

   // First, we send ourselves a FLOWGRAPH_PROCESS_FRAME message.
   // This message serves as a "stopper" in the message queue.  When we
   // handle that message, we know that we have processed all of the messages
   // for the flowgraph (and its resources) that had arrived prior to the
   // start of this frame processing interval.

   if (NULL == pStopMsg) {
      pStopMsg = new MpFlowGraphMsg(MpFlowGraphMsg::FLOWGRAPH_PROCESS_FRAME);
      pStopMsg->setReusable(TRUE);
   }

   res = postMessage(*pStopMsg);
   assert(res == OS_SUCCESS);

   done = FALSE;
   while (!done)
   {
      // get the next message
      OsMsg* pMsg ;

      res = mMessages.receive(pMsg, OsTime::NO_WAIT);

      assert(res == OS_SUCCESS);

      if (pMsg->getMsgType() == OsMsg::MP_FLOWGRAPH_MSG)
      {
         MpFlowGraphMsg* pRcvdMsg = (MpFlowGraphMsg*) pMsg ;
         // determine if this message is intended for a resource in the
         // flow graph (as opposed to a message for the flow graph itself)
         pMsgDest = pRcvdMsg->getMsgDest();


         if (pMsgDest != NULL)
         {
            // deliver the message if the resource is still part of this graph
            if (pMsgDest->getFlowGraph() == this)
            {
               handled = pMsgDest->handleMessage(*pRcvdMsg);
               assert(handled);
            }
         }
         else
         {
            // since pMsgDest is NULL, this msg is intended for the flow graph
            switch (pRcvdMsg->getMsg())
            {
            case MpFlowGraphMsg::FLOWGRAPH_PROCESS_FRAME:
               done = TRUE;    // "stopper" message encountered -- we are done
               break;          //  processing messages for this frame interval

            default:
               handled = handleMessage(*pRcvdMsg);
               assert(handled);
               break;
            }
         }
         pRcvdMsg->releaseMsg();    // free the msg
      }
      else
      {
         handled = handleMessage(*pMsg);
         assert(handled);
         pMsg->releaseMsg() ;
      }

   }

   return OS_SUCCESS;
}

/* ============================ FUNCTIONS ================================= */
