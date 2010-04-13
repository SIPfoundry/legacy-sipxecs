//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _TestUtilities_h_
#define _TestUtilities_h_

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

// MACROS
#define MAX_BUG_MESSAGE_LEN 1024

/**
 * Assert 2 character strings match character for character, NULLs ok
 */
#define ASSERT_STR_EQUAL(expected, actual) \
    ( TestUtilities::assertEquals((expected), (actual), \
      CPPUNIT_SOURCELINE(), ""))

/**
 * Assert 2 character strings match character for character, NULLs ok
 * and include message to report if they aren't
 */
#define ASSERT_STR_EQUAL_MESSAGE(message, expected, actual) \
    ( TestUtilities::assertEquals((expected), (actual), \
      CPPUNIT_SOURCELINE(), (message)))

/**
 * Call this just before a crash is known to happen. Obvisouly
 * you've attempted to fix the bug, but this is for bugs you cannot
 * fix for a very good reason, in which case you log a bug in JIRA
 * and you enter that bug number here. BUG NUMBER MANDATORY!!!
 *
 * THIS MACRO WILL NOT RETURN, Assertion Exception will be thrown
 * and handled by framework.
 *
 * Example:
 *   // see bug for more details
 *   KNOWN_FATAL_BUG("Segmentation Fault", "XPL-34");
 *   CPPUNIT_ASSERT_EQUAL(3, db->getRowCount());
 *
 */
#define KNOWN_FATAL_BUG(message, bugNo) \
    ( TestUtilities::knownFatalBug((message), (bugNo), \
      CPPUNIT_SOURCELINE()))

/**
 * Call this just before an assertion is known to fail. Obvisouly
 * you've attempted to fix the bug, but this is for bugs you cannot
 * fix for a very good reason, in which case you log a bug in JIRA
 * and you enter that bug number here. BUG NUMBER MANDATORY!!!
 *
 * Example:
 *   KNOWN_BUG("Will be zero until Joe can finish module", "XPL-36");
 *   CPPUNIT_ASSERT_EQUAL(3, db->getRowCount());
 */
#define KNOWN_BUG(message, bugNo) \
    ( TestUtilities::knownBug((message), (bugNo), \
      CPPUNIT_SOURCELINE()))

/**
 * Fatal bugs you only get when running Electric Fence. This will be
 * fatal if efence on, otherwise non-fatal bug. (Experimental)
 */
#define KNOWN_EFENCE_BUG(message, bugNo) \
    ( TestUtilities::knownEfenceBug((message), (bugNo), \
      CPPUNIT_SOURCELINE()))



// FORWARD DECLARATIONS

/**
 * Common utility functions for unittests
 */
class TestUtilities
{
 public:

    /** Create a message using the arguments passed. The message is created by
    *   by appending all the arguments (which have to be of the type char*) starting
    *   off from the 3rd argument.
    *
    *   @param
    *   For example, if this method is called as <br>
    *       <code>createMessage(4, &msg, "Composite of ", "one ", "and two ", " as strings")</code>
    *   this would cause the variable msg to have the following string:-
    *       Composite of one and two as strings
    */
   static void createMessage(int num,             /**< Number of char* messages that have been passed as arguments */
                             std::string* outMsg, /**< string instance into which the generated message is written.
                                                   *   It is the responsibility of the caller to create the string */
                             ...                  /**< variable list arguments of char* which represents the fragments
                                                   *   of the message  */
                             ) ;

    /** Use macro, do not this call directly. */
    static void assertEquals(const char* expected, const char* actual,
            CppUnit::SourceLine sourceLine, const std::string &message);

    /** Use macro, do not this call directly. */
    static bool testingKnownBug();

    /** Last known non-fatal bug messages return here, valid until next test starts */
    static const char *getKnownBugMessage();

    /** Called by unittest framework after exiting a test method */
    static void resetKnownBugTesting();

    /**
     * Prints quotes around strings, work when w/trailing whitespaces
     *  important. Also will print NULL (no quotes) when sz is NULL.
     **/
    static std::string printString(const char* sz);

    /** Use macro, do not this call directly. */
    static void knownFatalBug(const char* message, const char* bugNo,
        CppUnit::SourceLine sourceLine);

    /** Use macro, do not this call directly. */
    static void knownBug(const char* message, const char* bugNo,
        CppUnit::SourceLine sourceLine);

    /** Use macro, do not this call directly. */
    static void knownEfenceBug(const char* message, const char* bugNo,
        CppUnit::SourceLine sourceLine);


 private:

    static std::string printBug(const char* message, const char *bugNo);

    static bool m_testingKnownBug;

    static char m_bugMessage[MAX_BUG_MESSAGE_LEN];

};

#endif
