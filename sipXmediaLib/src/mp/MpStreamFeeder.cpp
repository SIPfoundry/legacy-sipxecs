//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/StreamDefs.h"
#include "mp/MpStreamFeeder.h"
#include "os/OsSysLogMsg.h"
#include "os/OsLock.h"

#include "os/OsFS.h"
#include "os/OsBSem.h"
#include "os/OsWriteLock.h"
#include "os/OsNotification.h"
#include "net/HttpMessage.h"

#ifdef INCL_RAW_DECODER
#include "mp/StreamRAWFormatDecoder.h"
#endif

#ifdef INCL_WAV_DECODER
#include "mp/StreamWAVFormatDecoder.h"
#endif

#ifdef INCL_MP3_DECODER
#include "mpegdec/StreamMP3FormatDecoder.h"
#endif

#include "mp/StreamFileDataSource.h"
#include "mp/StreamHttpDataSource.h"
#include "mp/StreamBufferDataSource.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
int MpStreamFeeder::s_iInstanceCount = 0 ;


/* //////////////////////////// PUBLIC //////////////////////////////////// */


/* ============================ CREATORS ================================== */

// Constructor accepting a url resource, type, and cache all flag
MpStreamFeeder::MpStreamFeeder(Url resource, int flags)
   : m_state(UnrealizedState)
   , m_pFormatDecoder(NULL)
   , m_pDataSource(NULL)
   , m_bMarkedPaused(FALSE)
   , m_pEventHandler(NULL)
   , m_eventGuard(OsMutex::Q_FIFO)
{
   mFlags = flags ;

   UtlString scheme ;

   m_iInstanceId = s_iInstanceCount++ ;

#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("MpStreamFeeder(%d): Construction url=%s, flags=0x%08X\n",
         m_iInstanceId, resource.toString().data(), flags) ;
#endif /* MP_STREAM_DEBUG ] */

   // Instantiate appropriate data source
   resource.getUrlType(scheme) ;
   if ( ( scheme.compareTo("http", UtlString::ignoreCase) == 0) ||
        ( scheme.compareTo("https", UtlString::ignoreCase) == 0) )
   {
      m_pDataSource = new StreamHttpDataSource(resource, flags) ;
      m_pDataSource->setListener(this) ;
   }
   else if (scheme.compareTo("file", UtlString::ignoreCase) == 0)
   {
      m_pDataSource = new StreamFileDataSource(resource, flags) ;
      m_pDataSource->setListener(this) ;
   }
   else
      return ;
}


// Constructor accepting a pre-populated buffer, type and cache all flag
MpStreamFeeder::MpStreamFeeder(UtlString* pBuffer, int flags)
   : m_state(UnrealizedState)
   , m_pFormatDecoder(NULL)
   , m_pDataSource(NULL)
   , m_bMarkedPaused(FALSE)
   , m_pEventHandler(NULL)
   , m_eventGuard(OsMutex::Q_FIFO)
{
   mFlags = flags ;

   m_iInstanceId = s_iInstanceCount++ ;

#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("MpStreamFeeder(%d): Construction buffer=0x%08X, flags=0x%08X\n",
         m_iInstanceId, pBuffer, flags) ;
#endif /* MP_STREAM_DEBUG ] */


   UtlString scheme ;

   // Instantiate appropriate data source
   m_pDataSource = new StreamBufferDataSource(pBuffer, flags) ;
   m_pDataSource->setListener(this) ;
}



/* ============================ MANIPULATORS ============================== */

// Realizes the data source
OsStatus MpStreamFeeder::realize()
{
   OsStatus rc = OS_NOT_SUPPORTED ;

#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("MpStreamFeeder(%d): realize()\n", m_iInstanceId) ;
#endif /* MP_STREAM_DEBUG ] */


   // 1st: Start the data source
   if ((m_pDataSource != NULL) && (m_pDataSource->open() == OS_SUCCESS))
   {
      // Init the decoding source (may need to auto detect)
      if (m_pFormatDecoder == NULL)
         initDecodingSource() ;

      // Start up the decoder
      if ((m_pFormatDecoder != NULL) &&
            (m_pFormatDecoder->init() == OS_SUCCESS))
      {
         // Mark the state
         setState(RealizedState) ;
         rc = OS_SUCCESS ;
      }
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
      osPrintf("MpStreamFeeder(%d): Unable to open data source in realize\n",
	        m_iInstanceId) ;
   }
#endif /* MP_STREAM_DEBUG ] */

   // If we have any errors, clean up.
   if (rc != OS_SUCCESS)
   {
      if (m_pDataSource)
         m_pDataSource->close() ;

      if (m_pFormatDecoder)
         m_pFormatDecoder->free() ;

      setState(FailedState) ;
   }

   return  rc ;
}


