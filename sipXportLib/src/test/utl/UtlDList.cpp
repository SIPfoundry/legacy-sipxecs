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
#include <limits.h>
#include <cstdarg>

#include <os/OsDefs.h>

#include <utl/UtlInt.h>
#include <utl/UtlString.h>
#include <utl/UtlDList.h>
#include <utl/UtlContainableTestStub.h>
#include <sipxunit/TestUtilities.h>

using namespace std ;


/**  This class is used to test the UtlDList utility class.
*
*    PLEASE READ THE README FILE THAT CAN FOUND IN THE PARENT OF THE DIRECTORY
*    OF THIS FILE. The Readme describes the organization / flow of tests and
*    without reading this file, the following class (and all unit tests)
*    may not make a lot of sense and might be difficult to comprehend.
*/
class UtlDListTest : public  CppUnit::TestCase
{

    CPPUNIT_TEST_SUITE(UtlDListTest);
    CPPUNIT_TEST(checkSanity_Append_Entries_And_At) ;
    CPPUNIT_TEST(testAppend) ;
    CPPUNIT_TEST(testInsert) ;
    CPPUNIT_TEST(testInsertAt_EmptyList) ;
    CPPUNIT_TEST(testInsertAt_NonEmptyList) ;
    CPPUNIT_TEST(testFirst_And_Last) ;
    CPPUNIT_TEST(testIndex) ;
    CPPUNIT_TEST(testFind) ;
    CPPUNIT_TEST(testContains) ;
    CPPUNIT_TEST(testContainsReference) ;
    CPPUNIT_TEST(testOccurancesOf) ;
    CPPUNIT_TEST(testRemove) ;
    CPPUNIT_TEST(testRemoveReference) ;
    CPPUNIT_TEST(testRemoveAndDestroy) ;
    CPPUNIT_TEST(testGet) ;
    CPPUNIT_TEST(testIsEmpty) ;
    CPPUNIT_TEST(testClear) ;
    CPPUNIT_TEST(testClearAndDestroy) ;
    CPPUNIT_TEST_SUITE_END();


private:

    static const int INDEX_NOT_EXIST ;
    static const int commonEntriesCount ;

    UtlDList commonList ;
    UtlDList emptyList ;
    UtlString commonString1 ;
    UtlString commonString2 ;
    UtlString commonString3 ;
    UtlInt commonInt1 ;
    UtlInt commonInt2 ;
    UtlInt commonInt3 ;

    UtlString commonString1_clone;
    UtlString commonString2_clone ;
    UtlString commonString3_clone ;
    UtlInt commonInt1_clone ;
    UtlInt commonInt2_clone ;
    UtlInt commonInt3_clone ;

    static const char* longAlphaNumString ;
    static const char* regularString ;

    UtlContainable** commonContainables ;
    UtlContainable** commonContainables_Clone ;

    enum IndexOrContains { TEST_INDEX, TEST_FIND, TEST_CONTAINS, TEST_CONTAINS_REF } ;
    enum TestInsertOrAppend {TEST_APPEND, TEST_INSERT} ;
    enum RemoveType {TEST_REMOVE, TEST_REMOVE_REF } ;

public:
    UtlDListTest()
    {
        commonContainables = new UtlContainable*[commonEntriesCount] ;
        commonContainables_Clone = new UtlContainable*[commonEntriesCount] ;
    }

    ~UtlDListTest()
    {
        delete[] commonContainables ;
        delete[] commonContainables_Clone ;
    }

