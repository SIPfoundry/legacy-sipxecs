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
#include <utl/UtlSList.h>
#include <xmlparser/tinyxml.h>
#include <xmlparser/TiXmlUtlStringWriter.h>

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

    //! Makes a list of data based on the unique_name
    //  found in the XML file and returns the size of
    //  the list.
    void xmlUtlSList(/// The list to add the unique XML attributes to.
                     UtlSList& list,
                     /// The node to parse through.
                     TiXmlNode* lists_node,
                     /// The type of node to looks at.
                     const char* node_value,
                     /// The unique attribute for comparing.
                     const char* unique_name);

    //! Compare two single linked list to each other
    //  and see what objects in the lists they share
    //  and see what objects they do not share.
    void listCompare(//! The xml list to read from and then store
                     //  the differences found in the xml list.
                     UtlSList& xmlList,
                     //! The rls list to read from and then store
                     //  the differences found in the rls list.
                     UtlSList& rlsList,
                     //! A list to store what objects the xml
                     //  and rls list shared.
                     UtlSList& similarList);

    /// Gets the display name out of the xml
    UtlString getDisplayName(/// The node to read the display name from
                             TiXmlElement* resource_element,
                             /// The XML
                             TiXmlUtlStringWriter writer);
};

#endif  // _ResourceListFileReader_h_
