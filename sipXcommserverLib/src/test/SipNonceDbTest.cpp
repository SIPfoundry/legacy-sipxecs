//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include "os/OsConfigDb.h"
#include "net/NetMd5Codec.h"
#include "sipXecsService/SipXecsService.h"
#include "sipXecsService/SipNonceDb.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SipNonceDbTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipNonceDbTest);

   CPPUNIT_TEST(testNonce);
   CPPUNIT_TEST(testSharedNonce);

   CPPUNIT_TEST_SUITE_END();

public:

   void setUp()
      {
         setenv(SipXecsService::ConfigurationDirType, TEST_DATA_DIR "/sharedsecret", true );
      }

   void tearDown()
      {
         unsetenv(SipXecsService::ConfigurationDirType);
      }

   void testNonce()
      {
         SipNonceDb nonceDb;

         UtlString testCallId("testCallId");
         UtlString testFromTag("testFromTag");
         UtlString testRealm("testRealm");
         UtlString testNonce;

         nonceDb.createNewNonce(testCallId, testFromTag, testRealm, testNonce);

         CPPUNIT_ASSERT(nonceDb.isNonceValid(testNonce, testCallId, testFromTag, testRealm, 3));

         CPPUNIT_ASSERT(!nonceDb.isNonceValid(testNonce,testCallId,testFromTag,testRealm,-3));

         UtlString testBadCallId("testBadCallId");
         CPPUNIT_ASSERT(!nonceDb.isNonceValid(testNonce,testBadCallId,testFromTag,testRealm,3));

         UtlString testBadFromTag("testBadFromTag");
         CPPUNIT_ASSERT(!nonceDb.isNonceValid(testNonce,testCallId,testBadFromTag,testRealm,3));

         UtlString testBadRealm("testBadRealm");
         CPPUNIT_ASSERT(!nonceDb.isNonceValid(testNonce,testCallId,testFromTag,testBadRealm,3));
      }

   void testSharedNonce()
      {
         SipNonceDb* sharedNonceDb = SharedNonceDb::get();

         UtlString testCallId("testCallId");
         UtlString testFromTag("testFromTag");
         UtlString testRealm("testRealm");
         UtlString testNonce;

         sharedNonceDb->createNewNonce(testCallId, testFromTag, testRealm, testNonce);

         CPPUNIT_ASSERT(sharedNonceDb->isNonceValid(testNonce, testCallId,
                                                    testFromTag, testRealm, 3));

         CPPUNIT_ASSERT(!sharedNonceDb->isNonceValid(testNonce,testCallId,
                                                     testFromTag,testRealm,-3));

         UtlString testBadCallId("testBadCallId");
         CPPUNIT_ASSERT(!sharedNonceDb->isNonceValid(testNonce,testBadCallId,
                                                     testFromTag,testRealm,3));

         UtlString testBadFromTag("testBadFromTag");
         CPPUNIT_ASSERT(!sharedNonceDb->isNonceValid(testNonce,testCallId,
                                                     testBadFromTag,testRealm,3));

         UtlString testBadRealm("testBadRealm");
         CPPUNIT_ASSERT(!sharedNonceDb->isNonceValid(testNonce,testCallId,
                                                     testFromTag,testBadRealm,3));
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipNonceDbTest);
