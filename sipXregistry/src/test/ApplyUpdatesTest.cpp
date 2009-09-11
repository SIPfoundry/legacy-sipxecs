//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <iostream>
#include <memory>
#include <sipxunit/TestUtilities.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include "net/Url.h"
#include "os/OsDateTime.h"
#include "os/OsFS.h"
#include "os/OsProcess.h"
#include "testlib/RegistrationDbTestContext.h"
#include "registry/SipRegistrar.h"
#include "sipdb/RegistrationBinding.h"
#include "utl/UtlHashMapIterator.h"
#include "utl/UtlSList.h"
#include "utl/UtlSListIterator.h"
#include "utl/UtlString.h"
#include "SipRegistrarServer.h"
#include "xmlparser/tinyxml.h"
#include "xmlparser/ExtractContent.h"

using namespace std;

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Read an XML file of Registration DB entries and fill a UtlSList
// with RegistrationBinding objects.
void readUpdateList(const char* filename,
                    UtlSList& updates,
                    int timeOffset = 0);

// Dump the contents of a list of RegistrationBinding's.
void dumpList(const UtlSList& list);

class ApplyUpdatesTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(ApplyUpdatesTest);
   CPPUNIT_TEST(testApplyUpdates);
   CPPUNIT_TEST_SUITE_END();

public:
   void setUp()
      {
         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );
         testDbContext.inputFile("applyUpdatesStart.xml");
      }

   void testApplyUpdates()
      {
         // Test that applying updates detects out-of-order situations correctly.

         OsConfigDb configuration;
         // Must declare an XMLRPC port to activate peering.
         configuration.set("SIP_REGISTRAR_XMLRPC_PORT", "DEFAULT");
         // Configure self to be R0 and R1 and R2 to be peers.
         configuration.set("SIP_REGISTRAR_NAME", "R0");
         configuration.set("SIP_REGISTRAR_SYNC_WITH", "R1, R2");

         SipRegistrar registrar(&configuration);

         // Create and start the persist thread, because applying updates
         // will try to trigger the persist thread.
         registrar.createAndStartPersist();

         // Get the updates to apply.

         unsigned long timeNow = OsDateTime::getSecsSinceEpoch();

         // update1 - one update with update number 11 that is in-order
         UtlSList update1;
         readUpdateList(TEST_DATA_DIR "/regdbdata/update1.xml", update1, timeNow);
         //dumpList(update1);

         // update2 - one update with update number 12 that is out-of-order
         UtlSList update2;
         readUpdateList(TEST_DATA_DIR "/regdbdata/update2.xml", update2, timeNow);
         //dumpList(update2);

         // update3 - two updates with update number 13 that are in-order
         UtlSList update3;
         readUpdateList(TEST_DATA_DIR "/regdbdata/update3.xml", update3, timeNow);
         //dumpList(update3);

         // update4 - two updates with update number 14 with the first out-of-order
         UtlSList update4;
         readUpdateList(TEST_DATA_DIR "/regdbdata/update4.xml", update4, timeNow);
         //dumpList(update4);

         // update5 - two updates with update number 15 with the second out-of-order
         UtlSList update5;
         readUpdateList(TEST_DATA_DIR "/regdbdata/update5.xml", update5, timeNow);
         //dumpList(update5);

         // update6 - two updates with update number 16 which would trigger
         // XECS-371, viz., they have the same Call-Id, CSeq, and AOR.
         UtlSList update6;
         readUpdateList(TEST_DATA_DIR "/regdbdata/update6.xml", update6, timeNow);
         //dumpList(update6);

         // Run the tests.

         UtlString err;

         CPPUNIT_ASSERT_EQUAL((Int64) 11,
                              registrar.getRegistrarServer().
                              applyUpdatesToDirectory(update1, &err));
         CPPUNIT_ASSERT_EQUAL((Int64) (-1),
                              registrar.getRegistrarServer().
                              applyUpdatesToDirectory(update2, &err));
         CPPUNIT_ASSERT_EQUAL((Int64) 13,
                              registrar.getRegistrarServer().
                              applyUpdatesToDirectory(update3, &err));
         CPPUNIT_ASSERT_EQUAL((Int64) (-1),
                              registrar.getRegistrarServer().
                              applyUpdatesToDirectory(update4, &err));
         CPPUNIT_ASSERT_EQUAL((Int64) (-1),
                              registrar.getRegistrarServer().
                              applyUpdatesToDirectory(update5, &err));
         CPPUNIT_ASSERT_EQUAL((Int64) 16,
                              registrar.getRegistrarServer().
                              applyUpdatesToDirectory(update6, &err));
      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(ApplyUpdatesTest);

// Read an XML file of Registration DB entries and fill a UtlSList
// with RegistrationBinding objects.
// The XML must contain all data fields, except for <identity>.
void readUpdateList(const char* filename,
                    UtlSList& updates,
                    int timeOffset)
{
   char msg[1024];

   // Clear the list.
   updates.destroyAll();

   // Initialize Tiny XML document object.
   TiXmlDocument document;
   TiXmlNode* items_element;

   sprintf(msg, "Data file '%s'", filename);

   // Load the XML into it.
   CPPUNIT_ASSERT_MESSAGE(msg, document.LoadFile(filename));

   // Find the top element, which should be an <items>.
   items_element = document.FirstChild("items");
   CPPUNIT_ASSERT_MESSAGE(msg, items_element != NULL);
   CPPUNIT_ASSERT_MESSAGE(msg, items_element->Type() == TiXmlNode::ELEMENT);

   // Find all the <item> elements.
   TiXmlNode* item_element;
   int item_no;
   for (item_element = 0, item_no = 0;
        (item_element = items_element->IterateChildren("item",
                                                       item_element));
        item_no++
         )
   {
      // Process each <item> element.

      // The UtlHashMap to convert into a RegistrationBinding.
      UtlHashMap hash_map;

      // Process the string elements.
      const char* string_elements[] = {
         "callid",
         "uri",
         "contact",
         "qvalue",
         "instance_id",
         "gruu",
         "path",
         "primary",
      };
      for (unsigned int i = 0;
           i < sizeof (string_elements) / sizeof (string_elements[0]);
           i++)
      {
         sprintf(msg, "Data file '%s', <item> number %d, element <%s>",
                 filename, item_no, string_elements[i]);

         // Get the element.
         TiXmlNode* element =
            item_element->FirstChild(string_elements[i]);
         CPPUNIT_ASSERT_MESSAGE(msg, element != NULL);
         UtlString* value = new UtlString;
         textContentShallow(*value, element->ToElement());

         // Insert it into the hash map.
         hash_map.insertKeyAndValue(new UtlString(string_elements[i]),
                                    value);

         // From the <uri> value, derive the <identity> value.
         if (strcmp(string_elements[i], "uri") == 0)
         {
            // Parse the <uri> value as a name-addr.
            Url url(value->data(), FALSE);
            // Remove any parameters.
            url.removeParameters();
            // Extract the URI proper.
            UtlString* identity = new UtlString;
            url.getUri(*identity);

            // Insert it into the hash map.
            hash_map.insertKeyAndValue(new UtlString("identity"),
                                       value);
         }
      }

      // Process the <cseq> element, which is a UtlInt.
      {
         sprintf(msg, "Data file '%s', <item> number %d, element <cseq>",
                 filename, item_no);

         // Get the element.
         TiXmlNode* element =
            item_element->FirstChild("cseq");
         CPPUNIT_ASSERT_MESSAGE(msg, element != NULL);
         UtlString s;
         textContentShallow(s, element->ToElement());
         CPPUNIT_ASSERT_MESSAGE(msg, !s.isNull());
         // Convert the value to a UtlInt.
         char* endptr;
         int i = strtol(s.data(), &endptr, 10);
         CPPUNIT_ASSERT_MESSAGE(msg, *endptr == '\0');
         UtlInt* value = new UtlInt(i);

         // Insert it into the hash map.
         hash_map.insertKeyAndValue(new UtlString("cseq"),
                                    value);
      }

      // Process the <expires> element, which is a UtlInt, and has the
      // offset added to it.
      {
         sprintf(msg, "Data file '%s', <item> number %d, element <expires>",
                 filename, item_no);

         // Get the element.
         TiXmlNode* element =
            item_element->FirstChild("expires");
         CPPUNIT_ASSERT_MESSAGE(msg, element != NULL);
         UtlString s;
         textContentShallow(s, element->ToElement());
         CPPUNIT_ASSERT_MESSAGE(msg, !s.isNull());
         // Convert the value to a UtlInt.
         char* endptr;
         int i = strtol(s.data(), &endptr, 10);
         CPPUNIT_ASSERT_MESSAGE(msg, *endptr == '\0');
         UtlInt* value = new UtlInt(i + timeOffset);

         // Insert it into the hash map.
         hash_map.insertKeyAndValue(new UtlString("expires"),
                                    value);
      }

      // Process the <update_number> element, which is a UtlLongLongInt.
      {
         sprintf(msg, "Data file '%s', <item> number %d, element <update_number>",
                 filename, item_no);

         // Get the element.
         TiXmlNode* element =
            item_element->FirstChild("update_number");
         CPPUNIT_ASSERT_MESSAGE(msg, element != NULL);
         UtlString s;
         textContentShallow(s, element->ToElement());
         CPPUNIT_ASSERT_MESSAGE(msg, !s.isNull());
         // Convert the value to a UtlInt.
         char* endptr;
         Int64 i = strtoll(s.data(), &endptr, 10);
         CPPUNIT_ASSERT_MESSAGE(msg, *endptr == '\0');
         UtlLongLongInt* value = new UtlLongLongInt(i);

         // Insert it into the hash map.
         hash_map.insertKeyAndValue(new UtlString("update_number"),
                                    value);
      }

      // Construct the RegistrationBinding object.
      RegistrationBinding* binding = new RegistrationBinding(hash_map);

      // Add it to the updates list.
      updates.append(binding);
   }
}

// Dump the contents of a list of RegistrationBinding's.
void dumpList(const UtlSList& list)
{
   fprintf(stderr, "=== start\n");
   UtlSListIterator list_iter(list);
   RegistrationBinding* item;
   int item_no = 0;
   while ((item = dynamic_cast <RegistrationBinding*> (list_iter())))
   {
      fprintf(stderr, "--- item %d\n", item_no);
      UtlHashMap contents;
      item->copy(contents);
      UtlHashMapIterator iter(contents);
      UtlString* name;
      while ((name = dynamic_cast <UtlString*> (iter())))
      {
         UtlContainable* value = iter.value();
         UtlContainableType type = value->getContainableType();
         if (type == UtlString::TYPE)
         {
            fprintf(stderr, "%s = '%s'\n",
                    name->data(),
                    (dynamic_cast <UtlString*> (iter.value()))->data());
         }
         else if (type == UtlLongLongInt::TYPE)
         {
            Int64 value =
               (dynamic_cast <UtlLongLongInt*> (iter.value()))->getValue();
            fprintf(stderr, "%s = 0x%" FORMAT_INTLL "x = %" FORMAT_INTLL "d\n",
                    name->data(), value, value);
         }
         else if (type == UtlInt::TYPE)
         {
            fprintf(stderr, "%s = %" PRIdPTR "\n",
                    name->data(),
                    (dynamic_cast <UtlInt*> (iter.value()))->getValue());
         }
         else
         {
            fprintf(stderr, "%s has unknown type %s\n",
                    name->data(), type);
         }
      }
      item_no++;
   }
   fprintf(stderr, "=== end\n");
}