// Renders the data source
OsStatus MpStreamFeeder::render()
{
   OsStatus status = OS_INVALID_STATE ;

#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("MpStreamFeeder(%d): render()\n", m_iInstanceId) ;
#endif /* MP_STREAM_DEBUG ] */

   if (getState() == RealizedState)
   {
      if (!m_pFormatDecoder->isDecoding())
      {
	     assert(getState() == RealizedState) ;
         status = m_pFormatDecoder->begin() ;
      }
      else
         status = OS_SUCCESS ;
   }
   else if (getState() == StoppedState)
   {
      status = rewind() ;
      if (status == OS_SUCCESS)
         status = m_pFormatDecoder->begin() ;
   }
   return status ;
}


// Rewinds the data source
OsStatus MpStreamFeeder::rewind()
{
    OsStatus status = OS_INVALID_STATE ;

#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("MpStreamFeeder(%d): rewind()\n", m_iInstanceId) ;
#endif /* MP_STREAM_DEBUG ] */

   // You cannot rewind a failed player
   if (getState() != FailedState)
   {
       m_pFormatDecoder->end() ;

       OsStatus status = m_pDataSource->seek(0) ;
       if (status == OS_SUCCESS)
       {
          m_pFormatDecoder->begin() ;
          fireEvent(FeederPrefetchedEvent) ;
          status = m_pFormatDecoder->init() ;
       }
       else
       {
          fireEvent(FeederFailedEvent) ;
       }
   }
   return status ;
}


// Stops the data source
OsStatus MpStreamFeeder::stop()
{
#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("MpStreamFeeder(%d): stop()\n", m_iInstanceId) ;
#endif /* MP_STREAM_DEBUG ] */

   OsStatus status = OS_INVALID_STATE ;

   FeederState state = getState() ;
   if ( (state == RealizedState)
		 || (state == PrefetchingState)
         || (state == PrefetchedState)
         || (state == RendereringState))
   {
      status = m_pDataSource->close() ;
      status = m_pFormatDecoder->end() ;
   }

   return status ;
}


// Marks the data source as paused
void MpStreamFeeder::markPaused(UtlBoolean bPaused)
{
   m_bMarkedPaused = bPaused ;
}


// Sets the event handler for this renderer
OsStatus MpStreamFeeder::setEventHandler(OsNotification* pEventHandler)
{
#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("MpStreamFeeder(%d): setEventHandler to %08X\n",
         m_iInstanceId, pEventHandler) ;
#endif /* ] */

   m_pEventHandler = pEventHandler ;

   return OS_SUCCESS ;
}



/* ============================ ACCESSORS ================================= */


// Gets the next available frame
OsStatus MpStreamFeeder::getFrame(uint16_t *samples)
{
   OsStatus rc = OS_INVALID ;

   if (getState() != FailedState)
   {
       rc = m_pFormatDecoder->getFrame(samples) ;
   }

   return rc ;
}


// Gets the paused state
UtlBoolean MpStreamFeeder::isMarkedPaused()
{
   return m_bMarkedPaused ;
}


// Gets flags specified during creation
OsStatus MpStreamFeeder::getFlags(int &flags)
{
   flags = mFlags ;

   return OS_SUCCESS ;
}

// Gets the renderer state
FeederState MpStreamFeeder::getState()
{
   return m_state ;
}

/* ============================ INQUIRY =================================== */


// Is the transition from state source to target valid?
UtlBoolean MpStreamFeeder::isValidStateChange(FeederState source, FeederState target)
{
   UtlBoolean bSuccess = TRUE ;

   // You can NEVER come out of a failed state, also disallow duplicate events
   if ((source == FailedState) || (source == target))
   {
      bSuccess = FALSE ;
   }

   return bSuccess ;
}

/* ============================ TESTING =================================== */

#ifdef MP_STREAM_DEBUG /* [ */

const char* MpStreamFeeder::getEventString(FeederEvent event)
{
   switch (event)
   {
      case FeederRealizedEvent:
         return "FeederRealizedEvent" ;
         break;
      case FeederPrefetchedEvent:
         return "FeederPrefetchedEvent" ;
         break ;
      case FeederRenderingEvent:
         return "FeederRenderingEvent" ;
         break ;
      case FeederStoppedEvent:
         return "FeederStoppedEvent" ;
         break ;
      case FeederFailedEvent:
         return "FeederFailedEvent" ;
         break ;
      case FeederStreamPlayingEvent:
         return "FeederStreamPlayingEvent" ;
         break ;
      case FeederStreamPausedEvent:
         return "FeederStreamPausedEvent" ;
         break ;
      case FeederStreamStoppedEvent:
         return "FeederStreamStoppedEvent" ;
         break ;
      case FeederStreamAbortedEvent:
         return "FeederStreamAbortedEvent" ;
         break ;
      case FeederStreamDestroyedEvent:
         return "FeederStreamDestroyedEvent" ;
         break ;
      default:
         return "FeederUnknownEvent" ;
         break ;
   }
}

#endif /* MP_STREAM_DEBUG ] */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Fires events to interested parties
void MpStreamFeeder::fireEvent(FeederEvent eventType)
{
   OsLock lock(m_eventGuard) ;

#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("MpStreamFeeder(%d): %s\n", m_iInstanceId, getEventString(eventType)) ;
#endif /* MP_STREAM_DEBUG ] */

   if (m_pEventHandler != NULL)
   {
      m_pEventHandler->signal(eventType) ;

      // Safe guard: once the destroyed event is signalled, the
      // m_pEventHandler can be deleted.
      if (eventType == FeederStreamDestroyedEvent)
      {
         m_pEventHandler = NULL ;
      }

#ifdef MP_STREAM_DEBUG /* [ */
      pthread_t taskId = -1 ;
      OsTask *pTask = OsTask::getCurrentTask() ;
      if (pTask  != NULL)
      {
         pTask->id(taskId) ;
      }

      osPrintf("MpStreamFeeder(%d-%08X): signaled event: %s\n",
         m_iInstanceId, taskId, getEventString(eventType)) ;
#endif /* MP_STREAM_DEBUG ] */

   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
      osPrintf("** WARNING: Null handler for event: %s\n", getEventString(eventType)) ;
   }
#endif /* MP_STREAM_DEBUG ] */
}


// Destructor
MpStreamFeeder::~MpStreamFeeder()
{
#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("MpStreamFeeder(%d): destructor\n", m_iInstanceId) ;
#endif /* MP_STREAM_DEBUG ] */

   // Stop the decoder
   if(m_pFormatDecoder)
   {
      m_pFormatDecoder->setListener(NULL) ;
      m_pFormatDecoder->end() ;
   }
   // Close the data source
   if (m_pDataSource)
   {
      m_pDataSource->setListener(NULL) ;
      m_pDataSource->close() ;
   }

   // Destroy the decoder
   if (m_pFormatDecoder != NULL)
   {
      delete m_pFormatDecoder ;
      m_pFormatDecoder = NULL ;
   }

   // Destroy the data source
   if (m_pDataSource != NULL)
   {
      m_pDataSource->destroyAndDelete() ;
      m_pDataSource = NULL ;
   }
}

// Sets the internal state for this stream renderer
void MpStreamFeeder::setState(FeederState state)
{
   if ((m_state != state) && isValidStateChange(m_state, state))
   {
      m_state = state ;

      switch (m_state)
      {
         case UnrealizedState:
            break ;
         case RealizedState:
            fireEvent(FeederRealizedEvent) ;
            break ;
         case PrefetchingState:
            break ;
         case PrefetchedState:
            fireEvent(FeederPrefetchedEvent) ;
            break ;
         case RendereringState:
            fireEvent(FeederRenderingEvent) ;
            break ;
         case StoppedState:
            fireEvent(FeederStoppedEvent) ;
            break ;
         case FailedState:
            fireEvent(FeederFailedEvent) ;
            break ;
      }
   }
}

