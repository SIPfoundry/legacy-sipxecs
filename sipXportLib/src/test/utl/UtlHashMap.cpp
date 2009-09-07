//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <cstdarg>
#include <time.h>

#include <utl/UtlString.h>
#include <utl/UtlInt.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlContainableTestStub.h>
#include <sipxunit/TestUtilities.h>

using namespace std ;

#define commonHashMapEntriesCount 4

static const char* regularString = "This makes sense" ;

static const char* longAlphaNumString = \
"abcdefghijklmnopqrstuvwzyz"
"abcdefghijklmnopqrstuvwzyz"
"abcdefghijklmnopqrstuvwzyz"
"abcdefghijklmnopqrstuvwzyz"
"abcdefghijklmnopqrstuvwzyz"
"abcdefghijklmnopqrstuvwzyz"
"abcdefghijklmnopqrstuvwzyz"
"abcdefghijklmnopqrstuvwzyz"
"abcdefghijklmnopqrstuvwzyz"
"abcdefghijklmnopqrstuvw" ;

class UtlHashMapTests : public CppUnit::TestCase
{

    CPPUNIT_TEST_SUITE(UtlHashMapTests);
    CPPUNIT_TEST(checkSanity_Insert_Entries_And_At) ;
//    CPPUNIT_TEST(DynaTest) ;
    CPPUNIT_TEST(testInsertKeyAndValue) ;
    CPPUNIT_TEST(testInsert) ;
    CPPUNIT_TEST(testFind) ;
    CPPUNIT_TEST(testContains) ;
    CPPUNIT_TEST(testRemove) ;
    CPPUNIT_TEST(testRemoveKeyAndValue) ;
    CPPUNIT_TEST(testRemoveAndDestroy) ;
    CPPUNIT_TEST(testIsEmpty) ;
    CPPUNIT_TEST(testClear) ;
    CPPUNIT_TEST(testClearAndDestroy) ;
    CPPUNIT_TEST(testCopyInto) ;
    CPPUNIT_TEST(testRemoveCollision) ;
    CPPUNIT_TEST(testOneThousandInserts) ;
    CPPUNIT_TEST_SUITE_END();

private:

    static const int INDEX_NOT_EXIST;
      //const int UtlHashMapTests::commonHashMapEntriesCount = 4 ;
      //static const int commonHashMapEntriesCount ;

    UtlHashMap commonList ;
    UtlHashMap commonList_clone ;
    UtlHashMap emptyList ;

    UtlString commonString1 ;
    UtlString commonString2 ;
    UtlString commonString3 ;
    UtlString commonString4 ;

    UtlInt commonInt1 ;
    UtlInt commonInt2 ;
    UtlInt commonInt3 ;
    UtlInt commonInt4 ;

    UtlString commonString1_clone;
    UtlString commonString2_clone ;
    UtlString commonString3_clone ;
    UtlString commonString4_clone ;

    UtlInt commonInt1_clone ;
    UtlInt commonInt2_clone ;
    UtlInt commonInt3_clone ;
    UtlInt commonInt4_clone ;

    UtlContainable* commonKeyValuePairs[commonHashMapEntriesCount][2] ;
    UtlContainable* commonKeyValuePairs_clone[commonHashMapEntriesCount][2] ;

    enum FindOrContains { TEST_FIND, TEST_CONTAINS} ;
    enum TestInsertOrAppend {TEST_APPEND, TEST_INSERT} ;
    enum RemoveType {TEST_REMOVE, TEST_REMOVE_KEY_AND_VALUE} ;

public:

