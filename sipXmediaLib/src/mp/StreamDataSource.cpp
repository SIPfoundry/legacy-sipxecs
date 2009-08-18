//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/StreamDataSource.h"
#include "mp/StreamDataSourceListener.h"
#include "mp/StreamDefs.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
StreamDataSource::StreamDataSource(int iFlags /* = 0 */)
   : mpListener(NULL)
{
   miFlags = iFlags ;
}


// Destructor
StreamDataSource::~StreamDataSource()
{
}

/* ============================ MANIPULATORS ============================== */


// Interrupts any time consuming operation.
OsStatus StreamDataSource::interrupt()
{
    // Default implemenation does nothing.
    return OS_NOT_SUPPORTED ;
}


// Sets a listener to receive StreamDataSourceEvent events for this data
// source.
void StreamDataSource::setListener(StreamDataSourceListener* pListener)
{
   mpListener = pListener ;
}


/* ============================ ACCESSORS ================================= */

// Gets the flags specified at time of construction
int StreamDataSource::getFlags()
{
   return miFlags;
}


/* ============================ INQUIRY =================================== */

/* ============================ TESTING =================================== */

#ifdef MP_STREAM_DEBUG /* [ */
const char* StreamDataSource::getEventString(StreamDataSourceEvent event)
{
   switch (event)
   {
      case LoadingStartedEvent:
         return "LoadingStartedEvent" ;
         break;
      case LoadingThrottledEvent:
         return "LoadingThrottledEvent" ;
         break ;
      case LoadingCompletedEvent:
         return "LoadingCompletedEvent" ;
         break ;
      case LoadingErrorEvent:
         return "LoadingErrorEvent" ;
         break ;
      default:
         return "LoadingUnknownEvent" ;
         break ;
   }
}
#endif /* MP_STREAM_DEBUG ] */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Assignment operator (not supported)
StreamDataSource&
StreamDataSource::operator=(const StreamDataSource& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

// Copy constructor (not supported)
StreamDataSource::StreamDataSource(const StreamDataSource& rStreamDataSource)
{
}

// Fires a data source event to the interested consumer.
void StreamDataSource::fireEvent(StreamDataSourceEvent event)
{
#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("StreamDataSource: %s\n", getEventString(event)) ;
#endif /* MP_STREAM_DEBUG ] */

   if (mpListener != NULL)
   {
      mpListener->dataSourceUpdate(this, event) ;
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
      osPrintf("** WARNING: unable to send event %s -- null listener\n", getEventString(event)) ;
   }
#endif /* MP_STREAM_DEBUG ] */
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */
