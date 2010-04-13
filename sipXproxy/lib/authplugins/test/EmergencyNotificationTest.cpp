//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/TestCase.h"
#include "sipxunit/TestUtilities.h"
#include "testlib/SipDbTestContext.h"
#include "os/OsDefs.h"
#include "os/OsConfigDb.h"
#include "utl/PluginHooks.h"

#include "AuthPlugin.h"
#include "EmergencyNotify.h"

class EmergencyRulesTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(EmergencyRulesTest);

   CPPUNIT_TEST(testEmergencyNotification);

   CPPUNIT_TEST_SUITE_END();

public:

   static EmergencyNotify* enforcer;

   // Test that only URIs that match an Emergency dial rule are matched.
   void testEmergencyNotification()
      {
         OsConfigDb configuration;
         configuration.set("EMERGRULES", TEST_DATA_DIR "/enforcerules.xml");

         enforcer->readConfig(configuration);

         Url requestUri("sip:911@emergency-gw");
         UtlString nameStr;
         UtlString descriptionStr;
         CPPUNIT_ASSERT(enforcer->getMatchedRule(requestUri, nameStr, descriptionStr));
         ASSERT_STR_EQUAL("Emergency", nameStr.data());
         ASSERT_STR_EQUAL("", descriptionStr.data());

         Url okRequestUri("sip:user@boat");
         CPPUNIT_ASSERT(!enforcer->getMatchedRule(okRequestUri, nameStr, descriptionStr));

         Url forbiddenRequestUri("sip:somewhere@forbidden");
         CPPUNIT_ASSERT(!enforcer->getMatchedRule(forbiddenRequestUri, nameStr, descriptionStr));
      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(EmergencyRulesTest);

EmergencyNotify* EmergencyRulesTest::enforcer = dynamic_cast<EmergencyNotify*>(getAuthPlugin("emerg"));