// Call back from data source updates
void MpStreamFeeder::dataSourceUpdate(StreamDataSource* pDataSource,
                                      StreamDataSourceEvent event)
{
   switch (event)
   {
      case LoadingStartedEvent:
         break ;
      case LoadingThrottledEvent:
         break ;
      case LoadingCompletedEvent:
         break ;
      case LoadingErrorEvent:
         setState(FailedState) ;
         break ;
   }
}

// Call back for MprFromStream updates
void MpStreamFeeder::fromStreamUpdate(FeederEvent event)
{
   fireEvent(event) ;
}


// Call back for decoder updates
void MpStreamFeeder::decoderUpdate(StreamFormatDecoder* pDecoder,
                                   StreamDecoderEvent event)
{
   switch (event)
   {
      case DecodingStartedEvent:
         setState(PrefetchingState) ;
         break ;
      case DecodingUnderrunEvent:
         break ;
      case DecodingThrottledEvent:
         setState(PrefetchedState) ;
         setState(RendereringState) ;
         break ;
      case DecodingCompletedEvent:
         if (m_state == PrefetchingState)
         {
            setState(PrefetchedState) ;
            setState(RendereringState) ;
         }
         break ;
      case DecodingErrorEvent:
         setState(FailedState) ;
         break ;
   }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */


// Construction helper: initialize the decoding source
void MpStreamFeeder::initDecodingSource()
{

   if (mFlags & STREAM_FORMAT_RAW)
   {
#ifdef INCL_RAW_DECODER /* [ */
      m_pFormatDecoder = new StreamRAWFormatDecoder(m_pDataSource) ;
      m_pFormatDecoder->setListener(this) ;
      m_pFormatDecoder->init() ;
#endif   /* INCL_RAW_DECODER ] */
   }
   else if (mFlags & STREAM_FORMAT_WAV)
   {
#ifdef INCL_WAV_DECODER /* [ */
      m_pFormatDecoder = new StreamWAVFormatDecoder(m_pDataSource) ;
      m_pFormatDecoder->setListener(this) ;
      m_pFormatDecoder->init() ;
#endif   /* INCL_WAV_DECODER ] */

   }
   else if (mFlags & STREAM_FORMAT_AU)
   {
      // AU
   }
   else if (mFlags & STREAM_FORMAT_MP3)
   {
      // MP3
#ifdef INCL_MP3_DECODER /* [ */
      m_pFormatDecoder = new StreamMP3FormatDecoder(m_pDataSource) ;
      m_pFormatDecoder->setListener(this) ;
      m_pFormatDecoder->init() ;
#endif   /* INCL_MP3_DECODER ] */
   }
   else
   {
      // Auto

      StreamFormatDecoder* pDecoder ;

#ifdef INCL_WAV_DECODER /* [ */
      pDecoder = new StreamWAVFormatDecoder(m_pDataSource) ;
      if (pDecoder->validDecoder())
      {
         m_pFormatDecoder = pDecoder ;
         m_pFormatDecoder->setListener(this) ;
         m_pFormatDecoder->init() ;
         return ;
      }
      else
      {
         delete pDecoder ;
      }
#endif   /* INCL_WAV_DECODER ] */

#ifdef INCL_MP3_DECODER /* [ */
      pDecoder = new StreamMP3FormatDecoder(m_pDataSource) ;
      if (pDecoder->validDecoder())
      {
         m_pFormatDecoder = pDecoder ;
         m_pFormatDecoder->setListener(this) ;
         m_pFormatDecoder->init() ;
         return ;
      }
      else
      {
         delete pDecoder ;
      }
#endif   /* INCL_MP3_DECODER ] */

#ifdef INCL_RAW_DECODER /* [ */
       m_pFormatDecoder = new StreamRAWFormatDecoder(m_pDataSource) ;
       m_pFormatDecoder->setListener(this) ;
       m_pFormatDecoder->init() ;
#endif   /* INCL_RAW_DECODER ] */
   }
}


/* ============================ FUNCTIONS ================================= */
