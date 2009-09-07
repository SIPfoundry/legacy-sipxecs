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

#include "utl/UtlString.h"

#include <os/OsNameDb.h>
#include <sipxunit/TestUtilities.h>

class OsNameDbTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsNameDbTest);
    CPPUNIT_TEST(testNameDb);
    CPPUNIT_TEST_SUITE_END();


public:
    void testNameDb()
    {
        OsNameDb* pNameDb;

        int storedInt;

        pNameDb = OsNameDb::getNameDb();
        /*
         * Because OsNameDb is a singleton, other tests may have already
         * instantiated it and stored things in it.  So this one can't assume
         * that it was initially empty.
         */
        int startingEntries;
        startingEntries = pNameDb->numEntries();

        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pNameDb->insert("test1", (void*)1));
        CPPUNIT_ASSERT(!pNameDb->isEmpty());
        CPPUNIT_ASSERT_EQUAL(startingEntries+1, pNameDb->numEntries());

        CPPUNIT_ASSERT_EQUAL(OS_NAME_IN_USE, pNameDb->insert("test1", (void*)2));

        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pNameDb->insert("test2", (void*)2));
        CPPUNIT_ASSERT_EQUAL(startingEntries+2, pNameDb->numEntries());

        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pNameDb->lookup("test1", NULL));
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pNameDb->lookup("test1", (void**)&storedInt));
        CPPUNIT_ASSERT_EQUAL(1, storedInt);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pNameDb->lookup("test2", (void**)&storedInt));
        CPPUNIT_ASSERT_EQUAL(2, storedInt);
        CPPUNIT_ASSERT_EQUAL(OS_NOT_FOUND, pNameDb->lookup("test3", NULL));

        pNameDb->remove("test1");
        pNameDb->remove("test2");
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsNameDbTest);
