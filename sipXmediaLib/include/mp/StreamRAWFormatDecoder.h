//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _StreamRAWFormatDecoder_h_
#define _StreamRAWFormatDecoder_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"

#include "mp/StreamQueueingFormatDecoder.h"
#include "os/OsStatus.h"
#include "os/OsTask.h"
#include "os/OsBSem.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:A simple RAW format Decoder
class StreamRAWFormatDecoder : public StreamQueueingFormatDecoder, public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   StreamRAWFormatDecoder(StreamDataSource* pDataSource);
     //:Default constructor


   virtual
   ~StreamRAWFormatDecoder();
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

   StreamRAWFormatDecoder(const StreamRAWFormatDecoder& rStreamRAWFormatDecoder);
     //:Copy constructor (not supported)

   StreamRAWFormatDecoder& operator=(const StreamRAWFormatDecoder& rhs);
     //:Assignment operator (not supported)

   int run(void* pArgs);
     //:Thread entry point

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlBoolean mbEnd ;       // Has the decoder completed?
   OsBSem    mSemExited ;  // Have we successfully exited?
};

/* ============================ INLINE METHODS ============================ */

#endif  // _StreamRAWFormatDecoder_h_
