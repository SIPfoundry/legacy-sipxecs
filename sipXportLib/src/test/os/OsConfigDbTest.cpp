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

#include <os/OsConfigDb.h>
#include <sipxunit/TestUtilities.h>


/**
 * Test OsConfigDb API
 */
class OsConfigDbTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsConfigDbTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testInsert);
    CPPUNIT_TEST(testManipulators);
    CPPUNIT_TEST(testAccessors);
    CPPUNIT_TEST_SUITE_END();

public:

    void testCreators()
    {
        OsConfigDb db;       // create an empty database
        CPPUNIT_ASSERT_MESSAGE("verify that it looks empty", db.isEmpty());
        CPPUNIT_ASSERT_MESSAGE("has zero entries", db.numEntries()==0);
    }

    void testInsert()
    {
        OsConfigDb db;       // create an empty database
        typedef struct
            {
               const char* line;
               const char* name;
               const char* value;
            }
            table_entry;
        table_entry test_table[] =
        {
           { "N1:V1\n", "N1", "V1" },
           { " N2:V2\n", "N2", "V2" },
           { "\tN3:V3\n", "N3", "V3" },
           { "N4 :V4\n", "N4", "V4" },
           { "N5\t:V5\n", "N5", "V5" },
           { "N6: V6\n", "N6", "V6" },
           { "N7:\tV7\n", "N7", "V7" },
           { "N8:V8 \n", "N8", "V8" },
           { "N9:V9\t\n", "N9", "V9" },
           { "Na:Va\r\n", "Na", "Va" },
        };

        for (unsigned int i = 0; i < sizeof (test_table) / sizeof (test_table[0]); i++)
        {
           char msg[1024];
           db.insertEntry(test_table[i].line);
           UtlString k(test_table[i].name);
           UtlString v;
           OsStatus ret = db.get(k, v);
           sprintf(msg,
                   "Setting and retrieving an OsConfigDb entry from line: %s\n"
                   "get() returned %d",
                   test_table[i].line, ret);
           CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, ret, OS_SUCCESS);
           sprintf(msg,
                   "Setting and retrieving an OsConfigDb entry from line: %s\n"
                   "get() returned '%s'",
                   test_table[i].line, v.data());
           CPPUNIT_ASSERT_MESSAGE(msg, v.compareTo(test_table[i].value) == 0);
        }
    }

    void testManipulators()
    {
        OsConfigDb *pDb = new OsConfigDb();
        pDb->set("Key1", "Value1");
        CPPUNIT_ASSERT_MESSAGE("verify that the database is not empty",
                               !pDb->isEmpty());
        CPPUNIT_ASSERT_MESSAGE("has one entry", pDb->numEntries()==1);

        // test the remove() method
        //
        // We put the following block in its own scope so that the UtlString
        // reference (stored in "value") is released as a side effect of going
        // out of scope.  Otherwise, it will look like a memory leak.
        {
            UtlString value;

            pDb->remove("Key1");
            CPPUNIT_ASSERT_MESSAGE("verify that it looks empty", pDb->isEmpty());
            CPPUNIT_ASSERT_MESSAGE("has zero entries", pDb->numEntries()==0);

            pDb->set("Key1", "Value1");   // add the entry back
            pDb->set("Key1", "Value1b");  // change the value for an existing entry
            CPPUNIT_ASSERT_MESSAGE("verify that the database is not empty",
                                   !pDb->isEmpty());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("has one entry", 1, pDb->numEntries());

            OsStatus res = pDb->get("Key1", value);
            CPPUNIT_ASSERT(res == OS_SUCCESS);
            CPPUNIT_ASSERT_MESSAGE("that contains the revised value",
                value.compareTo("Value1b") == 0);

            pDb->set("Key2", "Value2");
            pDb->set("Key3", "Value3");
            pDb->set("Key4", "Value4");
            CPPUNIT_ASSERT_MESSAGE("check the number of entries",
                                   pDb->numEntries()==4);
            value.remove(0);
        }

        // test the storeToFile() method
        pDb->storeToFile("tmpdb");         // store the config db to the file
        delete pDb;                   // delete the database

        // test the loadFromFile() method
        //
        // We put the following block in its own scope so that the UtlString
        // reference (stored in "value") is released as a side effect of going
        // out of scope.  Otherwise, it will look like a memory leak.
        {
            UtlString  value;

            pDb = new OsConfigDb();       // create an empty database
            pDb->loadFromFile("tmpdb");        // load the data from a file
        }

        CPPUNIT_ASSERT_MESSAGE("verify the database is not empty",
                               !pDb->isEmpty());

        CPPUNIT_ASSERT_MESSAGE("has four entries", pDb->numEntries()==4);

        UtlString value;
        OsStatus res = pDb->get("Key1", value);
        CPPUNIT_ASSERT_MESSAGE("contains correct data",
            res == OS_SUCCESS && value.compareTo("Value1b") == 0);

        res = pDb->get("Key2", value);
        CPPUNIT_ASSERT_MESSAGE("contains correct data",
            res == OS_SUCCESS && value.compareTo("Value2") == 0);

        res = pDb->get("Key3", value);
        CPPUNIT_ASSERT_MESSAGE("contains correct data",
            res == OS_SUCCESS && value.compareTo("Value3") == 0);

        res = pDb->get("Key4", value);
        CPPUNIT_ASSERT_MESSAGE("contains correct data",
            res == OS_SUCCESS && value.compareTo("Value4") == 0);

        delete pDb;                   // delete the database
        value.remove(0);
    }


    void testAccessors()
    {
        OsConfigDb *pDb = new OsConfigDb();

        // the get() method is tested by testManipulators()

        // test the getNext() method
        //
        // We put the following block in its own scope so that the UtlString
        // references (stored in "name" and "value") are released as a side effect
        // of going out of scope.  Otherwise, it will look like a memory leak.
        {
            UtlString  name;
            UtlString  value;

            pDb->set("Key3", "Value3");   // add several entries (not in
            pDb->set("Key2", "Value2");   //  alphabetical or reverse alphabetical
            pDb->set("Key4", "Value4");   //  order
            pDb->set("Key1", "Value1");
            CPPUNIT_ASSERT_MESSAGE("verify that the database is not empty",
                                   !pDb->isEmpty());
            CPPUNIT_ASSERT_MESSAGE("has four entries", pDb->numEntries()==4);

            OsStatus res = pDb->getNext("", name, value);      // call getNext("", ...)
            CPPUNIT_ASSERT_MESSAGE("verify that Key1/Value1",
                res == OS_SUCCESS &&
                name.compareTo("Key1") == 0 &&     //  is returned
                value.compareTo("Value1") == 0);

            res = pDb->getNext("Key1", name, value);
            CPPUNIT_ASSERT_MESSAGE("call getNext(\"Key1\", ...)",
                res == OS_SUCCESS &&               //  verify that Key2/Value2
                name.compareTo("Key2") == 0 &&     //  is returned
                value.compareTo("Value2") == 0);

            res = pDb->getNext("Key2", name, value);
            CPPUNIT_ASSERT_MESSAGE("call getNext(\"Key2\", ...)",
                res == OS_SUCCESS &&               //  verify that Key3/Value3
                name.compareTo("Key3") == 0 &&     //  is returned
                value.compareTo("Value3") == 0);

            res = pDb->getNext("Key3", name, value);
            CPPUNIT_ASSERT_MESSAGE("call getNext(\"Key3\", ...)",
                res == OS_SUCCESS &&
                name.compareTo("Key4") == 0 &&
                value.compareTo("Value4") == 0);

            res = pDb->getNext("Key4", name, value);
            CPPUNIT_ASSERT_MESSAGE("call getNext(\"Key4\", ...)",
                 res == OS_NO_MORE_DATA &&
                 name.compareTo("") == 0 &&
                 value.compareTo("") == 0);

            res = pDb->getNext("XXX", name, value);
            CPPUNIT_ASSERT_MESSAGE("call getNext with a key not in the database and verify",
                res == OS_NOT_FOUND &&
                name.compareTo("") == 0 &&      //  that empty strings are
               value.compareTo("") == 0);       //  returned for the next name
                                                //  and value pair.

            delete pDb;                               // delete the database
            name.remove(0);
            value.remove(0);
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsConfigDbTest);
