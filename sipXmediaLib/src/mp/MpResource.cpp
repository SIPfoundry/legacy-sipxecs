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
#include "mp/MpFlowGraphBase.h"
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpResource.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType MpResource::TYPE = "MpResource";

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpResource::MpResource(const UtlString& rName, int minInputs, int maxInputs,
                       int minOutputs, int maxOutputs,
                       int samplesPerFrame, int samplesPerSec)
:  mRWMutex(OsRWMutex::Q_PRIORITY),
   mpFlowGraph(NULL),
   mIsEnabled(FALSE),
   mMaxInputs(maxInputs),
   mMaxOutputs(maxOutputs),
   mMinInputs(minInputs),
   mMinOutputs(minOutputs),
   mName(rName),
   mNumActualInputs(0),
   mNumActualOutputs(0),
   mSamplesPerFrame(samplesPerFrame),
   mSamplesPerSec(samplesPerSec),
   mVisitState(NOT_VISITED)
{
   int i;

   // Perform a sanity check on the input arguments
   assert((minInputs >= 0) && (minOutputs >= 0) &&
          (maxInputs >= 0) && (maxOutputs >= 0) &&
          (minInputs <= maxInputs) && (minOutputs <= maxOutputs));

   // Allocate arrays for input/output link objects and input/output
   // buffer.  Size the arrays so as to accommodate the maximum number of
   // input and output links supported for this resource.
   mpInConns  = new Conn[maxInputs];
   mpOutConns = new Conn[maxOutputs];
   mpInBufs   = new MpBufPtr[maxInputs];
   mpOutBufs  = new MpBufPtr[maxOutputs];

   for (i=0; i < maxInputs; i++)       // initialize the input port storage
   {
      mpInConns[i].pResource = NULL;
      mpInConns[i].portIndex = -1;
      mpInBufs[i] = NULL;
   }

   for (i=0; i < maxOutputs; i++)      // initialize the output port storage
   {
      mpOutConns[i].pResource = NULL;
      mpOutConns[i].portIndex = -1;
      mpOutBufs[i] = NULL;
   }

}

// Destructor
MpResource::~MpResource()
{
   int i;

   for (i=0; i < mMaxInputs; i++)
      MpBuf_delRef(mpInBufs[i]);       // free all input buffers

   for (i=0; i < mMaxOutputs; i++)
      MpBuf_delRef(mpOutBufs[i]);      // free all output buffers

   delete[] mpInConns;
   mpInConns = 0;
   delete[] mpOutConns;
   mpOutConns = 0;

   delete[] mpInBufs;
   mpInBufs = 0;
   delete[] mpOutBufs;
   mpOutBufs = 0;
}

/* ============================ MANIPULATORS ============================== */

// Disable this resource.
// Returns TRUE if successful, FALSE otherwise.
// The "enabled" flag is passed to the doProcessFrame() method
// and will likely affect the media processing that is performed by this
// resource.  Typically, if a resource is not enabled,
// doProcessFrame() will perform only minimal processing (for
// of a one input / one output resource).
UtlBoolean MpResource::disable(void)
{
   MpFlowGraphMsg msg(MpFlowGraphMsg::RESOURCE_DISABLE, this);
   OsStatus       res;

   res = postMessage(msg);
   return (res == OS_SUCCESS);
}

// Enable this resource.
// Returns TRUE if successful, FALSE otherwise.
// The "enabled" flag is passed to the doProcessFrame() method
// and will likely affect the media processing that is performed by this
// resource.  Typically, if a resource is not enabled,
// doProcessFrame() will perform only minimal processing (for
// example, passing the input straight through to the output in the case
// of a one input / one output resource).
UtlBoolean MpResource::enable(void)
{
   MpFlowGraphMsg msg(MpFlowGraphMsg::RESOURCE_ENABLE, this);
   OsStatus       res;

   res = postMessage(msg);
   return (res == OS_SUCCESS);
}

