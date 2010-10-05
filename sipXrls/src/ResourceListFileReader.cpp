//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "ResourceListFileReader.h"
#include "ResourceListSet.h"
#include "main.h"
#include <os/OsSysLog.h>
#include <xmlparser/tinystr.h>
#include <xmlparser/ExtractContent.h>
#include <xmlparser/TiXmlUtlStringWriter.h>
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlString.h>
#include <utl/XmlContent.h>
#include <net/NameValueTokenizer.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


//: Object that encapsulates the work of comparing a sequence of
//  <resource> elements to a ResourceList, and updating the
//  ResourceList to match if they differ.

// To use:
// Create a ResourceListUpdater object for the 'user' value of the
// <list> element.
// Parse each <resource> child of the <list>, passing the resulting
// data to ::compare().
// After the last <resource> is processed, call ::finalize(), then
// destroy the object.

// If the <resource> children match the ResourceList, no changes
// will be made.
// If there are differences, the ResourceList is updated to match
// the <resource> children.  ResourceReference's will be added before
// they are deleted (maximizing the chance that existing subscriptions
// will be reused).

// The implementation is as follows:
// As <resource> elements are parsed, they are compared with
// successive ResourceReference objects.
// If each pair compares equal, no action is taken.
// If a pair compares not-equal, the flag mChanges is set, and all further parsed
// <resource> elements (starting with that one) are added to the end
// of the ResourceList.  When ::finalize() is called, the superseded
// ResourceReference objects (from the one found non-equal to the end
// of the original list) are deleted.

// No ResourceListUpdater methods hold the lock themselves, but during
// the lifetime of the object, all changes to the ResourceList should be
// done only via the object.

class ResourceListUpdater : UtlContainableAtomic
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /// Constructor
   ResourceListUpdater(/// The ResourceListSet to modify
                       ResourceListSet* resourceListSet,
                       /// user name of the resource list to update
                       const UtlString& user);

   /// Destructor
   //  Usually, ::finalize() should be called before destroying.
   ~ResourceListUpdater();

/* ============================ MANIPULATORS ============================== */

   /// Compare the next ResourceReference in the ResourceList with the data
   /// from a parsed <resource> element and do the necessary processing.
   //  (See algorithm description above.)
   //  May cause delay.
   void compare(/// The resource URI.
                const char* uri,
                /// The XML for the name of the resource.
                const char* nameXml,
                /// The display name for consolidated event notices
                const char* display_name);

   /// Perform the work to be done after ::compare() has been called
   /// for the last <resource> element.
   //  (See algorithm description above.)
   //  May cause delay.
   //  Aborts processing if shutting down.
   void finalize();

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

   /**
    * Get the ContainableType for a UtlContainable-derived class.
    */
   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    /// The ResourceListSet to modify.
    ResourceListSet* mResourceListSet;

    /// user value for the ResourceList.
    UtlString mUser;

    /// true if differences have been seen and changes are being made
    bool mChanges;

    /// 0-base index of the first ResourceReference that was different
    /// from the corresponding <resource> data
    size_t mFirstDifference;

    /// number of entries in the ResourceList before updating
    size_t mInitialEntries;

    /// number of entries in the ResourceList that have been compared so far
    size_t mComparedEntries;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    ResourceListUpdater(const ResourceListUpdater& rResourceListUpdater);
    //:Copy constructor
    // Not implemented.

    ResourceListUpdater& operator=(const ResourceListUpdater& rResourceListUpdater);
    //:Assignment operator
    // Not implemented.
};

/// Constructor.
ResourceListUpdater::ResourceListUpdater(ResourceListSet* resourceListSet,
                                         const UtlString& user) :
   mResourceListSet(resourceListSet),
   mUser(user),
   mChanges(false),
   mFirstDifference(-1),         // not used
   mInitialEntries(mResourceListSet->getResourceListEntries(user.data())),
   mComparedEntries(0)
{
}

/// Destructor.
ResourceListUpdater::~ResourceListUpdater()
{
}

/// Compare the next ResourceReference in the ResourceList with the data
/// from a parsed <resource> element and do the necessary processing.
void ResourceListUpdater::compare(const char* uri,
                                  const char* nameXml,
                                  const char* display_name)
{
   // If differences have not yet been seen, compare the parsed data
   // with the ResourceList data.
   if (!mChanges)
   {
      UtlString current_uri;
      UtlString current_nameXml;
      UtlString current_display_name;

      mResourceListSet->getResourceInfoAt(mUser.data(),
                                          mComparedEntries,
                                          current_uri,
                                          current_nameXml,
                                          current_display_name);
      // If the current location (mComparedEntries) is beyond the
      // end of the ResourceList, the content_* values are null.

      if (!(current_uri.compareTo(uri) == 0 &&
            current_nameXml.compareTo(nameXml) == 0 &&
            current_display_name.compareTo(display_name) == 0))
      {
         // If a difference is seen, set the flags to start adding
         // new ResourceReference's.
         mChanges = true;
         mFirstDifference = mComparedEntries;
         OsSysLog::add(FAC_RLS, PRI_DEBUG,
                       "ResourceListUpdater::compare "
                       "Difference found at location %d",
                       (int) mFirstDifference);
      }
   }

   // Increment the count of compared entries.
   mComparedEntries++;

   // If differences have not yet been seen, do nothing.
   // Otherwise, add a new resource at the end of the ResourceList.
   if (mChanges)
   {
      bool r =
         mResourceListSet->addResource(mUser.data(),
                                       uri,
                                       nameXml,
                                       display_name,
                                       mFirstDifference,
                                       mInitialEntries - 1);
      if (!r)
      {
         OsSysLog::add(FAC_RLS, PRI_WARNING,
                       "ResourceListUpdater::compare "
                       "Could not add resource '%s' to ResourceList '%s' -- "
                       "probable duplicate resource",
                       uri, mUser.data());
      }
   }
}

/// Perform the work to be done after ::compare() has been called
/// for the last <resource> element.
void ResourceListUpdater::finalize()
{
   // If we didn't compare as many entries as were there initially,
   // all entries in the ResourceList after the first mComparedEntries
   // have been deleted from the XML.
   if (!mChanges && (mComparedEntries < mInitialEntries))
   {
      mFirstDifference = mComparedEntries;
      mChanges = true;
   }

   // If differences have not been seen, do nothing.  Otherwise,
   // finish up by deleting the ResourceReference elements that have
   // been replaced by new ones.
   if (mChanges)
   {
      for (size_t i = mFirstDifference;
           i < mInitialEntries && !gShutdownFlag;
           i++)
      {
         // Note: because we're deleting, the element we want to remove
         // remains at position "mFirstDifference" on each iteration.
         mResourceListSet->deleteResourceAt(mUser.data(),
                                            mFirstDifference);
      }
   }

   // Clear mChanges so that if ::finalize() is called twice, nothing
   // bad happens.
   mChanges = false;
}

/**
 * Get the ContainableType for a UtlContainable-derived class.
 */
UtlContainableType ResourceListUpdater::getContainableType() const
{
   return ResourceListUpdater::TYPE;
}

const UtlContainableType ResourceListUpdater::TYPE = "ResourceListUpdater";


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ResourceListFileReader::ResourceListFileReader(const UtlString& resourceListFile,
                                               ResourceListSet* resourceListSet) :
   RefreshingFileReader(),
   mResourceListSet(resourceListSet)
{
   setFileName(&resourceListFile);
}

