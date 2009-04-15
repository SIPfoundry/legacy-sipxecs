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

#include <cgicc/Cgicc.h>
#include <cgicc/CgiInput.h>
#include <os/OsFS.h>
#include <sipxcgi/CgiValues.h>
#include <MockCgiInput.h>


/**
 * Test class the adds a convience layer to CgiCC library.
 */
class CgiValuesTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(CgiValuesTest);
    CPPUNIT_TEST(testGetValue);
    CPPUNIT_TEST_SUITE_END();

private:
    
public:

    void setUp()
    {
    }

    void testGetValue()
    {
        MockCgiInput input((char*)"CgiValuesTestInput.txt");
        cgicc::Cgicc cgi(&input);
        CgiValues values(&cgi);
            
        ASSERT_STR_EQUAL_MESSAGE("Expected form value", "B", values.valueOf("A"));
        CPPUNIT_ASSERT_MESSAGE("Expected missing value", NULL == values.valueOf("NotInForm"));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CgiValuesTest);
