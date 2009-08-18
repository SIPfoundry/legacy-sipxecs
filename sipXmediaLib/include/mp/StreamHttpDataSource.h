//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _StreamHttpDataSource_h_
#define _StreamHttpDataSource_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/StreamDataSource.h"
#include "net/Url.h"
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsTask.h"
#include "os/OsBSem.h"


// DEFINES
#define DEFAULT_BUFFER_SIZE   128*1024
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// FORWARD DECLARATIONS
class OsConnectionSocket ;
class HttpMessage ;
// TYPEDEFS
typedef UtlBoolean (*GetDataCallbackProc)(char* pData,
                                         ssize_t iLength,
                                         void* pOptionalData,
                                         HttpMessage* pMsg);


//:Defines a stream data source built ontop of a Http Stream.
//
// Deletion of this class is very problematic, because the thread
// that pumps the data from the stream may be blocked for a long
// period of time and most cases the media task is player
// destroying this resource.  Ideally, we would interrupt the socket
// read, however, that is not possible.  So, to avoid blocking the
// thread context used to destroy this object, we use a variable
// mbDeleteOnCompletion to mark if/when we need to destroy this class
// once the run method has completed.  This method is wrapped in
// a static binary semaphore to guard against races.
class StreamHttpDataSource : public StreamDataSource, public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   StreamHttpDataSource(Url url, int iFlags);
     //:Default constructor

   virtual
   ~StreamHttpDataSource();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus open() ;
     //:Opens the data source

   virtual OsStatus close() ;
     //:Closes the data source

   virtual OsStatus destroyAndDelete() ;
     //:Destroys and deletes the data source object

   virtual OsStatus read(char *szBuffer, ssize_t iLength, ssize_t& iLengthRead) ;
     //:Reads iLength bytes of data from the data source and places the
     //:data into the passed szBuffer buffer.
     //
     //!param szBuffer - Buffer to place data
     //!param iLength - Max length to read
     //!param iLengthRead - The actual amount of data read.

   virtual OsStatus peek(char* szBuffer, ssize_t iLength, ssize_t& iLengthRead) ;
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

   virtual OsStatus seek(size_t iLocation) ;
     //:Moves the stream pointer to the an absolute location.
     //
     //!param iLocation - The desired seek location


   UtlBoolean deliverData(char *szData, ssize_t iLength, ssize_t iMaxLength) ;
     //:Callback routine that is invoked whenever new data is available from
     //:http socket.

/* ============================ ACCESSORS ================================= */

   virtual OsStatus getLength(ssize_t& iLength);
     //:Gets the length of the stream (if available)

   virtual OsStatus getPosition(ssize_t& iPosition) ;
     //:Gets the current position within the stream.

   virtual OsStatus toString(UtlString& string) ;
     //:Renders a string describing this data source.
     // This is often used for debugging purposes.

   virtual int getBufferedLength();
     //:Gets the amount of data presently buffered.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   StreamHttpDataSource(const StreamHttpDataSource& rStreamHttpDataSource);
     //:Copy constructor (not supported)

   StreamHttpDataSource& operator=(const StreamHttpDataSource& rhs);
     //:Assignment operator (not supported)

   int run(void *pArgs);
     //:Thread entry point


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   Url        m_url ;                  // Source url
   UtlString  mBuffer ;                // Buffered content
   OsBSem     mSemNeedData ;           // Used block when we need more data
   OsBSem     mSemLimitData ;          // Used to block when need to throttle
   OsBSem     mSemGuardData;           // Protected data structures
   OsBSem     mSemGuardStartClose;     // Protected Opening/Closing the DataSource
   UtlBoolean  mbDone ;                 // Is the data source complete
   UtlBoolean  mbQuit ;                 // Are we quiting?
   UtlBoolean  mbDeleteOnCompletion ;   // Should the run method delete the object?
   unsigned int  miMaxData ;           // Max amount of data to buffer
   size_t      miDSLength ;          // Data Stream Length
   size_t      miOffset ;            // Present offset into the buffer
   size_t      miBufferOffset ;      // Buffer offset from start of stream
   UtlBoolean  mbFiredThrottledEvent ;  // Should fire event on next throttle?
   UtlBoolean  mbClosed ;               // Have we closed this down?
   UtlBoolean  mbInterrupt ;            // Interrupt current operation?

   static OsBSem sSemGuardDelete;      // Guard deletion of data sources
};

/* ============================ INLINE METHODS ============================ */

#endif  // _StreamHttpDataSource_h_
