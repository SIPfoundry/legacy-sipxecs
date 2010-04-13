//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "os/OsSysLog.h"
#include "utl/UtlString.h"
#include "xmlparser/tinystr.h"
#include "AppearanceGroupFileReader.h"
#include "AppearanceGroupSet.h"
#include "main.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
AppearanceGroupFileReader::AppearanceGroupFileReader(const UtlString& appearanceGroupFile,
                                               AppearanceGroupSet* appearanceGroupSet
                                               ) :
   RefreshingFileReader(),
   mAppearanceGroupSet(appearanceGroupSet)
{
   setFileName(&appearanceGroupFile);
}

// Destructor
AppearanceGroupFileReader::~AppearanceGroupFileReader()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Read and parse the appearance group file and install the appearance groups
// into the AppearanceGroupSet.
OsStatus AppearanceGroupFileReader::initialize()
{
   OsSysLog::add(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroupFileReader::initialize entered");

   int changeDelay = mAppearanceGroupSet->getAppearanceAgent()->getChangeDelay();

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
         // Find the top element, which should be <appearanceGroups>.
         (lists_node = document.FirstChild("appearanceGroups")) != NULL &&
         lists_node->Type() == TiXmlNode::ELEMENT)
      {
         UtlSList newGroupList;

         // Find all the <resource> children and add them to the AppearanceGroup.
         for (TiXmlNode* resource_node = 0; (resource_node = lists_node->IterateChildren(
               "resource", resource_node));)
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
                  OsSysLog::add(FAC_SAA, PRI_ERR, "AppearanceGroupFileReader::initialize "
                     "uri attribute of <resource> was missing or null");
                  resource_valid = false;
                  ret = OS_FAILED;
               }

               if (resource_valid)
               {
                  newGroupList.append(new UtlString(uri_attribute));
               }
            }
         }

         OsSysLog::add(FAC_SAA, PRI_DEBUG,
                       "AppearanceGroupFileReader::initialize Done loading file '%s'",
                       mFileName.data());
         // For all groups in the current GroupSet, remove them if not in the new list.
         // Since these loops contain a delay and can run for a long time,
         // we have to check gShutdownFlag to abort processing when
         // a shutdown has been requested.
         UtlSList oldGroupList;
         mAppearanceGroupSet->getAllAppearanceGroups(oldGroupList);
         UtlSListIterator oldGroupItor(oldGroupList);
         UtlString* group;
         while (!gShutdownFlag && (group = dynamic_cast <UtlString*> (oldGroupItor())))
         {
            UtlSListIterator newGroupItor(newGroupList);
            UtlString* newGroup;
            bool found = false;
            while (!found && (newGroup = dynamic_cast <UtlString*> (newGroupItor())))
            {
               if (newGroup->compareTo(*group) == 0)
               {
                  found = true;
               }
            }
            if (!found)
            {
               mAppearanceGroupSet->removeAppearanceGroup(group->data());
               OsTask::delay(changeDelay);
            }
         }
         // For all groups in the new list, add to GroupSet if they are not there.
         UtlSListIterator groupItor(newGroupList);
         while (!gShutdownFlag && (group = dynamic_cast <UtlString*> (groupItor())))
         {
            if (!mAppearanceGroupSet->findAppearanceGroup(group->data()))
            {
               mAppearanceGroupSet->addAppearanceGroup(group->data());
               OsTask::delay(changeDelay);
            }
         }
         newGroupList.destroyAll();
      }
      else
      {
         // Report error parsing file.
         OsSysLog::add(FAC_SAA, PRI_CRIT,
                       "AppearanceGroupFileReader::initialize "
                       "Appearance group file '%s' could not be parsed.",
                       mFileName.data());
         ret = OS_FAILED;
      }
   }
   else
   {
      // Report that there is no file.
      OsSysLog::add(FAC_SAA, PRI_WARNING,
                    "AppearanceGroupFileReader::initialize No Appearance group file set.");
   }

   return ret;
}
