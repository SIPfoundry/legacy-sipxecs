//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "mp/StreamDefs.h"
#include "mp/StreamHttpDataSource.h"
#include "net/HttpMessage.h"
#include "os/OsDateTime.h"
#include "os/OsConnectionSocket.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
OsBSem StreamHttpDataSource::sSemGuardDelete(OsBSem::Q_FIFO, OsBSem::FULL) ;

// LOCAL FUNCTIONS
UtlBoolean DataCallbackProc(char* pData,
                           ssize_t iLength,
                           void* pOptionalData,
                           HttpMessage* pMsg) ;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
StreamHttpDataSource::StreamHttpDataSource(Url url, int iFlags)
   : StreamDataSource(iFlags)
   , OsTask("HttpFetch-%d")
   , mSemNeedData(OsBSem::Q_PRIORITY, OsBSem::EMPTY)
   , mSemLimitData(OsBSem::Q_PRIORITY, OsBSem::EMPTY)
   , mSemGuardData(OsBSem::Q_PRIORITY, OsBSem::FULL)
   , mSemGuardStartClose(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   m_url = url ;

   mbFiredThrottledEvent = FALSE ;
   miOffset = 0 ;
   miDSLength = 0 ;
   miBufferOffset = 0 ;
   miMaxData = DEFAULT_BUFFER_SIZE ;
   mbDone = FALSE ;
   mbQuit = FALSE ;
   mbInterrupt = FALSE ;
   mbClosed = FALSE ;
   mbDeleteOnCompletion = FALSE ;
}

// Destructor
StreamHttpDataSource::~StreamHttpDataSource()
{
   close() ;
}

/* ============================ MANIPULATORS ============================== */

// Opens the data source
OsStatus StreamHttpDataSource::open()
{
#ifdef MP_STREAM_DEBUG /* [ */
   UtlString url ;
   m_url.toString(url) ;
   osPrintf("StreamHttpDataSource: Open %s\n", url.data()) ;
#endif /* MP_STREAM_DEBUG ] */

   mSemGuardStartClose.acquire() ;

   mbFiredThrottledEvent = FALSE ;
   miOffset = 0 ;
   miDSLength = 0 ;
   miBufferOffset = 0 ;
   miMaxData = DEFAULT_BUFFER_SIZE ;
   mbDone = FALSE ;
   mbQuit = FALSE ;
   mbInterrupt = FALSE ;
   mbClosed = FALSE ;
   mbDeleteOnCompletion = FALSE ;

   // Fire started event to subscribed listeners
   fireEvent(LoadingStartedEvent) ;

   if (!mbDone)
   {
      if (start() == FALSE)
      {
         // Handle / recover from inability to create thread
         fireEvent(LoadingErrorEvent) ;
      }
   }
   else
   {
      fireEvent(LoadingCompletedEvent) ;
   }

   mSemGuardStartClose.release() ;

   return OS_SUCCESS ;
}


// Closes the data source
OsStatus StreamHttpDataSource::close()
{
   mSemGuardStartClose.acquire() ;
   if (!mbClosed)
   {
      mbClosed = TRUE ;
#ifdef MP_STREAM_DEBUG /* [ */
      UtlString url ;
      m_url.toString(url) ;
      osPrintf("StreamHttpDataSource: Close %s\n", url.data()) ;
#endif /* MP_STREAM_DEBUG ] */
      interrupt() ;

      mbQuit = TRUE ;
      mbDone = TRUE ;
      mSemLimitData.release() ;
      mSemNeedData.release() ;

   }
   mSemGuardStartClose.release() ;


   if (!mbClosed)
   {
      // Fire off an error event in the case that anyone is waiting for this to
      // shutdown. It is possible that we are pulling down the stream and
      // MpStreamFeeder will remove the listener before the run() method kicks
      // out.
      fireEvent(LoadingCompletedEvent) ;
   }

   return OS_SUCCESS ;
}

// Destroys and deletes the data source object
OsStatus StreamHttpDataSource::destroyAndDelete()
{
    OsLock lock(sSemGuardDelete) ;
    OsStatus status = OS_SUCCESS ;

    close() ;

    // See the class level documentation in the header file for a description
    // of the destruction process.
    if (isStarted())
    {
        mbDeleteOnCompletion = true ;
    }
    if (!mbDeleteOnCompletion)
    {
        delete this ;
    }

    return status ;
}