// Wrapper around doProcessFrame().
// Returns TRUE if successful, FALSE otherwise.
// This method prepares the input buffers before calling
// doProcessFrame() and distributes the output buffers to the
// appropriate downstream resources after doProcessFrame()
// returns.
UtlBoolean MpResource::processFrame(void)
{
   int       i;
   UtlBoolean res;

#define WATCH_FRAME_PROCESSING
#undef  WATCH_FRAME_PROCESSING
#ifdef WATCH_FRAME_PROCESSING /* [ */
   char      z[500];
   const char* pName;
   int       len;
#endif /* WATCH_FRAME_PROCESSING ] */

#ifdef WATCH_FRAME_PROCESSING /* [ */
   pName = mName;
   len = sprintf(z, "%s(", pName);
   for (i=0; i < mMaxInputs; i++)
   {
      if (mpInBufs[i] != NULL)
      {
         len += sprintf(z+len, "%d,", MpBuf_bufNum(mpInBufs[i]));
      } else {
         len += sprintf(z+len, "-,");
      }
   }
   if (mMaxInputs > 0) len--;
   len += sprintf(z+len, ")..(");
#endif /* WATCH_FRAME_PROCESSING ] */

   // call doProcessFrame to do any "real" work
   res = doProcessFrame(mpInBufs, mpOutBufs,
                        mMaxInputs, mMaxOutputs, mIsEnabled,
                        mSamplesPerFrame, mSamplesPerSec);

#ifdef WATCH_FRAME_PROCESSING /* [ */
   for (i=0; i < mMaxInputs; i++)
   {
      if (mpInBufs[i] != NULL)
      {
         len += sprintf(z+len, "%d,", MpBuf_bufNum(mpInBufs[i]));
      } else {
         len += sprintf(z+len, "-,");
      }
   }
   if (mMaxInputs > 0) len--;
   len += sprintf(z+len, ")..(");

   for (i=0; i < mMaxOutputs; i++)
   {
      if (mpOutBufs[i] != NULL)
      {
         len += sprintf(z+len, "%d,", MpBuf_bufNum(mpOutBufs[i]));
      } else {
         len += sprintf(z+len, "-,");
      }
   }
   if (mMaxOutputs > 0) len--;
   len += sprintf(z+len, ")\n");
   z[len] = 0;
   Zprintf("%s", (int) z, 0,0,0,0,0);
#endif /* WATCH_FRAME_PROCESSING ] */

   // delete any input buffers that were not consumed by doProcessFrame()
   for (i=0; i < mMaxInputs; i++)
   {
      if (mpInBufs[i] != NULL)
      {
         MpBuf_delRef(mpInBufs[i]);
         mpInBufs[i] = NULL;
      }
   }

   // pass the output buffers downstream
   for (i=0; i < mMaxOutputs; i++)
   {
      if (!setOutputBuffer(i, mpOutBufs[i])) MpBuf_delRef(mpOutBufs[i]);
      mpOutBufs[i] = NULL;
   }

   return res;
}

// Sets the number of samples expected per frame.
// Returns FALSE if the specified rate is not supported, TRUE otherwise.
UtlBoolean MpResource::setSamplesPerFrame(int samplesPerFrame)
{
   MpFlowGraphMsg msg(MpFlowGraphMsg::RESOURCE_SET_SAMPLES_PER_FRAME, this,
                      NULL, NULL, samplesPerFrame);
   OsStatus       res;

   res = postMessage(msg);
   return (res == OS_SUCCESS);
}

// Sets the number of samples expected per second.
// Returns FALSE if the specified rate is not supported, TRUE otherwise.
UtlBoolean MpResource::setSamplesPerSec(int samplesPerSec)
{
   MpFlowGraphMsg msg(MpFlowGraphMsg::RESOURCE_SET_SAMPLES_PER_SEC, this,
                      NULL, NULL, samplesPerSec);
   OsStatus       res;

   res = postMessage(msg);
   return (res == OS_SUCCESS);
}

// Sets the visit state for this resource (used in performing a
// topological sort on the resources contained within a flow graph).
void MpResource::setVisitState(int newState)
{
   assert(newState >= NOT_VISITED && newState <= FINISHED);

   mVisitState = newState;
}

/* ============================ ACCESSORS ================================= */

