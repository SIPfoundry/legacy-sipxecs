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
#include "mp/StreamFormatDecoder.h"
#include "mp/StreamDecoderListener.h"
#include "mp/StreamDefs.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
StreamFormatDecoder::StreamFormatDecoder(StreamDataSource* pDataSource)
   : mpListener(NULL)
{
   mpDataSource = pDataSource ;
}

// Destructor
StreamFormatDecoder::~StreamFormatDecoder()
{
}

/* ============================ MANIPULATORS ============================== */

// Sets a listener to receive StreamDecoderEvents.
void StreamFormatDecoder::setListener(StreamDecoderListener* pListener)
{
   mpListener = pListener ;
}

/* ============================ ACCESSORS ================================= */

// Gets the the data source for this decoder
StreamDataSource* StreamFormatDecoder::getDataSource()
{
   return mpDataSource ;
}

/* ============================ INQUIRY =================================== */

/* ============================ TESTING =================================== */


#ifdef MP_STREAM_DEBUG /* [ */
const char* StreamFormatDecoder::getEventString(StreamDecoderEvent event)
{
   switch (event)
   {
      case DecodingStartedEvent:
         return "DecodingStartedEvent" ;
         break;
      case DecodingUnderrunEvent:
         return "DecodingUnderrunEvent" ;
         break ;
      case DecodingThrottledEvent:
         return "DecodingThrottledEvent" ;
         break ;
      case DecodingCompletedEvent:
         return "DecodingCompletedEvent" ;
         break ;
      case DecodingErrorEvent:
         return "DecodingErrorEvent" ;
         break ;
      default:
         return "DecodingUnknownEvent" ;
         break ;
   }
}

#endif /* MP_STREAM_DEBUG ] */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Copy constructor (not supported)
StreamFormatDecoder::StreamFormatDecoder(const StreamFormatDecoder& rStreamFormatDecoder)
{
}

// Assignment operator (not supported)
StreamFormatDecoder&
StreamFormatDecoder::operator=(const StreamFormatDecoder& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


// Fire an event to an interested consumer.
void StreamFormatDecoder::fireEvent(StreamDecoderEvent event)
{
#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("StreamFormatDecoder: %s\n", getEventString(event)) ;
#endif /* MP_STREAM_DEBUG ] */

   if (mpListener != NULL)
   {
      mpListener->decoderUpdate(this, event) ;
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
	  osPrintf("StreamFormatDecoder: Null listener for event %s\n", getEventString(event)) ;
   }
#endif /* MP_STREAM_DEBUG ] */
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */
