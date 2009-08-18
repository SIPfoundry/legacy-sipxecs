//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _StreamDataSource_h_
#define _StreamDataSource_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include "mp/StreamDefs.h"
#include "os/OsStatus.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

typedef enum                  // Data source event definitions
{
   LoadingStartedEvent,       // Data source statred loading
   LoadingThrottledEvent,     // Data source throttled
   LoadingCompletedEvent,     // Data source completed loading
   LoadingErrorEvent,         // Data source error

} StreamDataSourceEvent;

// FORWARD DECLARATIONS
class StreamDataSourceListener;


//:An abstraction definition of a stream data source
class StreamDataSource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   StreamDataSource(int iFlags = 0);
     //:Constructors a StreamDataSource given optional flags.
     // See StreamDefs.h for a description of available flags.

   virtual ~StreamDataSource();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus open() = 0 ;
     //:Opens the data source

   virtual OsStatus close() = 0 ;
     //:Closes the data source

   virtual OsStatus destroyAndDelete() = 0 ;
     //:Destroys and deletes the data source object

   virtual OsStatus read(char *szBuffer, ssize_t iLength, ssize_t& iLengthRead) = 0 ;
     //:Reads iLength bytes of data from the data source and places the
     //:data into the passed szBuffer buffer.
     //
     //!param szBuffer - Buffer to place data
     //!param iLength - Max length to read
     //!param iLengthRead - The actual amount of data read.

   virtual OsStatus peek(char* szBuffer, ssize_t iLength, ssize_t& iLengthRead) = 0 ;
     //:Identical to read, except the stream pointer is not advanced.
     //
     //!param szBuffer - Buffer to place data
     //!param iLength - Max length to read
     //!param iLengthRead - The actual amount of data read.

   virtual OsStatus interrupt() ;
     //:Interrupts any time consuming operation.
     // For example, some data sources may require network access (e.g. http)
     // to read or fetch data.  Invoking an interrupt() will cause any
     // time consuming or blocking calls to exit with more quickly with an
     // OS_INTERRUPTED return code.

   virtual OsStatus seek(size_t iLocation) = 0 ;
     //:Moves the stream pointer to the an absolute location.
     //
     //!param iLocation - The desired seek location

   void setListener(StreamDataSourceListener* pListener);
     //:Sets a listener to receive StreamDataSourceEvent events for this
     //:data source.

/* ============================ ACCESSORS ================================= */

   virtual OsStatus getLength(ssize_t& iLength) = 0 ;
     //:Gets the length of the stream (if available)

   virtual OsStatus getPosition(ssize_t& iPosition) = 0 ;
     //:Gets the current position within the stream.

   virtual OsStatus toString(UtlString& string) = 0 ;
     //:Renders a string describing this data source.
     // This is often used for debugging purposes.

   int getFlags() ;
     //:Gets the flags specified at time of construction

/* ============================ INQUIRY =================================== */

/* ============================ TESTING =================================== */

#ifdef MP_STREAM_DEBUG /* [ */
static const char* getEventString(StreamDataSourceEvent event);
#endif /* MP_STREAM_DEBUG ] */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   StreamDataSource(const StreamDataSource& rStreamDataSource);
     //:Copy constructor (not supported)

   StreamDataSource& operator=(const StreamDataSource& rhs);
     //:Assignment operator (not supported)

   void fireEvent(StreamDataSourceEvent event);
     //:Fires a data source event to the interested consumer.


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   StreamDataSourceListener* mpListener ; // data source listener
   int                       miFlags; // flags specified during construction
};

/* ============================ INLINE METHODS ============================ */

#endif  // _StreamDataSource_h_