    void setUp()
    {
        UtlContainableTestStub::clearCount() ;
        commonString1 = UtlString(regularString) ;
        commonString1_clone = UtlString(regularString) ;
        commonString2 = UtlString("") ;
        commonString2_clone = UtlString("") ;
        commonString3 = UtlString(longAlphaNumString) ;
        commonString3_clone = UtlString(longAlphaNumString) ;

        commonInt1 = UtlInt(0) ;
        commonInt1_clone = UtlInt(0) ;
        commonInt2 = UtlInt(INT_MAX) ;
        commonInt2_clone = UtlInt(INT_MAX) ;
        commonInt3 = UtlInt(INT_MIN) ;
        commonInt3_clone = UtlInt(INT_MIN) ;

        commonList.append(&commonString1) ;
        commonContainables[0] = &commonString1 ;
        commonContainables_Clone[0] = &commonString1_clone ;
        commonList.append(&commonInt1) ;
        commonContainables[1] = &commonInt1 ;
        commonContainables_Clone[1] = &commonInt1_clone ;
        commonList.append(&commonInt2) ;
        commonContainables[2] = &commonInt2 ;
        commonContainables_Clone[2] = &commonInt2_clone;
        commonList.append(&commonString2) ;
        commonContainables[3] = &commonString2 ;
        commonContainables_Clone[3] = &commonString2_clone ;
        commonList.append(&commonInt3) ;
        commonContainables[4] = &commonInt3 ;
        commonContainables_Clone[4] = &commonInt3_clone ;
        commonList.append(&commonString3) ;
        commonContainables[5] = &commonString3 ;
        commonContainables_Clone[5] = &commonString3_clone ;
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
    *    the basic append(), entries() and at() methods work as expected.
    *    All future tests will depend heavily on the at() method
    *    and the most common way of having something in the list is
    *    by means of the append() method.
    *
    */
    void checkSanity_Append_Entries_And_At()
    {
        for (int i = 0 ; i < commonEntriesCount; i++)
        {
            UtlContainable* ucExpected = commonContainables[i] ;
            UtlContainable* ucActual = commonList.at(i) ;
            string msg ;
            char strItr[33] ;
            sprintf(strItr, "%d", i);
            TestUtilities::createMessage(3, &msg, "Verify that the at(n) method, where n = ", \
                strItr, " ;") ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), ucExpected, ucActual) ;
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Verify that the entries() for an empty list returns 0", \
                (int)emptyList.entries(), 0) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Verify the entries() method for a list", \
                (int)commonList.entries(), commonEntriesCount) ;
    }// checkSanity_Append_And_At()

    /*a! Test the append method for a list that is not empty
    *    The test data for this test are :-
    *     a) Append a CollectableString
    *     b) Append a CollectableInt
    *
    */
    void testAppend()
    {
        utlTestAppend_Insert(TEST_APPEND) ;
    }

    /*a! Test the insert method for a list that is not empty
    *
    *    Since the insert method does exactly the same thing as
    *    the append method, the test data for these two are the same
    */
    void testInsert()
    {
       utlTestAppend_Insert(TEST_INSERT) ;
    }

    void utlTestAppend_Insert(TestInsertOrAppend type)
    {
        int testCount = 2 ;
        const char* prefix = "";
        UtlInt testInt(1234) ;
        UtlString testString("Test String") ;
        if (type == TEST_APPEND)
        {
            commonList.append(&testInt) ;
            commonList.append(&testString) ;
            prefix = "Test the append(UtlContainable*) method for a non empty list" ;
        }
        else if (type == TEST_INSERT)
        {
            commonList.insert(&testInt) ;
            commonList.insert(&testString) ;
            prefix = "Test the insert(UtlContainable*) method for a non empty list" ;
        }
        int expectedCount  = commonEntriesCount + testCount ;

        UtlContainable* uActual ;
        UtlContainable* uExpected ;
        string msg ;

        // Verify that the number of entries has increased accordingly
        TestUtilities::createMessage(2, &msg, prefix, " :- Verify the number of entries") ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedCount, \
            (int)commonList.entries()) ;

        // Verify that the first entry has still not changed.
        uActual = commonList.at(0) ;
        uExpected = commonContainables[0] ;
        TestUtilities::createMessage(2, &msg, prefix, \
            " :- Verify that the first entry is not changed") ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), uActual, uExpected) ;

        // Verify the entry at the previous last position
        TestUtilities::createMessage(2, &msg, prefix, \
            " :- Verify that the previous last entry is intact") ;

        // Verify that the number of entries has increased accordingly
        TestUtilities::createMessage(2, &msg, prefix, \
            " :- Verify the number of entries") ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedCount, \
           (int)commonList.entries()) ;

        // Verify that the first entry has still not changed.
        uActual = commonList.at(0) ;
        uExpected = commonContainables[0] ;
        TestUtilities::createMessage(2, &msg, prefix, \
            " :- Verify that the first entry is not changed") ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), uActual, uExpected) ;

        // Verify the entry at the previous last position
        TestUtilities::createMessage(2, &msg, prefix, \
            " :- Verify that the previous last entry is intact") ;
        uActual = commonList.at(commonEntriesCount-1) ;
        uExpected = commonContainables[commonEntriesCount-1] ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), uActual, uExpected) ;

        // Verify that the two new entries are added.
        TestUtilities::createMessage(2, &msg, prefix, \
            " :- Verify that the Collectable Integer has been added") ;
        uActual = commonList.at(commonEntriesCount) ;
        uExpected = &testInt ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), uActual, uExpected) ;

        TestUtilities::createMessage(2, &msg, prefix, \
            " :- Verify that the Collectable String has been added") ;
        uActual = commonList.at(commonEntriesCount + 1) ;
        uExpected = &testString ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), uActual, uExpected) ;

    } //testAppend

    /*!a! Test case to verify insertAt(size_t, UtlContainable*) for an
    *     empty list.
    *     The test data for this test are
    *     a) Insert a UtlString to the 0th location,
    *     b) Insert a UtlInt to the 0th location,
    *     c) Insert any UtlContainable object to a 'non-zero' location
    */
    void testInsertAt_EmptyList()
    {
        const int testCount = 3 ;
        const char* prefix = "Test insert(n, Collectable*) for an empty list; "\
             "where Collectable is "  ;
        const char* Msgs[] = { \
               "a UtlString and n = 0", \
               "a UtlInt and n = 0", \
               "a UtlContainableXXX and n > 0" \
        };
        const char* suffix1 = " :- Verify return value" ;
        const char* suffix2 = " :- Verify value is appended"  ;

        UtlInt testInt(102) ;
        UtlString testString("Test String") ;
        UtlString testNegative("This should not get added") ;
        UtlContainable* itemToAdd[] = { &testString, &testInt, &testNegative } ;
        UtlContainable* expectedValue[] = { &testString, &testInt, NULL} ;
        int insertLocation[] = { 0, 0, 1} ;
        for (int i = 0 ; i < testCount ; i++)
        {
            UtlDList testList ;

            string msg ;
            // insertAt now returns void. Retain this block of comment in case
            // we (I think we should return a Collectable / bool) decide to return
            // a collectable.

            UtlContainable* result = testList.insertAt(insertLocation[i], itemToAdd[i]);
            //verify that the right value is returned.
            TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix1) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedValue[i], result) ;

            testList.insertAt(insertLocation[i], itemToAdd[i]) ;
            // verify that the value is inserted
            TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix2) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedValue[i], testList.at(0)) ;
        }
    }//testInsertAt_EmptyList

    /*!a Test case to verify insertAt(size_t, UtlContainable*) for a
    *     list that is not empty.
    *     The test data for this test are
    *     a) Insert any UtlContainable to the 0th location,
    *     b) Insert a UtlInt to a 'mid' location,
    *     c) Insert any UtlString object to a 'mid' location
    *     d) Insert any UtlContainable object to the last location
    */
    void testInsertAt_NonEmptyList()
    {
        const int testCount = 4 ;
        const char* prefix = "Test insert(n, Collectable*) for a list that is not empty; "\
              "where Collectable is "  ;
        const char* Msgs[] = { \
               "a UtlContainableXXX and n = 0", \
               "a UtlString and n > 0 && n < size", \
               "a UtlInt and n > 0 && n < size", \
               "a UtlContainableXXX where n = size-1" \
        };
        const char* suffix1 = " :- Verify return value" ;
        const char* suffix2 = " :- Verify value is appended"  ;
        const char* suffix3 = " :- Verify new list size" ;

        UtlString testFirst("First Entry") ;
        UtlInt testInt(102) ;
        UtlString testString("Test String") ;
        UtlInt testLast(99999) ;
        UtlContainable* itemToAdd[] = { &testFirst, &testInt, &testString, &testLast } ;
        UtlContainable* expectedValue[] = { &testFirst, &testInt, &testString, &testLast} ;
        int insertLocation[] = { 0, 2, 3, commonEntriesCount+3} ;
        int tmpCount = commonEntriesCount ;
        int expectedEntries[] = {++tmpCount, ++tmpCount, ++tmpCount, ++tmpCount} ;

        for (int i = 0 ; i < testCount ; i++)
        {
            UtlContainable* uActual ;
            string msg ;

            // comment out for now. Uncomment if implementation returns Collectable
            uActual = commonList.insertAt(insertLocation[i], itemToAdd[i]);
            //verify that the right value is returned.
            TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix1) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedValue[i], uActual) ;
            //`commonList.insertAt(insertLocation[i], itemToAdd[i]);

            // verify that the value is inserted
            TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix2) ;
            uActual = commonList.at(insertLocation[i]) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedValue[i], uActual) ;

            //verify that the total number of entries has incremented by one.
            TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix3) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedEntries[i], \
                (int)commonList.entries()) ;

        }
    }//testInsertAt_NonEmptyList()

    /*!a  Test case to test the first() and last() method.
    *
    *     The test data for this test case is :-
    *     a) Test the first and last element after appending
    *     b) Test the first and last element after insertAt(midlevel)
    *     c) Test the first and last element after insertAt(0)
    *     d) Test the first and last element after insertAt(last)
    */
    void testFirst_And_Last()
    {
        const char* prefix1 = "Test the first() method ";
        const char* prefix2 = "Test the last() method " ;
        string msg ;
        UtlContainable* uActual ;
        UtlContainable* uData ;
        UtlContainable* uExpected ;

        const char* Msgs[] = { \
               "after appending ", \
               "after insertAt(0) ", \
               "after insertAt(last) ", \
               "after insertAt(mid-level) " \
        } ;

        // Since this testcase requires a different test data
        // for each of its test data, the regula test-matrix
        // technique is not being used here.

        // create a new list and append one element to it.
        UtlDList testList ;
        uData = commonContainables[0] ;
        testList.append(uData);

        // Test the first() and last() element immeidately after
        // appending to an empty list.
        TestUtilities::createMessage(2, &msg, prefix1, Msgs[0]);
        uActual = testList.first() ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), uData, uActual) ;
        uActual = testList.last() ;
        TestUtilities::createMessage(2, &msg, prefix2, Msgs[0]) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), uData, uActual) ;

        // insert more values to populate the List
        testList.append(commonContainables[1]) ;
        testList.append(commonContainables[2]) ;

        // test the first() / last() methods
        // after insertAt(0..)
        uData = commonContainables[3] ;
        testList.insertAt(0, uData) ;
        uExpected = commonContainables[3] ;
        uActual = testList.first() ;
        TestUtilities::createMessage(2, &msg, prefix1, Msgs[1]) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), uExpected, uActual) ;
        uExpected = commonContainables[2] ;
        uActual = testList.last() ;
        TestUtilities::createMessage(2, &msg, prefix2, Msgs[1]) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), uExpected, uActual) ;

        // test after inserting at the last location
        uData = commonContainables[4] ;
        testList.insertAt(4, uData) ;
        uExpected = commonContainables[3] ;
        uActual = testList.first() ;
        TestUtilities::createMessage(2, &msg, prefix1, Msgs[2]) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), uExpected, uActual) ;
        uExpected = commonContainables[4] ;
        uActual = testList.last() ;
        TestUtilities::createMessage(2, &msg, prefix2, Msgs[2]) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), uExpected, uActual) ;

        //test after inserting at the midLocation
        uData = commonContainables[5] ;
        testList.insertAt(2, uData) ;
        uExpected = commonContainables[3] ;
        uActual = testList.first() ;
        TestUtilities::createMessage(2, &msg, prefix1, Msgs[3]) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), uExpected, uActual) ;
        uExpected = commonContainables[4] ;
        uActual = testList.last() ;
        TestUtilities::createMessage(2, &msg, prefix2, Msgs[3]) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), uExpected, uActual) ;

    } //testFirst_And_Last

    /*!a Test case for the index() method
    *
    *    The test data for this test case are :-
    *      a) When the match is the first element.
    *      b) When the match is the last element.
    *      c) When the match is a mid element(unique).
    *      d) When the match has two value matches (but a single ref match)
    *      e) When the match has two ref matches.
    *      f) When there is no match at all!
    */
    void testIndex()
    {
        utlTestIndex_Find_And_Contains(TEST_INDEX) ;
    }

    /*!a Test case for the find() method.
    *
    *    The test data for this test case are the same as
    *    testIndex() test case.
    */
    void testFind()
    {
        utlTestIndex_Find_And_Contains(TEST_FIND) ;
    }

    /*!a Test case for the contains() method
    *
    *    The test data for this test case are the same as
    *    testIndex() test case.
    */
    void testContains()
    {
        utlTestIndex_Find_And_Contains(TEST_CONTAINS) ;
    }

    /*!a Test case for the containsReference() method
    *
    *    The test data for this test case are the same as
    *    testIndex() test case.
    */
    void testContainsReference()
    {
        utlTestIndex_Find_And_Contains(TEST_CONTAINS_REF) ;
    }


    // Since the test setup / preconditions for the index, find,
    // contains and containsReference are all the same, these
    // tests have been combined into one utility function. Based
    // on the type argument, the method to be tested is varied.
    void utlTestIndex_Find_And_Contains(IndexOrContains type)
    {
        const int testCount = 7 ;
        const char* prefixIndex = "Test the index() method when the match " ;
        const char* prefixFind = "Test the find() method when the match " ;
        const char* prefixContains = "Test the contains() method when the match " ;
        const char* prefixContainsRef = "Test the containsReference() method when the match " ;
        const char* Msgs[] = { \
               "is the first element ", \
               "is the last element ", \
               "is a mid element (unique match) ", \
               "has two value matches but a single ref match ", \
               "has two ref matches", \
               "has a value match but no ref match", \
               "has no match at all" \
        } ;
        // insert a clone of the 4th element to the 1st position
        commonList.insertAt(1, commonContainables_Clone[4]) ;
       // The new index for a value match of commonContainables[4] must be 1.

       // insert another copy of the 3rd element to the 2nd position.
       commonList.insertAt(2, commonContainables[3]) ;
       // The new index for commonContainables[3] must be 2) ;
       // what used to be the second element has now moved to 4.

       UtlString noExist("This cannot and should not exist!!!") ;

       UtlContainable* searchValues[] = { \
                 commonContainables[0], commonContainables[5], commonContainables[2], \
                 commonContainables[4], commonContainables[3], \
                 commonContainables_Clone[2], &noExist \
       } ;

       ssize_t expectedValues_Index[] = { 0, 7, 4, 1, 2, 4, UTL_NOT_FOUND } ;

       bool expectedValues_Contains[]    = {true, true, true, true, true, true, false } ;
       bool expectedValues_ContainsRef[] = {true, true, true, true, true, false, false} ;

       UtlContainable* searchValuesForFind[] = { \
                 commonContainables[0], commonContainables[5], commonContainables[2], \
                 commonContainables[4], commonContainables[3], \
                 commonContainables_Clone[1], &noExist \
       } ;
       UtlContainable* expectedValuesForFind[] = { \
                 commonContainables[0], commonContainables[5], commonContainables[2], \
                 commonContainables_Clone[4], commonContainables[3], \
                 commonContainables[1], NULL \
       } ;

       for (int i = 0 ; i < testCount ; i++)
       {
           string msg ;
           if (type == TEST_INDEX)
           {
               ssize_t actual = commonList.index(searchValues[i]) ;
               TestUtilities::createMessage(2, &msg, prefixIndex, Msgs[i]) ;
               CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedValues_Index[i], actual) ;
           }
           else if (type == TEST_FIND)
           {
               UtlContainable* actual = commonList.find(searchValuesForFind[i]) ;
               TestUtilities::createMessage(2, &msg, prefixFind, Msgs[i]) ;
               CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedValuesForFind[i], actual) ;
           }
           else if (type == TEST_CONTAINS)
           {
               UtlBoolean actual = commonList.contains(searchValues[i]) ;
               TestUtilities::createMessage(2, &msg, prefixContains, Msgs[i]) ;
               CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedValues_Contains[i], \
                  (TRUE == actual)) ;
           }
           else if (type == TEST_CONTAINS_REF)
           {
               UtlBoolean actual = commonList.containsReference(searchValues[i]) ;
               TestUtilities::createMessage(2, &msg, prefixContainsRef, Msgs[i]) ;
               CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedValues_ContainsRef[i], \
                   (TRUE == actual)) ;
           }
       }
    }//utlTestIndex

    /*!a  Test the occurancesOf() method .
    *
    *     The test data for this test are :-
    *        a) When the search data is the first entry
    *        b) When the search data is the last entry
    *        c) When the search data is the mid(unique) entry
    *        d) When the search data has a matching value but not ref.
    *        e) When the search data has multiple matches - mixture of ref / values
    *        f) When the search data has no match at all.
    */
    void testOccurancesOf()
    {
        const int testCount = 6 ;
        const char* prefix = "Test the occurancesOf(UtlContainable* cl); where cl " ;
        const char* Msgs[] = { \
               "is the first entry ", \
               "is the last entry and ref matches ", \
               "is the mid entry and is unique ", \
               "has a matching value but not reference ", \
               "has multiple matches ", \
               "has no match at all " \
        } ;

        commonList.insertAt(3, commonContainables_Clone[4]) ;
        commonList.insertAt(5, commonContainables[4]) ;
        UtlString notExistCollectable("This cannot and willnot exist");

        UtlContainable* searchValues[] = { \
                   commonContainables[0], commonContainables[commonEntriesCount -1], \
                   commonContainables[2], commonContainables_Clone[3], \
                   commonContainables[4], &notExistCollectable \
        } ;
        int matchCount[] = { 1, 1, 1, 1, 3, 0 } ;
        for (int i = 0 ; i < testCount ; i++)
        {
            string msg ;
            TestUtilities::createMessage(2, &msg, prefix, Msgs[i]) ;
            int actual = commonList.occurrencesOf(searchValues[i]) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), matchCount[i], actual) ;
        }
    } //testOccurancesOf


    /*!a Test case for testRemove()
    *
    *     The Test data for this test case is
    *          a) is the first entry's reference
    *          b) is the last entry' reference
    *          c) is the mid entry's value(not reference)
    *          d) is the first of multiple matches and is the value match
    *          e) has no match at all
    */
    void testRemove()
    {
        utlTestRemove(TEST_REMOVE) ;

    }

    /*!a Test case for testRemoveReference()
    *
    *    The test data for this test case is
    *    the same as teh remove() method.
    */
    void testRemoveReference()
    {
        utlTestRemove(TEST_REMOVE_REF) ;
    }

    /*!a Test case to test the destroy()
    *    method.
    */
    void testRemoveAndDestroy()
    {
        const char* prefix  = "test the destroy() method " ;

        UtlContainableTestStub uStub(0) ;
        UtlContainableTestStub* uStubPtr ;
        uStubPtr = new UtlContainableTestStub(1) ;
        commonList.append(&uStub) ;
        commonList.append(uStubPtr) ;

        int cCountBefore = UtlContainableTestStub :: getCount() ;

        UtlBoolean returnValue = commonList.destroy(uStubPtr) ;
        UtlContainable* uLast = commonList.last() ;
        string msg ;
        TestUtilities::createMessage(2, &msg, prefix, ":- Verify the return value") ;
        CPPUNIT_ASSERT_MESSAGE(msg.data(), returnValue) ;
        TestUtilities::createMessage(2, &msg, prefix, ":- Verify that the entry is removed") ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)&uStub, (void*)uLast) ;
        // The CollectableTestStub has been implemented such that a static counter gets decremented
        // for every descruction of an object instance. To verify that the object was destroyed,
        // verify that the static count went down.
        int cCountAfter = UtlContainableTestStub :: getCount() ;
        TestUtilities::createMessage(2, &msg, prefix, ":- Verify that the object was deleted") ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), cCountBefore -1, cCountAfter) ;
    }

    void utlTestRemove(RemoveType type)
    {

        int testCount = 5 ;
        const char* prefix = "";
        if (type == TEST_REMOVE)
        {
            prefix = "test the remove(UtlContainable* c) method where c" ;

        }
        else if (type == TEST_REMOVE_REF)
        {
            prefix = "test the removeReference(UtlContainable* c) where c" ;
        }
        const char* Msgs[] = { \
               "is the first entry's reference ", \
               "is the last entry' reference ", \
               "is the mid entry's value(not reference) ", \
               "is the first of multiple matches and is the value match ", \
               "has no match at all " \
        } ;
        const char* suffix1 = " :- Verify returned value" ;
        const char* suffix2 = " :- Verify total entries" ;

        commonList.insertAt(2, commonContainables_Clone[4]) ;

        UtlString notExistCollectable("This cannot and willnot exist");

        UtlContainable* itemToRemove[] = { \
                           commonContainables[0], commonContainables[commonEntriesCount -1 ], \
                           commonContainables_Clone[2], commonContainables[4], \
                           &notExistCollectable \
        } ;

        int totalEnt = commonEntriesCount + 1;

        UtlContainable* expectedValue[] = { \
                           commonContainables[0], commonContainables[commonEntriesCount -1 ], \
                           commonContainables[2], commonContainables_Clone[4], \
                           NULL \
        };


        int entriesValue[] = { --totalEnt, --totalEnt, --totalEnt, --totalEnt, totalEnt } ;

        totalEnt = commonEntriesCount + 1;

        UtlContainable* expectedRef[] = { \
                           commonContainables[0], commonContainables[commonEntriesCount -1 ], \
                           NULL, commonContainables[4], \
                           NULL \
        };
        int entriesRef[] = { --totalEnt, --totalEnt, totalEnt, --totalEnt, totalEnt } ;

        for (int i = 0 ; i < testCount ; i++)
        {

            string msg ;
            if (type == TEST_REMOVE)
            {
                TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix1) ;

                UtlContainable* retValue = commonList.remove(itemToRemove[i]) ;
                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(),  expectedValue[i], retValue) ;
                TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix2) ;
                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), entriesValue[i], (int)commonList.entries()) ;
            }
            else if (type == TEST_REMOVE_REF)
            {

                UtlContainable* uRemoved = commonList.removeReference(itemToRemove[i]) ;
                TestUtilities::createMessage(3, &msg, prefix, Msgs[i],  suffix2) ;
                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedRef[i], uRemoved) ;
                TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix2) ;
                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), entriesRef[i], (int)commonList.entries()) ;
            }
        }
    } //utlRemove


    /*!a Test case for the get() method.
    *
    *    The test data for this test is :-
    *       1) The first entry is a CollectableString
    *       2) The first entry is a CollectableInt
    *       3) The List has only one entry
    *       4) The List has no entries
    */
    void testGet()
    {
        const int testCount = 4 ;
        const char* prefix = "Verify the get() method for a list when " ;
        const char* Msgs[] = { \
                     "the first entry is a CollectableString", \
                     "the first entry is a CollectableInt", \
                     "when the list has only one entry", \
                     "when the list is empty" \
        } ;
        const char* suffix1 = ":- verify return value" ;
        const char* suffix2 = ":- verify the number of entries in the list" ;
        UtlDList testList ;
        testList.append(&commonString1) ;
        testList.append(&commonInt1) ;
        testList.append(&commonString2) ;

        UtlContainable* expectedValue[] = { \
                          &commonString1 , &commonInt1, &commonString2, NULL \
        } ;
        int entryCount[]  = { 2, 1, 0, 0 } ;
        for (int i = 0 ; i < testCount ; i++)
        {
            UtlContainable* actual = testList.get() ;
            string msg ;
            TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix1) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedValue[i], actual) ;
            TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix2) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), entryCount[i], (int)testList.entries()) ;
        }
    } //testGet()

    /*!a  Test case for the isEmpty() method.
    *     The test data for this test are :-
    *        a) When the list has just been created.
    *        b) When the list has one entry in it
    *        c) When the list has multiple entries in it.
    *        d) When all the entries in a list have been removed using get.
    *        e) When all the entries in a list have been removed using removeAll()
    */
    void testIsEmpty()
    {
        const int testCount = 5 ;
        const char* prefix = "Test the isEmpty() method when " ;
        const char* Msgs[] = { \
                "the list has just been created" , \
                "the list has just one entry in it", \
                "the list has multiple entries in it", \
                "all the list entries have been retreieved using get()", \
                "all the list entries have been retreived using removeAll()" \
        } ;

        UtlDList newList ;
        UtlDList secondNewList ;
        UtlDList commonList_Clone ;

        // first populate a list and then retreive all elements using get
        for (int i = 0 ; i < commonEntriesCount ; i++)
        {
            commonList_Clone.append(commonContainables_Clone[i]) ;
        }
        for (int j = 0 ; j < commonEntriesCount; j++)
        {
            commonList_Clone.get();
        }

        UtlString uS1 = UtlString("Lone Entry") ;
        newList.append(&uS1) ;

        // populate the second list and then clear all entries.
        secondNewList.append(&uS1) ;
        UtlInt uI1 = UtlInt(232) ;
        secondNewList.append(&uI1) ;
        secondNewList.removeAll() ;

        UtlDList* testLists[] = { \
                 &emptyList, &newList, &commonList, &commonList_Clone,  &secondNewList \
        } ;

        bool expectedValue[] = { true, false, false, true, true } ;
        for (int k = 0 ; k < testCount; k++)
        {
            string msg ;
            TestUtilities::createMessage(2, &msg, prefix, Msgs[k]) ;
            UtlBoolean actual = testLists[k] -> isEmpty() ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (UtlBoolean)expectedValue[k], \
                actual) ;
        }
    } // testIsEmpty

    /*!a test the removeAll() method.
    *
    *    The test data for this method is
    *        a) When the list is empty
    *        b) When the list has one entry.
    *        c) When the list multiple entries
    *        d) When removeAll has been called and entries are added again
    *        d) When the removeAll is called twice on the list.
    */
    void testClear()
    {
        const int testCount = 5 ;
        const char* prefix = "Test the removeAll() method when :- " ;
        const char* Msgs[] = { \
               "the list is empty", \
               "the list has one entry", \
               "the list has multiple entries", \
               "removeAll() has been called and entries are added again", \
               "removeAll() has already been called", \
        } ;
        const char* suffix = " :- Verify number of entries after removeAll()"  ;
        UtlDList uSingleList ;
        UtlDList uAddAfterClear ;
        UtlDList uDoubleClear ;

        uSingleList.append(&commonString1) ;

        // call removeAll() on a list and then add entries again.
        uAddAfterClear.append(&commonInt1) ;
        uAddAfterClear.append(&commonString1) ;
        uAddAfterClear.removeAll() ;
        uAddAfterClear.append(&commonInt2) ;

        // call removeAll on a list twice.
        uDoubleClear.append(&commonString3) ;
        uDoubleClear.append(&commonInt3) ;
        uDoubleClear.removeAll() ;

        UtlDList* testLists[] = { \
                     &emptyList, &uSingleList, &commonList, &uAddAfterClear, &uDoubleClear
        } ;
        int expectedEntries[] = { 0 , 0, 0, 1, 0 } ;

        // since we are not calling removeAll for all the data, do it outside the for loop.
        emptyList.removeAll() ;
        uSingleList.removeAll() ;
        commonList.removeAll() ;
        // no removeAll() for uAddAfterClear
        uDoubleClear.removeAll() ;
        for ( int i = 0 ; i < testCount ; i++)
        {
            string msg ;
            TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data() , expectedEntries[i], \
                (int)testLists[i]->entries()) ;
        }
    } //testClear()


    /*!a Test case to test the destroyAll()
    *    method.
    */
    void testClearAndDestroy()
    {
        const char* prefix  = "test the destroyAll() method " ;

        const char* suffix1 = ":- Verify that all entries are removed" ;
        const char* suffix2 = ":- The objects are deleted" ;

        UtlContainableTestStub* uStub ;
        UtlContainableTestStub* uStubPtr ;
        uStub = new UtlContainableTestStub(0) ;
        uStubPtr = new UtlContainableTestStub(1) ;
        emptyList.append(uStub) ;
        emptyList.append(uStubPtr) ;

        emptyList.destroyAll() ;
        int cCountAfter = UtlContainableTestStub::getCount() ;

        string msg ;
        TestUtilities::createMessage(2, &msg, prefix, suffix1) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), 0, (int)emptyList.entries()) ;

        // Since the TestStub has been implemented such that destructor
        // decrements the static counter, to verify that the objects have
        // been deleted, verify that the static counter has been decremented.
        TestUtilities::createMessage(2, &msg, prefix, suffix2) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), 0, cCountAfter) ;

    } //testClearAndDestroy


};

const int UtlDListTest::INDEX_NOT_EXIST = -1;
const int UtlDListTest::commonEntriesCount = 6;
const char* UtlDListTest::longAlphaNumString = \
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

const char* UtlDListTest::regularString = "This makes sense" ;



CPPUNIT_TEST_SUITE_REGISTRATION(UtlDListTest);