// Destructor
ResourceListFileReader::~ResourceListFileReader()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Read and parse the resource list file and install the resource lists
// into the ResourceListSet.
OsStatus ResourceListFileReader::initialize()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListFileReader::initialize entered");

   // The status to return.
   OsStatus ret = OS_SUCCESS;

   // Suspend publishing to prevent generating incomplete NOTIFYs.
   mResourceListSet->suspendPublishing();

   // No work to be done if file name is not set.
   if (!mFileName.isNull())
   {
      // Initialize Tiny XML document object.
      TiXmlDocument document;
      TiXmlNode* lists_node;
      if (
         // Load the XML into it.
         document.LoadFile(mFileName.data()) &&
         // Find the top element, which should be a <lists>.
         (lists_node = document.FirstChild("lists")) != NULL &&
         lists_node->Type() == TiXmlNode::ELEMENT)
      {
         UtlSList xmlList;
         UtlSList rlsList;
         UtlSList similarList;

         // Get the list of the 'user' attributes of child <list> elements.
         xmlElemList(xmlList,
                     lists_node,
                     "list",
                     "user");

         // Get the 'user' values of all the existing resource lists.
         mResourceListSet->getAllResourceLists(rlsList);

         OsSysLog::add(FAC_RLS, PRI_DEBUG,
                       "ResourceListFileReader::initialize "
                       "ResourceList count -- XML: %lu RLS: %lu",
                       (unsigned long) xmlList.entries(),
                       (unsigned long) rlsList.entries());

         // Determine what resource lists have been added and deleted.
         listDiff(xmlList,
                  rlsList,
                  similarList);
         // xmlList is now the list of resource list ('user' values) that have
         // been added, and rlsList is the list of those that have been deleted.

         // Find all the <list> elements.
         // Since this loop contains a delay and can run for a long time,
         // we have to check gShutdownFlag to abort processing when
         // a shutdown has been requested.
         for (TiXmlNode* list_node = 0;
              (list_node = lists_node->IterateChildren("list",
                                                       list_node)) &&
              !gShutdownFlag;
            )
         {
            if (list_node->Type() == TiXmlNode::ELEMENT)
            {
               TiXmlElement* list_element = list_node->ToElement();

               // Process each <list> element.
               bool list_valid = true;

               // Process the 'user' attribute.
               const char* user_attribute = list_element->Attribute("user");
               if (!user_attribute || *user_attribute == '\0')
               {
                  // User missing or null.
                  OsSysLog::add(FAC_RLS, PRI_ERR,
                                "user attribute of <list> was missing or null");
                  list_valid = false;
                  ret = OS_FAILED;
               }

               // Process the 'user-cons' attribute.
               const char* user_cons_attribute =
                  list_element->Attribute("user-cons");
               if (!user_cons_attribute || *user_cons_attribute == '\0')
               {
                  // user-cons missing or null.
                  OsSysLog::add(FAC_RLS, PRI_ERR,
                                "user-cons attribute of <list> was missing or null");
                  list_valid = false;
                  ret = OS_FAILED;
               }

               // If the <list> appears to be valid, we update the RLS
               // data structures to match it.
               if (list_valid)
               {
                  UtlString user_attribute_string(user_attribute);

                  // If the 'user' value is in xmlList, it is new and needs
                  // to have a ResourceList created for it.
                  if (xmlList.find(&user_attribute_string))
                  {
                     // Add this resource list to the set of all resource lists.
                     // (No NAME XML for the resource list.)
                     bool r =
                        mResourceListSet->addResourceList(user_attribute,
                                                          user_cons_attribute,
                                                          "");
                     if (!r)
                     {
                        OsSysLog::add(FAC_RLS, PRI_WARNING,
                                      "ResourceListFileReader::initialize "
                                      "ResourceList '%s' already exists -- "
                                      "continuing adding resources to the list",
                                      user_attribute);
                     }
                  }

                  // Initialize the ResourceListUpdater object.
                  // As we parse the <resource> children, it will
                  // compare the parsed data with the current contents of the
                  // ResourceList object and take appropriate action
                  // (maintaining the necessary state information).
                  // For the details of that process, see the declaration of
                  // the ResourceListUpdater class.
                  ResourceListUpdater updater(mResourceListSet,
                                              user_attribute_string);

                  // Find all the <resource> children, parse them, and compare them
                  // to the current resource list.
                  // Since this loop contains a delay and can run for a long time,
                  // we have to check gShutdownFlag to abort processing when
                  // a shutdown has been requested.
                  for (TiXmlNode* resource_node = 0;
                       (resource_node = list_element->IterateChildren("resource",
                                                                      resource_node)) &&
                       !gShutdownFlag;
                     )
                  {
                     if (resource_node->Type() == TiXmlNode::ELEMENT)
                     {
                        TiXmlElement* resource_element = resource_node->ToElement();

                        // Process each <resource> element.
                        bool resource_valid = true;

                        // Process the 'uri' attribute.
                        const char* uri_attribute = resource_element->Attribute("uri");
                        if (!uri_attribute || *uri_attribute == '\0')
                        {
                           // URI missing or null.
                           OsSysLog::add(FAC_RLS, PRI_ERR,
                                         "main "
                                         "uri attribute of <resource> was missing or null");
                           resource_valid = false;
                           ret = OS_FAILED;
                        }

                        // If the <resource> element was OK, create the
                        // ResourceListResource from it.
                        if (resource_valid)
                        {
                           // Find all the <name> children and add them to the
                           // names string (as XML).
                           UtlString names;
                           TiXmlUtlStringWriter writer(&names);
                           // Extract the content of the first
                           // non-empty <name> child, and use it as
                           // the display name.
                           UtlString display_name;
                           getDisplayName(resource_element, writer, display_name);

                           // At this point, we have parsed a valid <resource> element.
                           // Hand it to 'updater' to determine what procesing needs to be done.

                           updater.compare(uri_attribute,
                                           names.data(),
                                           display_name.data());
                        }
                     }
                  }

                  // Do any final work for updating the ResourceList.
                  updater.finalize();
               }
            }
         }

         // Delete the resource lists that must be removed after adding
         // the ones that must be added.  We add first because there might be
         // resource URIs in common between the lists that are added
         // and deleted, and doing the creations first allows the reuse of
         // existing subscriptions via the ResourceCache mechanism.

         UtlSListIterator itor(rlsList);
         UtlString* userPart;
         // From the resource lists mentioned in rlsList, because they are not
         // in the new XML.
         while ((userPart = dynamic_cast <UtlString*> (itor())))
         {
            mResourceListSet->deleteResourceList(userPart->data());
         }

         OsSysLog::add(FAC_RLS, PRI_DEBUG,
                       "ResourceListFileReader::initialize Done loading file '%s'",
                       mFileName.data());

         // Free the lists.
         xmlList.destroyAll();
         rlsList.destroyAll();
         similarList.destroyAll();
      }
      else
      {
         // Report error parsing file.
         OsSysLog::add(FAC_PARK, PRI_CRIT,
                       "main "
                       "Resource list file '%s' could not be parsed.",
                       mFileName.data());
         ret = OS_FAILED;
      }
   }
   else
   {
      // Report that there is no file.
      OsSysLog::add(FAC_PARK, PRI_CRIT,
                    "main No resource list file set.");

      // Leave the current resource list configuration unchanged.
   }

   // Resume publishing.
   mResourceListSet->resumePublishing();

   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListFileReader::initialize exited");

   return ret;
}