    void setUp()
    {
        commonString1 = UtlString(regularString) ;
        commonString1_clone = UtlString(regularString) ;
        commonString2 = UtlString("") ;
        commonString2_clone = UtlString("") ;
        commonString3 = UtlString(longAlphaNumString) ;
        commonString3_clone = UtlString(longAlphaNumString) ;
        commonString4 = UtlString("Another String") ;
        commonString4_clone = UtlString("Another String") ;

        commonInt1 = UtlInt(0) ;
        commonInt1_clone = UtlInt(0) ;
        commonInt2 = UtlInt(INT_MAX) ;
        commonInt2_clone = UtlInt(INT_MAX) ;
        commonInt3 = UtlInt(INT_MIN) ;
        commonInt3_clone = UtlInt(INT_MIN) ;
        commonInt4 = UtlInt(101) ;
        commonInt4_clone = UtlInt(101) ;

        // Insert 3 name value pairs into the commonList :-
        //   a) Key = String, Value = Int
        //   b) Key = Int, Value = String:
        //   c) Key = String, Value = String
        commonList.insertKeyAndValue(&commonString1, &commonInt1) ;
        commonList.insertKeyAndValue(&commonInt2, &commonString2) ;
        commonList.insertKeyAndValue(&commonString3, &commonString4) ;
        commonList.insertKeyAndValue(&commonInt3, &commonInt4) ;

        // The above values also become the name-value pair array
        // The first index of the 2nd dimension represents the key
        // while the second index represents the value expected.
        commonKeyValuePairs[0][0] = &commonString1 ;
        commonKeyValuePairs[0][1] = &commonInt1 ;

        commonKeyValuePairs[1][0] = &commonInt2 ;
        commonKeyValuePairs[1][1] = &commonString2 ;

        commonKeyValuePairs[2][0] = &commonString3 ;
        commonKeyValuePairs[2][1] = &commonString4 ;

        commonKeyValuePairs[3][0] = &commonInt3 ;
        commonKeyValuePairs[3][1] = &commonInt4 ;

        // Similarly create a clone of the list and the expected data.
        commonList_clone.insertKeyAndValue(&commonString1_clone, &commonInt1_clone) ;
        commonList_clone.insertKeyAndValue(&commonInt2_clone, &commonString2_clone) ;
        commonList_clone.insertKeyAndValue(&commonString3_clone, &commonString4_clone) ;
        commonList_clone.insertKeyAndValue(&commonInt3_clone, &commonInt4_clone) ;

        // The above values also become the name-value pair array
        // The first index of the 2nd dimension represents the key
        // while the second index represents the value expected.
        commonKeyValuePairs_clone[0][0] = &commonString1_clone ;
        commonKeyValuePairs_clone[0][1] = &commonInt1_clone ;

        commonKeyValuePairs_clone[1][0] = &commonInt2_clone ;
        commonKeyValuePairs_clone[1][1] = &commonString2_clone ;

        commonKeyValuePairs_clone[2][0] = &commonString3_clone ;
        commonKeyValuePairs_clone[2][1] = &commonString4_clone ;

        commonKeyValuePairs_clone[3][0] = &commonInt3_clone ;
        commonKeyValuePairs_clone[3][1] = &commonInt4_clone ;
    }

    void tearDown()
    {
    }

    /* Sandbox - please ignore
    */
    void DynaTest()
    {
    }

    ~UtlHashMapTests()
    {
    }

    void testCopyInto()
    {
        UtlString testData[] = {
            UtlString("key1"),  UtlString("value1")
        };

        UtlHashMap *source = new UtlHashMap();
        source->insertKeyAndValue(&testData[0], &testData[1]);
        UtlHashMap *destination = new UtlHashMap();
        source->copyInto(*destination);

        // elements should live on in copy
        delete source;

        CPPUNIT_ASSERT_MESSAGE("Copy received", 1 == destination->entries());
        UtlContainable *key = destination->find(&testData[0]);
        CPPUNIT_ASSERT_MESSAGE("Found key in copy", key != NULL);
        CPPUNIT_ASSERT_MESSAGE("Found expected key in copy", key->isEqual(&testData[0]));

        UtlContainable *value = destination->findValue(&testData[0]);
        CPPUNIT_ASSERT_MESSAGE("Found key in copy", value != NULL);
        CPPUNIT_ASSERT_MESSAGE("Found expected value in copy", value->isEqual(&testData[1]));

        delete destination;

    }

