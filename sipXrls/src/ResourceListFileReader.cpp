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
#include <utl/UtlSListIterator.h>
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <utl/XmlContent.h>
#include <net/NameValueTokenizer.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


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

//! Read and parse the resource list file and compares the difference of the
//  resource list's id(user-part) in the resource list set.  It will add or
//  remove a resource list depending on the difference.  Then for the
//  resource list's id(user-part) that matched between the resource list file
//  and resource list set, it then checks the resource reference's id(uri-part)
//  in the resource list for differences between the resource list file and
//  resource list set.  It will add or remove a resource reference depending on
//  the difference.  Then for the resource reference's id(uri-part) that
//  matched between the resource list file and resource list set, it will
//  find differences between the order in the list, name, and name xml data
//  of the resource reference which will delete the old resource reference and
//  add a new one to the list in order to update it.
OsStatus ResourceListFileReader::initialize()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListFileReader::initialize entered");

   int changeDelay = mResourceListSet->getResourceListServer()->getChangeDelay();

   // The status to return.
   OsStatus ret = OS_SUCCESS;

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

          // Loads XML data
          xmlUtlSList(xmlList,
                      lists_node,
                      "list",
                      "user");

          // Load RLS data
          mResourceListSet->getAllResourceLists(rlsList);

          OsSysLog::add(FAC_RLS, PRI_DEBUG,
                        "ResourceListFileReader::initialize "
                        "ResourceList count -- XML: %lu RLS: %lu",
                        (unsigned long)xmlList.entries(), (unsigned long)rlsList.entries());

          // Gets the similarities and differences
          listCompare(xmlList,
                      rlsList,
                      similarList);

          UtlSListIterator resourceListItor(rlsList);
          UtlString* userPart;
          // From the list of differences found in the rlsList, delete
          // all the resources and the resource list.
          while ((userPart = dynamic_cast <UtlString*> (resourceListItor())))
          {
             mResourceListSet->deleteResourcesList(userPart->data());
          }

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

                // Note: If there are duplicates of the ResourceList, the duplicate will erase the previous
                // occurrence.
                // If the <list> element was OK and is a new ResourceList, create the ResourceList from it.
                if (list_valid &&
                      xmlList.find(new UtlString(user_attribute)))
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
                   OsTask::delay(changeDelay);

                   // Find all the <resource> children and add them to the
                   // ResourceList.
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

                         // Gets the uri of the previous node, if no previous node then it is null
                         const char* previous_uri_attribute =
                               resource_node->PreviousSibling("resource") ?
                                     resource_node->PreviousSibling("resource")->ToElement()->Attribute("uri") : NULL;


                         OsSysLog::add(FAC_RLS, PRI_DEBUG,
                                       "ResourceListFileReader::initialize "
                                       "Previous URI Attribute: '%s'",
                                       previous_uri_attribute);

                         // If the <resource> element was OK, create the
                         // ResourceListResource from it.
                         if (resource_valid)
                         {
                            // Find all the <name> children and add them to the
                            // name string.
                            UtlString names;
                            TiXmlUtlStringWriter writer(&names);
                            // Extract the content of the first
                            // non-empty <name> child, and use it as
                            // the display name.
                            UtlString display_name = getDisplayName(resource_element,
                                                                    writer);

                            bool r =
                               mResourceListSet->addResource(user_attribute,
                                                             uri_attribute,
                                                             names.data(),
                                                             display_name.data(),
                                                             previous_uri_attribute);
                            if (!r)
                            {
                               OsSysLog::add(FAC_RLS, PRI_WARNING,
                                             "ResourceListFileReader::initialize "
                                             "Resource '%s' already exists in ResourceList '%s' -- "
                                             "not adding a second time",
                                             uri_attribute, user_attribute);
                            }
                            OsTask::delay(changeDelay);
                         }
                      }
                   }
                }
                // If the <list> element was OK and the ResourceList already exist, check all the ReferenceResources.
                // in the ResourceList to leave them alone or refresh them.
                else if (list_valid &&
                      similarList.find(new UtlString(user_attribute)))
                {
                   UtlSList rrXmlList;
                   UtlSList rrRlsList;
                   UtlSList rrSimilarList;

                   // Loads data from XML for a user
                   xmlUtlSList(rrXmlList,
                               list_node,
                               "resource",
                               "uri");

                   // Loads data from RLS for a user
                   mResourceListSet->getResourceReferences(user_attribute,
                                                           rrRlsList);

                   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                                 "ResourceListFileReader::initialize "
                                 "ResourceReferences count for user %s -- XML: %d RLS: %d",
                                 user_attribute, (int)rrXmlList.entries(), (int)rrRlsList.entries());

                   // Gets the similarities and differences
                   listCompare(rrXmlList,
                               rrRlsList,
                               rrSimilarList);

                   UtlSListIterator resourceListItor(rrRlsList);
                   UtlString* uriPart;
                   // From the list of differences found in the rlsList, delete
                   // all the resources.
                   while ((uriPart = dynamic_cast <UtlString*> (resourceListItor())))
                   {
                      mResourceListSet->deleteResource(user_attribute,
                                                       uriPart->data());
                   }

                   // Find all the <resource> children and find any differences
                   // in the ResourceList.
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

                         // Gets the uri of the previous node, if no previous node then it is null
                         const char* previous_uri_attribute =
                               resource_node->PreviousSibling("resource") ?
                                     resource_node->PreviousSibling("resource")->ToElement()->Attribute("uri") : NULL;


                         OsSysLog::add(FAC_RLS, PRI_DEBUG,
                                       "ResourceListFileReader::initialize "
                                       "Previous URI Attribute: '%s'",
                                       previous_uri_attribute);

                         // If the <resource> element was OK, check for
                         // differences.
                         if (resource_valid)
                         {
                            // Find all the <name> children and add them to the
                            // name string.
                            UtlString names;
                            TiXmlUtlStringWriter writer(&names);
                            // Extract the content of the first
                            // non-empty <name> child, and use it as
                            // the display name.

                            UtlString display_name = getDisplayName(resource_element,
                                                                    writer);

                            //  Add new resources to RLS.
                            if(rrXmlList.find(new UtlString(uri_attribute)))
                            {
                               // Removing Resources that has been processed in order to prevent duplicates.
                               rrXmlList.remove(new UtlString(uri_attribute));

                               bool r =
                                  mResourceListSet->addResource(user_attribute,
                                                                uri_attribute,
                                                                names.data(),
                                                                display_name.data(),
                                                                previous_uri_attribute);
                               if (!r)
                               {
                                  OsSysLog::add(FAC_RLS, PRI_WARNING,
                                                "ResourceListFileReader::initialize "
                                                "Resource '%s' already exists in ResourceList '%s' -- "
                                                "not adding a second time",
                                                uri_attribute, user_attribute);
                               }
                               else
                               {
                                  OsSysLog::add(FAC_RLS, PRI_DEBUG,
                                                "ResourceListFileReader::initialize "
                                                "Resource '%s' - '%s' has been added to ResourceList '%s'",
                                                uri_attribute, names.data(), user_attribute);
                               }
                            }
                            // Update info in RLS to match the XML if needed.
                            else if(rrSimilarList.find(new UtlString(uri_attribute)))
                            {
                               // Removing Resources that has been processed in order to prevent duplicates.
                               rrSimilarList.remove(new UtlString(uri_attribute));

                               bool is_changed = mResourceListSet->resourceChanged(user_attribute,
                                                                                   uri_attribute,
                                                                                   names.data(),
                                                                                   display_name.data(),
                                                                                   previous_uri_attribute);

                               // Update resource references that changed
                               if(is_changed)
                               {
                                  mResourceListSet->deleteResource(user_attribute,
                                                                   uri_attribute);

                                  mResourceListSet->addResource(user_attribute,
                                                                uri_attribute,
                                                                names.data(),
                                                                display_name.data(),
                                                                previous_uri_attribute);

                                  OsSysLog::add(FAC_RLS, PRI_DEBUG,
                                                "ResourceListFileReader::initialize "
                                                "Resource '%s' - '%s' has been changed",
                                                uri_attribute, names.data());
                               }
                               else
                               {
                                  OsSysLog::add(FAC_RLS, PRI_DEBUG,
                                                "ResourceListFileReader::initialize "
                                                "Resource '%s' - '%s' has not been changed",
                                                uri_attribute, names.data());
                               }
                            }
                            else
                            {
                               OsSysLog::add(FAC_RLS, PRI_WARNING,
                                             "ResourceListFileReader::initialize "
                                             "Resource '%s' already exists in ResourceList '%s' -- "
                                             "not adding/updating a second time",
                                             uri_attribute, user_attribute);
                            }

                            OsTask::delay(changeDelay);
                         }
                      }
                   }
                }
             }
          }
          OsSysLog::add(FAC_PARK, PRI_DEBUG,
                        "ResourceListFileReader::initialize Done loading file '%s'",
                        mFileName.data());
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
    }

    return ret;
}