//! Examine the children of list_node and compose a UtlSList of
//  UtlString's that contain the value of attribute
//  'attribute' of all child nodes of type 'element'.
void ResourceListFileReader::xmlElemList(UtlSList& list,
                                         TiXmlNode* list_node,
                                         const char* element,
                                         const char* attribute)
{
   // Find all the <element> children.
   for (TiXmlNode* child_node = 0;
        (child_node = list_node->IterateChildren(element,
                                                 child_node)) &&
         !gShutdownFlag;
      )
   {
      if (child_node->Type() == TiXmlNode::ELEMENT)
      {
         TiXmlElement* child_element = child_node->ToElement();

         // If the attribute 'attribute' is found in the XML, append its value
         // to the list.
         const char* attribute_value = child_element->Attribute(attribute);
         if (attribute_value && *attribute_value != '\0')
         {
            // Check if a duplicate is present in the list; if not, add it.
            UtlString* s = new UtlString(attribute_value);
            if(!list.find(s))
            {
               list.append(s);

               OsSysLog::add(FAC_RLS, PRI_DEBUG,
                             "ResourceListFileReader::xmlElemList "
                             "<%s> - '%s' added to XML list",
                             element, attribute_value);
            }
            else
            {
               delete s;

               OsSysLog::add(FAC_RLS, PRI_WARNING,
                             "ResourceListFileReader::xmlElemList "
                             "Duplicate: <%s> - '%s' found in XML list",
                             element, attribute_value);
            }
         }
      }
   }
}

//! Compare two UtlSList's of UtlStrings to each other.
void ResourceListFileReader::listDiff(UtlSList& list1,
                                      UtlSList& list2,
                                      UtlSList& common)
{
   UtlSListIterator itor(list1);
   UtlString* list1_element;

   // Iterate through list1.
   while ((list1_element = dynamic_cast <UtlString*> (itor())))
   {
      // Find the node in list2.
      UtlContainable* list2_element = list2.find(list1_element);

      if (list2_element)
      {
         // Remove the equal UtlStrings from both lists.
         list1.remove(list1_element);
         list2.remove(list2_element);
         // Append to the list of similar nodes
         common.append(list1_element);
         // Delete the unneeded element.
         delete list2_element;

         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                       "ResourceListFileReader::listCompare "
                       "'%s' has been found in list2",
                       list1_element->data());
      }
      else
      {
         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                       "ResourceListFileReader::listCompare "
                       "'%s' has not been found in list2",
                       list1_element->data());
      }
   }
}

/// Gets the display name out of the xml
void ResourceListFileReader::getDisplayName(TiXmlElement* resource_element,
                                            TiXmlUtlStringWriter& writer,
                                            UtlString& display_name)
{
   display_name.remove(0);

   for (TiXmlNode* name_node = 0;
        (name_node = resource_element->IterateChildren("name",
                                                       name_node)) &&
         !gShutdownFlag;
      )
   {
      if (name_node->Type() == TiXmlNode::ELEMENT)
      {
         writer << *name_node;
         if (display_name.isNull())
         {
            textContentShallow(display_name,
                               name_node);
         }
      }
   }
}
