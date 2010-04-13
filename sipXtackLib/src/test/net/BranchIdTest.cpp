//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>
#include <os/OsSysLog.h>

#include <net/BranchId.h>
#include <net/SipMessage.h>

// SYSTEM INCLUDES

// APPLICATION INCLUDES

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// constructor
/**
 * Unit tests for BranchId.
 */
class BranchIdTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(BranchIdTest);

   /* ****************************************************************
    * NOTE:
    *   When adding to this test suite, it will be much more convenient
    *   to add tests to the end.  This is because the values generated
    *   by BranchId include a monotonically increasing number; if you add
    *   one before the end, you will change all values generated after
    *   the one you insert, and will have to manually recheck and repair
    *   all the expected results after your new one.
    * **************************************************************** */

   CPPUNIT_TEST(testBranchIdisRFC3261);
   CPPUNIT_TEST(testBranchIdparse);
   CPPUNIT_TEST(testBranchIdUniqueness);
   CPPUNIT_TEST(testBranchCopy);
   CPPUNIT_TEST(testLoopDetection);
   CPPUNIT_TEST(testLoopDetectionWithExclusions);

   CPPUNIT_TEST_SUITE_END();

public:

   void testBranchIdisRFC3261()
      {
         CPPUNIT_ASSERT(BranchId::isRFC3261("z9hG4bK11e0671922444aa6a5f7f10b5"));
         CPPUNIT_ASSERT(!BranchId::isRFC3261("az9hG4bK11e0671922444aa6a5f7f10b5"));
         CPPUNIT_ASSERT(!BranchId::isRFC3261("x9hG4bK11e0671922444aa6a5f7f10b5"));
      }

   void testBranchIdparse()
      {
         size_t       counter;
         UtlString    uniqueValue;
         UtlString    loopDetectKey;

         CPPUNIT_ASSERT(!BranchId::parse("z9hG4bK11e0671922444aa6a5f7f10b5",
                                         counter, uniqueValue, loopDetectKey));
         CPPUNIT_ASSERT(uniqueValue.isNull());
         CPPUNIT_ASSERT(loopDetectKey.isNull());

         CPPUNIT_ASSERT(!BranchId::parse("az9hG4bK11e0671922444aa6a5f7f10b5",
                                         counter, uniqueValue, loopDetectKey));
         CPPUNIT_ASSERT(uniqueValue.isNull());
         CPPUNIT_ASSERT(loopDetectKey.isNull());

         CPPUNIT_ASSERT(!BranchId::parse("x9hG4bK11e0671922444aa6a5f7f10b5",
                                         counter, uniqueValue, loopDetectKey));
         CPPUNIT_ASSERT(uniqueValue.isNull());
         CPPUNIT_ASSERT(loopDetectKey.isNull());

         CPPUNIT_ASSERT(BranchId::parse("z9hG4bK-XX-0014xXPnvpKqfTHmjFlrAD5sPA~1Pke3N5l0uqpDw00P`ZWYw",
                                        counter, uniqueValue, loopDetectKey));
         CPPUNIT_ASSERT_EQUAL((size_t)20, counter);
         ASSERT_STR_EQUAL("xXPnvpKqfTHmjFlrAD5sPA", uniqueValue.data());
         ASSERT_STR_EQUAL("1Pke3N5l0uqpDw00P`ZWYw", loopDetectKey.data());
      }

   void testBranchIdUniqueness()
      {
#        define CASES 10
         // The test succeeds if at least 9 of the 16 digits differ between all
         // pairs of Call-Ids we generate.  If the generated Call-Ids are random, this
         // test will fail 0.00023% of the time for any one pair, or about 0.011%
         // of the time for one of the 45 pairs.
#        define MIN_DIFFS 9

         UtlString stableTestSecret("stable");
         BranchId::setSecret(stableTestSecret);

         const char* testMsg =
            "INVITE sip:someone@example.com SIP/2.0"
            "To: sip:someone@example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage sipMsg(testMsg);

         UtlString output[CASES];

         // Generate some BranchIds.
         OsSysLog::add(FAC_SIP,PRI_DEBUG,"BranchIdTest::testBranchIdUniqueness unique cases");
         for (int i = 0; i < CASES; i++)
         {
            BranchId branchId(sipMsg);
            output[i].append(branchId.data());

            OsSysLog::add(FAC_SIP, PRI_DEBUG, "case %d initial: %s\n", i, output[i].data());
         }

         // Compare that they're different enough.
         for (int i = 0; i < CASES; i++)
         {
            for (int j = i+1; j < CASES; j++)
            {
               UtlString* s1 = &output[i];
               UtlString* s2 = &output[j];

               int differences = 0;
               for (size_t k = 0; k < s1->length() && k < s1->length(); k++)
               {
                  if ((*s1)(k) != (*s2)(k))
                  {
                     differences++;
                  }
               }
               if (differences < MIN_DIFFS)
               {
                  char msg[200];
                  sprintf(msg,
                          "BranchId '%s' and '%s' have %d different characters, "
                          "which is less than the minimum, %d",
                          s1->data(), s2->data(), differences, MIN_DIFFS);
                  CPPUNIT_ASSERT_MESSAGE(msg, FALSE);
               }
            }
         }

         // Now regenerate those as child ids without forks
         OsSysLog::add(FAC_SIP,PRI_DEBUG,"BranchIdTest::testBranchIdUniqueness child cases");
         for (int i = 0; i < CASES; i++)
         {
            BranchId parentBranchId(output[i]);
            // no addFork calls before creating the child
            BranchId branchId(parentBranchId, sipMsg);
            output[i].remove(0);
            output[i].append(branchId.data());

            OsSysLog::add(FAC_SIP, PRI_DEBUG, "case %d child  : %s\n", i, output[i].data());
         }

         // Compare that they're different enough.
         for (int i = 0; i < CASES; i++)
         {
            for (int j = i+1; j < CASES; j++)
            {
               UtlString* s1 = &output[i];
               UtlString* s2 = &output[j];

               int differences = 0;
               for (size_t k = 0; k < s1->length() && k < s1->length(); k++)
               {
                  if ((*s1)(k) != (*s2)(k))
                  {
                     differences++;
                  }
               }
               if (differences < MIN_DIFFS)
               {
                  char msg[200];
                  sprintf(msg,
                          "BranchId '%s' and '%s' have %d different characters, "
                          "which is less than the minimum, %d",
                          s1->data(), s2->data(), differences, MIN_DIFFS);
                  CPPUNIT_ASSERT_MESSAGE(msg, FALSE);
               }
            }
         }

         // And then add a fork to each to see if the forks are different
         OsSysLog::add(FAC_SIP,PRI_DEBUG,"BranchIdTest::testBranchIdUniqueness forked cases");
         for (int i = 0; i < CASES; i++)
         {
            BranchId parentBranchId(output[i]);

            char forkContact[32];
            sprintf(forkContact, "sip:someuser%02d@domain", i);
            Url contactUrl(forkContact);

            parentBranchId.addFork(contactUrl);

            BranchId branchId(parentBranchId, sipMsg);
            output[i].remove(0);
            output[i].append(branchId.data());

            OsSysLog::add(FAC_SIP, PRI_DEBUG, "case %d forked : %s\n", i, output[i].data());
         }

         // Compare that they're different enough.
         for (int i = 0; i < CASES; i++)
         {
            for (int j = i+1; j < CASES; j++)
            {
               UtlString* s1 = &output[i];
               UtlString* s2 = &output[j];

               int differences = 0;
               for (size_t k = 0; k < s1->length() && k < s1->length(); k++)
               {
                  if ((*s1)(k) != (*s2)(k))
                  {
                     differences++;
                  }
               }
               if (differences < MIN_DIFFS)
               {
                  char msg[200];
                  sprintf(msg,
                          "BranchId '%s' and '%s' have %d different characters, "
                          "which is less than the minimum, %d",
                          s1->data(), s2->data(), differences, MIN_DIFFS);
                  CPPUNIT_ASSERT_MESSAGE(msg, FALSE);
               }
            }
         }
      }

   void testBranchCopy()
      {
         UtlString sipXbranchId("z9hG4bK-XX-000af33a294c2143da892d81dbc8183fd7f8");
         BranchId  sipXcopied(sipXbranchId);

         ASSERT_STR_EQUAL(sipXbranchId.data(), sipXcopied.data());

         UtlString non_sipXbranchId("z9hG4bK-94rkdeeepdeloepw%ww-lflll");
         BranchId  non_sipXcopied(non_sipXbranchId);

         ASSERT_STR_EQUAL(sipXbranchId.data(), sipXcopied.data());
      }

   void testLoopDetection()
      {
         const char* testMsg0 =
            "INVITE sip:someone@example.com SIP/2.0\r\n"
            "To: sip:someone@example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         ssize_t msgLength;
         UtlString msgBytes;

         // initial send - no forking
         SipMessage sipMsg1(testMsg0);

         BranchId branchId1(sipMsg1);
         CPPUNIT_ASSERT(!branchId1.loopDetected(sipMsg1));
         sipMsg1.setMaxForwards(19);
         sipMsg1.addVia("example.com",PORT_NONE,"TCP",branchId1.data());
         sipMsg1.getBytes(&msgBytes, &msgLength);

         OsSysLog::add(FAC_SIP, PRI_DEBUG, "message %d: \n%s\n----\n", 1, msgBytes.data());

         // forward once - just one target

         SipMessage sipMsg2(sipMsg1);  // construct server transaction copy
         BranchId branchId2s(sipMsg2); // construct parent branch id

         branchId2s.addFork("sip:someone-else@example.com");
         CPPUNIT_ASSERT(!branchId2s.loopDetected(sipMsg2));

         /// this is the message that will later be detected as a loop
         sipMsg2.changeUri("sip:someone-else@example.com");
         sipMsg2.setMaxForwards(18);
         BranchId branchId2c1(branchId2s, sipMsg2);

         sipMsg2.addVia("example.com",PORT_NONE,"TCP",branchId2c1.data());
         sipMsg2.getBytes(&msgBytes, &msgLength);

         OsSysLog::add(FAC_SIP, PRI_DEBUG, "message %s: \n%s\n----\n", "2c1", msgBytes.data());

         // forward twice

         SipMessage sipMsg3(sipMsg2); // construct server transaction copy
         BranchId branchId3s(sipMsg3); // construct parent branch id

         branchId3s.addFork("sip:another-someone@example.com");
         branchId3s.addFork("sip:someone@example.com");

         CPPUNIT_ASSERT(!branchId3s.loopDetected(sipMsg3));

         SipMessage sipMsg3c1(sipMsg3);
         sipMsg3c1.changeUri("sip:another-someone@example.com");
         sipMsg3c1.setMaxForwards(17);
         BranchId branchId3c1(branchId3s, sipMsg3c1);
         sipMsg3c1.addVia("example.com",PORT_NONE,"TCP",branchId3c1.data());

         sipMsg3c1.getBytes(&msgBytes, &msgLength);
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "message %s: \n%s\n----\n", "3c1", msgBytes.data());

         SipMessage sipMsg3c2(sipMsg3);
         sipMsg3c2.changeUri("sip:someone@example.com");
         sipMsg3c2.setMaxForwards(17);
         BranchId branchId3c2(branchId3s, sipMsg3c2);
         sipMsg3c2.addVia("example.com",PORT_NONE,"TCP",branchId3c2.data());

         sipMsg3c2.getBytes(&msgBytes, &msgLength);
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "message %s: \n%s\n----\n", "3c2", msgBytes.data());

         SipMessage sipMsg4s1(sipMsg3c1);  // construct server transaction copy
         BranchId branchId4s1(sipMsg4s1);  // construct parent branch id

         branchId4s1.addFork("sip:yet-another-someone@example.com");

         CPPUNIT_ASSERT(!branchId4s1.loopDetected(sipMsg4s1));

         SipMessage sipMsg4c1(sipMsg4s1);
         sipMsg4c1.changeUri("sip:yet-another-someone@example.com");
         sipMsg3c1.setMaxForwards(16);
         BranchId branchId4c1(branchId4s1, sipMsg4c1);
         sipMsg4c1.addVia("example.com",PORT_NONE,"TCP",branchId4c1.data());

         sipMsg4c1.getBytes(&msgBytes, &msgLength);
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "message %s: \n%s\n----\n", "4c1", msgBytes.data());

         SipMessage sipMsg4s2(sipMsg3c2);  // construct server transaction copy
         BranchId branchId4s2(sipMsg4s2);  // construct parent branch id

         branchId4s2.addFork("sip:someone-else@example.com"); // loop

         sipMsg4s2.getBytes(&msgBytes, &msgLength);
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "message %s: \n%s\n----\n", "4s2", msgBytes.data());

         CPPUNIT_ASSERT_EQUAL(2U,branchId4s2.loopDetected(sipMsg4s2));
      }

   void testLoopDetectionWithExclusions()
      {
         const char* testMsg0 =
            "INVITE sip:someone@example.com SIP/2.0\r\n"
            "To: sip:someone@example.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         ssize_t msgLength;
         UtlString msgBytes;

         // initial send - no forking
         SipMessage sipMsg1(testMsg0);

         BranchId branchId1(sipMsg1);
         CPPUNIT_ASSERT(!branchId1.loopDetected(sipMsg1));
         sipMsg1.setMaxForwards(19);
         sipMsg1.addVia("example.com",PORT_NONE,"TCP",branchId1.data());
         sipMsg1.getBytes(&msgBytes, &msgLength);

         OsSysLog::add(FAC_SIP, PRI_DEBUG, "message %d: \n%s\n----\n", 1, msgBytes.data());

         // forward once - just one target

         SipMessage sipMsg2(sipMsg1);  // construct server transaction copy
         BranchId branchId2s(sipMsg2); // construct parent branch id

         UtlString str2s("<"
                         "sip:someone-else@example.com"
                         ";sipx-noroute=Voicemail"
                         "?expires=30"
                         "&X-sipX-Authidentity=%3Csip%3Asomeone%40example.com%3Bsignature%3D49CB467E%253Afe14b92247d326150a06b981f60fd883%3E"
                         "&ROUTE=%3Csip%3A192.168.2.1%3A5060%3Blr%3E"
                         ">"
                         );
         Url url2s(str2s, Url::NameAddr);
         branchId2s.addFork(url2s);
         CPPUNIT_ASSERT(!branchId2s.loopDetected(sipMsg2));

         /// this is the message that will later be detected as a loop
         sipMsg2.changeUri("sip:someone-else@example.com");
         sipMsg2.setMaxForwards(18);
         BranchId branchId2c1(branchId2s, sipMsg2);

         sipMsg2.addVia("example.com",PORT_NONE,"TCP",branchId2c1.data());
         sipMsg2.getBytes(&msgBytes, &msgLength);

         OsSysLog::add(FAC_SIP, PRI_DEBUG, "message %s: \n%s\n----\n", "2c1", msgBytes.data());

         // forward twice

         SipMessage sipMsg3(sipMsg2); // construct server transaction copy
         BranchId branchId3s(sipMsg3); // construct parent branch id

         branchId3s.addFork("sip:another-someone@example.com");
         branchId3s.addFork("sip:someone@example.com");

         CPPUNIT_ASSERT(!branchId3s.loopDetected(sipMsg3));

         SipMessage sipMsg3c1(sipMsg3);
         sipMsg3c1.changeUri("sip:another-someone@example.com");
         sipMsg3c1.setMaxForwards(17);
         BranchId branchId3c1(branchId3s, sipMsg3c1);
         sipMsg3c1.addVia("example.com",PORT_NONE,"TCP",branchId3c1.data());

         sipMsg3c1.getBytes(&msgBytes, &msgLength);
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "message %s: \n%s\n----\n", "3c1", msgBytes.data());

         SipMessage sipMsg3c2(sipMsg3);
         sipMsg3c2.changeUri("sip:someone@example.com");
         sipMsg3c2.setMaxForwards(17);
         BranchId branchId3c2(branchId3s, sipMsg3c2);
         sipMsg3c2.addVia("example.com",PORT_NONE,"TCP",branchId3c2.data());

         sipMsg3c2.getBytes(&msgBytes, &msgLength);
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "message %s: \n%s\n----\n", "3c2", msgBytes.data());

         SipMessage sipMsg4s1(sipMsg3c1);  // construct server transaction copy
         BranchId branchId4s1(sipMsg4s1);  // construct parent branch id

         branchId4s1.addFork("sip:yet-another-someone@example.com");

         CPPUNIT_ASSERT(!branchId4s1.loopDetected(sipMsg4s1));

         SipMessage sipMsg4c1(sipMsg4s1);
         sipMsg4c1.changeUri("sip:yet-another-someone@example.com");
         sipMsg3c1.setMaxForwards(16);
         BranchId branchId4c1(branchId4s1, sipMsg4c1);
         sipMsg4c1.addVia("example.com",PORT_NONE,"TCP",branchId4c1.data());

         sipMsg4c1.getBytes(&msgBytes, &msgLength);
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "message %s: \n%s\n----\n", "4c1", msgBytes.data());

         SipMessage sipMsg4s2(sipMsg3c2);  // construct server transaction copy
         BranchId branchId4s2(sipMsg4s2);  // construct parent branch id

         // this contact matches the first forward, but with a different authorization signature
         UtlString str4s2("<"
                          "sip:someone-else@example.com"
                          ";sipx-noroute=Voicemail"
                          "?expires=30"
                          "&X-sipX-Authidentity=%3Csip%3Ayet-another-someone%40example.com%3Bsignature%3D49CB4680%253A1b8cc19bd81162eeec3a2ff809f1a58a%3E"
                          "&ROUTE=%3Csip%3A192.168.2.1%3A5060%3Blr%3E"
                          ">");
         Url url4s2(str4s2, Url::NameAddr);
         branchId4s2.addFork(url4s2); // loop

         sipMsg4s2.getBytes(&msgBytes, &msgLength);
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "message %s: \n%s\n----\n", "4s2", msgBytes.data());

         CPPUNIT_ASSERT_EQUAL(2U,branchId4s2.loopDetected(sipMsg4s2));
      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(BranchIdTest);