// (static) Displays information on the console about the specified flow
// graph.
void MpResource::resourceInfo(MpResource* pResource, int index)
{
   int         i;
   const char*       name;

   name = pResource->getName();
   printf("    Resource[%d]: %p, %s (%sabled)\n",
          index, pResource, name, pResource->mIsEnabled ? "En" : "Dis");

   for (i=0; i<pResource->mMaxInputs; i++) {
      if (NULL != pResource->mpInConns[i].pResource) {
         name = pResource->mpInConns[i].pResource->getName();
         printf("        Input %d from %s:%d\n", i,
            name, pResource->mpInConns[i].portIndex);
      }
   }

   for (i=0; i<pResource->mMaxOutputs; i++) {
      if (NULL != pResource->mpOutConns[i].pResource) {
         name = pResource->mpOutConns[i].pResource->getName();
         printf("        Output %d to %s:%d\n", i,
            name, pResource->mpOutConns[i].portIndex);
      }
   }
}


// Returns the flow graph that contains this resource or NULL if the
// resource is not presently part of any flow graph.
MpFlowGraphBase* MpResource::getFlowGraph(void) const
{
   return mpFlowGraph;
}

// Returns information about the upstream end of a link to the
// "inPortIdx" input on this resource.  If "inPortIdx" is invalid or
// there is no link, then "rpUpstreamResource" will be set to NULL.
void MpResource::getInputInfo(int inPortIdx, MpResource*& rpUpstreamResource,
                              int& rUpstreamPortIdx) const
{
   if (inPortIdx < 0 || inPortIdx >= mMaxInputs)
   {
      rpUpstreamResource = NULL;        // inPortIdx is out of range
      rUpstreamPortIdx   = -1;
   }
   else
   {
      rpUpstreamResource = mpInConns[inPortIdx].pResource;
      rUpstreamPortIdx   = mpInConns[inPortIdx].portIndex;
   }
}

// Returns the name associated with this resource.
UtlString MpResource::getName(void) const
{
   return mName;
}

// Returns information about the downstream end of a link to the
// "outPortIdx" output on this resource.  If "outPortIdx" is invalid or
// there is no link, then "rpDownstreamResource" will be set to NULL.
void MpResource::getOutputInfo(int outPortIdx,
                               MpResource*& rpDownstreamResource,
                               int& rDownstreamPortIdx) const
{
   if (outPortIdx < 0 || outPortIdx >= mMaxOutputs)
   {
      rpDownstreamResource = NULL;      // outPortIdx is out of range
      rDownstreamPortIdx   = -1;
   }
   else
   {
      rpDownstreamResource = mpOutConns[outPortIdx].pResource;
      rDownstreamPortIdx   = mpOutConns[outPortIdx].portIndex;
   }
}

// Returns the current visit state for this resource (used in performing
// a topological sort on the resources contained within a flow graph).
int MpResource::getVisitState(void)
{
   return mVisitState;
}

// Returns the maximum number of inputs supported by this resource.
int MpResource::maxInputs(void) const
{
   return mMaxInputs;
}

// Returns the maximum number of outputs supported by this resource.
int MpResource::maxOutputs(void) const
{
   return mMaxOutputs;
}

// Returns the minimum number of inputs required by this resource.
int MpResource::minInputs(void) const
{
   return mMinInputs;
}

// Returns the minimum number of outputs required by this resource.
int MpResource::minOutputs(void) const
{
   return mMinOutputs;
}

// Returns the number of resource inputs that are currently connected.
int MpResource::numInputs(void) const
{
   return mNumActualInputs;
}

// Returns the number of resource outputs that are currently connected.
int MpResource::numOutputs(void) const
{
   return mNumActualOutputs;
}

// Calculate a unique hash code for this object.
unsigned MpResource::hash() const
{
    return hashPtr(this);
}

// Get the ContainableType for a UtlContainable derived class.
UtlContainableType MpResource::getContainableType() const
{
    return TYPE;
}

/* ============================ INQUIRY =================================== */

// Returns TRUE is this resource is currently enabled, FALSE otherwise.
UtlBoolean MpResource::isEnabled(void) const
{
   return mIsEnabled;
}

// Returns TRUE if portIdx is valid and the indicated input is connected,
// FALSE otherwise.
UtlBoolean MpResource::isInputConnected(int portIdx) const
{
   if (portIdx < 0 || portIdx >= mMaxInputs)  // portIdx out of range
      return FALSE;

   return (mpInConns[portIdx].pResource != NULL);
}

// Returns TRUE if portIdx is valid and the indicated input is not connected,
// FALSE otherwise.
UtlBoolean MpResource::isInputUnconnected(int portIdx) const
{
   if (portIdx < 0 || portIdx >= mMaxInputs)  // portIdx out of range
      return FALSE;

   return (mpInConns[portIdx].pResource == NULL);
}

// Returns TRUE if portIdx is valid and the indicated output is connected,
// FALSE otherwise.
UtlBoolean MpResource::isOutputConnected(int portIdx) const
{
   if (portIdx < 0 || portIdx >= mMaxOutputs) // portIdx out of range
      return FALSE;

   return (mpOutConns[portIdx].pResource != NULL);
}

// Returns TRUE if portIdx is valid and the indicated output is not connected,
// FALSE otherwise.
UtlBoolean MpResource::isOutputUnconnected(int portIdx) const
{
   if (portIdx < 0 || portIdx >= mMaxOutputs) // portIdx out of range
      return FALSE;

   return (mpOutConns[portIdx].pResource == NULL);
}

// Compare the this object to another like-object.
int MpResource::compareTo(UtlContainable const * inVal) const
{
   int result ;

   if (inVal->isInstanceOf(getContainableType()))
   {
      result = comparePtrs(this, inVal);
   }
   else
   {
      result = -1;
   }

   return result;
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Returns a pointer to the incoming buffer for the inPortIdx
// input port if a buffer is available.  Returns NULL if either no
// buffer is available or there is no resource connected to the
// specified port or the inPortIdx is out of range.
MpBufPtr MpResource::getInputBuffer(int inPortIdx) const
{
   if ((inPortIdx < 0) || (inPortIdx >= mMaxInputs) ||// port out of range
       (mpInConns[inPortIdx].pResource == NULL))       // no connected resource
      return NULL;

   return mpInBufs[inPortIdx];
}

// Handles an incoming message for this media processing object.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpResource::handleMessage(MpFlowGraphMsg& rMsg)
{
   UtlBoolean msgHandled;

   msgHandled = TRUE;                       // assume we'll handle the msg
   switch (rMsg.getMsg())
   {
   case MpFlowGraphMsg::RESOURCE_DISABLE:   // disable this resource
      mIsEnabled = FALSE;
      break;
   case MpFlowGraphMsg::RESOURCE_ENABLE:    // enable this resource
      mIsEnabled = TRUE;
      break;
   case MpFlowGraphMsg::RESOURCE_SET_SAMPLES_PER_FRAME:
      mSamplesPerFrame = rMsg.getInt1();    // set the samples per frame
      break;
   case MpFlowGraphMsg::RESOURCE_SET_SAMPLES_PER_SEC:
      mSamplesPerSec = rMsg.getInt1();      // set the samples per second
      break;
   default:
      msgHandled = FALSE;                   // we didn't handle the msg
      break;                                //  after all
   }

   return msgHandled;
}

// If there already is a buffer stored for this input port, delete it.
// Then store pBuf for the indicated input port.
void MpResource::setInputBuffer(int inPortIdx, MpBufPtr pBuf)
{
   // make sure we have a valid port that is connected to a resource
   assert((inPortIdx >= 0) && (inPortIdx < mMaxInputs) &&
          (mpInConns[inPortIdx].pResource != NULL));

   MpBuf_delRef(mpInBufs[inPortIdx]);  // delete any existing buffer
   mpInBufs[inPortIdx] = pBuf;         // store the new buffer
}

// Post a message to this resource.
// If this resource is not part of a flow graph, then rMsg is
// immediately passed to the handleMessage() method for this
// resource.  If this resource is part of a flow graph, then
// rMsg will be sent to the message queue for the flow graph
// that this resource belongs to.  The handleMessage() method
// for this resource will be invoked at the start of the next frame
// processing interval.
OsStatus MpResource::postMessage(MpFlowGraphMsg& rMsg)
{
   UtlBoolean res;

   if (mpFlowGraph == NULL)
   {
      res = handleMessage(rMsg);
      assert(res);
      return OS_SUCCESS;
   }
   else
   {
      return mpFlowGraph->postMessage(rMsg, OsTime::NO_WAIT);
   }
}

// Makes pBuf available to resource connected to the outPortIdx output
// port of this resource.
// Returns TRUE if there is a resource connected to the specified output
// port, FALSE otherwise.
UtlBoolean MpResource::setOutputBuffer(int outPortIdx, MpBufPtr pBuf)
{
   MpResource* pDownstreamInput;
   int         downstreamPortIdx;

   if (outPortIdx < 0 || outPortIdx >= mMaxOutputs)  // port  out of range
      return FALSE;

   pDownstreamInput  = mpOutConns[outPortIdx].pResource;
   downstreamPortIdx = mpOutConns[outPortIdx].portIndex;
   if (pDownstreamInput == NULL)                     // no connected resource
      return FALSE;

   pDownstreamInput->setInputBuffer(downstreamPortIdx, pBuf);
   return TRUE;
}

int MpResource::getSamplesPerFrame()
{
   return mSamplesPerFrame;
}

int MpResource::getSamplesPerSec()
{
   return mSamplesPerSec;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Connects the toPortIdx input port on this resource to the
// fromPortIdx output port of the rFrom resource.
// Returns TRUE if successful, FALSE otherwise.
UtlBoolean MpResource::connectInput(MpResource& rFrom, int fromPortIdx,
                                   int toPortIdx)
{
   if (toPortIdx < 0 ||                // bad port index
       toPortIdx >= mMaxInputs)        // bad port index
      return FALSE;

   MpBuf_delRef(mpInBufs[toPortIdx]);  // get rid of old buffer (if any)
   mpInBufs[toPortIdx]            = NULL;
   mpInConns[toPortIdx].pResource = &rFrom;
   mpInConns[toPortIdx].portIndex = fromPortIdx;

   mNumActualInputs++;

   return TRUE;
}

// Connects the fromPortIdx output port on this resource to the
// toPortIdx input port of the rTo resource.
// Returns TRUE if successful, FALSE otherwise.
UtlBoolean MpResource::connectOutput(MpResource& rTo, int toPortIdx,
                                    int fromPortIdx)
{
   if (fromPortIdx < 0 ||              // bad port index
       fromPortIdx >= mMaxOutputs)     // bad port index
      return FALSE;

   MpBuf_delRef(mpOutBufs[fromPortIdx]);  // get rid of old buffer (if any)
   mpOutBufs[fromPortIdx]            = NULL;
   mpOutConns[fromPortIdx].pResource = &rTo;
   mpOutConns[fromPortIdx].portIndex = toPortIdx;

   mNumActualOutputs++;

   return TRUE;
}

// Removes the link to the inPortIdx input port of this resource.
// Returns TRUE if successful, FALSE otherwise.
UtlBoolean MpResource::disconnectInput(int inPortIdx)
{
   if (mpInConns[inPortIdx].pResource == NULL || // no connected resource
       inPortIdx < 0 ||                          // bad port index
       inPortIdx >= mMaxInputs)                  // bad port index
      return FALSE;

   MpBuf_delRef(mpInBufs[inPortIdx]);        // get rid of old buffer (if any)
   mpInBufs[inPortIdx]            = NULL;
   mpInConns[inPortIdx].pResource = NULL;
   mpInConns[inPortIdx].portIndex = -1;

   mNumActualInputs--;

   return TRUE;
}

// Removes the link to the outPortIdx output port of this resource.
// Returns TRUE if successful, FALSE otherwise.
UtlBoolean MpResource::disconnectOutput(int outPortIdx)
{
   if (mpOutConns[outPortIdx].pResource == NULL || // no connected resource
       outPortIdx < 0 ||                           // bad port index
       outPortIdx >= mMaxOutputs)                  // bad port index
      return FALSE;

   MpBuf_delRef(mpOutBufs[outPortIdx]);     // get rid of old buffer (if any)
   mpOutBufs[outPortIdx]            = NULL;
   mpOutConns[outPortIdx].pResource = NULL;
   mpOutConns[outPortIdx].portIndex = -1;

   mNumActualOutputs--;

   return TRUE;
}

// Associates this resource with the indicated flow graph.
// For now, this method always returns success
OsStatus MpResource::setFlowGraph(MpFlowGraphBase* pFlowGraph)
{
   mpFlowGraph = pFlowGraph;

   return OS_SUCCESS;
}

// Sets the name that is associated with this resource.
void MpResource::setName(const UtlString& rName)
{
   mName = rName;
}

/* ============================ FUNCTIONS ================================= */
