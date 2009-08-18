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
#include "mp/StreamBufferDataSource.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
StreamBufferDataSource::StreamBufferDataSource(UtlString *pBuffer, int iFlags)
   : StreamDataSource(iFlags)
{
   miPosition = 0 ;
   mpBuffer = pBuffer ;
}


// Destructor
StreamBufferDataSource::~StreamBufferDataSource()
{
   if (mpBuffer != NULL)
   {
      delete mpBuffer ;
      mpBuffer = NULL ;
   }
}

/* ============================ MANIPULATORS ============================== */

// Opens the data source
OsStatus StreamBufferDataSource::open()
{
   OsStatus status = (mpBuffer != NULL) ? OS_SUCCESS : OS_FAILED ;

   // Fire started event to subscribed listeners
   fireEvent(LoadingStartedEvent) ;

   // Fire completed/error event to subscribed listeners
   if (status == OS_SUCCESS)
      fireEvent(LoadingCompletedEvent) ;
   else
      fireEvent(LoadingErrorEvent) ;

   return status ;
}


// Closes the data source
OsStatus StreamBufferDataSource::close()
{
   return OS_SUCCESS ;
}

// Destroys and deletes the data source object
OsStatus StreamBufferDataSource::destroyAndDelete()
{
    OsStatus status = OS_SUCCESS ;

    delete this ;

    return status ;
}



// Reads iLength bytes of data from the data source and places the data into
// the passed szBuffer buffer.
OsStatus StreamBufferDataSource::read(char *szBuffer, ssize_t iLength, ssize_t& iLengthRead)
{
   OsStatus rc = OS_FAILED;
   iLengthRead = 0;

   if (mpBuffer != NULL)
   {
      ssize_t iBufferLength = mpBuffer->length();

      if (miPosition < iBufferLength)
      {
         ssize_t iMaxCopy = __min(iBufferLength - miPosition, iLength);
         memcpy(szBuffer, &mpBuffer->data()[miPosition], iMaxCopy);
         miPosition += iMaxCopy;

         iLengthRead = iMaxCopy;
         rc = OS_SUCCESS;
      }
   }

   return rc;
}


// Identical to read, except the stream pointer is not advanced.
OsStatus StreamBufferDataSource::peek(char *szBuffer, ssize_t iLength, ssize_t& iLengthRead)
{
   OsStatus rc = OS_FAILED ;
   size_t iPosition = miPosition ;

   if (mpBuffer != NULL)
   {
      size_t iBufferLength = mpBuffer->length() ;

      if (iPosition < iBufferLength)
      {
         size_t iMaxCopy = __min(iBufferLength - iPosition, (size_t)iLength) ;
         memcpy(szBuffer, &mpBuffer->data()[iPosition], iMaxCopy) ;
         iPosition += iMaxCopy ;

         rc = OS_SUCCESS ;
      }
   }

   return rc ;
}



// Moves the stream pointer to the an absolute location.
OsStatus StreamBufferDataSource::seek(size_t iLocation)
{
   OsStatus status = OS_FAILED ;

   if (mpBuffer != NULL)
   {
      if ((iLocation > 0) && (iLocation <= mpBuffer->length()))
      {
         miPosition = iLocation ;
         status = OS_SUCCESS ;
      }
   }

   return status;
}


/* ============================ ACCESSORS ================================= */


// Gets the length of the stream (if available)
OsStatus StreamBufferDataSource::getLength(ssize_t& iLength)
{
   OsStatus status = OS_FAILED ;

   if (mpBuffer != NULL)
   {
      iLength = mpBuffer->length() ;
      status = OS_SUCCESS ;
   }

   return status ;
}


// Gets the current position within the stream.
OsStatus StreamBufferDataSource::getPosition(ssize_t& iPosition)
{
   OsStatus status = OS_FAILED ;

   if (mpBuffer != NULL)
   {
      iPosition = miPosition ;
      status = OS_SUCCESS ;
   }

   return status ;
}


// Renders a string describing this data source.
OsStatus StreamBufferDataSource::toString(UtlString& string)
{
   string = "[buffer] size=" + mpBuffer->length() ;

   return OS_SUCCESS ;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Copy constructor (not supported)
StreamBufferDataSource::StreamBufferDataSource(const StreamBufferDataSource& rStreamBufferDataSource)
{
}


// Assignment operator (not supported)
StreamBufferDataSource&
StreamBufferDataSource::operator=(const StreamBufferDataSource& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */
