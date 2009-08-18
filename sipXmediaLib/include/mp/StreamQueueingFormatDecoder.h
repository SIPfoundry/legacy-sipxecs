//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _StreamQueueingFormatDecoder_h_
#define _StreamQueueingFormatDecoder_h_

// SYSTEM INCLUDES
#include "time.h"

// APPLICATION INCLUDES
#include "mp/StreamFormatDecoder.h"
#include "os/OsDefs.h"
#include "os/OsMsgPool.h"
#include "os/OsMsgQ.h"
#include "os/OsStatus.h"
#include "os/OsMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:The Stream Queueing Format Decoder builds upon the abstract
//:StreamFormatDecoder by adding a mechanism to queue a max number
//:of rendered frames.
class StreamQueueingFormatDecoder : public StreamFormatDecoder
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   StreamQueueingFormatDecoder(StreamDataSource* pDataSource,
                               int               iQueueLength);
     //:Constructs a queueing format decoder given a data source and queue
     //:length

   virtual
   ~StreamQueueingFormatDecoder();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus getFrame(uint16_t* samples);
     //: Gets the next available frame
     //! returns OS_SUCCESS if a frame is available

   virtual OsStatus queueFrame(const uint16_t* pSamples);
     //: Queues a frame of data

   virtual OsStatus queueEndOfFrames();
     //: Queues an end of frame marker.  This informs MprFromStream that the
     //: Stream has ended.

   virtual OsStatus drain();
     //: Drains any queued frames

/* ============================ ACCESSORS ================================= */

   int getMaxQueueLength() ;
     //: Gets the maximum number of frames that can be queued before the
     //: queueing routines will block.

   int getNumQueuedFrames() ;
     //: Gets the current number of queued frames.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   StreamQueueingFormatDecoder(const StreamQueueingFormatDecoder& rStreamQueueingFormatDecoder);
     //:Copy constructor (not supported)

   StreamQueueingFormatDecoder& operator=(const StreamQueueingFormatDecoder& rhs);
     //:Assignment operator (not supported)

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsMsgQ    mMsgqFrames ;        // Queue of frames
   OsMsgPool mMsgPool;            // Pool for msg containers
   int       miMaxQueueLength ;   // Max size of the queue length
   UtlBoolean mbReportThrottle ;   // Should this report throttles?
   UtlBoolean mbDraining ;

   static OsMutex mMutReport ;// Thread safety for performance monitoring

   static time_t       sLastReported ;  // When we last reported status

   static unsigned int sDeltaFrames ;	// Frames since last report
   static unsigned int sDeltaStreams ;	// Streams since last report
   static unsigned int sDeltaUnderruns ;// Underruns since last report
   static unsigned int sDeltaThrottles ;// Throttles since last report

   static unsigned int sTotalFrames ;	// Cumulative number of frames
   static unsigned int sTotalStreams ;	// Cumulative number of streams
   static unsigned int sTotalUnderruns ;// Cumulative number of underruns
   static unsigned int sTotalThrottles ;// Cumulative number of throttles

   static void reportFrame(UtlBoolean bUnderrun) ;
     //:Reports that a frame has been processed by media processing.

   static void reportThrottle() ;
    //: Reports that the decoder has been throttled (decoding faster
    //: then data is being requested).

   static void reportStream() ;
     //:Reports that a stream has been created

   virtual void checkThrottle() ;
     //:Determines if throttling is needed
};

/* ============================ INLINE METHODS ============================ */

#endif  // _StreamQueueingFormatDecoder_h_