// Reads iLength bytes of data from the data source and places the data into
// the passed szBuffer buffer.
OsStatus StreamHttpDataSource::read(char *szBuffer, ssize_t iLength, ssize_t& iLengthRead)
{
   OsStatus rc = OS_INVALID ;
   OsTime timeout(20,0);

   // Make sure enough data is avaiable
   while (((iLength+miOffset) > (mBuffer.length()+miBufferOffset)) && !mbDone
         && !mbInterrupt)
   {
      OsStatus retval = mSemNeedData.acquire(timeout);
      if (retval != OS_SUCCESS)
      {
          interrupt();
          break;
      }
   }

   if (mbInterrupt)
   {
      // Set the return code, clear the flag, and set length read.
      rc = OS_INTERRUPTED ;
      mbInterrupt = FALSE ;
      iLengthRead = 0 ;
   }
   else
   {
      // Copy data...
      mSemGuardData.acquire() ;
      if (mbDone && ((iLength+miOffset) > (mBuffer.length()+miBufferOffset)))
      {
         iLengthRead = (mBuffer.length()+miBufferOffset) - miOffset;
         if (iLengthRead < 0)
            iLengthRead = 0 ;
         memcpy(szBuffer, &mBuffer.data()[miOffset-miBufferOffset], iLengthRead) ;
      }
      else
      {
         iLengthRead = iLength ;
         int pos = miOffset-miBufferOffset;
         if (pos < 0)
            pos = 0;
         memcpy(szBuffer, &mBuffer.data()[pos], iLengthRead) ;
      }


      // Increment the data offset
      miOffset += iLengthRead ;

      if (!(getFlags() & STREAM_HINT_CACHE))
      {
         if (iLengthRead > 0)
            mBuffer.remove(0, iLengthRead) ;

         miBufferOffset += iLengthRead ;
      }
      mSemGuardData.release() ;

      rc = (iLengthRead > 0) ? OS_SUCCESS : OS_FAILED ;
   }

   // Whenever we read, allow the feeder to unblock
   mSemLimitData.release() ;

   return rc ;
}


// Identical to read, except the stream pointer is not advanced.
OsStatus StreamHttpDataSource::peek(char *szBuffer, ssize_t iLength, ssize_t& iLengthRead)
{
   OsStatus rc = OS_INVALID ;

   // Make sure enough data is avaiable
   while (((iLength+miOffset) > (mBuffer.length()+miBufferOffset)) && !mbDone
         && !mbInterrupt)
   {
      mSemNeedData.acquire() ;
   }

   if (mbInterrupt)
   {
      // Set the return code, clear the flag, and set length read.
      rc = OS_INTERRUPTED ;
      mbInterrupt = FALSE ;
      iLengthRead = 0 ;
   }
   else
   {
      // Copy data...
      mSemGuardData.acquire() ;
      if (mbDone && ((iLength+miOffset) > (mBuffer.length()+miBufferOffset)))
      {
         iLengthRead = (mBuffer.length()+miBufferOffset) - miOffset;
         if (iLengthRead < 0)
            iLengthRead = 0 ;
         memcpy(szBuffer, &mBuffer.data()[miOffset-miBufferOffset], iLengthRead) ;
      }
      else
      {
         iLengthRead = iLength ;
         memcpy(szBuffer, &mBuffer.data()[miOffset-miBufferOffset], iLengthRead) ;
      }
      mSemGuardData.release() ;

      rc = (iLengthRead > 0) ? OS_SUCCESS : OS_FAILED ;
   }

   return rc ;
}


// Interrupts any time consuming operation.
OsStatus StreamHttpDataSource::interrupt()
{
   if (!mbDone)
   {
      mbInterrupt = true ;
      mSemNeedData.release() ;
   }

   return OS_SUCCESS ;
}


// Moves the stream pointer to the an absolute location.
OsStatus StreamHttpDataSource::seek(size_t iLocation)
{
   OsStatus status = OS_FAILED ;

   // We need to seek to the position within the http data source.  This
   // is ugly because of the different modes of the HttpDataSource:
   //
   //  Windowing
   //    - Could have passed the data
   //    - Did not fetch the data yet
   //  Full-download mode
   //    - Did not fetch the data yet


   // Wait for the data if we haven't fetched it yet
   while ((iLocation > getBufferedLength()+miBufferOffset) && !mbDone
         && !mbInterrupt)
   {
      mSemNeedData.acquire() ;
   }

   if (mbInterrupt)
   {
      // Set the return code, clear the flag
      status = OS_INTERRUPTED ;
      mbInterrupt = FALSE ;
   }
   else
   {
      // Verify that we didn't pass the data
      if (!(getFlags() & STREAM_HINT_CACHE))
      {
         if (iLocation >= miBufferOffset)
         {
            status = OS_SUCCESS ;
         }
         else
         {
            // The requested location is below the current window and
            // we need to rewind the data source.  This is
            // accomplished by closing it, re-opening it, and seeking
            // to the correct position.
            status = close() ;
            if (status == OS_SUCCESS)
            {
               status = open() ;
               if (status == OS_SUCCESS)
               {
                  status = seek(iLocation) ;
               }
            }
         }
      }
      else if (iLocation < getBufferedLength()+miBufferOffset)
      {
         status = OS_SUCCESS ;
      }

      if (status == OS_SUCCESS)
         miOffset = iLocation ;
   }

   return (status) ;
}