//! Makes a list of data based on the unique_name
//  found in the XML file and returns the size of
//  the list.
void ResourceListFileReader::xmlUtlSList(UtlSList& list,
                                         TiXmlNode* lists_node,
                                         const char* node_value,
                                         const char* unique_name)
{
   // Find all the <node_value> elements.
   for (TiXmlNode* list_node = 0;
        (list_node = lists_node->IterateChildren(node_value,
                                                 list_node)) &&
         !gShutdownFlag;
      )
   {
      if (list_node->Type() == TiXmlNode::ELEMENT)
      {
         TiXmlElement* list_element = list_node->ToElement();

         // If the unique_name attribute is found in the XML append to the list
         const char* unique_name_attribute = list_element->Attribute(unique_name);
         if (unique_name_attribute && *unique_name_attribute != '\0')
         {
            // Checks if a duplicate is found in the XML
            if(!list.find(new UtlString(unique_name_attribute)))
            {
               list.append(new UtlString(unique_name_attribute));

               OsSysLog::add(FAC_RLS, PRI_DEBUG,
                             "ResourceListFileReader::xmlUtlSList "
                             "<%s> - '%s' added to XML list",
                             node_value, unique_name_attribute);
            }
            else
            {
               OsSysLog::add(FAC_RLS, PRI_WARNING,
                             "ResourceListFileReader::xmlUtlSList "
                             "Duplicate: <%s> - '%s' found in XML list",
                             node_value, unique_name_attribute);
            }
         }
      }
   }
}

//! Compare two single linked list to each other
//  and see what objects in the lists they share
//  and see what objects they do not share.
void ResourceListFileReader::listCompare(UtlSList& xmlList,
                                         UtlSList& rlsList,
                                         UtlSList& similarList)
{
   UtlSListIterator resourceListItor(xmlList);
   UtlString* userPart;

   // Iterate through the xml list.
   while ((userPart = dynamic_cast <UtlString*> (resourceListItor())))
   {
      // Finds the node in the RLS.
      bool found = rlsList.find(userPart);

      if (found)
      {
         // Removes the similars from both list so the difference is only in the list.
         xmlList.remove(userPart);
         rlsList.remove(userPart);
         // Appends to similar nodes
         similarList.append(userPart);

         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                       "ResourceListFileReader::listCompare "
                       "%s has been found in the RLS",
                       userPart->data());
      }
      else{
         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                       "ResourceListFileReader::listCompare "
                       "%s has not been found in the RLS",
                       userPart->data());
      }
   }
}

/// Gets the display name out of the xml
UtlString ResourceListFileReader::getDisplayName(TiXmlElement* resource_element,
                                                 TiXmlUtlStringWriter writer)
{
   UtlString display_name;

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

   return display_name;
}
