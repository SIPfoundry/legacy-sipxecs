//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#include <stdlib.h>

// APPLICATION INCLUDES

#include "os/OsSysLog.h"
#include <os/OsFS.h>
#include <os/OsDateTime.h>
#include "xmlparser/tinyxml.h"
#include "xmlparser/ExtractContent.h"
#include <utl/UtlHashMapIterator.h>
#include "filereader/OrbitFileReader.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

const UtlContainableType OrbitData::TYPE = "OrbitData";

const int OrbitData::NO_TIMEOUT = -1;
const int OrbitData::NO_KEYCODE = -1;
const int OrbitData::UNLIMITED_CAPACITY = 1000000;

// String containing the characters used in the orbits.xml file to represent
// the RFC 2833 keycodes in order by their RFC 2833 values.
const static UtlString valid_keycodes = "0123456789*#";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OrbitFileReader::OrbitFileReader()
{
}

// Destructor
OrbitFileReader::~OrbitFileReader()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

// Return pointer to the OrbitData structure if the argument is an
// orbit name listed in the orbits.xml file.
OrbitData* OrbitFileReader::findInOrbitList(const UtlString& user)
{
   OrbitData* ret;

   // Refresh mOrbitList if necessary.
   refresh();

   // Check to see if 'user' is in it.  If so, return a pointer to its
   // data.
   ret = dynamic_cast <OrbitData*> (mOrbitList.findValue(&user));

   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "OrbitFileReader::findInOrbitList "
                 "user = '%s', ret = %p",
                 user.data(), ret);
   return ret;
}

// Retrieve the "music on hold" file name.
void OrbitFileReader::getMusicOnHoldFile(UtlString& file)
{
   // Refresh mMusicOnHoldFile if necessary.
   refresh();

   // Get the value.
   file = mMusicOnHoldFile;

   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "OrbitFileReader::getMusicOnHoldFile "
                 "file = '%s'",
                 file.data());
   return;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Read and parse the orbits.xml file into the data structures.
OsStatus OrbitFileReader::initialize()
{
   // Clear the list of the previous orbit names.
   mOrbitList.destroyAll();
   // Forget the music-on-hold file.
   mMusicOnHoldFile.remove(0);

   // Initialize Tiny XML document object.
   TiXmlDocument document;
   TiXmlNode* orbits_element;
   if (
      // Load the XML into it.
      document.LoadFile(mFileName.data()) &&
      // Find the top element, which should be an <orbits>.
      (orbits_element = document.FirstChild("orbits")) != NULL &&
      orbits_element->Type() == TiXmlNode::ELEMENT)
   {
      // Find all the <orbit> elements.
      for (TiXmlNode* orbit_element = 0;
           (orbit_element = orbits_element->IterateChildren("orbit",
                                                            orbit_element));
         )
      {
         // Process each <orbit> element.
         bool orbit_valid = true;

         // Process the <extension> element.
         TiXmlNode* extension_element =
            orbit_element->FirstChild("extension");
         UtlString extension;
         if (extension_element)
         {
            textContentShallow(extension, extension_element->ToElement());
            if (extension.isNull())
            {
               // Extension had zero length
               OsSysLog::add(FAC_PARK, PRI_ERR,
                             "OrbitFileReader::parseOrbitFile "
                             "<extension> was null.");
               orbit_valid = false;
            }
         }
         else
         {
            // Extension was missing.
            OsSysLog::add(FAC_PARK, PRI_ERR,
                          "OrbitFileReader::parseOrbitFile "
                          "<extension> was missing in an <orbit> element.");
            orbit_valid = false;
         }

         // Process the <background-audio> element.
         TiXmlNode* audio_element =
            orbit_element->FirstChild("background-audio");
         UtlString audio;
         if (audio_element)
         {
            textContentShallow(audio, audio_element->ToElement());
            if (audio.isNull())
            {
               // Extension had zero length
               OsSysLog::add(FAC_PARK, PRI_ERR,
                             "OrbitFileReader::parseOrbitFile "
                             "<background-audio> was null for extension '%s'",
                             extension.data());
               orbit_valid = false;
            }
         }
         else
         {
            // Background-audio was missing.
            OsSysLog::add(FAC_PARK, PRI_ERR,
                          "OrbitFileReader::parseOrbitFile "
                          "<background-audio> was missing for extension '%s'",
                          extension.data());
            orbit_valid = false;
         }

         // Process the <time-out> element to set mTimeout.
         int timeout = OrbitData::NO_TIMEOUT;	// Assume no value present.
         TiXmlNode* timeout_element =
            orbit_element->FirstChild("time-out");
         if (timeout_element)
         {
            UtlString temp;
            textContentShallow(temp, timeout_element->ToElement());
            char *endptr;
            timeout = strtol(temp.data(), &endptr, 10);
            if (temp.isNull() ||
                endptr - temp.data() != temp.length() ||
                timeout < 5)
            {
               // Timeout was null or unparsable.
               OsSysLog::add(FAC_PARK, PRI_ERR,
                             "OrbitFileReader::parseOrbitFile "
                             "<time-out> '%s' was null, unparsable, or less than 5 for extension '%s'",
                             temp.data(), extension.data());
               orbit_valid = false;
            }
         }

         // Process the <transfer-key> element to set mKeycode.
         int keycode = OrbitData::NO_KEYCODE;	// Assume no value present.
         TiXmlNode* keycode_element =
            orbit_element->FirstChild("transfer-key");
         if (keycode_element)
         {
            UtlString temp;
            textContentShallow(temp, keycode_element->ToElement());
            if (temp.length() == 1 &&
                (keycode = valid_keycodes.index(temp[0]),
                 keycode != UTL_NOT_FOUND))
            {
               /* null */ ;
            }
            else
            {
               // Keycode was null or unparsable.
               OsSysLog::add(FAC_PARK, PRI_ERR,
                             "OrbitFileReader::parseOrbitFile "
                             "<transfer-key> '%s' was invalid for extension '%s'",
                             temp.data(), extension.data());
               orbit_valid = false;
            }
         }

         // Process the <capcity> element to set mCapacity.
         int capacity = OrbitData::UNLIMITED_CAPACITY;	// Assume no value present.
         TiXmlNode* capacity_element =
            orbit_element->FirstChild("capacity");
         if (capacity_element)
         {
            UtlString temp;
            textContentShallow(temp, capacity_element->ToElement());
            char *endptr;
            capacity = strtol(temp.data(), &endptr, 10);
            if (temp.isNull() ||
                endptr - temp.data() != temp.length() ||
                capacity < 0)
            {
               // Capacity was null or unparsable.
               OsSysLog::add(FAC_PARK, PRI_ERR,
                             "OrbitFileReader::parseOrbitFile "
                             "<capacity> '%s' was null, unparsable, or negative for extension '%s'",
                             temp.data(), extension.data());
               orbit_valid = false;
            }
         }

         // If no errors were found, create the values to insert into
         // mOrbitList.
         if (orbit_valid)
         {
            // Allocate the objects and assign their values.
            UtlString* extension_heap = new UtlString;
            *extension_heap = extension;
            OrbitData* orbit_data_heap = new OrbitData;
            orbit_data_heap->mTimeout = timeout;
            orbit_data_heap->mAudio = audio;
            orbit_data_heap->mKeycode = keycode;
            orbit_data_heap->mCapacity = capacity;

            // Attempt to insert the user into the orbit list.
            if (mOrbitList.insertKeyAndValue(extension_heap, orbit_data_heap))
            {
               // Insertion succeeded.
               // *extension_heap and *orbit_data_heap are now owned
               // by mOrbitList.
            }
            else
            {
               // Insertion failed, presumably because the extension was
               // already in there.
               OsSysLog::add(FAC_PARK, PRI_ERR,
                             "OrbitFileReader::parseOrbitFile "
                             "Inserting extension '%s' failed -- specified as an orbit twice?",
                             extension_heap->data());
               // mOrbitList does not own the objects, so we must delete them.
               delete extension_heap;
               delete orbit_data_heap;
            }
         }
      }

      if (OsSysLog::willLog(FAC_PARK, PRI_DEBUG))
      {
         // Output the list of orbits.
         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                       "OrbitFileReader::parseOrbitFile "
                       "Valid orbits are:");
         UtlHashMapIterator itor(mOrbitList);
         while (itor())
         {
            UtlString* key = dynamic_cast<UtlString*> (itor.key());
            OrbitData* value = dynamic_cast<OrbitData*> (itor.value());
            OsSysLog::add(FAC_PARK, PRI_DEBUG,
                          "OrbitFileReader::parseOrbitFile "
                          "Orbit '%s', mTimeout = %d, mAudio = '%s', "
                          "mKeycode = %d, mCapacity = %d",
                          key->data(),
                          value->mTimeout,
                          value->mAudio.data(),
                          value->mKeycode,
                          value->mCapacity);
         }
         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                       "OrbitFileReader::parseOrbitFile "
                       "End of list");
      }

      // Find the <music-on-hold> element.
      TiXmlNode *groupNode = orbits_element->FirstChild("music-on-hold");
      if (groupNode != NULL)
      {
         TiXmlNode* audioNode = groupNode->FirstChild("background-audio");
         if ((audioNode != NULL)
             && (audioNode->FirstChild() != NULL))
         {
            mMusicOnHoldFile = (audioNode->FirstChild())->Value();
         }
      }
      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                    "OrbitFileReader::parseOrbitFile "
                    "mMusicOnHoldFile = '%s'",
                    mMusicOnHoldFile.data());

      // In any of these cases, attempt to do call retrieval.
      return OS_SUCCESS;
   }
   else
   {
      // Report error parsing file.
      OsSysLog::add(FAC_PARK, PRI_CRIT,
                    "OrbitFileReader::parseOrbitFile "
                    "Orbit file '%s' could not be parsed.", mFileName.data());
      // No hope of doing call retrieval.
      return OS_FAILED;
   }
}