// Callback routine that is invoked whenever new data is available from http
// socket.
UtlBoolean StreamHttpDataSource::deliverData(char *szData, ssize_t iLength, ssize_t iMaxLength)
{
   // Set the max length
   if (iMaxLength >= 0 )
   {
      miDSLength = iMaxLength ;
   }

   // Reset capacity if warranted
   if (getFlags() & STREAM_HINT_CACHE)
   {
      mBuffer.capacity(miDSLength) ;
      miMaxData = iMaxLength ;
   }

   if (iLength > 0)
   {
      mSemGuardData.acquire() ;
      mBuffer.append(szData, iLength) ;
      mSemGuardData.release() ;
   }
   else
      mbDone = true ;

   mSemNeedData.release() ;

   // Throttle data if needed
   if (!(getFlags() & STREAM_HINT_CACHE))
   {
      while ((mBuffer.length() > miMaxData) && !mbDone)
      {
         if (!mbFiredThrottledEvent)
         {
            // Fire started event to subscribed listeners
            fireEvent(LoadingStartedEvent) ;

            mbFiredThrottledEvent = TRUE ;
         }

         mSemNeedData.release() ;     // Should NOT be needed

         if (!mbDone)
            mSemLimitData.acquire() ;
      }
   }

   return !mbQuit ;
}


/* ============================ ACCESSORS ================================= */

// Gets the length of the stream (if available)
OsStatus StreamHttpDataSource::getLength(ssize_t& iLength)
{
   OsStatus status = OS_FAILED ;

   if (miDSLength >= 0)
   {
      iLength = miDSLength ;
      status = OS_SUCCESS ;
   }
   return status ;
}


// Gets the current position within the stream.
OsStatus StreamHttpDataSource::getPosition(ssize_t& iPosition)
{
   OsStatus status = OS_SUCCESS ;

   iPosition = miOffset ;

   return status ;
}


// Renders a string describing this data source.
OsStatus StreamHttpDataSource::toString(UtlString& string)
{
   UtlString url ;
   string = "[Http] " ;
   m_url.toString(url) ;
   string.append(url) ;

   return OS_SUCCESS ;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Copy constructor (not supported)
StreamHttpDataSource::StreamHttpDataSource(const StreamHttpDataSource& rStreamHttpDataSource)
   : StreamDataSource(0)
   , mSemNeedData(OsBSem::Q_PRIORITY, OsBSem::EMPTY)
   , mSemLimitData(OsBSem::Q_PRIORITY, OsBSem::EMPTY)
   , mSemGuardData(OsBSem::Q_PRIORITY, OsBSem::FULL)
   , mSemGuardStartClose(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
}


// Assignment operator (not supported)
StreamHttpDataSource&
StreamHttpDataSource::operator=(const StreamHttpDataSource& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

// Gets the amount of data presently buffered.
int StreamHttpDataSource::getBufferedLength()
{
   int iLength;

   mSemGuardData.acquire() ;
   iLength = mBuffer.length() ;
   mSemGuardData.release() ;

   return  iLength ;
}


// Thread entry point
int StreamHttpDataSource::run(void *pArgs)
{
   HttpMessage* pMsg ;                // Http Message (response) for stream
   int          iResponseCode = 0 ;

   pMsg = new HttpMessage() ;

#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("StreamHttpDataSource: Starting...\n") ;
#endif /* MP_STREAM_DEBUG ] */

   pMsg->get(m_url, 30*1000, DataCallbackProc, this) ;

   // Fire completed/error event to subscribed listeners
   iResponseCode = pMsg->getResponseStatusCode() ;

   // See the class level documentation in the header file for a description
   // of the destruction process.
   {
      OsLock lock(sSemGuardDelete) ;   // Scoping is important
      if (!mbDeleteOnCompletion)
      {
         if (iResponseCode == 200)
         {
            fireEvent(LoadingCompletedEvent) ;
         }
         else
         {
            fireEvent(LoadingErrorEvent) ;
         }
      }
      delete pMsg ;

#ifdef MP_STREAM_DEBUG /* [ */
      osPrintf("StreamHttpDataSource: Exiting Naturally...\n") ;
#endif /* MP_STREAM_DEBUG ] */
   }

   if (mbDeleteOnCompletion)
   {
      delete this ;
   }
   return 0 ;
}




/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

// Call back for data gathering from socket
UtlBoolean DataCallbackProc(char* pData,
                           ssize_t iLength,
                           void* pOptionalData,
                           HttpMessage* pMsg)
{
   StreamHttpDataSource* pSource = (StreamHttpDataSource*) pOptionalData ;
   static int iTrueLength = -1 ;

#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("DataCallbackProc: length=%d\n", iLength) ;
#endif /* MP_STREAM_DEBUG ] */

   const char* pValue = pMsg->getHeaderValue(0, HTTP_CONTENT_LENGTH_FIELD) ;
   if (pValue != NULL)
   {
      iTrueLength = atoi(pValue) ;
   }

   return pSource->deliverData(pData, iLength, iTrueLength) ;
}
