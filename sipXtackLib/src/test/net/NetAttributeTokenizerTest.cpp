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

#include <net/NetAttributeTokenizer.h>

/**
 * Unittest for NetAttributeTokenizer
 */
class NetAttributeTokenizerTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(NetAttributeTokenizerTest);
    CPPUNIT_TEST(testManipulators);
    CPPUNIT_TEST_SUITE_END();


public:

    void testManipulators()
    {
       NetAttributeTokenizer a("Basic realm = \"/root/subdir\", ddd=\"no end quote");
       UtlString name;
       UtlString value;

       a.getNextAttribute(name, value);
       CPPUNIT_ASSERT_MESSAGE("no value given", value.isNull());
       CPPUNIT_ASSERT_MESSAGE("first key", name.compareTo("Basic") == 0);

       a.getNextAttribute(name, value);
       CPPUNIT_ASSERT_MESSAGE("escaped value test 1", value.compareTo("/root/subdir") == 0);
       CPPUNIT_ASSERT_MESSAGE("2nd key", name.compareTo("realm") == 0);

       a.getNextAttribute(name, value);
       CPPUNIT_ASSERT_MESSAGE("graceful handle of no end quote", value.compareTo("no end quote") == 0);
       CPPUNIT_ASSERT_MESSAGE("3rd key", name.compareTo("ddd") == 0);

       NetAttributeTokenizer b("ab =\"bunch\\\' \\\" of escaped \\\\\\\' quotes\\\\\"");
       b.getNextAttribute(name, value);
       CPPUNIT_ASSERT_MESSAGE("parade of toothpicks", value.compareTo("bunch\\\' \\\" of escaped \\\\\\\' quotes\\\\") == 0);
       CPPUNIT_ASSERT_MESSAGE("toothpicks's key value", name.compareTo("ab") == 0);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(NetAttributeTokenizerTest);
