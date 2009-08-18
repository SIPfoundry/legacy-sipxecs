//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpStreamPlayer_h_
#define _MpStreamPlayer_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpPlayer.h"
#include "mp/StreamDefs.h"
#include "net/Url.h"
#include "os/OsBSem.h"
#include "os/OsDefs.h"
#include "os/OsServerTask.h"
#include "os/OsStatus.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlString;
class OsQueuedEvent;

//: Player capable of controlling a single audio source (Url or Buffer).
//
// NOTE: This player creates and communicates with a number of objects within
//       the flowgraph, primarily MpStreamFeeder.  However, the creating and
//       connection to these objects are not made until the "realize" method
//       is invoked.
//
class MpStreamPlayer : public OsServerTask, public MpPlayer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum SourceType      // Type of source data (url or buffer)
   {
      SourceUrl,
      SourceBuffer,
   } ;

/* ============================ CREATORS ================================== */

   MpStreamPlayer(OsMsgQ* pMsg, Url url, int flags, const char* pTarget = NULL) ;
     //:Contructs a stream player given a msgq, stream url, and
     //:playing flags.
     //
     //!param pMsg - Destination for MpStreamMsg commands
     //!param url - Url identifing the source data stream
     //!param flags - Playing flags (see StreamDefs.h)
     //!param target - Target Id used by the msg receiver to help with
     //       dispatching

   MpStreamPlayer(OsMsgQ* pMsg, UtlString* pBuffer,  int flags, const char* pTarget = NULL) ;
     //:Constructs a stream player given a msgq, net buffer, and
     //:playing flags.
     //
     //!param pMsg - Destination for MpStreamMsg commands
     //!param pBuffer - Net Buffer containing buffered audio data.  The
     //       MpStreamPlayer resource will delete the pBuffer upon destruction
     //       of itself.
     //!param flags - Playing flags (see StreamDefs.h)
     //!param target - Target Id used by the msg receiver to help with
     //       dispatching

   virtual ~MpStreamPlayer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus realize(UtlBoolean bBlock = TRUE);
     //: Realizes the player by initiating a connection to the target,
     //: allocates buffers, etc.
     //
     //!param bBlock - TRUE if the method should block until completion,
     //       otherwise FALSE.

   virtual OsStatus prefetch(UtlBoolean bBlock = TRUE);
     //: Prefetch enough of the data source to ensure a smooth playback.
     //
     //!param bBlock - TRUE if the method should block until completion,
     //       otherwise FALSE.

   virtual OsStatus play(UtlBoolean bBlock = TRUE);
     //: Plays the media stream.
     //
     //!param bBlock - TRUE if the method should block until completion,
     //       otherwise FALSE.

   virtual OsStatus rewind(UtlBoolean bBlock = TRUE);
     //: Rewinds a previously played media stream.  In some cases this
     //  may result in a re-connect/refetch.
     //
     //!param bBlock - TRUE if the method should block until completion,
     //       otherwise FALSE.

   virtual OsStatus pause();
     //: Pauses the media stream temporarily.

   OsStatus setLoopCount(int iLoopCount);
   //: sets the loop count.
   //: default is 1. -1 means infinite loop.
   //: 0 is invalid.

   virtual OsStatus stop();
     //: Stops play the media stream and resources used for buffering
     //: and streaming.

   virtual OsStatus destroy() ;
     //: Marks the player as destroy and frees all allocated resources
     //  in media processing.

   virtual void waitForDestruction() ;
     //: Blocks until the the lower layer stream player is destroyed

/* ============================ ACCESSORS ================================= */

   virtual OsStatus getState(PlayerState& state) ;
     //: Gets the player state

   virtual OsStatus getSourceType(int& iType) const;
     //: Gets the source type for this player (SourceUrl or SourceBuffer)

   virtual OsStatus getSourceUrl(Url& url) const;
     //: Gets the url if the source type is a SourceUrl

   virtual OsStatus getSourceBuffer(UtlString*& pBuffer) const;
     //: Gets the source buffer if the source type is a SourceBuffer

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   MpStreamPlayer(const MpStreamPlayer& rMpStreamPlayer);
     //:Copy constructor (not supported)

   MpStreamPlayer& operator=(const MpStreamPlayer& rhs);
     //:Assignment operator (not supported)

   virtual UtlBoolean handleMessage(OsMsg& rMsg) ;
     //:Handles OS server task events/messages

   void setState(PlayerState iState);
     //:Sets the internal state for this resource

   PlayerState getState();
     //: Gets the player state; internal use

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsMsgQ*          mpMsgQ;
   int              mSourceType;    // Data source type (buffer | url)
   Url              mUrl;           // Url for our stream (if specified)
   UtlString*       mpBuffer;       // Buffer data source (if specified)
   PlayerState      mState;         // Present state of the player
   StreamHandle     mHandle;        // StreamHandle from lower layers
   UtlString         mTarget;        // target id (callId)
   int              mFlags;         // Player Flags
   OsQueuedEvent*   mpQueueEvent;   // Used for comm. w/ Flowgraph
   OsBSem           mSemStateChange;// Synch for state changes
   OsBSem           mSemStateGuard; // Guard for state changes
   int              miLoopCount;    // default is 1. -1 means infinite loop.
                                    // 0 is invalid.
   int              miTimesAlreadyLooped;
   UtlBoolean        mbRealized;     // Has this player been realized?

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpStreamPlayer_h_
