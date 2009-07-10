//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

#ifndef _ResourceListFileReader_h_
#define _ResourceListFileReader_h_

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

class ResourceListSet;


//: Class construct an object that reads the resource-list.xml file that describes
//: the resource lists.
class ResourceListFileReader : public RefreshingFileReader
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   ResourceListFileReader(/// file name to read from or ""
                          const UtlString& resourceListFile,
                          /// ResourceListSet to load
                          ResourceListSet* resourceListSet);

   //:Default constructor
   ~ResourceListFileReader();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    ResourceListFileReader(const ResourceListFileReader& rResourceListFileReader);
    //:Copy constructor

    ResourceListFileReader& operator=(const ResourceListFileReader& rResourceListFileReader);
    //:Assignment operator

    //! The ResourceListSet to load from the file.
    ResourceListSet* mResourceListSet;

    //! (Re-)read the file and (re)initialize the object.
    //  If there is no file name, "initialize()" will be called with
    //  mFileName = "".
    virtual OsStatus initialize();

};

#endif  // _ResourceListFileReader_h_
