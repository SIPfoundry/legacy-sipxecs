//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsLock.h"
#include "mp/StreamQueueingFormatDecoder.h"
#include "mp/StreamQueueMsg.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define PERFORMANCE_REPORT_ERROR_PERIOD_SECS    300    // Report every 5
                                                       // minute if underruns
#define PERFORMANCE_REPORT_CLEAN_PERIOD_SECS    3600   // Report every hour
                                                       // if running clean

// STATIC VARIABLE INITIALIZATIONS
OsMutex StreamQueueingFormatDecoder::mMutReport(OsMutex::Q_FIFO) ;
time_t StreamQueueingFormatDecoder::sLastReported = 0 ;
unsigned int StreamQueueingFormatDecoder::sDeltaFrames = 0 ;
unsigned int StreamQueueingFormatDecoder::sDeltaStreams = 0 ;
unsigned int StreamQueueingFormatDecoder::sDeltaUnderruns = 0 ;
unsigned int StreamQueueingFormatDecoder::sDeltaThrottles = 0 ;
unsigned int StreamQueueingFormatDecoder::sTotalFrames = 0 ;
unsigned int StreamQueueingFormatDecoder::sTotalStreams = 0 ;
unsigned int StreamQueueingFormatDecoder::sTotalUnderruns = 0 ;
unsigned int StreamQueueingFormatDecoder::sTotalThrottles = 0 ;



/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */


// Constructor
StreamQueueingFormatDecoder::
StreamQueueingFormatDecoder(StreamDataSource* pDataSource, int iQueueLength)
   : StreamFormatDecoder(pDataSource)
   , mMsgqFrames(iQueueLength)
   , mMsgPool("StreamQueueingFormatDecoder", StreamQueueMsg(),
         iQueueLength+1,iQueueLength+2,
         iQueueLength+2, 1, OsMsgPool::SINGLE_CLIENT)
   , mbReportThrottle(TRUE)
{

   miMaxQueueLength = iQueueLength ;

   mbDraining = FALSE ;
   reportStream() ;
}

// Destructor
StreamQueueingFormatDecoder::~StreamQueueingFormatDecoder()
{
}

/* ============================ MANIPULATORS ============================== */

// Gets the next available frame
OsStatus StreamQueueingFormatDecoder::getFrame(uint16_t* samples)
{
   OsStatus status = OS_SUCCESS ;

   int iNumQueuedFrames = getNumQueuedFrames() ;

   // Report Throttle / Underruns
   if (iNumQueuedFrames <= 0)
   {
      fireEvent(DecodingUnderrunEvent) ;

      // Clear throttle reporting
      mbReportThrottle = TRUE ;
      if (!mbDraining)
        reportFrame(TRUE) ;
   }
   else
   {
      if (!mbDraining)
         reportFrame(FALSE) ;
   }

   // If a sample is available, get it.
   if (iNumQueuedFrames > 0)
   {
      StreamQueueMsg *pMsg ;
      mMsgqFrames.receive((OsMsg*&)pMsg) ;

      // If we have hit the end of the frames, then return non OS_SUCCESS
      if (!pMsg->getSamples((int16_t*)samples))
      {
         status = OS_INVALID ;
         memset(samples, 0, sizeof(int16_t) * 80) ;
      }
      pMsg->releaseMsg() ;
   }
   else
   {
      // If not samples are available, return silence.
      memset(samples, 0, sizeof(int16_t) * 80) ;
   }

   return status ;
}

// Checks to see if queue is full, and if so needs to report throttle
// activity to avoid a stall
void StreamQueueingFormatDecoder::checkThrottle()
{
   if (getNumQueuedFrames() == getMaxQueueLength())
   {
      // Only report once (reset if we ever have an underrun)
      if (mbReportThrottle)
      {
         fireEvent(DecodingThrottledEvent) ;
         mbReportThrottle = FALSE ;
      }
      if (!mbDraining)
      {
         reportThrottle();
      }
   }
}

// Queues a frame of data
OsStatus StreamQueueingFormatDecoder::queueFrame(const uint16_t* pSamples)
{
   OsStatus status = OS_SUCCESS;

   // check if throttling needs to happen
   checkThrottle() ;

   // Queue frame
   StreamQueueMsg* pMsg = (StreamQueueMsg*) mMsgPool.findFreeMsg() ;
   if (pMsg)
   {
      pMsg->setSamples((const int16_t*)pSamples);
         mMsgqFrames.send(*pMsg) ;
   }
   else
   {
      status = OS_FAILED;
      OsSysLog::add(FAC_MP, PRI_ERR, "StreamQueueingFormatDecoder::queueFrame failed: free msg is NULL!\n");
   }

   return status ;
}


