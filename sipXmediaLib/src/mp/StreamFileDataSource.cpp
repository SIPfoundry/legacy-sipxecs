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
#include "mp/StreamFileDataSource.h"
#include "os/OsLock.h"
#include "net/HttpMessage.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
StreamFileDataSource::StreamFileDataSource(Url url, int iFlags)
   : StreamDataSource(iFlags)
   , mFileGuard(OsMutex::Q_FIFO)
{
   mUrl = url ;
   mpFile = NULL ;
}

// Destructor
StreamFileDataSource::~StreamFileDataSource()
{
   OsLock lock(mFileGuard) ;

   if (mpFile != NULL)
   {
      delete mpFile ;
      mpFile = NULL ;
   }
}

/* ============================ MANIPULATORS ============================== */

// Opens the data source
OsStatus StreamFileDataSource::open()
{
   OsLock lock(mFileGuard) ;
   UtlString hostName ;
   UtlString pathName ;
   OsStatus status = OS_FAILED ;

   if (mpFile == NULL)
   {
      // Fire started event to subscribed listeners
      fireEvent(LoadingStartedEvent) ;

      mUrl.getPath(pathName) ;
      HttpMessage::unescape(pathName) ; // unescape it, so %xx become chars
      mpFile = new OsFile(pathName) ;

	  if (mpFile != NULL)
	  {
		 status = mpFile->open(OsFile::READ_ONLY) ;

         // Fire completed/error event to subscribed listeners
         if (status == OS_SUCCESS)
            fireEvent(LoadingCompletedEvent) ;
         else
            fireEvent(LoadingErrorEvent) ;
	  }
	  else
	  {
         fireEvent(LoadingErrorEvent) ;
	  }

   }

   return status ;
}


// Closes the data source
OsStatus StreamFileDataSource::close()
{
   OsLock lock(mFileGuard) ;
   OsStatus status = OS_FAILED ;

   if (mpFile != NULL)
   {
      mpFile->close() ;
      delete mpFile ;
      mpFile = NULL ;
      status = OS_SUCCESS ;
   }
   return status ;
}

// Destroys and deletes the data source object
OsStatus StreamFileDataSource::destroyAndDelete()
{
    OsStatus status = OS_SUCCESS ;
    delete this ;

    return status ;
}



// Reads iLength bytes of data from the data source and places the data into
// the passed szBuffer buffer.
OsStatus StreamFileDataSource::read(char *szBuffer, ssize_t iLength, ssize_t& iLengthRead)
{
   OsLock lock(mFileGuard) ;
   OsStatus rc = OS_FAILED ;

   if (mpFile != NULL)
   {
      rc = mpFile->read(szBuffer, iLength, (size_t&)iLengthRead);

   }

   return rc ;
}


// Identical to read, except the stream pointer is not advanced.
OsStatus StreamFileDataSource::peek(char *szBuffer, ssize_t iLength, ssize_t& iLengthRead)
{
   OsLock lock(mFileGuard) ;
   OsStatus rc = OS_FAILED ;
   size_t lFilePosition = 0 ;

   if (mpFile != NULL)
   {
      rc = mpFile->getPosition(lFilePosition) ;
      if (rc == OS_SUCCESS)
      {
         rc = mpFile->read(szBuffer, iLength, (size_t&)iLengthRead) ;
         if (rc == OS_SUCCESS)
         {
            rc = mpFile->setPosition(lFilePosition) ;
         }
      }
   }

   return rc ;
}



// Moves the stream pointer to the an absolute location.
OsStatus StreamFileDataSource::seek(size_t iLocation)
{
   OsLock lock(mFileGuard) ;
   OsStatus rc = OS_FAILED ;

   if (mpFile != NULL)
   {
      rc = mpFile->setPosition(iLocation, OsFile::START) ;
   }
   else
   {
      rc = open();
   }

   return rc ;
}


/* ============================ ACCESSORS ================================= */

// Gets the length of the stream (if available)
OsStatus StreamFileDataSource::getLength(ssize_t& iLength)
{
   OsLock lock(mFileGuard) ;
   OsStatus rc = OS_FAILED ;

   size_t lLength = 0;
   if (mpFile != NULL)
      rc = mpFile->getLength(lLength) ;

   iLength = lLength ;
   return rc ;
}


// Gets the current position within the stream.
OsStatus StreamFileDataSource::getPosition(ssize_t& iPosition)
{
   OsLock lock(mFileGuard) ;
   OsStatus status = OS_FAILED ;

   if (mpFile != NULL)
   {
      size_t lPosition ;
      status = mpFile->getPosition(lPosition) ;
      iPosition = lPosition ;
   }

   return status ;
}



// Renders a string describing this data source.
OsStatus StreamFileDataSource::toString(UtlString& string)
{
   UtlString url ;
   string = "[File] " ;
   mUrl.toString(url) ;
   string.append(url) ;

   return OS_SUCCESS ;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Copy constructor (not supported)
StreamFileDataSource::StreamFileDataSource(const StreamFileDataSource& rStreamFileDataSource)
   : StreamDataSource(0)
   , mFileGuard(OsMutex::Q_FIFO)
{
	assert(FALSE) ;
}

// Assignment operator (not supported)
StreamFileDataSource&
StreamFileDataSource::operator=(const StreamFileDataSource& rhs)
{
    assert(FALSE) ;

   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */
