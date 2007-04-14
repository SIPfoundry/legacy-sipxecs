// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <net/SipPresenceEvent.h>


/**
 * Unit test for SipPresenceEvent
 */
class SipPresenceEventTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipPresenceEventTest);
   CPPUNIT_TEST(testGetPresencePackage);
   CPPUNIT_TEST(testPresencePackageParser);
   CPPUNIT_TEST_SUITE_END();

public:

   void testGetPresencePackage()
      {
         const char *package = 
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"dong@panther.pingtel.com\">\r\n"
            "<tuple id=\"1\">\r\n"
            "<status>\r\n"
            "<basic>open</basic>\r\n"
            "</status>\r\n"
            "<contact>tel:+0123456789</contact>\r\n"
            "</tuple>\r\n"
            "</presence>\r\n"
            ;
       
         SipPresenceEvent presenceEvent("dong@panther.pingtel.com");
         
         UtlString id = "1";
         Tuple* tuple = new Tuple(id);

         tuple->setStatus("open");
         tuple->setContact("tel:+0123456789", 1.0);

         presenceEvent.insertTuple(tuple);

         UtlString bodyString;
         int bodyLength;
       
         presenceEvent.getBytes(&bodyString, &bodyLength);
         //printf("body = \n%s\n", bodyString.data());

         CPPUNIT_ASSERT(strcmp(bodyString.data(), package) == 0);

         int otherLength = presenceEvent.getLength();
         CPPUNIT_ASSERT_EQUAL_MESSAGE("content length is not equal",
                                      bodyLength, otherLength);
      }

   void testPresencePackageParser()
      {
         const char *package = 
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"dong@panther.pingtel.com\">\r\n"
            "<tuple id=\"1\">\r\n"
            "<status>\r\n"
            "<basic>open</basic>\r\n"
            "</status>\r\n"
            "<contact>tel:+0123456789</contact>\r\n"
            "</tuple>\r\n"
            "</presence>\r\n"
            ;
       
         SipPresenceEvent body("dong@panther.pingtel.com", package);

         UtlString bodyString;
         int bodyLength;
       
         body.getBytes(&bodyString, &bodyLength);
         //printf("body = \n%s\n", bodyString.data());
       
         CPPUNIT_ASSERT(strcmp(bodyString.data(), package) == 0);

         int otherLength = body.getLength();
         CPPUNIT_ASSERT_EQUAL_MESSAGE("content length is not equal",
                                      bodyLength, otherLength);
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipPresenceEventTest);
