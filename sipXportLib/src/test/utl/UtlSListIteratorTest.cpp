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
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>
#include <sipxunit/TestUtilities.h>

using namespace std ;


/**  This class is used to test the UtlSListIterator utility class.
*
*    PLEASE READ THE README FILE THAT CAN FOUND IN THE PARENT OF THE DIRECTORY
*    OF THIS FILE. The Readme describes the organization / flow of tests and
*    without reading this file, the following class (and all unit tests)
*    may not make a lot of sense and might be difficult to comprehend.
*/
class UtlSListIteratorTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(UtlSListIteratorTest);
    CPPUNIT_TEST(testAdvancingOperator) ;
    CPPUNIT_TEST(testFindNext) ;
    CPPUNIT_TEST(testFindNextForDummies) ;
    CPPUNIT_TEST(testLast) ;
    CPPUNIT_TEST(testOffEnd);
    CPPUNIT_TEST(testInsertAfterPoint_EmptyList) ;
    CPPUNIT_TEST(testInsertAfterPoint) ;
    CPPUNIT_TEST(removeItem) ;
    CPPUNIT_TEST(testPeekAtNext) ;
    CPPUNIT_TEST_SUITE_END();

private:

    static const int INDEX_NOT_EXIST ;
    static const int commonEntriesCount ;

    UtlSList commonList ;
    UtlSList emptyList ;

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

    UtlContainable** commonContainables;
    UtlContainable** commonContainables_Clone;

    enum IndexOrContains { TEST_INDEX, TEST_FIND, TEST_CONTAINS, TEST_CONTAINS_REF } ;
    enum TestInsertOrAppend {TEST_APPEND, TEST_INSERT} ;
    enum RemoveType {TEST_REMOVE, TEST_REMOVE_REF } ;

public:

    void setUp()
    {
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

    UtlSListIteratorTest()
    {
        commonContainables = new UtlContainable*[commonEntriesCount] ;
        commonContainables_Clone = new UtlContainable*[commonEntriesCount] ;
    }

    ~UtlSListIteratorTest()
    {
        delete[] commonContainables ;
        delete[] commonContainables_Clone ;
    }

    /*!a Test case for the () operator.
    *
    *    The test data for this test is :-
    *       1) The next entry is a UtlString
    *       2) The next entry is a UtlInt
    *       3) The next entry is the last entry
    *       4) All entries have been read
    */
    void testAdvancingOperator()
    {
        struct TestAdvancingOperatorStruct
        {
            const char* testDescription ;
            const UtlContainable* expectedValue ;
        } ;

        const int testCount = 4 ;
        const char* prefix = "Verify the () operator for an iterator when " ;

        const char* suffix1 = " :- verify return value" ;
        const char* suffix2 = " :- verify number of entries in the list" ;

        UtlSList testList ;
        testList.append(&commonString1) ;
        testList.append(&commonInt1) ;
        testList.append(&commonString2) ;
        UtlSListIterator iter(testList) ;

        TestAdvancingOperatorStruct testData[] = { \
            { "the first entry is a UtlString", &commonString1 }, \
            { "the first entry is a UtlInt", &commonInt1 }, \
            { "when the list has only one entry", &commonString2 }, \
            { "when the list is empty", NULL } \
        } ;
        int expectedEntries = 3 ;

        for (int i = 0 ; i < testCount ; i++)
        {
             string msg ;
             TestUtilities::createMessage(3, &msg, prefix, \
                 testData[i].testDescription, suffix1) ;
             CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)testData[i].expectedValue, \
                 (void*)iter()) ;
             TestUtilities::createMessage(3, &msg, prefix, testData[i].testDescription, \
                 suffix2);
             CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedEntries, \
                 (int)testList.entries()) ;
        }

        // Test the () operator for an empty list
        UtlSListIterator emptyIter(emptyList) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test the () operator for an empty list iterator" , \
            (void*)NULL, (void*)emptyIter()) ;

    } //testAdvancingOperator()


    /**
     * Test for SListIterator::findNext only at places that could break
     *  - find item not in list
     *  - find items at endpoints
     *  - find items after reseting iterator
     */
    void testFindNextForDummies()
    {
        UtlString fruitData[] = {
            UtlString("apple"),         // 0
            UtlString("orange"),        // 1
            UtlString("banana"),        // 2
            UtlString("apple"),         // 3
            UtlString("banana"),        // 4
            UtlString("apple")          // 5
        };
        int n = sizeof(fruitData) / sizeof(fruitData[0]);

        UtlSList fruit;
        for (int i = 0; i < n; i++)
        {
            fruit.append(&fruitData[i]);
        }

        // nonexistant items
        UtlSListIterator pineapples(fruit);
        UtlString pineapple("pineapple");
        CPPUNIT_ASSERT_MESSAGE("Do not find item not in list", pineapples.findNext(&pineapple) == NULL);

        // test items at endpoints
        UtlSListIterator apples(fruit);
        UtlString apple("apple");
        CPPUNIT_ASSERT_MESSAGE("Find first item", apples.findNext(&apple) == &fruitData[0]);
        CPPUNIT_ASSERT_MESSAGE("Find second item", apples.findNext(&apple) == &fruitData[3]);
        CPPUNIT_ASSERT_MESSAGE("Find third item", apples.findNext(&apple) == &fruitData[5]);
        CPPUNIT_ASSERT_MESSAGE("Find no more items", apples.findNext(&apple) == NULL);

        // test scattered items
        UtlSListIterator bananas(fruit);
        UtlString banana("banana");
        CPPUNIT_ASSERT_MESSAGE("Find first item", bananas.findNext(&banana) == &fruitData[2]);
        CPPUNIT_ASSERT_MESSAGE("Find second item", bananas.findNext(&banana) == &fruitData[4]);
        CPPUNIT_ASSERT_MESSAGE("Find no more items", bananas.findNext(&banana) == NULL);

        // test adjustments in iterator
        UtlSListIterator picker(fruit);
        CPPUNIT_ASSERT_MESSAGE("Find first item", picker.findNext(&apple) == &fruitData[0]);
        picker.reset();
        CPPUNIT_ASSERT_MESSAGE("Find first item again after reset",
            picker.findNext(&apple) == &fruitData[0]);
        CPPUNIT_ASSERT_MESSAGE("Find second item after initial reset",
            picker.findNext(&apple) == &fruitData[3]);
    }


    /*!a Test case for the findNext() method
    *
    *    The test data for this test case are :-
    *      a) When the match is the first element.
    *      b) When the match is the last element.
    *      c) When the match is a mid element(unique).
    *      d) When the match has two value matches (but a single ref match)
    *      e) When the match has two ref matches.
    *      f) When there is no match at all!
    *      g) When the match is after the current find.
    */
    void testFindNext()
    {
        struct TestFindNextStruct
        {
            const char* testDescription ;
            const UtlContainable* searchValue ;
            const UtlContainable* expectedValue ;
        } ;
        const char* prefix = "Test the find() method when the match " ;

        UtlString noExist("This cannot and should not exist!!!") ;
        TestFindNextStruct testData[] = { \
            { "is the first element ", commonContainables[0], commonContainables[0] }, \
            { "is the last element ", commonContainables[5], commonContainables[5] }, \
            { "is a mid element (unique match) ", commonContainables[2], \
              commonContainables[2] }, \
            { "has two value matches but a single ref match ", commonContainables[4], \
              commonContainables_Clone[4] }, \
            { "has two ref matches", commonContainables[3], commonContainables[3] }, \
            { "has a value match but no ref match", commonContainables_Clone[1], \
              commonContainables[1] }, \
            { "has no match at all", &noExist, NULL } \
        } ;
       // insert a clone of the 4th element to the 1st position
       commonList.insertAt(1, (UtlContainable*)commonContainables_Clone[4]) ;
       // The new index for a value match of commonContainables[4] must be 1.

       // insert another copy of the 3rd element to the 2nd position.
       commonList.insertAt(2, (UtlContainable*)commonContainables[3]) ;
       // The new index for commonContainables[3] must be 2) ;
       // what used to be the second element has now moved to 4.


        const int testCount = sizeof(testData)/sizeof(testData[0])  ;
        UtlSListIterator iter(commonList) ;
        for (int i = 0 ; i < testCount ; i++)
        {
            string msg ;

            const UtlContainable* actualValue = iter.findNext(testData[i].searchValue) ;
            TestUtilities::createMessage(2, &msg, prefix, testData[i].testDescription) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), testData[i].expectedValue, \
                actualValue) ;
            iter.reset() ;
        }

        // Now test the case where the iterator is 'past' the index
        iter.reset() ;
        iter() ;
        iter() ;
        iter() ;
        iter() ;
        iter() ;
        UtlContainable* actualValue = iter.findNext(commonContainables[1]) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("test findNext() when the iterator has " \
           "moved past the search index", (void*)NULL, (void*)actualValue) ;
    }//testFindNext

    /*!a  Test case to test the toLast() and atLast() methods.
    *
    *     The test data for this test case is :-
    *     a) An empty list
    *     b) A non-empty list iterator that is in its initial position
    *     c) A non-empty list iterator that is in its last position.
    *     d) A non-empty list that is in its mid position
    */
    void testLast()
    {
        UtlSList testList ;
        const char* prefix1 = "Test the toLast() method for " ;
        const char* prefix2 = "Test the atLast() method(after calling toLast()) for " ;
        string msg ;
        UtlContainable* actualValue ;
        bool isAtLast = false ;

        const char* Msgs[] = { \
               "an empty list iterator ", \
               "a list iterator that is in its zeroth position ", \
               "a non empty list that is already in its last position ", \
               "a non empty list that is in its mid position " \
        } ;

        UtlSListIterator iter(emptyList)  ;
        UtlSListIterator iter2(commonList) ;

        // since this test requires adifferent test setup for each
        // of the test data, tests are done individually rather than using
        // the test array style of testing.

        // Test#1 - Test the methods for an empty list.
        int ti = 0 ;
        iter.toLast() ;
        isAtLast = (TRUE == iter.atLast()) ;
        actualValue = iter() ;
        TestUtilities::createMessage(2, &msg, prefix1, Msgs[ti]) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)NULL, (void*)actualValue) ;
        TestUtilities::createMessage(2, &msg, prefix2, Msgs[ti]) ;
        // since the list is empty, any position is __PAST__ the last position
        CPPUNIT_ASSERT_MESSAGE(msg.data(), !isAtLast) ;

        // Test#2 - Test the methods for a list that is not empty.
        ti++ ;
        iter2.reset() ;
        iter2.toLast() ;
        isAtLast = (TRUE == iter2.atLast()) ;
        actualValue = iter2() ;
        TestUtilities::createMessage(2, &msg, prefix1, Msgs[ti]) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)NULL, (void*)actualValue) ;
        TestUtilities::createMessage(2, &msg, prefix2, Msgs[ti]) ;
        CPPUNIT_ASSERT_MESSAGE(msg.data(), isAtLast) ;

        // Test#3 - Test the methods for a list that is not empty when
        // the list is already in its last position.
        ti++ ;
        iter2.reset() ;
        iter2.toLast() ;
        iter2.toLast() ;
        isAtLast = (TRUE == iter2.atLast()) ;
        actualValue = iter2() ;
        TestUtilities::createMessage(2, &msg, prefix1, Msgs[ti]) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)NULL, (void*)actualValue) ;
        TestUtilities::createMessage(2, &msg, prefix2, Msgs[ti]) ;
        CPPUNIT_ASSERT_MESSAGE(msg.data(), isAtLast) ;

        // Test#4 - Test the methods for a list that is not empty when the
        // list has been iterated to somewhere in its middle position.
        ti++ ;
        iter2.reset() ;
        iter2() ;
        iter2() ;
        iter2.toLast() ;
        isAtLast = (TRUE == iter2.atLast()) ;
        actualValue = iter2() ;
        TestUtilities::createMessage(2, &msg, prefix1, Msgs[ti]) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)NULL, (void*)actualValue) ;
        TestUtilities::createMessage(2, &msg, prefix2, Msgs[ti]) ;
        CPPUNIT_ASSERT_MESSAGE(msg.data(), isAtLast) ;
    } //testFirst_And_Last

      /*!a  Test case to test walking off the end of a list
       *
       *     The test data for this test case is :-
       *     a) An empty list
       *     b) A non-empty list
       */
      void testOffEnd()
      {
         UtlSList testList ;
         const char* prefix1 = "Test the ()() method after end of the list for " ;
         const char* prefix2 = "Test the atLast() method after end of the list for " ;
         string msg ;
         UtlContainable* actualValue ;
         bool isAtLast = false ;

         const char* Msgs[] = { \
                          "an empty list iterator ", \
                          "a list iterator that is in its zeroth position ", \
                          "a non empty list that is already in its last position ", \
                          "a non empty list that is in its mid position " \
         } ;

         UtlSListIterator iter(emptyList)  ;
         UtlSListIterator iter2(commonList) ;

         // since this test requires adifferent test setup for each
         // of the test data, tests are done individually rather than using
         // the test array style of testing.

         // Test#1 - Test the methods for an empty list.
         int ti = 0 ;
         iter.toLast() ;
         CPPUNIT_ASSERT(iter()==NULL);
         actualValue=iter();
         isAtLast = (TRUE == iter.atLast()) ;
         TestUtilities::createMessage(2, &msg, prefix1, Msgs[ti]) ;
         CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)NULL, (void*)actualValue) ;
         TestUtilities::createMessage(2, &msg, prefix2, Msgs[ti]) ;
         // since the list is empty, any position is __PAST__ the last position
         CPPUNIT_ASSERT_MESSAGE(msg.data(), !isAtLast) ;

         // Test#2 - Test the methods for a list that is not empty.
         ti++ ;
         iter2.toLast() ;
         CPPUNIT_ASSERT(iter2()==NULL);// go off the end
         isAtLast = (TRUE == iter2.atLast()) ;
         actualValue = iter2() ;
         TestUtilities::createMessage(2, &msg, prefix1, Msgs[ti]) ;
         CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)NULL, (void*)actualValue) ;
         TestUtilities::createMessage(2, &msg, prefix2, Msgs[ti]) ;
         CPPUNIT_ASSERT_MESSAGE(msg.data(), !isAtLast) ;
      } //testOffEnd



    /*!a Test case for the insertAfterPoint() method when the
    *    the list is empty
    */
    void testInsertAfterPoint_EmptyList()
    {
        const char* prefix = "Test the insertAfterPoint() method for an empty list " ;
        const char* suffix1 = ":- Verify return value" ;
        const char* suffix2 = ":- Verify that the entry has been  added" ;
        string msg ;
        UtlSListIterator iter(emptyList) ;
        const UtlContainable* uReturn = \
              iter.insertAfterPoint((UtlContainable*)commonContainables[0]) ;
        TestUtilities::createMessage(2, &msg, prefix, suffix1) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)commonContainables[0], \
            (void*)uReturn) ;
        iter.reset() ;
        UtlContainable* uAppended = iter() ;
        TestUtilities::createMessage(2, &msg, prefix, suffix2) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)commonContainables[0], \
            (void*)uAppended) ;

    }

    /*!a Test case for the insertAfterPoint() method.
    *
    *    The test data is :-
    *      a) Insert when the iterator is the starting position
    *      b) Insert when the iterator is at mid position
    *      c) Insert when the iterator is at the last position
    *      d) Insert to an empty Iterator.
    */
    void testInsertAfterPoint()
    {
        struct TestInsertAfterStruct
        {
            const char* testDescription;
            UtlContainable* insertValue ;
            UtlContainable* oldValue ;
        };
        const char* prefix = "Test the insertAfterPoint() method when " ;
        const char* suffix1 = ":- Verify return value" ;
        const char* suffix2 = ":- Verify value is inserted" ;
        const char* suffix3 = ":- Verify that previous value is not lost" ;
        string msg ;

        UtlSListIterator iter(commonList) ;

        UtlString newColString1("Insert at starting position") ;
        UtlInt newColInt2(101) ;
        UtlString newColString3 ("Insert at last position") ;
        TestInsertAfterStruct testData[] = { \
            { "the iterator is the starting position " , &newColString1, \
              commonContainables[0] }, \
            { "the iterator is at mid-position ", &newColInt2, commonContainables[1] }, \
            { "the iterator is at the last position ", &newColString3, \
              commonContainables[5] } \
        };

        const UtlContainable* uReturn ;
        UtlContainable* uAppended ;
        UtlContainable* originalValue ;
        // Since this test requires different steps for the different test data,
        // steps are executed individually rather than the regular technique of
        // iterating through the test-array loop

        //Test#1 - Verify the case when the iterator has been reset
        int ti = 0 ;
        iter.reset() ;
        uReturn = iter.insertAfterPoint(testData[ti].insertValue) ;
        TestUtilities::createMessage(3, &msg, prefix, testData[ti].testDescription, suffix1) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE (msg.data(), (void*)testData[ti].insertValue, \
            (void*)uReturn) ;
        // The item is inserted at first position
        // old[0] is now @ pos1. old[1] is now @ pos2
        iter.reset() ;
        uAppended = iter() ;
        TestUtilities::createMessage(3, &msg, prefix, testData[ti].testDescription, suffix2) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)testData[ti].insertValue, \
            (void*)uAppended) ;
        // Verify that the original item is still retained.
        originalValue = iter() ;
        TestUtilities::createMessage(3, &msg, prefix, testData[ti].testDescription, suffix2) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)testData[ti].oldValue, \
            (void*)originalValue) ;

        //Test#2 - inserting at mid position
        ti = 1;
        iter.reset() ;
        iter() ; //moves cursor to 0
        iter() ; //moves cursor to 1
        iter() ; //moves cursor to 2
        // Value is now inserted after pos2 (to pos3)
        // old[1] stays at pos2
        uReturn = iter.insertAfterPoint(testData[ti].insertValue) ;
        TestUtilities::createMessage(3, &msg, prefix, testData[ti].testDescription, suffix1) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE (msg.data(), (void*)testData[ti].insertValue, \
            (void*)uReturn) ;
        iter.reset() ;
        iter() ;  // moves cursor to 0
        iter() ; // moves cursor to 1
        // Verify that the original item is still retained.
        originalValue = iter() ;
        TestUtilities::createMessage(3, &msg, prefix, testData[ti].testDescription, suffix2) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)testData[ti].oldValue, \
            (void*)originalValue) ;
        // The item is inserted just after the position.
        uAppended = iter() ; //moves cursor to pos3 and returns item at pos2
        TestUtilities::createMessage(3, &msg, prefix, testData[ti].testDescription, suffix3) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)testData[ti].insertValue, \
            (void*)uAppended) ;

        // Test#3 - Now verify when the cursor is at the last position.
        ti = 2 ;
        iter.reset() ;
        iter.toLast() ;
        uReturn = iter.insertAfterPoint(testData[ti].insertValue) ;
        TestUtilities::createMessage(3, &msg, prefix, testData[ti].testDescription, suffix1) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)testData[ti].insertValue, \
            (void*)uReturn) ;
        iter.reset() ;
        // now move the cursor all the way to the penultimate position
        for (size_t i = 0 ; i < commonList.entries() - 1; i++)
        {
            originalValue = iter() ;
        }
        // verify original is still retained.
        TestUtilities::createMessage(3, &msg, prefix, testData[ti].testDescription, suffix3) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)testData[ti].oldValue, \
                                     (void*)originalValue) ;
        uAppended = iter() ;
        TestUtilities::createMessage(3, &msg, prefix, testData[ti].testDescription, suffix2) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( msg.data(), (void*)testData[ti].insertValue, \
            (void*)uAppended) ;

    } //testInsertAfterPoint


    /*!a test case for the interaction of remove() and item().
    */
    void removeItem()
    {
       UtlString v1("a");
       UtlString v2("b");
       UtlString v3("c");
       UtlString v4("d");
       UtlContainable* e;

       UtlSList h;
       h.insert(&v1);
       h.insert(&v2);
       h.insert(&v3);
       h.insert(&v4);

       UtlSListIterator iter(h);

       // Check that item() returns NULL in the initial state.
       CPPUNIT_ASSERT(iter.item() == NULL);

       // Step the iterator and check that item() returns v1.
       e = iter();
       CPPUNIT_ASSERT(e == &v1);
       CPPUNIT_ASSERT(iter.item() == &v1);

       // Delete the element and check that item() returns NULL.
       h.remove(e);
       CPPUNIT_ASSERT(iter.item() == NULL);

       // Step the iterator and check that item() returns v2.
       e = iter();
       CPPUNIT_ASSERT(e == &v2);
       CPPUNIT_ASSERT(iter.item() == &v2);

       // Step the iterator and check that item() returns v3.
       e = iter();
       CPPUNIT_ASSERT(e == &v3);
       CPPUNIT_ASSERT(iter.item() == &v3);

       // Delete the element and check that item() returns v2.
       // (Because deleting an element of a list backs the iterator up
       // to the previous element.)
       h.remove(e);
       CPPUNIT_ASSERT(iter.item() == &v2);

       // Step the iterator and check that item() returns v4.
       e = iter();
       CPPUNIT_ASSERT(e == &v4);
       CPPUNIT_ASSERT(iter.item() == &v4);

       // Step the iterator after the last element and check that
       // item() returns NULL.
       e = iter();
       CPPUNIT_ASSERT(e == NULL);
       CPPUNIT_ASSERT(iter.item() == NULL);

    } //removeItem()

    void testPeekAtNext()
    {
       UtlString v1("a");
       UtlString v2("b");
       UtlString v3("c");
       UtlString v4("d");
       UtlContainable* e;

       UtlSList h;
       h.insert(&v1);
       h.insert(&v2);
       h.insert(&v3);
       h.insert(&v4);

       UtlSListIterator iter(h);

       // check that peekAtNext() returns v1 while iterator points at NULL
       e = iter.peekAtNext();
       CPPUNIT_ASSERT(e == &v1);
       CPPUNIT_ASSERT(iter.item() == NULL );

       // Step the iterator and check that peekAtNext() returns v2
       // while iterator points at v1
       iter();
       e = iter.peekAtNext();
       CPPUNIT_ASSERT(e == &v2);
       CPPUNIT_ASSERT(iter.item() == &v1);

       // Step the iterator and check that peekAtNext() returns v3
       // while iterator points at v2
       iter();
       e = iter.peekAtNext();
       CPPUNIT_ASSERT(e == &v3);
       CPPUNIT_ASSERT(iter.item() == &v2);

       // Step the iterator and check that peekAtNext() returns v4
       // while iterator points at v3
       iter();
       e = iter.peekAtNext();
       CPPUNIT_ASSERT(e == &v4);
       CPPUNIT_ASSERT(iter.item() == &v3);

       // Step the iterator and check that peekAtNext() returns NULL
       // while iterator points at v4
       iter();
       e = iter.peekAtNext();
       CPPUNIT_ASSERT(e == NULL);
       CPPUNIT_ASSERT(iter.item() == &v4);
    }
};

const int UtlSListIteratorTest::INDEX_NOT_EXIST = -1;
const int UtlSListIteratorTest::commonEntriesCount = 6;

const char* UtlSListIteratorTest::longAlphaNumString = \
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

const char* UtlSListIteratorTest::regularString = "This makes sense" ;

CPPUNIT_TEST_SUITE_REGISTRATION(UtlSListIteratorTest);
