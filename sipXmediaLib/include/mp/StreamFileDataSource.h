//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _StreamFileDataSource_h_
#define _StreamFileDataSource_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/StreamDataSource.h"
#include "net/Url.h"
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsFS.h"
#include "os/OsMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Defines a stream data source built ontop of a OsFile
class StreamFileDataSource : public StreamDataSource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   StreamFileDataSource(Url url, int iFlags);
     //:Default constructor


   virtual
   ~StreamFileDataSource();
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

   virtual OsStatus seek(size_t iLocation) ;
     //:Moves the stream pointer to the an absolute location.
     //
     //!param iLocation - The desired seek location

/* ============================ ACCESSORS ================================= */

   virtual OsStatus getLength(ssize_t& iLength) ;
     //:Gets the length of the stream (if available)

   virtual OsStatus getPosition(ssize_t& iPosition) ;
     //:Gets the current position within the stream.

   virtual OsStatus toString(UtlString& string) ;
     //:Renders a string describing this data source.
     // This is often used for debugging purposes.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   StreamFileDataSource(const StreamFileDataSource& rStreamFileDataSource);
     //:Copy constructor

   StreamFileDataSource& operator=(const StreamFileDataSource& rhs);
     //:Assignment operator


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   Url     mUrl ;    // File Url source
   OsFile* mpFile ;  // Actual File data source

   OsMutex mFileGuard; // Guard closing/touch file from multiple threads
};

/* ============================ INLINE METHODS ============================ */

#endif  // _StreamFileDataSource_h_
