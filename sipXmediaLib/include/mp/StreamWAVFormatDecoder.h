//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _StreamWAVFormatDecoder_h_
#define _StreamWAVFormatDecoder_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/StreamQueueingFormatDecoder.h"
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsTask.h"
#include "os/OsBSem.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
struct WAVChunkID
{
   char     ckID[4];          // chunk id 'RIFF'
   uint32_t ckSize;           // chunk size      = 4 byte value in WAV file
};

struct FORMATChunkInfo
{
   uint16_t formatTag;       // format tag currently pcm   = 2 byte value in WAV file
   uint16_t nChannels;       // number of channels         = 2 byte value in WAV file
   uint32_t nSamplesPerSec;  // sample rate in hz          = 4 byte value in WAV file
   uint32_t nAvgBytesPerSec; // average bytes per second   = 4 byte value in WAV file
   uint16_t nBlockAlign;     // number of bytes per sample = 2 byte value in WAV file
   uint16_t nBitsPerSample;  // number of bits in a sample = 2 byte value in WAV file
};


// TYPEDEFS
// FORWARD DECLARATIONS

//:A WAV format Decoder
class StreamWAVFormatDecoder : public StreamQueueingFormatDecoder, public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   StreamWAVFormatDecoder(StreamDataSource* pDataSource);
     //:Default constructor

   virtual
   ~StreamWAVFormatDecoder();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus init();
     //:Initializes the decoder

   virtual OsStatus free();
     //:Frees all resources consumed by the decoder

   virtual OsStatus begin();
     //:Begins decoding

   virtual OsStatus end();
     //:Ends decoding

/* ============================ ACCESSORS ================================= */

   virtual OsStatus toString(UtlString& string);
     //:Renders a string describing this decoder.
     // This is often used for debugging purposes.

/* ============================ INQUIRY =================================== */

   virtual UtlBoolean isDecoding();
     //:Gets the decoding status.  TRUE indicates decoding activity, false
     //:indicates the decoder has completed.

   virtual UtlBoolean validDecoder();
     //:Determines if this is a valid decoder given the associated data
     //:source.
     // The data source is "peeked" for data, so that the stream's
     // data is not disturbed.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   StreamWAVFormatDecoder(const StreamWAVFormatDecoder& rStreamWAVFormatDecoder);
     //:Copy constructor (not supported)

   StreamWAVFormatDecoder& operator=(const StreamWAVFormatDecoder& rhs);
     //:Assignment operator (not supported)

   int run(void* pArgs);
     //:Thread entry point

   UtlBoolean nextDataChunk(ssize_t& iLength);
     //:Advances the mCurrentChunk to the next data chunk within the stream

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlBoolean mbEnd ;    // Has the decoder completed?
   OsBSem    mSemExited ;  // Have we successfully exited?
   struct FORMATChunkInfo mFormatChunk ;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _StreamWAVFormatDecoder_h_
