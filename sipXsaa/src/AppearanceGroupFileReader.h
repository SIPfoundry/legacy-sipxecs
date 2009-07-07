//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

#ifndef _AppearanceGroupFileReader_h_
#define _AppearanceGroupFileReader_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "filereader/RefreshingFileReader.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class AppearanceGroupSet;


//: Class construct an object that reads the appearance-groups.xml file that describes
//: the list of shared numbers.
class AppearanceGroupFileReader : public RefreshingFileReader
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   AppearanceGroupFileReader(/// file name to read from or ""
                          const UtlString& appearanceGroupFile,
                          AppearanceGroupSet* appearanceGroupSet
         );

   //:Default constructor
   ~AppearanceGroupFileReader();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    AppearanceGroupFileReader(const AppearanceGroupFileReader& rAppearanceGroupFileReader);
    //:Copy constructor

    AppearanceGroupFileReader& operator=(const AppearanceGroupFileReader& rAppearanceGroupFileReader);
    //:Assignment operator

    //! The AppearanceGroupSet to load from the file.
    AppearanceGroupSet* mAppearanceGroupSet;

    //! (Re-)read the file and (re)initialize the object.
    //  If there is no file name, "initialize()" will be called with
    //  mFileName = "".
    virtual OsStatus initialize();

};

#endif  // _AppearanceGroupFileReader_h_
