//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpStreamFeeder_h_
#define _MpStreamFeeder_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsTask.h"
#include "os/OsCSem.h"
#include "os/OsMsgQ.h"
#include "os/OsMutex.h"
#include "net/Url.h"
#include "mp/StreamDefs.h"
#include "mp/StreamDataSourceListener.h"
#include "mp/StreamDecoderListener.h"

// DEFINES
#define INCL_RAW_DECODER         // Include the RAW decoder
#define INCL_WAV_DECODER         // Include the WAV decoder
// #define INCL_MP3_DECODER         // Include the MP3 decoder


// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS

//: Feeder states (match player states for the most part, but add a
//: realizing and rendering states)
typedef enum tagFeederState
{
   UnrealizedState,
   RealizedState,
   PrefetchingState,
   PrefetchedState,
   RendereringState,
   StoppedState,
   FailedState,

} FeederState ;

// FORWARD DECLARATIONS
class StreamFormatDecoder ;
class StreamDataSource ;
class OsNotification ;

//:The MpStreamFeed coordinates with the data source and decoder to ready the
//:input stream and then plugs into the MprFromStream resource to supply
//:audio info the flowgraph.
//
//                                                 ------------------
//                                            +-> | StreamDataSource |
//  ---------------           ----------------     ------------------
// | MprFromStream | 1..N -> | MpStreamFeeder |
//  ---------------           ----------------    ---------------------
//                                            +-> | StreamFormatDecoder |
//                                                 ---------------------
//
// The MpStreamFeeder has its own state table that mostly matches the generic
// player's state table:
//
// Unrealized -> Realized -> Prefetching -> Prefetched -> Rendering -> Stopped
//
//            *->FailedState
//
// Communications:
//
// Events are send to the MpStreamPlayer resource indirectly through a
// settable OsNotifyEvent (setEventHandler).
//
// Events are received from the MprFromStream object by the fromStreamUpdate
// callback function.
//
class MpStreamFeeder : public StreamDataSourceListener, public StreamDecoderListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   MpStreamFeeder(Url resource, int flags);
     //:Constructor accepting a url resource, type, and cache all flag

   MpStreamFeeder(UtlString* pBuffer, int flags);
     //:Constructor accepting a pre-populated buffer, type and cache all flag

   virtual ~MpStreamFeeder();
     //:Destructor
     // As part of destroying the task, flush all messages from the incoming
     // OsMsgQ.

/* ============================ MANIPULATORS ============================== */

   OsStatus realize() ;
     //: Initiates the connection with the outbound party to validate the
     //: connection.  Buffers are allocated at this point.

   OsStatus render() ;
     //: Begin downloading and decoding data.  The state is not moved to
     //  prefetched until a sensible amount of data has been received and
     //  decoded.

   OsStatus rewind() ;
     //: Rewind the data source to the prefetched state.  This may result
     //  in re-prefetching data in cases.

   OsStatus stop();
     //: Stop collecting/rendering data

   OsStatus setEventHandler(OsNotification* pEventHandler) ;
     //: Set the event handler for this renderer.  All events will be
     //: delievered to this handler

   void markPaused(UtlBoolean bPaused) ;
     //: Marks that playing is pauses; however, continues to render and
     //: stream.

/* ============================ ACCESSORS ================================= */

   OsStatus getFrame(uint16_t *samples) ;
     //: Get a frames worth of data (80 samples per frame) ;

   UtlBoolean isMarkedPaused() ;
     //: Has the rendered been marked as paused?

   OsStatus getFlags(int &flags) ;
     //: Get the flags for this renderer

   FeederState getState() ;
     //: Query the state for this renderer

   void fromStreamUpdate(FeederEvent event) ;
     //: Called by the MprFromStream resource when events changes.

/* ============================ INQUIRY =================================== */

   UtlBoolean isValidStateChange(FeederState source, FeederState target) ;
     //: Is the transition from state source to target valid?

/* ============================ TESTING =================================== */

#ifdef MP_STREAM_DEBUG /* [ */
static const char* getEventString(FeederEvent event);
#endif /* MP_STREAM_DEBUG ] */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   void fireEvent(FeederEvent eventType) ;
     //:Fires renderer event to interested consumers

   void setState(FeederState state);
     //:Sets the internal renderer state

   virtual void decoderUpdate(StreamFormatDecoder* pDecoder, StreamDecoderEvent event) ;
     //:Call back for decoder updates

   virtual void dataSourceUpdate(StreamDataSource* pDataSource, StreamDataSourceEvent event) ;
     //:Call back for data source updates

#ifdef MP_STREAM_DEBUG /* [ */
   const char* getDataSourceEventString(StreamDataSourceEvent);
   const char* getDecoderEventString(StreamDecoderEvent);
   const char* getFeederEventString(FeederEvent event);
#endif /* MP_STREAM_DEBUG ] */


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static int s_iInstanceCount ;

   FeederState m_state;                      // State of the Feeder
   StreamFormatDecoder* m_pFormatDecoder;    // Decoder
   StreamDataSource* m_pDataSource ;         // Data Source
   int mFlags ;                              // Flags given at creation

   UtlBoolean m_bMarkedPaused;                // Is this marked as paused?
   OsNotification* m_pEventHandler;          // Event sink.
   int m_iInstanceId ;
   OsMutex m_eventGuard;                     // Guards multiple threads from
                                             // calling fireEvent at same time




   void initDecodingSource() ;
     //: Construction helper: initialize the decoding source

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpStreamFeeder_h_
