//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "mp/StreamRAWFormatDecoder.h"
#include "mp/StreamDataSource.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
StreamRAWFormatDecoder::StreamRAWFormatDecoder(StreamDataSource* pDataSource)
   : StreamQueueingFormatDecoder(pDataSource, 1600)
   , OsTask("RawDecoder-%d")
   , mSemExited(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
}


// Destructor
StreamRAWFormatDecoder::~StreamRAWFormatDecoder()
{
}

/* ============================ MANIPULATORS ============================== */

// Initializes the decoder
OsStatus StreamRAWFormatDecoder::init()
{
   return OS_SUCCESS ;
}


// Frees all resources consumed by the decoder
OsStatus StreamRAWFormatDecoder::free()
{
   return OS_SUCCESS ;
}


// Begins decoding
OsStatus StreamRAWFormatDecoder::begin()
{
   mbEnd = FALSE ;
   mSemExited.acquire() ;

   fireEvent(DecodingStartedEvent) ;
   if (start() == FALSE)
   {
      syslog(FAC_STREAMING, PRI_CRIT, "Failed to create thread for StreamWAVFormatDecoder") ;

      // If we fail to create the thread, send out failure events
      // and clean up
      mbEnd = TRUE ;
      fireEvent(DecodingErrorEvent) ;
      fireEvent(DecodingCompletedEvent) ;
      mSemExited.release() ;
   }

   return OS_SUCCESS ;
}


// Ends decoding
OsStatus StreamRAWFormatDecoder::end()
{
   mbEnd = TRUE ;

   // Interrupt any inprocess reads/seeks.  This speeds up the end.
   StreamDataSource* pSrc = getDataSource() ;
   if (pSrc != NULL)
   {
       pSrc->interrupt() ;
   }

   // Draw the decoded queue
   drain() ;

   // Wait for the run method to exit.
   mSemExited.acquire() ;

   // Draw the decoded queue again to verify that nothing is left.
   drain() ;

   mSemExited.release() ;

   return OS_SUCCESS ;
}


/* ============================ ACCESSORS ================================= */

// Renders a string describing this decoder.
OsStatus StreamRAWFormatDecoder::toString(UtlString& string)
{
   string.append("RAW") ;

   return OS_SUCCESS ;
}

/* ============================ INQUIRY =================================== */


// Gets the decoding status.
UtlBoolean StreamRAWFormatDecoder::isDecoding()
{
   return (isStarted() || isShuttingDown());
}


// Determines if this is a valid decoder given the associated data source.
UtlBoolean StreamRAWFormatDecoder::validDecoder()
{
   return TRUE ;
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Copy constructor (not supported)
StreamRAWFormatDecoder::StreamRAWFormatDecoder(const StreamRAWFormatDecoder& rStreamRAWFormatDecoder)
   : StreamQueueingFormatDecoder(NULL, 1600)
   , mSemExited(OsBSem::Q_PRIORITY, OsBSem::FULL)
{

}

// Assignment operator (not supported)
StreamRAWFormatDecoder&
StreamRAWFormatDecoder::operator=(const StreamRAWFormatDecoder& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


// Thread entry point
int StreamRAWFormatDecoder::run(void* pArgs)
{
   ssize_t iRead ;
   ssize_t iLength = sizeof(uint16_t) * 80;

   StreamDataSource* pSrc = getDataSource() ;
   if (pSrc != NULL)
   {
      char* pEventSamples = new char[iLength];
      while ((pSrc->read(pEventSamples, iLength, iRead) == OS_SUCCESS) && !mbEnd)
      {
         queueFrame((const uint16_t*)pEventSamples) ;
      }
      delete pEventSamples ;

      queueEndOfFrames() ;
      pSrc->close();
   }

   fireEvent(DecodingCompletedEvent) ;

   mSemExited.release() ;

   return 0 ;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */
