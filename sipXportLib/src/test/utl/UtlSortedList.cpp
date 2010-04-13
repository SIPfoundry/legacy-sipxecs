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

#include <string.h>
#include <stdlib.h>
#include <cstdarg>
#include <os/OsDefs.h>
#include <utl/UtlVoidPtr.h>
#include <utl/UtlInt.h>
#include <utl/UtlString.h>
#include <utl/UtlSortedList.h>
#include <utl/UtlContainableTestStub.h>
#include <sipxunit/TestUtilities.h>

using namespace std ;

/**  This class is used to test the UtlInt utility class.
*
*    PLEASE READ THE README FILE THAT CAN FOUND IN THE PARENT OF THE DIRECTORY
*    OF THIS FILE. The Readme describes the organization / flow of tests and
*    without reading this file, the following class (and all unit tests)
*    may not make a lot of sense and might be difficult to comprehend.
*/
class UtlSortedListTest : public  CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(UtlSortedListTest);
    CPPUNIT_TEST(checkSanity_Insert_Entries_And_At) ;
    CPPUNIT_TEST(testInsert_StringList);
    CPPUNIT_TEST(testInsert_IntList);
    CPPUNIT_TEST(testInsert_MixedContainableObjects) ;
    CPPUNIT_TEST(testInsert_LikeExistingItem);
    CPPUNIT_TEST(testIndex) ;
    CPPUNIT_TEST(testFind) ;
    CPPUNIT_TEST(testContains) ;
    CPPUNIT_TEST(testRemove) ;
    CPPUNIT_TEST(testRemoveAt) ;
    CPPUNIT_TEST(testDestroy) ;
    CPPUNIT_TEST(testRemoveAll) ;
    CPPUNIT_TEST(testDestroyAll) ;
    CPPUNIT_TEST(testIsEmpty) ;
    CPPUNIT_TEST_SUITE_END();

private:
    struct TestSortedListStruct
    {
        const char* testDescription ;
        UtlContainable* item ;
        ssize_t expectedIndex ;    // Needs to be able to hold UTL_NOT_FOUND.
    };

    enum FindOrContains { TEST_FIND, TEST_CONTAINS} ;

    enum RemoveType { REMOVE_CONTAINABLE, REMOVE_INDEX } ;

    static const int INDEX_NOT_EXIST ;
    static const int stringListCount ;
    static const int intListCount ;

    UtlSortedList emptyList ;
    UtlSortedList stringList ;
    UtlSortedList intList ;
    TestSortedListStruct* testDataForStringList ;
    TestSortedListStruct* testDataForIntList ;

public:

    UtlSortedListTest()
    {
        testDataForStringList = new TestSortedListStruct[stringListCount] ;
        testDataForIntList = new TestSortedListStruct[stringListCount] ;
    }


    ~UtlSortedListTest()
    {
        for (int i = 0 ; i < stringListCount; i++)
        {
            delete testDataForStringList[i].item ;
        }
        for (int j = 0 ; j < stringListCount; j++)
        {
            delete testDataForIntList[j].item ;
        }
        delete[] testDataForStringList ;
        delete[] testDataForIntList ;
    }


    void setUp()
    {
        testDataForStringList[0].testDescription = "A numeric string(alphabetically first!)" ;
        testDataForStringList[0].item = new UtlString("123") ;
        testDataForStringList[0].expectedIndex = 0 ;

        testDataForStringList[1].testDescription = "An alphabetic string(alphatically second!)" ;
        testDataForStringList[1].item = new UtlString("Abcd") ;
        testDataForStringList[1].expectedIndex = 1 ;

        testDataForStringList[2].testDescription = "An alphabetic string(alphabetically third!)" ;
        testDataForStringList[2].item = new UtlString("Zyxw") ;
        testDataForStringList[2].expectedIndex = 2 ;

        testDataForStringList[3].testDescription =
                             "An alpha numeric string(alphabetically fourth!)" ;
        testDataForStringList[3].item = new UtlString("ab#34cd") ;
        testDataForStringList[3].expectedIndex = 3 ;

        // Add the strings in a random (non alphabetical order)
        stringList.insert(testDataForStringList[2].item) ;
        stringList.insert(testDataForStringList[0].item) ;
        stringList.insert(testDataForStringList[3].item) ;
        stringList.insert(testDataForStringList[1].item) ;

        testDataForIntList[0].testDescription = "A negative integer(first item)" ;
        testDataForIntList[0].item = new UtlInt(-25023) ;
        testDataForIntList[0].expectedIndex = 0 ;

        testDataForIntList[1].testDescription = "Integer whose value = 0(second item)" ;
        testDataForIntList[1].item = new UtlInt(0) ;
        testDataForIntList[1].expectedIndex = 1 ;

        testDataForIntList[2].testDescription = "Positive integer(third item)" ;
        testDataForIntList[2].item = new UtlInt(35) ;
        testDataForIntList[2].expectedIndex = 2 ;

        testDataForIntList[3].testDescription = "A positive integer(fourth item)" ;
        testDataForIntList[3].item = new UtlInt(25023) ;
        testDataForIntList[3].expectedIndex = 3 ;

        // Add the ints in a random (non alphabetical order)
        intList.insert(testDataForIntList[2].item) ;
        intList.insert(testDataForIntList[0].item) ;
        intList.insert(testDataForIntList[3].item) ;
        intList.insert(testDataForIntList[1].item) ;
    }

    void tearDown()
    {
    }


    /** Sandbox method for experimenting with the API Under Test.
    *   This method MUST be empty when the test drivers are being
    *   checked in (final checkin) to the repository.
    */
    void DynaTest()
    {
    }


    /*a! This test is more of a sanity check to verify that
    *    the basic insert(), entries() and at() methods work as expected.
    *    All future tests will depend heavily on the at() method
    *    and the most common way of having something in the list is
    *    by means of the append() method.
    */
    void checkSanity_Insert_Entries_And_At()
    {
        const char* prefix1 = "For a SortedList with UtlStrings, ";
        const char* prefix2 = "For a SortedList with UtlInts, " ;
        const char* suffix = "Verify that the at(n) method, where n = " ;
        for (int i = 0 ; i < stringListCount; i++)
        {
            size_t expectedIndex = testDataForStringList[i].expectedIndex ;
            const UtlContainable* ucAct = stringList.at(expectedIndex) ;
            string msg ;
            char strItr[33] ;
            sprintf(strItr, "%zu", expectedIndex);
            TestUtilities::createMessage(3, &msg, prefix1, suffix, strItr) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(),
                        (void*)testDataForStringList[expectedIndex].item, (void*)ucAct) ;
        }

        for (int j = 0 ; j < intListCount; j++)
        {
            size_t expectedIndex = testDataForIntList[j].expectedIndex ;
            const UtlContainable* ucAct = intList.at(expectedIndex) ;
            string msg ;
            char strItr[33] ;
            sprintf(strItr, "%zu", expectedIndex);
            TestUtilities::createMessage(3, &msg, prefix2, suffix, strItr) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(),
                        (void*)testDataForIntList[expectedIndex].item, (void*)ucAct) ;
        }

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Verify that the entries() for an empty list returns 0",
                                   0, (int)emptyList.entries()) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Verify the entries() method for a list",
                                   stringListCount, (int)stringList.entries() ) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Verify the entries() method for a int list",
                                   intListCount, (int)intList.entries()) ;
    }// checkSanity_Append_And_At()

    /** Test the insert() method for a SortedList of strings
    *
    *   Test Data:-
    *      a) The inserted string would now be the first item in the list
    *      b) The inserted string would now be in the middle
    *      c) The inserted string would be the last in the list.
    */
    void testInsert_StringList()
    {
        const char* prefix = "For sortedlist consisting of UtlStrings, "
                             "test the insert(Containable cont) method, where cont = " ;
        const char* suffix1 = " :- check return value" ;
        const char* suffix2 = " :- check if inserted at correct position" ;
        string Message ;

        UtlString string1("") ;
        UtlString string2("BCD") ;
        UtlString string3("zzzzz") ;

        TestSortedListStruct testData[] = {
           { "the new first list item", &string1, 0},
           { "the new fourth item", &string2, 3 },
           { "the new last item", &string3, 6}
        } ;
        // Adjust the expected indices for the original test data
        testDataForStringList[0].expectedIndex = 1 ;
        testDataForStringList[1].expectedIndex = 2 ;
        testDataForStringList[2].expectedIndex = 4 ;
        testDataForStringList[3].expectedIndex = 5 ;


        const int newItemsCount = sizeof(testData)/sizeof(testData[0]) ;
        for (int i = 0 ; i < newItemsCount; i++)
        {
            UtlContainable* returnValue = stringList.insert(testData[i].item) ;
            // Verify return value
            TestUtilities::createMessage(3,&Message, prefix,
                                         testData[i].testDescription, suffix1) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), (void*)testData[i].item,
                                      (void*)returnValue) ;
        }
        // Verify that the total number of entries has increased accordingly
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Verify that the total number of entries goes up"
                    "accordingly after insert()ing items",
                    stringListCount + newItemsCount, (int)stringList.entries()) ;

        // Verify that the new items are inserted according to their sorting order.
        for (int j=0 ; j<newItemsCount ; j++)
        {
            const UtlContainable* returnValue = stringList.at(testData[j].expectedIndex);

            TestUtilities::createMessage(3, &Message, prefix,
                                         testData[j].testDescription, suffix2) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), (void*)testData[j].item,
                                      (void*)returnValue) ;
        }

        // Verify that the old items are still intact.
        for (int k=0; k < stringListCount; k++)
        {
            size_t expectedIndex = testDataForStringList[k].expectedIndex ;
            char strOldItr[33] ;
            char strItr[33] ;
            sprintf(strOldItr, "%d", k) ;
            sprintf(strItr, "%zu", expectedIndex);

            const UtlContainable* returnValue = stringList.at(expectedIndex) ;

            TestUtilities::createMessage(5, &Message, "Verify that old item at ", strOldItr,
                           " is re-sorted to position # ", strItr, " after insert()ing") ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), (void*)testDataForStringList[k].item,
                                      (void*)returnValue) ;
        }

    } //testInsert_StringList


    /** Test the insert() method for a SortedList of ints
    *
    *   Test Data:-
    *      a) The inserted int would now be the first item in the list
    *      b) The inserted int would now be in the middle
    *      c) The inserted int would be the last in the list.
    */
    void testInsert_IntList()
    {
        const char* prefix = "For sortedlist consisting of UtlInts, "
                             "test the insert(Containable cont) method, where cont = " ;
        const char* suffix1 = " :- check return value" ;
        const char* suffix2 = " :- check if inserted at correct position" ;
        string Message ;

        UtlInt int1(-50000) ;
        UtlInt int2(10) ;
        UtlInt int3(50000) ;

        TestSortedListStruct testData[] = {
           { "the new first list item", &int1, 0},
           { "the new fourth item", &int2, 3 },
           { "the new last item", &int3, 6}
        } ;
        // Adjust the expected indices for the original test data
        testDataForIntList[0].expectedIndex = 1 ;
        testDataForIntList[1].expectedIndex = 2 ;
        testDataForIntList[2].expectedIndex = 4 ;
        testDataForIntList[3].expectedIndex = 5 ;

        const int newItemsCount = sizeof(testData)/sizeof(testData[0]) ;
        for (int i = 0 ; i < newItemsCount; i++)
        {
            UtlContainable* returnValue = intList.insert(testData[i].item) ;
            // Verify return value
            TestUtilities::createMessage(3,&Message, prefix,
                                         testData[i].testDescription, suffix1) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), (void*)testData[i].item,
                                      (void*)returnValue) ;
        }

        // Verify that the total number of entries has increased accordingly
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Verify that the total number of entries goes up"
                    "accordingly after insert()ing items",
                    intListCount + newItemsCount, (int)intList.entries()) ;

        // Verify that the new items are inserted according to their sorting order.
        for (int j=0 ; j<newItemsCount ; j++)
        {
            const UtlContainable* returnValue = intList.at(testData[j].expectedIndex);

            TestUtilities::createMessage(3, &Message, prefix,
                                         testData[j].testDescription, suffix2) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), (void*)testData[j].item,
                                      (void*)returnValue) ;
        }

        // Verify that the old items are still intact.
        for (int k=0; k < intListCount; k++)
        {
            size_t expectedIndex = testDataForIntList[k].expectedIndex ;
            char strOldItr[33] ;
            char strItr[33] ;
            sprintf(strOldItr, "%d", k) ;
            sprintf(strItr, "%zu", expectedIndex);

            const UtlContainable* returnValue = intList.at(expectedIndex) ;

            TestUtilities::createMessage(5, &Message, "Verify that old item at ", strOldItr,
                           " is re-sorted to position # ", strItr, " after insert()ing") ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(),(void*)testDataForIntList[k].item,
                                      (void*)returnValue) ;
        }
    } //testInsert_IntList


    /** Test Case to verify that different types of UtlContainable objects
    *   can be added to the same SortedList. In this case we are not worried
    *   about the order in which the items are sorted(since this is not defined)
    *   However what we are worried is the fact that items *can* be added at all
    */
    void testInsert_MixedContainableObjects()
    {
        emptyList.insert(testDataForStringList[0].item) ;
        emptyList.insert(testDataForIntList[0].item) ;
        UtlVoidPtr objVoid((void*)this) ;
        emptyList.insert(&objVoid) ;
        int expectedCount = 3;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Verify that different types of UtlContainable objects can "
            "be added to the same UtlSortedList", expectedCount, (int)emptyList.entries()) ;
    } //testInsert_MixedContainableObjects()

    /**  Test the insert(Containable* cont) method when cont is equal to an
    *    existing item in the list.
    */
    void testInsert_LikeExistingItem()
    {
        const char* prefix = "Test the insert(UtlContainable cont) method when cont is "
                    "equal to an existing item in the list" ;
        const char* suffix1 = " Verify the position of the old element" ;
        const char* suffix2 = " Verify the position of the new element" ;
        string Message ;

        // insert the item such that is equal to the second item .
        UtlString stringTest("Abcd") ;
        stringList.insert(&stringTest) ;
        const UtlContainable* returnValue ;
        UtlContainable* secondExpectedValue = NULL ;
        bool wasFound = false ;

        const char* firstSuffix = "";
        const char* secondSuffix = "";
        // The item at the position 1 can be one of the following:-
        // a) testDataForStringList[1].item or b) &stringTest
        returnValue = stringList.at(1) ;
        if (returnValue == testDataForStringList[1].item)
        {
            wasFound = true ;
            firstSuffix = suffix1 ;
            secondSuffix = suffix2 ;
            secondExpectedValue = &stringTest ;
        }
        else if(returnValue == &stringTest)
        {
            wasFound = true ;
            firstSuffix = suffix2 ;
            secondSuffix = suffix1 ;
            secondExpectedValue = testDataForStringList[1].item ;
        }
        TestUtilities::createMessage(2, &Message, prefix, firstSuffix) ;
        CPPUNIT_ASSERT_MESSAGE(Message.data(), wasFound) ;

        // After the previous test passed, of the two strings,
        // the only allowed value for the item at position2 is the value
        // that was *NOT* found at position1.
        returnValue = stringList.at(2) ;
        TestUtilities::createMessage(2, &Message, prefix, secondSuffix) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), (void*)secondExpectedValue,
                                  (void*)returnValue) ;
    } //testInsert_LikeExistingItem()


    /*!a Test case for the index() method
    *
    *    The test data for this test case are :-
    *      a) When the match is the first element.
    *      b) When the match is the last element.
    *      c) When the match is a mid element and the search key isEqual to one
              the items
    *      f) When there is no match at all!
    */
    void testIndex()
    {
        const char* prefix = "Test the index(UtlContainable* cont) method when cont = " ;
        string Message ;
        UtlString noExistString("CannotExist") ;
        UtlString midItemClone(((UtlString*)testDataForStringList[2].item) -> data()) ;

        TestSortedListStruct testData[] = {
            { "first item in the list", testDataForStringList[0].item,
                     testDataForStringList[0].expectedIndex },
            { "last item in the list", testDataForStringList[stringListCount-1].item,
                     testDataForStringList[stringListCount-1].expectedIndex },
            { "like a mid item in the list", &midItemClone,
                     testDataForStringList[2].expectedIndex},
            { "item doesn't exist in the list", &noExistString, UTL_NOT_FOUND }
        };
        const int testCount = sizeof(testData)/sizeof(testData[0]);
        for (int i =0 ; i<testCount; i++)
        {
            ssize_t returnValue = stringList.index(testData[i].item) ;
            TestUtilities::createMessage(2, &Message, prefix, testData[i].testDescription) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(),
                                         (int)testData[i].expectedIndex,
                                         (int)returnValue) ;
        }
    }//testIndex()

    /** Test the find() method.
    *
    *   The TestData for this test are :-
    *       a) The element is the first element.
    *       b) The element is the last element.
    *       c) The element is like a mid element.
    *       d) The element has multiple matches.
    *       e) The element has no matches at all
    *
    */
    void testFind()
    {
        utlTestFindOrContains(TEST_FIND) ;
    }

    /** Test the contains() method. The test data for this
    *   method is the same as that for the find() method
    */
    void testContains()
    {
        utlTestFindOrContains(TEST_CONTAINS) ;
    }

    /** Utility method to run the tests for find or contains.
    */
    void utlTestFindOrContains(FindOrContains type)
    {
        struct TestFindDataStructure
        {
            const char* testDescription ;
            UtlContainable* searchData ;
            UtlContainable* expectedData ;
        };
        const char* prefix1 = "Test the find(UtlContainable* cont) when cont = " ;
        const char* prefix2 = "Test the contains(UtlContainable* cont) when cont = " ;
        string Message ;

        UtlInt noExist(8736232) ;
        UtlInt midElementClone(((UtlInt*)testDataForIntList[2].item)->getValue()) ;

        // now insert a duplicate element for position #3
        intList.insert(testDataForIntList[2].item) ;
        TestFindDataStructure testData[] = {
            { "first element", testDataForIntList[0].item,
                     testDataForIntList[0].item},
            { "last element", testDataForIntList[intListCount -1].item,
                     testDataForIntList[intListCount-1].item },
            { "like a mid element", &midElementClone,
                     testDataForIntList[2].item },
            { "is a duplicate match", testDataForIntList[2].item,
                     testDataForIntList[2].item },
            { "has no match", &noExist, NULL }
        };

        const int testCount = sizeof(testData)/sizeof(testData[0]) ;
        for (int i=0 ; i < testCount ; i++)
        {
            switch(type)
            {
                case TEST_FIND:
                {
                    UtlContainable* foundValue = intList.find(testData[i].searchData) ;
                    TestUtilities::createMessage(2, &Message, prefix1,
                        testData[i].testDescription) ;
                    CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), testData[i].expectedData,
                           foundValue) ;
                    break ;
                }
                case TEST_CONTAINS:
                {
                    UtlBoolean wasFound = intList.contains(testData[i].searchData) ;
                    TestUtilities::createMessage(2, &Message, prefix2,
                        testData[i].testDescription) ;
                    CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), testData[i].expectedData!=NULL,
                            (TRUE == wasFound)) ;
                    break ;
                }
            }
        }
    }//testFind()


    /** Test the remove(UtlContainable*) method.
    *
    *   The TestData for this test are :-
    *       a) The element is like a mid element.
    *       b) The element is the first element.
    *       c) The element is the last element.
    *       d) The element has multiple matches.
    *       e) The element has no matches at all
    *
    */
    void testRemove()
    {
        utlTestRemove(REMOVE_CONTAINABLE) ;
    } //testRemove()

    /** Test the removeAt(size_t) method.
    *   The test data for this method are the same as the remove(containable)
    *   test method
    */
    void testRemoveAt()
    {
        utlTestRemove(REMOVE_INDEX) ;
    }

    /** This utility method is used to test either the remove(UtlContainable*)
    *   or removeAt(size_t). The actual method being tested depends on the type
    *   parameter that was passed.
    */
    void utlTestRemove(RemoveType remType)
    {
        struct TestRemoveDataStructure
        {
            const char* testDescription ;
            UtlContainable* itemToRemove ;
            int removeIndex ;
            ssize_t expectedIndex ;
            bool expectedResult ;
            size_t expectedEntries ;
        };
        const char* prefix = "";
        switch(remType)
        {
            case REMOVE_CONTAINABLE :
                prefix = "Test the remove(UtlContainable* cont) method when cont =" ;
                break ;
            case REMOVE_INDEX :
                prefix = "Test the removeAt(size_t index) method when cont = " ;
                break ;
        } ;
        const char* suffix1 = " :- Verify return value" ;
        const char* suffix2 = " :- Verify the total number of entries" ;
        const char* suffix3 = " :- Verify index() after remove" ;
        string Message ;

        UtlString noExistString("CannotExist") ;
        UtlString midItemClone(((UtlString*)testDataForStringList[1].item)->data()) ;
        //insert a duplicate element.
        stringList.insert(testDataForStringList[2].item) ;
        int elementsCount = stringListCount + 1 ;
        // As a result of the above operation, item at position 0 & 1 are unchanged
        // Item that was at position 2 is now either 2 or 3 (before remove)
        // The duplicate item inserted is now at either position 2 or 3 (before rem)
        // The item that was at position 3 (last item) is now at
        // position 4 (add 2) (before remove)


        TestRemoveDataStructure testData[] = {
            { "like a mid element", &midItemClone, 1, -1, true, --elementsCount },
            { "is the first element", testDataForStringList[0].item, 0, -1,
               true, --elementsCount},
            { "is the last element", testDataForStringList[stringListCount-1].item,
               2, -1, true, --elementsCount},
                     //initialy the last index was 3. We inserted one item and made it 4.
                     // during the above 2 steps we removed 2 items.
            { "is an element with duplicate entries", testDataForStringList[2].item,
               0, 0, true, --elementsCount},
                     //It is still -2 & not -3. that is because the
                     //previous item's position was always after and doesn't affect the cur index.
            { "is an element with no match", &noExistString,
               -1, -1, false, elementsCount}
        };
        const int testCount = sizeof(testData)/sizeof(testData[0]) ;
        for (int i = 0 ; i < testCount; i++)
        {
            UtlContainable* actualResult;
            switch(remType)
            {
                case REMOVE_CONTAINABLE :
                    actualResult = stringList.remove(testData[i].itemToRemove) ;
                    break ;
                case REMOVE_INDEX :
                    actualResult = stringList.removeAt(testData[i].removeIndex) ;
                    break ;
               default:
                    actualResult = NULL;
                    break;
            }

            TestUtilities::createMessage(3, &Message, prefix,
                testData[i].testDescription, suffix1) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), testData[i].expectedResult,
                (NULL != actualResult)) ;
            TestUtilities::createMessage(3, &Message, prefix,
                testData[i].testDescription, suffix2) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(),
                 (int)testData[i].expectedEntries, (int)stringList.entries()) ;
            ssize_t indexAfterRemove = stringList.index(testData[i].itemToRemove) ;
            TestUtilities::createMessage(3, &Message, prefix,
                 testData[i].testDescription, suffix3) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(),
                                         testData[i].expectedIndex,
                                         indexAfterRemove) ;
        }
    } //utlTestRemove


    /** Testcase for the destroy() method.
    */
    void testDestroy()
    {
        const char* prefix = "test the destroy() method " ;
        const char* suffix1 = " :- verify the return value" ;
        const char* suffix2 = " :- verify that the entry is removed" ;
        const char* suffix3 = " :- verify that the object was deleted" ;
        string Message ;

        UtlContainableTestStub stubObject(987323) ;
        UtlContainableTestStub* stubPtr ;
        stubPtr = new UtlContainableTestStub(987344) ;

        intList.insert(&stubObject) ;
        intList.insert(stubPtr) ;

        int cCountBefore = UtlContainableTestStub::getCount() ;
        UtlBoolean wasRemoved = intList.destroy(stubPtr) ;
        TestUtilities::createMessage(2, &Message, prefix, suffix1) ;
        CPPUNIT_ASSERT_MESSAGE(Message.data(), (TRUE == wasRemoved)) ;

        // The CollectableTestStub has been implemented such that a static counter gets decremented
        // for every descruction of an object instance. To verify that the object was destroyed,
        // verify that the static count went down.
        int cCountAfter = UtlContainableTestStub::getCount() ;
        TestUtilities::createMessage(2, &Message, prefix, suffix3) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), cCountBefore-1, cCountAfter) ;

        // Now to verify that the object was indeed removed from the list, create a
        // clone of the stub and try to find it in the list
        stubPtr = new UtlContainableTestStub(987344) ;
        UtlContainable* searchResult = intList.find(stubPtr) ;
        delete stubPtr ;
        TestUtilities::createMessage(2, &Message, prefix, suffix2) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), (void*)NULL, (void*)searchResult) ;

    }//testDestroy()

    /*!a test the removeAll() method.
    *
    *    The test data for this method is
    *        a) When the list is empty
    *        b) When the list has one entry.
    *        c) When the list multiple entries
    *        d) When removeAll has been called and entries are added again
    *        d) When the removeAll is called twice on the list.
    */
    void testRemoveAll()
    {
        const char* prefix = "Test the removeAll() method when " ;
        const char* suffix = " :- Verify number of entries after removeAll()"  ;
        string Message ;

        //Since this method requires different lists and different operations to
        //be performed for each test, the conventional data driven loop will not
        // be used here.

        // Verify the case for an empty list.
        emptyList.removeAll() ;
        TestUtilities::createMessage(3, &Message, prefix, "the list is empty", suffix) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), 0, (int)emptyList.entries()) ;

        //Verify the case for a list with multiple entries.
        stringList.removeAll() ;
        TestUtilities::createMessage(3, &Message, prefix, "the list has entries in it", suffix) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), 0, (int)stringList.entries()) ;

        //Verify the case when entries are added after removeAll
        intList.removeAll() ;
        intList.insert(testDataForIntList[1].item) ;
        intList.insert(testDataForIntList[2].item) ;
        TestUtilities::createMessage(3, &Message, prefix,
            "entries are inserted after removeAll", suffix) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), 2, (int)intList.entries()) ;

        //Verify the case when removeAll() is called twice.
        emptyList.insert(testDataForStringList[0].item) ;
        emptyList.insert(testDataForStringList[2].item) ;
        emptyList.insert(testDataForStringList[3].item) ;
        emptyList.removeAll() ;
        emptyList.removeAll() ;
        TestUtilities::createMessage(3, &Message, prefix,
            "removeAll() is called twice", suffix) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), 0, (int)emptyList.entries()) ;
    }//testRemoveAll()

    /** Testcase to test the destroyAll() method
    *
    */
    void testDestroyAll()
    {
        const char* prefix = "test the destroyAll() method " ;
        const char* suffix1 = " :- Verify that all entries are removed" ;
        const char* suffix2 = " :- Verify that all objects are deleted" ;
        string msg ;

        UtlContainableTestStub* stub1 ;
        UtlContainableTestStub* stub2 ;

        stub1 = new UtlContainableTestStub(100) ;
        stub2 = new UtlContainableTestStub(200) ;

        emptyList.insert(stub1) ;
        emptyList.insert(stub2) ;
        emptyList.destroyAll() ;
        int cCountAfter =  UtlContainableTestStub::getCount() ;

        TestUtilities::createMessage(2, &msg, prefix, suffix1) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), 0, (int)emptyList.entries()) ;

        // Since the TestStub has been implemented such that destructor
        // decrements the static counter, to verify that the objects have
        // been deleted, verify that the static counter has been decremented.
        TestUtilities::createMessage(2, &msg, prefix, suffix2) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), 0, cCountAfter) ;
    } //testDestroyAll()

    /*!a  Test case for the isEmpty() method.
    *     The test data for this test are :-
    *        a) When the list has just been created.
    *        b) When the list has one entry in it
    *        c) When the list has multiple entries in it.
    *        d) When all the entries in a list have been removed using removeAll()
    */
    void testIsEmpty()
    {
        const char* prefix = "Test the isEmpty() method when " ;
        string Message ;
        UtlSortedList newList ;
        TestUtilities::createMessage(2, &Message, prefix, "the list has just been created") ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), true, (TRUE == newList.isEmpty())) ;

        newList.insert(testDataForStringList[0].item) ;
        TestUtilities::createMessage(2, &Message, prefix, "the list has a single entry") ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), false, (TRUE == newList.isEmpty())) ;

        TestUtilities::createMessage(2, &Message, prefix,
            "the list has multiple entries in it") ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), false, (TRUE == stringList.isEmpty())) ;

        stringList.removeAll() ;
        TestUtilities::createMessage(2, &Message, prefix,
            "removeAll() has been called on the list") ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), true, (TRUE == stringList.isEmpty())) ;
    }//testIsEmpty()

};

const int UtlSortedListTest::INDEX_NOT_EXIST = -1 ;
const int UtlSortedListTest::stringListCount = 4 ;
const int UtlSortedListTest::intListCount = 4 ;


CPPUNIT_TEST_SUITE_REGISTRATION(UtlSortedListTest);
