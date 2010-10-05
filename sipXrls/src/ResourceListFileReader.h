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


//: Object that reads the resource-list.xml file which describes
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

    //! Examine the children of list_node and compose a UtlSList of
    //  UtlString's that contain the value of attribute
    //  'attribute' of all child nodes of type 'element'.
    //  Children should have unique  values of 'attribute'; duplicates are reported
    //  as errors and not included in the list.
    //  Abort execution if mService->getShutdownFlag() becomes true.
    void xmlElemList(/// The list to add the strings to.
                     UtlSList& list,
                     /// The node whose children to examine.
                     TiXmlNode* list_node,
                     /// The type of child node to look for.
                     const char* element,
                     /// The attribute to examine.
                     const char* attribute);

    //! Compare two UtlSList's of UtlStrings to each other.
    //  Separate the elements that appear in only one list from
    //  those that appear in both lists.
    void listDiff(//! Input list 1; result is strings only in list 1.
                  UtlSList& list1,
                  //! Input list 2; result is strings only in list 2.
                  UtlSList& list2,
                  //! Input empty list; result is strings in common.
                  UtlSList& common);

    /// Gets the display name information out of a <resource> element.
    void getDisplayName(/// the node to read
                        TiXmlElement* resource_element,
                        /// writer to which to write the <name> children
                        TiXmlUtlStringWriter& writer,
                        /// the string to load with the text of the first
                        /// <name> child
                        UtlString& display_name);
};

#endif  // _ResourceListFileReader_h_