// Queues an end of frame marker.  This informs MprFromStream that the Stream
// has ended.
OsStatus StreamQueueingFormatDecoder::queueEndOfFrames()
{
   OsStatus status = OS_SUCCESS ;

   // check if throttling needs to happen
   checkThrottle() ;

   // Queue an end of frame marker
   StreamQueueMsg* pMsg = (StreamQueueMsg*) mMsgPool.findFreeMsg() ;
   if (pMsg)
   {
       pMsg->setMsgSubType(StreamQueueMsg::EndOfFrameMarker) ;
       mMsgqFrames.send(*pMsg) ;
   }
   else
   {
    status = OS_FAILED;
    OsSysLog::add(FAC_MP, PRI_ERR, "StreamQueueingFormatDecoder::queueEndOfFrames failed: free msg is NULL!\n");
   }

   return OS_SUCCESS ;
}

//: Drains any queued frames
OsStatus StreamQueueingFormatDecoder::drain()
{
   uint16_t samples[80] ;

   mbDraining = TRUE ;
   while (getNumQueuedFrames() > 0)
   {
      getFrame(samples) ;
   }
   mbDraining = FALSE ;

   return OS_SUCCESS ;
}



/* ============================ ACCESSORS ================================= */

// Gets the maximum number of frames that can be queued before the queueing
// routines will block.
int StreamQueueingFormatDecoder::getMaxQueueLength()
{
   return miMaxQueueLength ;
}

// Gets the current number of queued frames.
int StreamQueueingFormatDecoder::getNumQueuedFrames()
{
   return mMsgqFrames.numMsgs() ;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Copy constructor (not supported)
StreamQueueingFormatDecoder::StreamQueueingFormatDecoder(
      const StreamQueueingFormatDecoder& rStreamQueueingFormatDecoder)
   : StreamFormatDecoder(NULL)
   , mMsgPool("StreamQueueingFormatDecoder", StreamQueueMsg(), 1001, 1001,
          1001, 1, OsMsgPool::SINGLE_CLIENT)
{
    assert(FALSE) ;
}

// Assignment operator (not supported)
StreamQueueingFormatDecoder&
StreamQueueingFormatDecoder::operator=(const StreamQueueingFormatDecoder& rhs)
{
   assert(FALSE) ;

   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


// Reports that a frame has been processed by media processing.
void StreamQueueingFormatDecoder::reportFrame(UtlBoolean bUnderrun)
{
   OsLock lock(mMutReport) ;
   time_t now ;

   // Get the current time and set last reported time if not initialized
   time(&now) ;
   if (sLastReported == 0)
      sLastReported = now ;

   // Increment the number of frames and underruns (if true)
   sDeltaFrames++ ;
   sDeltaUnderruns += (int) bUnderrun ;

   // If we have underruns, report more frequenently --
   // PERFORMANCE_REPORT_ERROR_PERIOD_SECS.  Otherwise, report after
   // PERFORMANCE_REPORT_CLEAN_PERIOD_SECS seconds.
   int iReportAfterSecs ;
   if (sDeltaUnderruns > 0)
      iReportAfterSecs = PERFORMANCE_REPORT_ERROR_PERIOD_SECS ;
   else
      iReportAfterSecs = PERFORMANCE_REPORT_CLEAN_PERIOD_SECS ;

   if (now > (sLastReported+iReportAfterSecs))
   {
      // Update totals on report
      sTotalStreams += sDeltaStreams ;
      sTotalFrames += sDeltaFrames ;
      sTotalUnderruns += sDeltaUnderruns ;
      sTotalThrottles += sDeltaThrottles ;

#if defined(_VXWORKS) || defined(_WIN32)
      // Under vxWorks / Windows (regression tests), report using osPrintf
      osPrintf("Last %4d secs: streams=%4d, frames=%6d, underruns=%4d, throttles=%5d\n"\
            "    Cumulative: streams=%4d, frames=%6d, underruns=%4d, throttles=%5d\n",
            now-sLastReported, sDeltaStreams, sDeltaFrames, sDeltaUnderruns, sDeltaThrottles,
            sTotalStreams, sTotalFrames, sTotalUnderruns, sTotalThrottles) ;
#else
      // Under linux, report using OsSysLog
      OsSysLog::add(FAC_MP, PRI_INFO,
            "Last %4ld secs: streams=%4d, frames=%6d, underruns=%4d, throttles=%5d\n"\
            "    Cumulative: streams=%4d, frames=%6d, underruns=%4d, throttles=%5d\n",
            now-sLastReported, sDeltaStreams, sDeltaFrames, sDeltaUnderruns, sDeltaThrottles,
            sTotalStreams, sTotalFrames, sTotalUnderruns, sTotalThrottles) ;
#endif
      sDeltaStreams = 0 ;
      sDeltaFrames = 0 ;
      sDeltaUnderruns = 0 ;
      sDeltaThrottles = 0 ;

      // Reset the last reported time
      sLastReported = now ;
   }
}

// Reports that the decoder has been throttled (decoding faster then data is
// being requested).
void StreamQueueingFormatDecoder::reportThrottle()
{
   OsLock lock(mMutReport) ;

   sDeltaThrottles++ ;
}

// Reports that a stream has been created
void StreamQueueingFormatDecoder::reportStream()
{
   OsLock lock(mMutReport) ;

   sDeltaStreams++ ;
}


/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */
