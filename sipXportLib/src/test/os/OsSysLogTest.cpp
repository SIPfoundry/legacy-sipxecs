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

#include <os/OsSysLog.h>
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
         ASSERT_STR_EQUAL("InvalidSyslogPriority", OsSysLog::priorityName((OsSysLogPriority)-1));

         ASSERT_STR_EQUAL("DEBUG", OsSysLog::priorityName(PRI_DEBUG));
         ASSERT_STR_EQUAL("INFO", OsSysLog::priorityName(PRI_INFO));
         ASSERT_STR_EQUAL("NOTICE", OsSysLog::priorityName(PRI_NOTICE));
         ASSERT_STR_EQUAL("WARNING", OsSysLog::priorityName(PRI_WARNING));
         ASSERT_STR_EQUAL("ERR", OsSysLog::priorityName(PRI_ERR));
         ASSERT_STR_EQUAL("CRIT", OsSysLog::priorityName(PRI_CRIT));
         ASSERT_STR_EQUAL("ALERT", OsSysLog::priorityName(PRI_ALERT));
         ASSERT_STR_EQUAL("EMERG", OsSysLog::priorityName(PRI_EMERG));

         ASSERT_STR_EQUAL("InvalidSyslogPriority", OsSysLog::priorityName((OsSysLogPriority) 55));
      }

   void testNameToPriority()
      {
         OsSysLogPriority priority;
         
         CPPUNIT_ASSERT(OsSysLog::priority("DEBUG", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_DEBUG, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("INFO", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_INFO, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("NOTICE", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_NOTICE, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("WARNING", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_WARNING, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("ERR", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_ERR, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("CRIT", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_CRIT, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("ALERT", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_ALERT, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("EMERG", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_EMERG, priority);

         CPPUNIT_ASSERT(OsSysLog::priority("debug", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_DEBUG, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("info", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_INFO, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("notice", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_NOTICE, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("warning", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_WARNING, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("err", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_ERR, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("crit", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_CRIT, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("alert", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_ALERT, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("emerg", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_EMERG, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("Debug", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_DEBUG, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("Info", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_INFO, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("Notice", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_NOTICE, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("Warning", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_WARNING, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("Err", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_ERR, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("Crit", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_CRIT, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("Alert", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_ALERT, priority);
         
         CPPUNIT_ASSERT(OsSysLog::priority("Emerg", priority));
         CPPUNIT_ASSERT_EQUAL(PRI_EMERG, priority);
         
         CPPUNIT_ASSERT(!OsSysLog::priority("Bogus", priority));
      }
   
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsSysLogTest);
