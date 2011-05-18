// 
// Copyright (C) 2008 Nortel, certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <os/OsLogger.h>
#include <sipxunit/TestUtilities.h>

/**
 * Test OsSysLog API
 */
class OsSysLogTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(OsSysLogTest);
   CPPUNIT_TEST(testPriorityToName);
   CPPUNIT_TEST(testNameToPriority);
   CPPUNIT_TEST_SUITE_END();

public:

   void testPriorityToName()
      {
         ASSERT_STR_EQUAL("InvalidSyslogPriority", Os::Logger::instance().priorityName((OsSysLogPriority)-1));

         ASSERT_STR_EQUAL("DEBUG", Os::Logger::instance().priorityName(PRI_DEBUG));
         ASSERT_STR_EQUAL("INFO", Os::Logger::instance().priorityName(PRI_INFO));
         ASSERT_STR_EQUAL("NOTICE", Os::Logger::instance().priorityName(PRI_NOTICE));
         ASSERT_STR_EQUAL("WARNING", Os::Logger::instance().priorityName(PRI_WARNING));
         ASSERT_STR_EQUAL("ERR", Os::Logger::instance().priorityName(PRI_ERR));
         ASSERT_STR_EQUAL("CRIT", Os::Logger::instance().priorityName(PRI_CRIT));
         ASSERT_STR_EQUAL("ALERT", Os::Logger::instance().priorityName(PRI_ALERT));
         ASSERT_STR_EQUAL("EMERG", Os::Logger::instance().priorityName(PRI_EMERG));

         ASSERT_STR_EQUAL("InvalidSyslogPriority", Os::Logger::instance().priorityName((OsSysLogPriority) 55));
      }

   void testNameToPriority()
      {
         OsSysLogPriority priority;
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("DEBUG", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_DEBUG, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("INFO", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_INFO, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("NOTICE", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_NOTICE, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("WARNING", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_WARNING, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("ERR", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_ERR, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("CRIT", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_CRIT, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("ALERT", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_ALERT, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("EMERG", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_EMERG, priority);

         CPPUNIT_ASSERT(Os::Logger::instance().priority("debug", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_DEBUG, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("info", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_INFO, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("notice", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_NOTICE, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("warning", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_WARNING, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("err", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_ERR, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("crit", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_CRIT, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("alert", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_ALERT, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("emerg", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_EMERG, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("Debug", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_DEBUG, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("Info", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_INFO, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("Notice", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_NOTICE, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("Warning", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_WARNING, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("Err", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_ERR, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("Crit", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_CRIT, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("Alert", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_ALERT, priority);
         
         CPPUNIT_ASSERT(Os::Logger::instance().priority("Emerg", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_EMERG, priority);
         
         CPPUNIT_ASSERT(!Os::Logger::instance().priority("Bogus", priority));
      }
   
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsSysLogTest);
