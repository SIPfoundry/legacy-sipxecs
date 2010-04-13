//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

#ifndef _OrbitFileReader_h_
#define _OrbitFileReader_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "filereader/RefreshingFileReader.h"
#include <utl/UtlHashMap.h>
#include <utl/UtlContainableAtomic.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class OrbitData;


//: Class construct an object that reads the orbits.xml file that describes
//: the "parking orbits".
class OrbitFileReader : public RefreshingFileReader
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OrbitFileReader();

   //:Default constructor
   ~OrbitFileReader();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

    // Look up a user name in the list of orbits.
    // If found, return a pointer to the orbit data for the user.
    OrbitData* findInOrbitList(const UtlString& user);

    // Retrieve the "music on hold" file name.
    void getMusicOnHoldFile(UtlString& file);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    OrbitFileReader(const OrbitFileReader& rOrbitFileReader);
    //:Copy constructor

    OrbitFileReader& operator=(const OrbitFileReader& rOrbitFileReader);
    //:Assignment operator

    // A hash map that has as keys all the call parking orbit users, and
    // as values OrbitData objects containing the information for the orbits.
    UtlHashMap mOrbitList;

    // The file containing the "music on hold" audio.
    UtlString mMusicOnHoldFile;

    //! (Re-)read the file and (re)initialize the object.
    //  If there is no file name, "initialize()" will be called with
    //  mFileName = "".
    virtual OsStatus initialize();

    OsStatus parseOrbitFile(UtlString& fileName);

};

// Object to contain the information in an orbit.xml entry.

class OrbitData : public UtlContainableAtomic
{

public:

   // The length of time in seconds before a parked call should be transferred
   // back to the parker.  NO_TIMEOUT means do not time out.
   int mTimeout;

   // The audio file to play for parked calls.
   UtlString mAudio;

   // The keycode for escaping from a parking orbit.
   // RFC 2833 code (as returned by enableDtmfEvent), or NO_KEYCODE
   // for no keycode.
   int mKeycode;

   // The maximum number of calls to handle in the orbit, or UNLIMITED_CAPACITY
   // if there is to be no limit.
   int mCapacity;

   virtual UtlContainableType getContainableType() const
      {
         return OrbitData::TYPE;
      };

   /** Class type used for runtime checking */
   static const UtlContainableType TYPE;

   // Negative value used to indicate no mTimeout value is present.
   static const int NO_TIMEOUT;

   // Negative value used to indicate no mKeycode value is present.
   static const int NO_KEYCODE;

   // Very large positive value used to indicate there is no limit to
   // the number of calls to be handled by the orbit.
   static const int UNLIMITED_CAPACITY;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _OrbitFileReader_h_
