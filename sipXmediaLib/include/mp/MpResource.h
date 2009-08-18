//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpResource_h_
#define _MpResource_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsRWMutex.h"
#include "os/OsStatus.h"
#include "mp/MpBuf.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// $$$ (rschaaf): keep for now
// typedef int* MpBufPtr;

// FORWARD DECLARATIONS
class MpFlowGraphBase;
class MpFlowGraphMsg;

//:Abstract base class for all media processing objects.
// Each resource has zero or more input ports and zero or more output ports.
// Each frame processing interval, the <i>processFrame()</i> method is
// invoked to process one interval's worth of media.
//
// Substantive changes to a resource can only be made:
// 1) when the resource is not part of flow graph, or
// 2) at the start of a frame processing interval

class MpResource : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   friend class MpFlowGraphBase;

   enum VisitState
   {
      NOT_VISITED,
      IN_PROGRESS,
      FINISHED
   };
     //:Graph traversal states that are used when running a topological sort
     //:to order resources within a flow graph.

/* ============================ CREATORS ================================== */

   MpResource(const UtlString& rName, int minInputs, int maxInputs,
              int minOutputs, int maxOutputs,
              int samplesPerFrame, int samplesPerSec);
     //:Constructor

   virtual
   ~MpResource();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean disable(void);
     //:Disable this resource.
     // Returns TRUE if successful, FALSE otherwise.
     // The "enabled" flag is passed to the <i>doProcessFrame()</i> method
     // and will likely affect the media processing that is performed by this
     // resource.  Typically, if a resource is not enabled,
     // <i>doProcessFrame()</i> will perform only minimal processing (for
     // example, passing the input straight through to the output in the case
     // of a one input / one output resource).

   virtual UtlBoolean enable(void);
     //:Enable this resource.
     // Returns TRUE if successful, FALSE otherwise.
     // The "enabled" flag is passed to the <i>doProcessFrame()</i> method
     // and will likely affect the media processing that is performed by this
     // resource.  Typically, if a resource is not enabled,
     // <i>doProcessFrame()</i> will perform only minimal processing (for
     // example, passing the input straight through to the output in the case
     // of a one input / one output resource).

   virtual UtlBoolean processFrame(void);
     //:Wrapper around <i>doProcessFrame()</i>.
     // Returns TRUE if successful, FALSE otherwise.
     // This method prepares the input buffers before calling
     // <i>doProcessFrame()</i> and distributes the output buffers to the
     // appropriate downstream resources after <i>doProcessFrame()</i>
     // returns.

   virtual UtlBoolean setSamplesPerFrame(int samplesPerFrame);
     //:Sets the number of samples expected per frame.
     // Returns FALSE if the specified rate is not supported, TRUE otherwise.

   virtual UtlBoolean setSamplesPerSec(int samplesPerSec);
     //:Sets the number of samples expected per second.
     // Returns FALSE if the specified rate is not supported, TRUE otherwise.

   void setVisitState(int newState);
     //:Sets the visit state for this resource (used in performing a
     //:topological sort on the resources contained within a flow graph).

/* ============================ ACCESSORS ================================= */

   static void resourceInfo(MpResource* pResource, int index);
     //:Displays information on the console about the specified resource.

   MpFlowGraphBase* getFlowGraph(void) const;
     //:Returns the flow graph that contains this resource or NULL if the
     //:resource is not presently part of any flow graph.

   void getInputInfo(int inPortIdx, MpResource*& rpUpstreamResource,
                     int& rUpstreamPortIdx) const;
     //:Returns information about the upstream end of a connection to the
     //:<i>inPortIdx</i> input on this resource.  If <i>inPortIdx</i> is
     //:invalid or there is no connection, then <i>rpUpstreamResource</i>
     //:will be set to NULL.

   UtlString getName(void) const;
     //:Returns the name associated with this resource.

   void getOutputInfo(int outPortIdx, MpResource*& rpDownstreamResource,
                     int& rDownstreamPortIdx) const;
     //:Returns information about the downstream end of a connection to the
     //:<i>outPortIdx</i> output on this resource.  If <i>outPortIdx</i> is
     //:invalid or there is no connection, then <i>rpDownstreamResource</i>
     //:will be set to NULL.

   int getVisitState(void);
     //:Returns the current visit state for this resource (used in performing
     //:a topological sort on the resources contained within a flow graph).

   int maxInputs(void) const;
     //:Returns the maximum number of inputs supported by this resource.

   int maxOutputs(void) const;
     //:Returns the maximum number of outputs supported by this resource.

   int minInputs(void) const;
     //:Returns the minimum number of inputs required by this resource.

   int minOutputs(void) const;
     //:Returns the minimum number of outputs required by this resource.

   int numInputs(void) const;
     //:Returns the number of resource inputs that are currently connected.

   int numOutputs(void) const;
     //:Returns the number of resource outputs that are currently connected.

   /**
    * Calculate a unique hash code for this object.  If the equals
    * operator returns true for another object, then both of those
    * objects must return the same hashcode.
    */
   virtual unsigned hash() const ;

   /**
    * Get the ContainableType for a UtlContainable derived class.
    */
   virtual UtlContainableType getContainableType() const ;
   static const UtlContainableType TYPE;

/* ============================ INQUIRY =================================== */

   UtlBoolean isEnabled(void) const;
     //:Returns TRUE is this resource is currently enabled, FALSE otherwise.

   UtlBoolean isInputConnected(int portIdx) const;
     //:Returns TRUE if portIdx is valid and the indicated input is
     //:connected, FALSE otherwise.

   UtlBoolean isInputUnconnected(int portIdx) const;
     //:Returns TRUE if portIdx is valid and the indicated input is
     //:not connected, FALSE otherwise.

   UtlBoolean isOutputConnected(int portIdx) const;
     //:Returns TRUE if portIdx is valid and the indicated output is
     //:connected, FALSE otherwise.

   UtlBoolean isOutputUnconnected(int portIdx) const;
     //:Returns TRUE if portIdx is valid and the indicated output is
     //:not connected, FALSE otherwise.

   /**
    * Compare the this object to another like-objects.  Results for
    * designating a non-like object are undefined.
    *
    * @returns 0 if equal, < 0 if less then and >0 if greater.
    */
   virtual int compareTo(UtlContainable const *) const ;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   // Conn is a local class definition

   //:The Conn object maintains information about the "far end" of a
   //:connection.
   struct Conn
   {
      MpResource* pResource;
      int         portIndex;
   };

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000) = 0;
     //:This method does the real work for the media processing resource and
     //:must de defined in each class derived from this one.
     // Returns TRUE if successful, FALSE otherwise.
     //!param: (in) inBufs - array of pointers to input buffers for the resource
     //!param: (out) outBufs - array of pointers to output buffers produce by the resource
     //!param: (in) maxInputs - size of the inBufs array
     //!param: (in) maxOutputs - size of the outBufs array
     //!param: (in) isEnabled - indicates whether this resource has been enabled
     //!param: (in) samplesPerFrame - samples to produce per frame processing interval
     //!param: (in) samplesPerSecond - samples to produce per second

   MpBufPtr getInputBuffer(int inPortIdx) const;
     //:Returns a pointer to the incoming buffer for the <i>inPortIdx</i>
     //:input port if a buffer is available.  Returns NULL if either no
     //:buffer is available or there is no resource connected to the
     //:specified port or the <i>inPortIdx</i> is out of range.

   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);
     //:Handles an incoming message for this media processing object.
     // Returns TRUE if the message was handled, otherwise FALSE.

   void setInputBuffer(int inPortIdx, MpBufPtr pBuf);
     //:If there already is a buffer stored for this input port, delete it.
     //:Then store <i>pBuf</i> for the indicated input port.

   OsStatus postMessage(MpFlowGraphMsg& rMsg);
     //:Post a message to this resource.
     // If this resource is not part of a flow graph, then <i>rMsg</i> is
     // immediately passed to the <i>handleMessage()</i> method for this
     // resource.  If this resource is part of a flow graph, then
     // <i>rMsg</i> will be sent to the message queue for the flow graph
     // that this resource belongs to.  The <i>handleMessage()</i> method
     // for this resource will be invoked at the start of the next frame
     // processing interval.

   UtlBoolean setOutputBuffer(int outPortIdx, MpBufPtr pBuf);
     //:Makes <i>pBuf</i> available to resource connected to the
     //:<i>outPortIdx</i> output port of this resource.
     // Returns TRUE if there is a resource connected to the specified output
     // port, FALSE otherwise.

   int getSamplesPerFrame();
   //:return number of samples per frame

   int getSamplesPerSec();
   //:return number of samples per second

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsRWMutex    mRWMutex;      // reader/writer lock for synchronization
   MpFlowGraphBase* mpFlowGraph;   // flow graph this resource belongs to
   MpBufPtr*    mpInBufs;      // input buffers for this resource
   Conn*        mpInConns;     // input connections for this resource
   MpBufPtr*    mpOutBufs;     // output buffers for this resource
   Conn*        mpOutConns;    // output connections for this resource
   UtlBoolean    mIsEnabled;    // TRUE if resource is enabled, FALSE otherwise
   int          mMaxInputs;    // maximum number of inputs
   int          mMaxOutputs;   // maximum number of outputs
   int          mMinInputs;    // number of required inputs
   int          mMinOutputs;   // number of required outputs
   UtlString     mName;         // name associated with this resource
   int          mNumActualInputs;   // actual number of connected inputs
   int          mNumActualOutputs;  // actual number of connected outputs
   int          mSamplesPerFrame;   // number of samples per frame
   int          mSamplesPerSec;     // number of samples per second
   int          mVisitState;   // (used by flow graph topological sort alg.)

   UtlBoolean connectInput(MpResource& rFrom, int fromPortIdx, int toPortIdx);
     //:Connects the <i>toPortIdx</i> input port on this resource to the
     //:<i>fromPortIdx</i> output port of the <i>rFrom</i> resource.
     // Returns TRUE if successful, FALSE otherwise.

   UtlBoolean connectOutput(MpResource& rTo, int toPortIdx, int fromPortIdx);
     //:Connects the <i>fromPortIdx</i> output port on this resource to the
     //:<i>toPortIdx</i> input port of the <i>rTo</i> resource.
     // Returns TRUE if successful, FALSE otherwise.

   UtlBoolean disconnectInput(int inPortIdx);
     //:Removes the connection to the <i>inPortIdx</i> input port of this
     //:resource.
     // Returns TRUE if successful, FALSE otherwise.

   UtlBoolean disconnectOutput(int outPortIdx);
     //:Removes the connection to the <i>outPortIdx</i> output port of this
     //:resource.
     // Returns TRUE if successful, FALSE otherwise.

   OsStatus setFlowGraph(MpFlowGraphBase* pFlowGraph);
     //:Associates this resource with the indicated flow graph.
     //!retcode: OS_SUCCESS - for now, this method always returns success

   void setName(const UtlString& rName);
     //:Sets the name that is associated with this resource.

   MpResource(const MpResource& rMpResource);
     //:Copy constructor (not implemented for this class)

   MpResource& operator=(const MpResource& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpResource_h_
