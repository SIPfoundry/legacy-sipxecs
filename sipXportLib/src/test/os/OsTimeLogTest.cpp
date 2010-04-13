//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <utl/UtlString.h>
#include <os/OsTimeLog.h>

#undef TIMELOG_SHOW // enable only for debugging

#ifdef TIMELOG_SHOW
#  define SHOWLOG(log) enableConsoleOutput(TRUE); log.dumpLog()
#else
#  define SHOWLOG(log) /* log */
#endif

/**
 * Unittest for OsTimeLog
 */
class OsTimeLogTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsTimeLogTest);
    CPPUNIT_TEST(testNoEntries);
    CPPUNIT_TEST(testEntries);
    CPPUNIT_TEST(testOverflow);
    CPPUNIT_TEST_SUITE_END();

public:
   void testNoEntries()
      {
         OsTimeLog log;
         UtlString results;

         results.append("NoEntries");

         log.getLogString(results);

         size_t hasNoEventsMsg = results.index("No Events Logged");
         CPPUNIT_ASSERT(hasNoEventsMsg != UTL_NOT_FOUND);

         SHOWLOG(log);
      }

   void testEntries()
      {
         OsTimeLog log;
         UtlString results;

         results.append("Entries");

         log.addEvent("event 1");
         log.addEvent("event 2");
         log.addEvent("event 3");
         log.addEvent("event 4");
         log.addEvent("event 5");

         log.getLogString(results);

         size_t hasNoEventsMsg = results.index("No Events Logged");
         CPPUNIT_ASSERT(hasNoEventsMsg == UTL_NOT_FOUND);

         size_t hasEventMsg = results.index("event 2");
         CPPUNIT_ASSERT(hasEventMsg != UTL_NOT_FOUND);

         SHOWLOG(log);
      }

   void testOverflow()
      {
         OsTimeLog log(3);
         UtlString results;

         results.append("Entries");

         log.addEvent("event 1");
         log.addEvent("event 2");
         log.addEvent("event 3");
         log.addEvent("event 4");
         log.addEvent("event 5");

         log.getLogString(results);

         size_t hasNoEventsMsg = results.index("No Events Logged");
         CPPUNIT_ASSERT(hasNoEventsMsg == UTL_NOT_FOUND);

         size_t hasOverflowMsg = results.index("Overflow");
         CPPUNIT_ASSERT(hasOverflowMsg != UTL_NOT_FOUND);

         size_t hasOverflowCount = results.index("2 events lost");
         CPPUNIT_ASSERT(hasOverflowCount != UTL_NOT_FOUND);

         SHOWLOG(log);
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsTimeLogTest);