    /*a! This test is more of a sanity check to verify that
    *    the basic insert(), entries() and at() methods work as expected.
    *    All future tests will depend heavily on the at() method
    *    and the most common way of having something in the list is
    *    by means of the insert() method.
    *
    */
    void checkSanity_Insert_Entries_And_At()
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Verify that the entries() for an empty HashDictionary returns 0", (int)emptyList.entries(), 0) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Verify the entries() method for a HashDictionary", commonHashMapEntriesCount, (int)commonList.entries()) ;
    }// checkSanity_Append_And_At()


    /*a! Test the insert method for a list that is not empty. Add both unique and non-unique entries.
    *
    *    The test data for this test is :-
    *      a) Add a new unique pair
    *      b) Add a new pair such that such a key doesn't exist but the table has a value that is equal to the key
    *      b) Add a new Containable such that a similar one (value match) exists already
    *      c) Add a new Containable such that the same one (reference match) exists already
    */
    void testInsertKeyAndValue()
    {
        int testCount = 7;
        const char* prefix = "Test the insert(UtlContainable*) method for a " \
            "non empty HashDictionary " ;

        const char* Msgs[] = { \
                "Add a new unique pair ", \
                "Add a new pair such that the key is unique, but another key has " \
                  "the pair's value ", \
                "Add a new pair such that there is no match for that key but the table " \
                  "has a value that is equal to the key ", \
                "Add a new pair such that the table has an existing key which is equal " \
                  "to a the key (but not same) ", \
                "Add a new pair such that an exact duplicate of the key " \
                  "(including reference match) exists in the table ", \
                "Add a new pair such that newKey.isEqual(oldKey) and " \
                  "newValue.isEqual(oldValue) for an existing key-value pair", \
                "Add a new pair such that that key-value pair is an exact duplicate " \
                  "of an existing pair" \
        } ;
        const char* suffix1 = " :- Verify the return value for the insert method" ;
        const char* suffix2 = " :- Verify the return value for the findKeyAndValue() method " ;
        const char* suffix3 = " :- Verify that the value is inserted and found by the " \
            "findKeyAndValue() method" ;
        const char* suffix4 = " :- Verify that the number of entries is incremented by one" ;

        UtlInt uInt = UtlInt(1234) ;
        UtlString uString = UtlString("Test String") ;

        UtlString strUniqueKey = UtlString("Unique_Key_For_Existing_Value") ;
        UtlString strUniqueValue = UtlString("Unique_Value_For_Existing_Key") ;
        UtlString strUniqueValue2 = UtlString("Another Unique Value") ;

        UtlContainable* ItemsToAdd[][2] = { \
                           {&uInt, &uString} , \
                           {&strUniqueKey, commonKeyValuePairs[0][1]}, \
                           {commonKeyValuePairs[1][1], &strUniqueValue},\
                           {commonKeyValuePairs_clone[2][0], &strUniqueValue2}, \
                           {commonKeyValuePairs[3][0], &uString}, \
                           {commonKeyValuePairs_clone[3][0], commonKeyValuePairs_clone[3][1]}, \
                           {&uInt, &uString} \
        } ;
        UtlContainable* ExpReturn_Key[] = { \
                          &uInt, \
                          &strUniqueKey, \
                          commonKeyValuePairs[1][1], \
                          NULL, \
                          NULL, \
                          NULL, \
                          NULL, \
        } ;
        UtlContainable* ExpFind_Return_Key[] = { \
                          &uInt, \
                          &strUniqueKey, \
                          commonKeyValuePairs[1][1], \
                          commonKeyValuePairs[2][0], \
                          commonKeyValuePairs[3][0], \
                          commonKeyValuePairs[3][0], \
                          &uInt \
        } ;
        UtlContainable* ExpFind_Value[] = { \
                          &uString, \
                          commonKeyValuePairs[0][1], \
                          &strUniqueValue, \
                          commonKeyValuePairs[2][1], \
                          commonKeyValuePairs[3][1], \
                          commonKeyValuePairs[3][1], \
                          &uString \
        } ;

        int expCount  = commonHashMapEntriesCount;

        string msg ;
        UtlContainable* uReturn_Key ;
        UtlContainable* uFound_Key ;
        UtlContainable* uFound_Value ;

        UtlContainable* uExp_Return_Key; //What is the expected return  when insert.. is called ?
        UtlContainable* uExp_Find_Return_Key; // What is the expected return  find... is called ?
        UtlContainable* uExp_Value ;  // What is value part for a name-value pair find ?

        for (int i = 0 ; i < testCount ; i++)
        {
            uExp_Return_Key = ExpReturn_Key[i];
            uExp_Find_Return_Key = ExpFind_Return_Key[i] ;
            uExp_Value = ExpFind_Value[i] ;

            uReturn_Key = commonList.insertKeyAndValue(ItemsToAdd[i][0], ItemsToAdd[i][1]) ;
            uFound_Key = commonList.find(ItemsToAdd[i][0]) ;
            uFound_Value = commonList.findValue(ItemsToAdd[i][0]) ;

            if (uExp_Return_Key != NULL)
            {
                expCount++ ;
            }

            // Verify that the correct value for the key was returned.
            TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix1) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), uExp_Return_Key, uReturn_Key) ;

            // Verify that the return value of the find returns the
            // correcy key
            TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix2) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), uExp_Find_Return_Key, uFound_Key) ;

            // verify that find() found the right value.
            TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix3) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), uExp_Value, uFound_Value) ;

            // Verify that the number of entries is accurate.
            TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix4) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expCount, (int)commonList.entries()) ;
        }

    } //testInsertKeyAndValue


    /*a! Test the insert method for a list that is not empty. Add both unique and non-unique entries.
    *
    *    The test data for this test is :-
    *      a) Add a new unique pair
    *      b) Add a new Containable such that a similar one (value match) exists already
    *      c) Add a new Containable such that the same one (reference match) exists already
    */
    void testInsert()
    {
        struct TestInsertKeyStruct
        {
            const char* testDescription ;
            UtlContainable* insertKey ;
            UtlContainable* expectedReturnKey ;
            UtlContainable* expectedSearchKey ;
            UtlContainable* expectedSearchValue ;
        } ;
        const char* prefix = "Test the insert(UtlContainable*) method for a " \
            "non empty HashDictionary " ;
        const char* suffix1 = " :- Verify the return value for the insert method" ;
        const char* suffix2 = " :- Verify the key returned by the find() method " ;
        const char* suffix3 = " :- Verify the value found by the findValue() method" ;
        string Message ;

        UtlString strUniqueKey = UtlString("Unique_Key") ;

        TestInsertKeyStruct testData[] = { \
            { "Add a new unique pair ", &strUniqueKey, &strUniqueKey, \
              &strUniqueKey, NULL }, \
            { "Add a new pair such that the table has an existing key which is equal " \
              "to a the key (but not same) ", commonKeyValuePairs_clone[1][0], NULL, \
              commonKeyValuePairs[1][0], commonKeyValuePairs[1][1] }, \
            { "Add a new pair such that an exact duplicate of the key " \
              "(including reference match) exists in the table ", commonKeyValuePairs[0][0], \
              NULL, commonKeyValuePairs[0][0], commonKeyValuePairs[0][1] } \
        } ;
        int testCount = sizeof(testData)/sizeof(testData[0]) ;
        for (int i = 0 ; i < testCount; i++)
        {
            UtlContainable* actualReturnKey = commonList.insert(testData[i].insertKey) ;
            UtlContainable* actualSearchKey = commonList.find(testData[i].insertKey) ;
            UtlContainable* actualSearchValue = commonList.findValue(testData[i].insertKey) ;

            TestUtilities::createMessage(3, &Message, prefix, testData[i].testDescription, \
                suffix1) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), testData[i].expectedReturnKey, \
                actualReturnKey) ;

            TestUtilities::createMessage(3, &Message, prefix, testData[i].testDescription, \
                suffix2) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), testData[i].expectedSearchKey, \
                actualSearchKey) ;

            TestUtilities::createMessage(3, &Message, prefix, testData[i].testDescription, \
                suffix3) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), testData[i].expectedSearchValue, \
                actualSearchValue) ;
        }
    }//testInsert()


    /*!a Test case for the find() method.
    *
    *    The test data for this test case are:-
    *        a) is a Containable String
    *        b) is ta UtlInt and the HashDictionary has this key
    *        c) is equal to(not the same) a key in the HashDictionary
    *        d)is equal to a value in the HashDictionary but the dictionary has no such key
    *        e) has no match at all
    */
    void testFind()
    {
        utlTestFind_Or_Contains(TEST_FIND) ;
    }

    /*!a Test case for the contains() method
    *
    *    The test data for this test case are the same as
    *    testFind() test case.
    */
    void testContains()
    {
        utlTestFind_Or_Contains(TEST_CONTAINS) ;
    }


    // Utility that can be used to either test the find or
    // contains methods.
    void utlTestFind_Or_Contains(FindOrContains type)
    {
        const int testCount = 5 ;
        const char* prefixFind = "Test the find() method when the search key " ;
        const char* prefixFindValue = "Test the findValue() method when the search key " ;
        const char* prefixContains = "Test the contains() method when the search key " ;
        const char* Msgs[] = { \
               "is a UtlString and the HashDictionary has this key ", \
               "is a UtlInt and the HashDictionary has this key ", \
               "is equal to(not the same) a key in the HashDictionary ", \
               "is equal to a value in the HashDictionary but the dictionary has no such key ", \
               "has no match at all" \
        } ;

       UtlString noExist("This cannot and should not exist!!!") ;

       UtlContainable* searchValues[] = { \
                 commonKeyValuePairs[0][0], \
                 commonKeyValuePairs[1][0], \
                 commonKeyValuePairs_clone[2][0], \
                 commonKeyValuePairs[3][1], \
                 &noExist \
       } ;

       bool expectedValues_Contains[]    = { \
                  true, true, true, false, false \
       } ;

       UtlContainable* expected_FindKeys[] = { \
                  commonKeyValuePairs[0][0], \
                  commonKeyValuePairs[1][0], \
                  commonKeyValuePairs[2][0], \
                  NULL, \
                  NULL \
       } ;

       UtlContainable* expected_FindValues[] = { \
                  commonKeyValuePairs[0][1], \
                  commonKeyValuePairs[1][1], \
                  commonKeyValuePairs[2][1], \
                  NULL, \
                  NULL \
       } ;

       for (int i = 0 ; i < testCount ; i++)
       {
           string msg ;
           if (type == TEST_FIND)
           {
               UtlContainable* foundKey = commonList.find(searchValues[i]) ;
               UtlContainable* foundValue = commonList.findValue(searchValues[i]) ;

               TestUtilities::createMessage(2, &msg, prefixFind, Msgs[i]) ;
               CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*) expected_FindKeys[i], (void*)foundKey) ;

               TestUtilities::createMessage(2, &msg, prefixFindValue, Msgs[i]) ;
               CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)expected_FindValues[i], (void*)foundValue) ;
           }
           else if (type == TEST_CONTAINS)
           {
               bool act = (TRUE == commonList.contains(searchValues[i])) ;
               bool exp = expectedValues_Contains[i] ;
               TestUtilities::createMessage(2, &msg, prefixContains, Msgs[i]) ;
               CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), exp, act) ;
           }
       }
    }//utlTestFindOrContains

    /*!a Test case to test the destroy()
    *    method.
    */
    void testRemoveAndDestroy()
    {
        const char* prefix  = "test the destroy() method " ;
        const char* suffix1 = ":- Verify the return value" ;
        const char* suffix2 = ":- Verify that the object was deleted" ;
        const char* suffix3 = ":- Verify that the entry is removed" ;
        const char* suffix4 = ":- Verify number of entries" ;

        UtlContainableTestStub uStub(0) ;
        UtlContainableTestStub uStubValue(1) ;

        UtlContainableTestStub* uStubPtr ;
        UtlContainableTestStub* uStubPtrValue ;

        uStubPtr = new UtlContainableTestStub(101) ;
        uStubPtrValue = new UtlContainableTestStub(202) ;
        commonList.insertKeyAndValue(&uStub, &uStubValue) ;
        commonList.insertKeyAndValue(uStubPtr, uStubPtrValue) ;
        int cCountBefore = UtlContainableTestStub :: getCount() ;
        string msg ;

        bool retValue = (TRUE == commonList.destroy(uStubPtr)) ;
        TestUtilities::createMessage(2, &msg, prefix, suffix1) ;
        CPPUNIT_ASSERT_MESSAGE(msg.data(), retValue) ;

        // The ContainableTestStub has been implemented such that a static
        // counter is decremented for every instance destruction. Since the
        // Hashtable has set of name-value pairs, calling the removeAndDestroy
        // should destroy both the key and the value; thus the count goes down
        // by two .
        int cCountAfter = UtlContainableTestStub :: getCount() ;
        TestUtilities::createMessage(2, &msg, prefix, suffix2) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), cCountBefore -2, cCountAfter) ;

        // THe way to verify if the object has been removed is to
        // create a new stub . This stub will now have the same value as
        // the deleted stub. Try to find the new stub
        UtlContainableTestStub uStubNew(101) ;
        UtlContainable* uSearch = commonList.find(&uStubNew) ;
        TestUtilities::createMessage(2, &msg, prefix, suffix3) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)NULL, (void*)uSearch) ;

        // In all the above tests, a total of 2 entries were added
       // and 1 wes removed.
        int finalCount = commonHashMapEntriesCount + 1 ;
        TestUtilities::createMessage(2, &msg, prefix, suffix4) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), finalCount, (int)commonList.entries()) ;

    }// testRemoveAndDestroy


    /*!a Test case for testRemove()
    *
    *     The Test data for this test case is
    *          a) Key is a Containable String
    *          b) Key is a Containable Int
    *          c) Key has a value match
    *          d) has no match at all
    */
    void testRemove()
    {
        utlTestRemove(TEST_REMOVE) ;
    }

    /** This test case verfies the removeKeyAndValue() method.
    *
    *    The test data for this case is the same as the testRemove()
    *    method
    */
    void testRemoveKeyAndValue()
    {
        utlTestRemove(TEST_REMOVE_KEY_AND_VALUE) ;
    }

    /** Utility method that is called by the testRemove() and
    *   testRemoveKeyAndValue() methods. Based on the argument
    *   passed as 'type', one of these methods is tested.
    */
    void utlTestRemove(RemoveType type)
    {
        const int testCount = 4;
        const char* prefix = "";

        if (type == TEST_REMOVE)
        {
            prefix = "Test the remove(UtlContainable* c) method where " ;
        }
        else if (type== TEST_REMOVE_KEY_AND_VALUE)
        {
            prefix = "Test the removeKeyAndValue(UtlContainable* c) method where " ;
        }
        const char* Msgs[] = { \
               "c is a Containable String that exists as a key in the Dictionary ", \
               "c is a Containable Int that exists as a key in the Dictionary ", \
               "a key that is equal to c exists in the Dictionary ", \
               "has no match at all " \
        } ;
        const char* suffix1 = " :- Verify returned key" ;
        const char* suffix2 = " :- Verify total entries" ;
        const char* suffix3 = " :- Verify returned value" ;

        UtlString noExist("This cannot and willnot exist");
        UtlContainable* testData_Remove[] = { \
                 commonKeyValuePairs[0][0], \
                 commonKeyValuePairs[1][0], \
                 commonKeyValuePairs_clone[2][0], \
                 &noExist \
         } ;
        UtlContainable* exp_ReturnKey[] = { \
                 commonKeyValuePairs[0][0], \
                 commonKeyValuePairs[1][0], \
                 commonKeyValuePairs[2][0], \
                 NULL \
        };
        UtlContainable* exp_ReturnValue[] = { \
                 commonKeyValuePairs[0][1], \
                 commonKeyValuePairs[1][1], \
                 commonKeyValuePairs[2][1], \
                 NULL \
        };

        int totalEnt = commonHashMapEntriesCount;
        int entriesValue[] = { --totalEnt, --totalEnt, --totalEnt, totalEnt } ;

        for (int i = 0 ; i < testCount ; i++)
        {
            string msg ;
            // Verify that remove() returns the correct value.
            TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix1) ;
            if (type == TEST_REMOVE)
            {

               UtlContainable* foundKey = commonList.remove(testData_Remove[i]) ;
               UtlContainable* expectedKey = exp_ReturnKey[i] ;
               CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)foundKey, (void*)expectedKey) ;
            }
            else if(type == TEST_REMOVE_KEY_AND_VALUE)
            {
                UtlContainable* expectedKey = exp_ReturnKey[i] ;
                UtlContainable* expectedValue = exp_ReturnValue[i] ;
                UtlContainable* foundValue ;
                UtlContainable* foundKey = commonList.removeKeyAndValue(testData_Remove[i], foundValue) ;

                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)expectedKey, (void*)foundKey) ;
                TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix3) ;
                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)expectedValue, (void*)foundValue) ;
            }

            // Verify the number of entries left in the hashdictionary after remove()
            TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix2) ;
            int expCount = (int)commonList.entries() ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), entriesValue[i], expCount) ;
        }
    } //testRemove


    /*!a  Test case for the isEmpty() method.
    *     The test data for this test are :-
    *        a) When the list has just been created.
    *        b) When the list has one entry in it
    *        c) When the list has multiple entries in it.
    *        e) When all the entries in a list have been removed using removeAll()
    */
    void testIsEmpty()
    {
        const int testCount = 4 ;
        const char* prefix = "Test the isEmpty() method when " ;
        const char* Msgs[] = { \
                "the list has just been created" , \
                "the list has just one entry in it", \
                "the list has multiple entries in it", \
                "all the list entries have been retreived using removeAll()" \
        } ;

        UtlHashMap newList ;
        UtlHashMap secondNewList ;

        // Add a single entry to the list.
        newList.insertKeyAndValue(&commonString1, &commonInt1)  ;

        // populate the second list and then clear all entries.
        UtlString uS1("Tester string") ;
        UtlInt uI1 = UtlInt(232) ;
        secondNewList.insertKeyAndValue(&uS1, &commonInt2) ;
        secondNewList.insertKeyAndValue(&uI1, &commonString2) ;
        secondNewList.insertKeyAndValue(&commonString3, &commonInt3) ;
        secondNewList.removeAll() ;

        UtlHashMap* testLists[] = { \
                 &emptyList, &newList, &commonList,  &secondNewList \
        } ;
        bool expected_IsEmpty[] = { true, false, false, true } ;
        for (int i = 0 ; i < testCount; i++)
        {
            string msg ;
            TestUtilities::createMessage(2, &msg, prefix, Msgs[i]) ;
            bool act_isEmpty = (TRUE == testLists[i] -> isEmpty()) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expected_IsEmpty[i], act_isEmpty) ;
        }
    } // testIsEmpty

    /*!a test the removeAll() method.
    *
    *    The test data for this method is
    *        a) When the list is empty
    *        b) When the list has one entry.
    *        c) When the list multiple entries
    *        d) When clear has been called and entries are added again
    *        e) When the clear is called twice on the list.
    *        f) When the clear is call on a list that has muliple entries
    *           for the same key.
    */
    void testClear()
    {
        const int testCount = 6 ;
        const char* prefix = "Test the removeAll() method when :- " ;
        const char* Msgs[] = { \
               "the list is empty", \
               "the list has one entry", \
               "the list has multiple entries", \
               " has been called and entries are added again", \
               "removeAll() has already been called", \
               "removeAll() is called on list that has multiple matches" \
        } ;
        const char* suffix = " :- Verify number of entries after removeAll()" ;
        UtlHashMap uSingleList ;
        UtlHashMap uAddAfterClear ;
        UtlHashMap uDoubleClear ;

        uSingleList.insertKeyAndValue(&commonString1, &commonInt1) ;

        // call removeAll() on a list and then add entries again.
        uAddAfterClear.insertKeyAndValue(&commonInt1, &commonString2) ;
        uAddAfterClear.insertKeyAndValue(&commonString1, &commonInt3) ;
        uAddAfterClear.removeAll() ;
        uAddAfterClear.insertKeyAndValue(&commonInt2, &commonInt4) ;

        // call clear on a list twice.
        uDoubleClear.insertKeyAndValue(&commonString3, &commonString1) ;
        uDoubleClear.insertKeyAndValue(&commonInt3, &commonString2) ;
        uDoubleClear.removeAll() ;

        UtlHashMap* testLists[] = { \
                     &emptyList, &uSingleList, &commonList, &uAddAfterClear, &uDoubleClear, &commonList_clone
        } ;
        int expected_Entries[] = {0 , 0, 0, 1, 0, 0} ;

        // since we are not calling clear for all the data, do it outside the for loop.
        emptyList.removeAll() ;
        uSingleList.removeAll() ;
        commonList.removeAll() ;
        commonList_clone.removeAll() ;
        // no removeAll() for uAddAfterClear
        uDoubleClear.removeAll() ;

        for ( int i = 0 ; i < testCount ; i++)
        {
            string msg ;
            TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data() , expected_Entries[i], (int)testLists[i] -> entries()) ;
        }

    } //testClear()

    /*!a Test case to test the destroyAll()
    *    method.
    */
    void testClearAndDestroy()
    {
        const char* prefix  = "Test the destroyAll() method " ;
        const char* suffix = ":- Verify that all entries are removed and the objects are deleted" ;
        int cCountBefore ;

        UtlContainableTestStub* uStub ;
        UtlContainableTestStub* uStub2 ;
        UtlContainableTestStub* uStubPtr ;
        UtlContainableTestStub* uStubPtr2 ;
        uStub = new UtlContainableTestStub(101) ;
        uStub2 = new UtlContainableTestStub(152) ;
        uStubPtr = new UtlContainableTestStub(202) ;
        uStubPtr2 = new UtlContainableTestStub(252) ;

        emptyList.insertKeyAndValue(uStub, uStub2) ;
        emptyList.insertKeyAndValue(uStubPtr, uStubPtr2) ;

        // The ContainableStub is implemented such that a static
        // counter is decremented for every call to the destrcutor.
        // Since four instances (2 sets of n-v pairs) were destroyed,
        // we should verify that he count goes down by 4.
        cCountBefore = UtlContainableTestStub :: getCount() ;
        emptyList.destroyAll() ;
        int cCountAfter = UtlContainableTestStub :: getCount() ;
        string msg ;

        TestUtilities::createMessage(2, &msg, prefix, suffix) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), 0, cCountAfter) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), cCountBefore -  4, (int)emptyList.entries()) ;
    } //testClearAndDestroy

   void testRemoveCollision()
      {
         // the following two entries collide if the initial bucket size is 16
         UtlInt int1(1);
         UtlInt int16(16);

         UtlInt int2(2);
         UtlInt int3(3);

         UtlHashMap map;

         CPPUNIT_ASSERT( map.numberOfBuckets() == 16 ); // check assumption of collision

         // Load all the test objects
         CPPUNIT_ASSERT( map.insert(&int1)  == &int1 );
         CPPUNIT_ASSERT( map.insert(&int2)  == &int2 );
         CPPUNIT_ASSERT( map.insert(&int3)  == &int3 );
         CPPUNIT_ASSERT( map.insert(&int16) == &int16 );

         // Check that everything is there
         CPPUNIT_ASSERT( map.entries() == 4 );
         CPPUNIT_ASSERT( map.contains(&int1) );
         CPPUNIT_ASSERT( map.contains(&int2) );
         CPPUNIT_ASSERT( map.contains(&int3) );
         CPPUNIT_ASSERT( map.contains(&int16) );

         // Take entry 1 out (will collide w/ 16)
         CPPUNIT_ASSERT( map.removeReference(&int1) == &int1 );

         // Check that everything except entry 1 is still there, and that 1 is gone
         CPPUNIT_ASSERT( map.entries() == 3 );
         CPPUNIT_ASSERT( ! map.contains(&int1) );
         CPPUNIT_ASSERT( map.contains(&int2) );
         CPPUNIT_ASSERT( map.contains(&int3) );
         CPPUNIT_ASSERT( map.contains(&int16) );

         // Put entry 1 back in (so that 16 will collide w/ it again)
         CPPUNIT_ASSERT( map.insert(&int1) == &int1 );

         // Check that everything is there
         CPPUNIT_ASSERT( map.entries() == 4 );
         CPPUNIT_ASSERT( map.contains(&int1) );
         CPPUNIT_ASSERT( map.contains(&int2) );
         CPPUNIT_ASSERT( map.contains(&int3) );
         CPPUNIT_ASSERT( map.contains(&int16) );

         // Take entry 16 out (will collide w/ 1)
         CPPUNIT_ASSERT( map.removeReference(&int16) == &int16 );

         // Check that everything except entry 16 is still there, and that 16 is gone
         CPPUNIT_ASSERT( map.entries() == 3 );
         CPPUNIT_ASSERT( map.contains(&int1) );
         CPPUNIT_ASSERT( map.contains(&int2) );
         CPPUNIT_ASSERT( map.contains(&int3) );
         CPPUNIT_ASSERT( ! map.contains(&int16) );

         CPPUNIT_ASSERT( map.removeReference(&int2) == &int2 );

         // Check that everything that should be is still there
         CPPUNIT_ASSERT( map.entries() == 2 );
         CPPUNIT_ASSERT( map.contains(&int1) );
         CPPUNIT_ASSERT( ! map.contains(&int2) );
         CPPUNIT_ASSERT( map.contains(&int3) );
         CPPUNIT_ASSERT( ! map.contains(&int16) );

         // remove 3 (no collision for this one)
         CPPUNIT_ASSERT( map.removeReference(&int3) == &int3 );

         // Check that everything that should be is still there
         CPPUNIT_ASSERT( map.entries() == 1 );
         CPPUNIT_ASSERT( map.contains(&int1) );
         CPPUNIT_ASSERT( ! map.contains(&int2) );
         CPPUNIT_ASSERT( ! map.contains(&int3) );
         CPPUNIT_ASSERT( ! map.contains(&int16) );

         // remove 3 again - should fail this time
         CPPUNIT_ASSERT( map.removeReference(&int3) == NULL );

         // Check that everything that should be is still there
         CPPUNIT_ASSERT( map.entries() == 1 );
         CPPUNIT_ASSERT( map.contains(&int1) );
         CPPUNIT_ASSERT( ! map.contains(&int2) );
         CPPUNIT_ASSERT( ! map.contains(&int3) );
         CPPUNIT_ASSERT( ! map.contains(&int16) );
      }

   void testOneThousandInserts()
      {
         // Test case used to validate fix to issue XPL-169
         const int COUNT = 1000;
         const char stringPrefix[] = "Apofur81Kb";
         UtlHashMap map;
         char tmpString[20];

         for( int i = 0; i < COUNT; i++)
         {
            sprintf(tmpString, "%s%d", stringPrefix, i);
            UtlString *stringToInsert = new UtlString();
            *stringToInsert = tmpString;

            CPPUNIT_ASSERT( ! map.contains( stringToInsert ) );
            map.insert( stringToInsert );
            CPPUNIT_ASSERT_EQUAL( i+1, (int)map.entries() );

            for( unsigned int j = 0; j < map.entries(); j++ )
            {
               // verify that all entries are indeed in the map
               sprintf( tmpString, "%s%d", stringPrefix, j );
               UtlString stringTolookUp( tmpString );
               CPPUNIT_ASSERT_MESSAGE( tmpString, map.contains( &stringTolookUp ) );
            }
         }
      }
};

const int UtlHashMapTests::INDEX_NOT_EXIST = -1;

CPPUNIT_TEST_SUITE_REGISTRATION(UtlHashMapTests);
