//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MprFromStream_h_
#define _MprFromStream_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/dtmflib.h"
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpResource.h"
#include "mp/StreamDefs.h"

#include "mp/MpStreamFeeder.h"
#include "os/OsLockingList.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef struct tagSTREAMDESC
{
    StreamHandle    handle ;
    MpStreamFeeder* pFeeder ;

} STREAMDESC ;
// FORWARD DECLARATIONS


//:This resource as an insertion point for the streaming
//:infrastructure byconnecting the flowgraph to MpStreamFeeders.
//
// Whenever requested to "realize" a stream, this code will create a
// MpStreamFeeder which inturn creates a StreamDataSource and a
// StreamFormatDecoder.  Many MpStreamFeeders may be managed/created by
// a single MprFromStream, however, only one can be active at only one
// point.
//
// For thread safety, all operations (play, destroy, rewind, etc.)
// result in a message being posted to itself.  This forces all of the
// tasks to be performed on the same task context.  The "realize"
// operation is an exception, where it is processed on the called
// context.
//
// Additionally, the code creates a level of indirection between between
// the implementation and calling parties (application layer).  All
// MpStreamFeeders created by this resource are collected within a list
// (mStreamList) and only a handle to the resource is exposed to
// application layer.  Handles are unique for an instance of a
// MprFromStream.  If the destruct is called for MprFromStream and the
// mStreamList is not null, the list is purged.
//
//
// This class bubbles events to the MpStreamFeeder by calling the
// fromStreamUpdate callback on the feeder itself.
//
class MprFromStream : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   MprFromStream(const UtlString& rName, int samplesPerFrame, int samplesPerSec);
     //:Constructor

   virtual
   ~MprFromStream();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsStatus realize(Url urlSource,
                    int flags,
                    StreamHandle &handle,
                    OsNotification* event = NULL);

   OsStatus realize(UtlString* pBuffer,
                    int flags,
                    StreamHandle &handle,
                    OsNotification* event = NULL);

   OsStatus prefetch(StreamHandle handle);

   OsStatus play(StreamHandle handle);

   OsStatus rewind(StreamHandle handle);

   OsStatus pause(StreamHandle handle);

   OsStatus stop(StreamHandle handle) ;

   OsStatus destroy(StreamHandle handle) ;

   OsStatus getFlags(StreamHandle handle, int& flags) ;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   OsStatus setStreamSource(MpStreamFeeder *pFeeder) ;
   MpStreamFeeder* getStreamSource() ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   enum AddlMsgTypes
   {
      SOURCE_PLAY = MpFlowGraphMsg::RESOURCE_SPECIFIC_START,
      SOURCE_PAUSE,
      SOURCE_REWIND,
      SOURCE_STOP,
      SOURCE_DESTROY,
      SOURCE_RENDER,
   };


   OsNotification* mpNotify ;
   MpStreamFeeder* mpStreamRenderer ;
   FeederEvent     mEventState ;
   UtlBoolean       mbStreamChange ;
   int             miStreamCount ;      // Count for generating unique handles
   OsLockingList   mStreamList ;        // List of stream players

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);
     //:Handle messages for this resource.

   UtlBoolean handleRender(MpStreamFeeder* pFeeder);
   UtlBoolean handlePlay(MpStreamFeeder* pFeeder);
   UtlBoolean handleRewind(MpStreamFeeder* pFeeder);
   UtlBoolean handlePause(MpStreamFeeder* pFeeder);
   UtlBoolean handleStop(MpStreamFeeder* pFeeder);
   UtlBoolean handleDestroy(MpStreamFeeder* pFeeder);


   MprFromStream(const MprFromStream& rMprFromStream);
     //:Copy constructor (not implemented for this class)

   MprFromStream& operator=(const MprFromStream& rhs);
     //:Assignment operator (not implemented for this class)

   MpStreamFeeder* getStreamFeeder(StreamHandle handle) ;
     //:Get the stream feeder for the given handle

   MpStreamFeeder* removeStreamFeeder(StreamHandle handle) ;
     //:Removes the stream feeder from the stream list.
     // The Stream feeder is returned if found, someone else
     // is responsible for deleting it.

   void destroyFeeders() ;
     //:Stops, destroys, and frees all stream feeders

};

/* ============================ INLINE METHODS ============================ */

#endif  /* _MprFromStream_h_ */
