//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include "utl/UtlSList.h"
#include "utl/UtlSListIterator.h"
#include "net/Url.h"
#include "os/OsDateTime.h"

#include "sipdb/RegistrationDB.h"
#include "sipdb/RegistrationBinding.h"
#include "sipdb/ResultSet.h"
#include "testlib/RegistrationDbTestContext.h"
//#include "sipdb/SIPDBManager.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class RegistryDbTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(RegistryDbTest);
   CPPUNIT_TEST(testIsOutOfSequence);
   CPPUNIT_TEST(testGetMaxUpdateNumberForRegistrar);
   CPPUNIT_TEST(testGetNextUpdateNumberForRegistrar);
   CPPUNIT_TEST(testGetNextUpdateForRegistrar);
   CPPUNIT_TEST(testGetNewUpdatesForRegistrar);
   CPPUNIT_TEST(testGetUnexpiredContacts);
   CPPUNIT_TEST(testUpdateExistingBinding);
   CPPUNIT_TEST(testUpdateNewBinding);
   CPPUNIT_TEST(testUpdateSameBinding);
   CPPUNIT_TEST(testUpdateExistingUriNewContact);
   CPPUNIT_TEST(testExpireAllBindings);
   CPPUNIT_TEST(testExpireOldBindings);
   //CPPUNIT_TEST(testCleanAndPersist);
   CPPUNIT_TEST(testLoadMissingColumns);
   CPPUNIT_TEST_SUITE_END();

public:

   void tearDown()
      {
         RegistrationDB::getInstance()->releaseInstance();
      }

   void testGetMaxUpdateNumberForRegistrar()
      {
         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );

         testDbContext.inputFile("getMaxUpdate.xml");

         RegistrationDB* regDb = RegistrationDB::getInstance();

         Int64 seqOneMax = regDb->getMaxUpdateNumberForRegistrar("seqOne");
         CPPUNIT_ASSERT_EQUAL(5LL, seqOneMax);

         // Get maximum number for the other registrar
         Int64 seqTwoMax = regDb->getMaxUpdateNumberForRegistrar("seqTwo");
         CPPUNIT_ASSERT_EQUAL(7LL, seqTwoMax);

         // Pass in unknown registrar
         Int64 seqThreeMax = regDb->getMaxUpdateNumberForRegistrar("seqThree");
         CPPUNIT_ASSERT_EQUAL(0LL, seqThreeMax);
      }

   void testGetNextUpdateNumberForRegistrar()
      {
         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );

         testDbContext.inputFile("getMaxUpdate.xml");

         RegistrationDB* regDb = RegistrationDB::getInstance();

         Int64 seqOneNext = regDb->getNextUpdateNumberForRegistrar("seqOne", 0LL);
         CPPUNIT_ASSERT_EQUAL(1LL, seqOneNext);

         seqOneNext = regDb->getNextUpdateNumberForRegistrar("seqOne", 1LL);
         CPPUNIT_ASSERT_EQUAL(2LL, seqOneNext);

         seqOneNext = regDb->getNextUpdateNumberForRegistrar("seqOne", 2LL);
         CPPUNIT_ASSERT_EQUAL(3LL, seqOneNext);

         seqOneNext = regDb->getNextUpdateNumberForRegistrar("seqOne", 3LL);
         CPPUNIT_ASSERT_EQUAL(5LL, seqOneNext);

         seqOneNext = regDb->getNextUpdateNumberForRegistrar("seqOne", 4LL);
         CPPUNIT_ASSERT_EQUAL(5LL, seqOneNext);

         seqOneNext = regDb->getNextUpdateNumberForRegistrar("seqOne", 5LL);
         CPPUNIT_ASSERT_EQUAL(0LL, seqOneNext);

         Int64 seqTwoNext = regDb->getNextUpdateNumberForRegistrar("seqTwo", 4);
         CPPUNIT_ASSERT_EQUAL(7LL, seqTwoNext);

         // Pass in unknown registrar
         Int64 seqThreeNext = regDb->getNextUpdateNumberForRegistrar("seqThree", 1);
         CPPUNIT_ASSERT_EQUAL(0LL, seqThreeNext);
      }

   void testGetNextUpdateForRegistrar()
      {
         UtlSList bindings;
         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );

         testDbContext.inputFile("getMaxUpdate.xml");

         RegistrationDB* regDb = RegistrationDB::getInstance();

         Int64 seqOneUpdates = regDb->getNextUpdateForRegistrar("seqOne", 2, bindings);
         CPPUNIT_ASSERT_EQUAL(1LL, seqOneUpdates);

         UtlSListIterator iterator(bindings);

         // Loop through all returned bindings and mark the ones we've seen.
         // Also check correctness of bindings
         RegistrationBinding *binding;
         while ((binding = (RegistrationBinding*)iterator()))
         {
             CPPUNIT_ASSERT_EQUAL(binding->getCallId()->compareTo("ID3"), 0);
             CPPUNIT_ASSERT_EQUAL(binding->getContact()->compareTo("sip:300@10.1.1.20"), 0);
             CPPUNIT_ASSERT_EQUAL(binding->getUri()->toString().compareTo("sip:300@testdomain.example.com"), 0);
             CPPUNIT_ASSERT_EQUAL(300, binding->getCseq());
         }

         bindings.destroyAll();
         // Pass in high update number value, expect nothing to be returned
         seqOneUpdates = regDb->getNewUpdatesForRegistrar("seqOne", 8, bindings);
         CPPUNIT_ASSERT_EQUAL(0LL, seqOneUpdates);

         bindings.destroyAll();
         // Test the other registrar, expect one binding
         Int64 seqTwoUpdates = regDb->getNewUpdatesForRegistrar("seqTwo", 2, bindings);
         CPPUNIT_ASSERT_EQUAL(1LL, seqTwoUpdates);

         binding = (RegistrationBinding*)bindings.first();
         CPPUNIT_ASSERT_EQUAL(binding->getContact()->compareTo("sip:800@10.1.1.20"), 0);
         CPPUNIT_ASSERT_EQUAL(binding->getUri()->toString().compareTo("sip:800@testdomain.example.com"), 0);
         CPPUNIT_ASSERT_EQUAL(800, binding->getCseq());
      }

   void testGetNewUpdatesForRegistrar()
      {
         UtlSList bindings;
         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );

         testDbContext.inputFile("getMaxUpdate.xml");

         RegistrationDB* regDb = RegistrationDB::getInstance();

         Int64 seqOneUpdates = regDb->getNewUpdatesForRegistrar("seqOne", 2, bindings);
         CPPUNIT_ASSERT_EQUAL(2LL, seqOneUpdates);

         UtlSListIterator iterator(bindings);

         // Loop through all returned bindings and mark the ones we've seen.
         // Also check correctness of bindings
         UtlBoolean bSeenId3 = FALSE;
         UtlBoolean bSeenId5 = FALSE;
         UtlBoolean bSeenAnythingElse = FALSE;
         RegistrationBinding *binding;

         while ((binding = (RegistrationBinding*)iterator()))
         {
             const UtlString *s = binding->getCallId();
             if (s->compareTo("ID3") == 0)
             {
                 bSeenId3 = TRUE;
                 CPPUNIT_ASSERT_EQUAL(binding->getContact()->compareTo("sip:300@10.1.1.20"), 0);
                 CPPUNIT_ASSERT_EQUAL(binding->getUri()->toString().compareTo("sip:300@testdomain.example.com"), 0);
                 CPPUNIT_ASSERT_EQUAL(300, binding->getCseq());
             }
             else if (s->compareTo("ID5") == 0)
             {
                 bSeenId5 = TRUE;
                 CPPUNIT_ASSERT_EQUAL(binding->getContact()->compareTo("sip:500@10.1.1.20"), 0);
                 CPPUNIT_ASSERT_EQUAL(binding->getUri()->toString().compareTo("sip:500@testdomain.example.com"), 0);
                 CPPUNIT_ASSERT_EQUAL(500, binding->getCseq());
             }
             else
             {
                 bSeenAnythingElse = TRUE;
             }
         }
         CPPUNIT_ASSERT(!bSeenAnythingElse);
         CPPUNIT_ASSERT(bSeenId3);
         CPPUNIT_ASSERT(bSeenId5);

         bindings.destroyAll();
         // Pass in high update number value, expect nothing to be returned
         seqOneUpdates = regDb->getNewUpdatesForRegistrar("seqOne", 8, bindings);
         CPPUNIT_ASSERT_EQUAL(0LL, seqOneUpdates);

         bindings.destroyAll();
         // Test the other registrar, expect one binding
         Int64 seqTwoUpdates = regDb->getNewUpdatesForRegistrar("seqTwo", 2, bindings);
         CPPUNIT_ASSERT_EQUAL(1LL, seqTwoUpdates);

         binding = (RegistrationBinding*)bindings.first();
         CPPUNIT_ASSERT_EQUAL(binding->getContact()->compareTo("sip:800@10.1.1.20"), 0);
         CPPUNIT_ASSERT_EQUAL(binding->getUri()->toString().compareTo("sip:800@testdomain.example.com"), 0);
         CPPUNIT_ASSERT_EQUAL(800, binding->getCseq());
      }

   void testGetUnexpiredContacts()
      {
         Url uri;
         ResultSet results;
         UtlHashMap record1;
         UtlHashMap record2;
         int timeNow = (int)OsDateTime::getSecsSinceEpoch();

         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );

         testDbContext.inputFile("getUnexpiredContacts.xml");

         RegistrationDB* regDb = RegistrationDB::getInstance();

         uri.setUserId("900");
         uri.setHostAddress("testdomain.example.com");


         regDb->getUnexpiredContactsUser(uri, timeNow, results);
         int numResults = results.getSize();
         CPPUNIT_ASSERT_EQUAL(2, numResults);


         if (numResults == 2)
         {
             // Get first record
             results.getIndex(0, record1);
             UtlString* value = (UtlString*)record1.findValue(&RegistrationDB::gContactKey);
             CPPUNIT_ASSERT_EQUAL(value->compareTo("sip:900@10.1.1.20"), 0);

             value = (UtlString*)record1.findValue(&RegistrationDB::gUriKey);
             CPPUNIT_ASSERT_EQUAL(value->compareTo("sip:900@testdomain.example.com"), 0);

             // Get second record
             results.getIndex(1, record2);
             value = (UtlString*)record2.findValue(&RegistrationDB::gContactKey);
             CPPUNIT_ASSERT_EQUAL(value->compareTo("sip:900@10.1.1.21"), 0);

             value = (UtlString*)record2.findValue(&RegistrationDB::gUriKey);
             CPPUNIT_ASSERT_EQUAL(value->compareTo("sip:900@testdomain.example.com"), 0);
         }
      }

   void testIsOutOfSequence()
      {
         Url uri;
         UtlString callId;

         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );

         testDbContext.inputFile("getMaxUpdate.xml");

         RegistrationDB* regDb = RegistrationDB::getInstance();

         uri.setUserId("200");
         uri.setHostAddress("testdomain.example.com");
         callId = "ID2";

         UtlBoolean bOutOfSequence = regDb->isOutOfSequence(uri, callId, 900);
         CPPUNIT_ASSERT(!bOutOfSequence);

         bOutOfSequence = regDb->isOutOfSequence(uri, callId, 200);
         CPPUNIT_ASSERT(bOutOfSequence);

         bOutOfSequence = regDb->isOutOfSequence(uri, callId, 0);
         CPPUNIT_ASSERT(bOutOfSequence);
      }

   void testUpdateExistingBinding()
      {
         UtlSList bindings;
         RegistrationBinding* binding;

         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );

         testDbContext.inputFile("updateBindings.xml");

         RegistrationDB* regDb = RegistrationDB::getInstance();

         // Get an existing binding
         Int64 seqOneUpdates = regDb->getNewUpdatesForRegistrar("seqOne", 2, bindings);
         CPPUNIT_ASSERT_EQUAL(1LL, seqOneUpdates);

         binding = (RegistrationBinding*)bindings.first();

         // Increment the CSeq number
         int newCseq = binding->getCseq() + 1;
         binding->setCseq(newCseq);
         regDb->updateBinding(*binding);
         binding->setCseq(0);

         // Get the same binding
         bindings.destroyAll();
         seqOneUpdates = regDb->getNewUpdatesForRegistrar("seqOne", 2, bindings);
         CPPUNIT_ASSERT_EQUAL(1LL, seqOneUpdates);

         // Test if the new CSeq number got updated
         binding = (RegistrationBinding*)bindings.first();
         CPPUNIT_ASSERT_EQUAL(newCseq, binding->getCseq());
      }

   void testUpdateNewBinding()
      {
         Url uri;
         UtlString contact;
         UtlSList bindings;
         RegistrationBinding* binding;
         RegistrationBinding newBinding;

         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );

         testDbContext.inputFile("updateBindings.xml");

         RegistrationDB* regDb = RegistrationDB::getInstance();

         // Make sure an existing binding doesn't exist
         Int64 seqOneUpdates = regDb->getNewUpdatesForRegistrar("seqOne", 3, bindings);
         CPPUNIT_ASSERT_EQUAL(0LL, seqOneUpdates);

         // Create new binding with update number 3
         uri.setUserId("300");
         uri.setHostAddress("testdomain.example.com");
         newBinding.setUri(uri);

         newBinding.setContact("sip:300@10.1.1.20");
         newBinding.setCallId("ID3");
         newBinding.setPrimary("seqOne");
         newBinding.setUpdateNumber(3);

         // Add new binding
         regDb->updateBinding(newBinding);

         bindings.destroyAll();

         // We should now have two bindings - the original with update number 2 and the new one
         seqOneUpdates = regDb->getNewUpdatesForRegistrar("seqOne", 2, bindings);
         CPPUNIT_ASSERT_EQUAL(2LL, seqOneUpdates);

         // Test if the registration data corresponds to what we expect
         UtlBoolean bSeenId2 = FALSE;
         UtlBoolean bSeenId3 = FALSE;
         UtlBoolean bSeenAnythingElse = FALSE;
         UtlSListIterator iterator(bindings);

         while ((binding = (RegistrationBinding*)iterator()))
         {
             const UtlString *s = binding->getCallId();
             if (s->compareTo("ID2") == 0)
             {
                 bSeenId2 = TRUE;
                 CPPUNIT_ASSERT_EQUAL(binding->getContact()->compareTo("sip:200@10.1.1.20"), 0);
                 CPPUNIT_ASSERT_EQUAL(binding->getUri()->toString().compareTo("sip:200@testdomain.example.com"), 0);
             }
             else if (s->compareTo("ID3") == 0)
             {
                 bSeenId3 = TRUE;
                 CPPUNIT_ASSERT_EQUAL(binding->getContact()->compareTo("sip:300@10.1.1.20"), 0);
                 CPPUNIT_ASSERT_EQUAL(binding->getUri()->toString().compareTo("sip:300@testdomain.example.com"), 0);
                 CPPUNIT_ASSERT_EQUAL(binding->getUpdateNumber(), 3LL);
             }
             else
             {
                 bSeenAnythingElse = TRUE;
             }
         }
         CPPUNIT_ASSERT(!bSeenAnythingElse);
         CPPUNIT_ASSERT(bSeenId2);
         CPPUNIT_ASSERT(bSeenId3);
      }

   void testUpdateSameBinding()
      {
         UtlSList bindings;
         RegistrationBinding* binding;

         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );

         testDbContext.inputFile("updateBindings.xml");

         RegistrationDB* regDb = RegistrationDB::getInstance();

         // Get an existing binding
         Int64 seqOneUpdates = regDb->getNewUpdatesForRegistrar("seqOne", 2, bindings);
         CPPUNIT_ASSERT_EQUAL(1LL, seqOneUpdates);

         binding = (RegistrationBinding*)bindings.first();

         // Call updateBinding with the same binding
         regDb->updateBinding(*binding);

         // Get it back and make sure we still only have one
         bindings.destroyAll();
         seqOneUpdates = regDb->getNewUpdatesForRegistrar("seqOne", 2, bindings);
         CPPUNIT_ASSERT_EQUAL(1LL, seqOneUpdates);
      }

   void testUpdateExistingUriNewContact()
      {
         UtlSList bindings;
         RegistrationBinding* binding;

         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );

         testDbContext.inputFile("updateBindings.xml");

         RegistrationDB* regDb = RegistrationDB::getInstance();

         // Get an existing binding
         Int64 seqOneUpdates = regDb->getNewUpdatesForRegistrar("seqOne", 2, bindings);
         CPPUNIT_ASSERT_EQUAL(1LL, seqOneUpdates);

         binding = (RegistrationBinding*)bindings.first();

         // Change the contact address
         const UtlString* oldContact = binding->getContact();
         UtlString newContact(*oldContact);
         newContact.replace('2', '3');
         binding->setContact(newContact);
         regDb->updateBinding(*binding);

         // We should have 2 bindings now
         bindings.destroyAll();
         seqOneUpdates = regDb->getNewUpdatesForRegistrar("seqOne", 2, bindings);
         CPPUNIT_ASSERT_EQUAL(2LL, seqOneUpdates);
      }

   void testExpireAllBindings()
      {
         Url uri;
         UtlString contact;
         UtlSList bindings;
         ResultSet results;
         int timeNow = (int)OsDateTime::getSecsSinceEpoch();

         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );

         testDbContext.inputFile("getUnexpiredContacts.xml");

         RegistrationDB* regDb = RegistrationDB::getInstance();

         uri.setUserId("900");
         uri.setHostAddress("testdomain.example.com");

         regDb->getUnexpiredContactsUser(uri, timeNow, results);
         int numResults = results.getSize();
         CPPUNIT_ASSERT_EQUAL(2, numResults);

         results.destroyAll();

         regDb->expireAllBindings(uri, "ID9", 900, timeNow, "seqOne", 7);

         regDb->getUnexpiredContactsUser(uri, timeNow, results);
         numResults = results.getSize();
         CPPUNIT_ASSERT_EQUAL(0, numResults);
      }

   void testExpireOldBindings()
      {
         Url uri;
         //UtlString contact;
         //UtlSList bindings;
         ResultSet results;
         int timeNow = (int)OsDateTime::getSecsSinceEpoch();

         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );

         testDbContext.inputFile("getUnexpiredContacts.xml");

         RegistrationDB* regDb = RegistrationDB::getInstance();

         uri.setUserId("900");
         uri.setHostAddress("testdomain.example.com");

         regDb->getUnexpiredContactsUser(uri, timeNow, results);
         int numResults = results.getSize();
         CPPUNIT_ASSERT_EQUAL(2, numResults);

         results.destroyAll();

         regDb->expireOldBindings(uri, "ID9", 900, timeNow, "seqOne", 7);

         regDb->getUnexpiredContactsUser(uri, timeNow, results);
         numResults = results.getSize();
         CPPUNIT_ASSERT_EQUAL(1, numResults);
      }

   void testCleanAndPersist()
      {
         int timeNow = (int)OsDateTime::getSecsSinceEpoch();

         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );

         testDbContext.inputFile("cleanAndPersist.xml");

         RegistrationDB* regDb = RegistrationDB::getInstance();

         regDb->cleanAndPersist(timeNow - 50);
      }

   /* Test that a file containing rows that do not contain all the
    * columns can be loaded without crashing.
    * This is important in upgrade scenarios, when new columns have
    * been added to the table.
    */
   void testLoadMissingColumns()
      {
         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );

         testDbContext.inputFile("missingColumns.xml");
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RegistryDbTest);
