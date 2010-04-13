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
#include <utl/UtlString.h>
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

// Read and parse the resource list file and install the resource lists
// into the ResourceListSet.
OsStatus ResourceListFileReader::initialize()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListFileReader::initialize entered");

   int changeDelay = mResourceListSet->getResourceListServer()->getChangeDelay();

   // The status to return.
   OsStatus ret = OS_SUCCESS;

   // Suspend publishing to prevent generating incomplete NOTIFYs.
   mResourceListSet->suspendPublishing();

   // Remove all existing resource lists.
   mResourceListSet->deleteAllResourceLists();

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

               // If the <list> element was OK, create the ResourceList from it.
               if (list_valid)
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
                           UtlString display_name;
                           for (TiXmlNode* name_node = 0;
                                (name_node = resource_element->IterateChildren("name",
                                                                               name_node));
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

                           bool r =
                              mResourceListSet->addResource(user_attribute,
                                                            uri_attribute,
                                                            names.data(),
                                                            display_name.data());
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

   // Resume publishing.
   mResourceListSet->resumePublishing();

   return ret;
}
