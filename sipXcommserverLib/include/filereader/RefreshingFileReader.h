//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

#ifndef _RefreshingFileReader_h_
#define _RefreshingFileReader_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include <utl/UtlString.h>
#include <os/OsTimer.h>
#include <os/OsStatus.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Abstract class for constructing objects that are initialized from the
//: contents of a file, and every time the object is consulted, it
//: checks to see if the file has changed, and if so, re-read it to
//: reinitialize the object.
class RefreshingFileReader
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   // Dummy time values.

   /** This value has the type of mFileLastModTimeCheck, and can never be
    *  returned by OsDateTime::getSecsSinceEpoch().
    */
   static const unsigned long sTimeCheckInvalid;
   /** These values have the type of mFileModTime, and can never be
    *  returnd by OsFileInfo::getModifiedTime().
    *  They must all be distinct, because we must re-process the file
    *  any time there is a transition between the states they represent.
    */
   static const OsTime sModTimeInvalid;      // To indicate mFileModTime is invalid.
   static const OsTime sModTimeNoFileName;   // To correspond to mFileName == "".
   static const OsTime sModTimeFileNotExist; // To indicate mFileName does not exist.

/* ============================ CREATORS ================================== */

   RefreshingFileReader();

   //! Set the file name, or clear it with NULL or "".
   //  Copies *fileName; does not save fileName.
   void setFileName(const UtlString* fileName);

   //! Destructor
   virtual ~RefreshingFileReader();

/* ============================ MANIPULATORS ============================== */

   //! Check to see if the file has changed, and if so, call initialize().
   virtual OsStatus refresh();

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   //! Reset the state variables to force the file to be re-read.
   void reinitialize_state();

   //! (Re-)read the file and (re)initialize the object.
   //  If there is no file name, "initialize()" will be called with
   //  mFileName = "".
   virtual OsStatus initialize() = 0;

   // The full name of the file, or "" if there is none.
   UtlString mFileName;

   // The last time we checked the modification time of mFileName.
   unsigned long mFileLastModTimeCheck;

   // The last known modification time of mFileName, or OS_INFINITY
   // if it did not exist.
   OsTime mFileModTime;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   RefreshingFileReader(const RefreshingFileReader& rRefreshingFileReader);
   //:Copy constructor

   RefreshingFileReader& operator=(const RefreshingFileReader& rRefreshingFileReader);
   //:Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _RefreshingFileReader_h_
