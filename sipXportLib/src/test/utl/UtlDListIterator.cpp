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
#include <utl/UtlDListIterator.h>
#include <sipxunit/TestUtilities.h>

using namespace std ;

class UtlDListIteratorTests : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(UtlDListIteratorTests);
    CPPUNIT_TEST(testAdvancingOperator) ;
    CPPUNIT_TEST(testFindNext) ;
    CPPUNIT_TEST(testLast) ;
    CPPUNIT_TEST(testInsertAfterPoint_EmptyList) ;
    CPPUNIT_TEST(testInsertAfterPoint) ;
    CPPUNIT_TEST(removeItem) ;
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

    /* Sandbox - please ignore
     */
    void DynaTest()
    {
    }

    UtlDListIteratorTests()
    {
        commonContainables = new UtlContainable*[commonEntriesCount] ;
        commonContainables_Clone = new UtlContainable*[commonEntriesCount] ;
    }

    ~UtlDListIteratorTests()
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
        const int testCount = 4 ;
        const char* prefix = "Verify the () operator for an iterator when " ;
        const char* Msgs[] = { \
                     "the first entry is a UtlString", \
                     "the first entry is a UtlInt", \
                     "when the list has only one entry", \
                     "when the list is empty" \
        } ;

        const char* suffix1 = " :- verify return value" ;
        const char* suffix2 = " :- verify number of entries in the list" ;

        UtlDList testList ;
        testList.append(&commonString1) ;
        testList.append(&commonInt1) ;
        testList.append(&commonString2) ;
        UtlDListIterator iter(testList) ;

        UtlContainable* exp[] = { \
                          &commonString1 , &commonInt1, &commonString2, NULL \
        } ;
        int expEntries = 3 ;

        for (int i = 0 ; i < testCount ; i++)
        {
             UtlContainable* act = iter() ;
             string msg ;
             TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix1) ;
             CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), exp[i], act) ;
             TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix2);
             CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expEntries, (int)testList.entries()) ;
        }

        // Test the () operator for an empty list
        UtlDListIterator emptyIter(emptyList) ;
        UtlContainable* act = emptyIter() ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test the () operator for an empty list iterator" , (void*)NULL, (void*)act) ;

    } //testAdvancingOperator()


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
        const int testCount = 7 ;
        const char* prefixFind = "Test the find() method when the match " ;
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
       commonList.insertAt(1, (UtlContainable*)commonContainables_Clone[4]) ;
       // The new index for a value match of commonContainables[4] must be 1.

       // insert another copy of the 3rd element to the 2nd position.
       commonList.insertAt(2, (UtlContainable*)commonContainables[3]) ;
       // The new index for commonContainables[3] must be 2) ;
       // what used to be the second element has now moved to 4.

       UtlString noExist("This cannot and should not exist!!!") ;
       const UtlContainable* searchValuesForFind[] = { \
                 commonContainables[0], commonContainables[5], commonContainables[2], \
                 commonContainables[4], commonContainables[3], \
                 commonContainables_Clone[1], &noExist \
       } ;
       const UtlContainable* expValuesForFind[] = { \
                 commonContainables[0], commonContainables[5], commonContainables[2], \
                 commonContainables_Clone[4], commonContainables[3], \
                 commonContainables[1], NULL \
       } ;

       UtlDListIterator iter(commonList) ;
       for (int i = 0 ; i < testCount ; i++)
       {
           string msg ;

           const UtlContainable* act = iter.findNext(searchValuesForFind[i]) ;
           const UtlContainable* exp = expValuesForFind[i] ;
           TestUtilities::createMessage(2, &msg, prefixFind, Msgs[i]) ;
           CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), exp, act) ;
           iter.reset() ;
       }

       // Now test the case where the iterator is 'past' the index
       iter.reset() ;
       iter() ;
       iter() ;
       iter() ;
       iter() ;
       iter() ;
       UtlContainable* act = iter.findNext(commonContainables[1]) ;
       CPPUNIT_ASSERT_EQUAL_MESSAGE("test findNext() when the iterator has moved past the search index", (void*)NULL, (void*)act) ;
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
        UtlDList testList ;
        const char* prefix1 = "Test the toLast() method for " ;
        const char* prefix2 = "Test the atLast() method(after calling toLast()) for " ;
        string msg ;
        UtlContainable* uAct ;
        bool isAtLast = false ;

        const char* Msgs[] = { \
               "an empty list iterator ", \
               "a list iterator that is in its zeroth position ", \
               "a non empty list that is already in its last position ", \
               "a non empty list that is in its mid position " \
        } ;

        UtlDListIterator iter(emptyList)  ;
        UtlDListIterator iter2(commonList) ;

        // since this test requires adifferent test setup for each
        // of the test data, tests are done individually rather than using
        // the test array style of testing.

        // Test#1 - Test the methods for an empty list.
        int ti = 0 ;
        iter.toLast() ;
        isAtLast = (TRUE == iter.atLast()) ;
        uAct = iter() ;
        TestUtilities::createMessage(2, &msg, prefix1, Msgs[ti]) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)NULL, (void*)uAct) ;
        TestUtilities::createMessage(2, &msg, prefix2, Msgs[ti]) ;
        // since the list is empty, any position is __PAST__ the last position
        CPPUNIT_ASSERT_MESSAGE(msg.data(), !isAtLast) ;

        // Test#2 - Test the methods for a list that is not empty.
        ti++ ;
        iter2.reset() ;
        iter2.toLast() ;
        isAtLast = (TRUE == iter2.atLast()) ;
        uAct = iter2() ;
        TestUtilities::createMessage(2, &msg, prefix1, Msgs[ti]) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)NULL, (void*)uAct) ;
        TestUtilities::createMessage(2, &msg, prefix2, Msgs[ti]) ;
        CPPUNIT_ASSERT_MESSAGE(msg.data(), isAtLast) ;

        // Test#3 - Test the methods for a list that is not empty when
        // the list is already in its last position.
        ti++ ;
        iter2.reset() ;
        iter2.toLast() ;
        iter2.toLast() ;
        isAtLast = (TRUE == iter2.atLast()) ;
        uAct = iter2() ;
        TestUtilities::createMessage(2, &msg, prefix1, Msgs[ti]) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)NULL, (void*)uAct) ;
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
        uAct = iter2() ;
        TestUtilities::createMessage(2, &msg, prefix1, Msgs[ti]) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)NULL, (void*)uAct) ;
        TestUtilities::createMessage(2, &msg, prefix2, Msgs[ti]) ;
        CPPUNIT_ASSERT_MESSAGE(msg.data(), isAtLast) ;

    } //testFirst_And_Last



    /*!a Test case for the insertAfterPoint() method when the
    *    the list is empty
    */
    void testInsertAfterPoint_EmptyList()
    {
        const char* prefix = "Test the insertAfterPoint() method for an empty list " ;
        const char* suffix1 = ":- Verify return value" ;
        const char* suffix2 = ":- Verify that the entry has been  added" ;
        string msg ;
        UtlDListIterator iter(emptyList) ;
        const UtlContainable* uReturn = iter.insertAfterPoint((UtlContainable*)commonContainables[0]) ;
        TestUtilities::createMessage(2, &msg, prefix, suffix1) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)commonContainables[0], (void*)uReturn) ;
        iter.reset() ;
        UtlContainable* uAppended = iter() ;
        TestUtilities::createMessage(2, &msg, prefix, suffix2) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)commonContainables[0], (void*)uAppended) ;

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
        const char* prefix = "Test the insertAfterPoint() method when " ;
        const char* Msgs[] = {\
               "the iterator is the starting position " , \
               "the iterator is at mid-position ", \
               "the iterator is at the last position " \
        } ;
        const char* suffix1 = ":- Verify return value" ;
        const char* suffix2 = ":- Verify value is inserted" ;
        const char* suffix3 = ":- Verify that previous value is not lost" ;
        UtlDListIterator iter(commonList) ;
        const UtlContainable* uReturn ;
        UtlContainable* uAppended ;
        UtlContainable* uOrig ;
        string msg ;

        UtlString newColString1("Insert at starting position") ;
        UtlInt newColInt2(101) ;
        UtlString newColString3 ("Insert at last position") ;

        UtlContainable* insertValues[] = { \
                 &newColString1, &newColInt2, &newColString3 \
        };
        const UtlContainable* oldValues[] = { \
                 commonContainables[0], commonContainables[1], commonContainables[5] \
        } ;

        // Since this test requires different steps for the different test data,
        // steps are executed individually rather than the regular technique of
        // iterating through the test-array loop

        //Test#1 - Verify the case when the iterator has been reset
        int ti = 0 ;
        iter.reset() ;
        uReturn = iter.insertAfterPoint(insertValues[ti]) ;
        TestUtilities::createMessage(3, &msg, prefix, Msgs[ti], suffix1) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE (msg.data(), (void*)insertValues[ti], (void*)uReturn) ;
        // The item is inserted at first position
        // old[0] is now @ pos1. old[1] is now @ pos2
        iter.reset() ;
        uAppended = iter() ;
        TestUtilities::createMessage(3, &msg, prefix, Msgs[ti], suffix2) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)insertValues[ti], (void*)uAppended) ;
        // Verify that the original item is still retained.
        uOrig = iter() ;
        TestUtilities::createMessage(3, &msg, prefix, Msgs[ti], suffix2) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)oldValues[ti], (void*)uOrig) ;

        //Test#2 - inserting at mid position
        ti = 1;
        iter.reset() ;
        iter() ; //moves cursor to 0
        iter() ; //moves cursor to 1
        iter() ; //moves cursor to 2
        // old[1] stays at pos2
        // Value is now inserted at pos3
        uReturn = iter.insertAfterPoint(insertValues[ti]) ;
        TestUtilities::createMessage(3, &msg, prefix, Msgs[ti], suffix1) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE (msg.data(), (void*)insertValues[ti], (void*)uReturn) ;
        iter.reset() ;
        iter() ;  // moves cursor to 0
        iter() ; // moves cursor to 1
        // Verify that the original item is still retained.
        uOrig = iter() ;
        TestUtilities::createMessage(3, &msg, prefix, Msgs[ti], suffix3) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)oldValues[ti], (void*)uOrig) ;
        // The item is inserted just after the position.
        uAppended = iter() ; //moves cursor to pos3 and returns item at pos2
        TestUtilities::createMessage(3, &msg, prefix, Msgs[ti], suffix2) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)insertValues[ti], (void*)uAppended) ;

        // Test#3 - Now verify when the cursor is at the last position.
        ti = 2 ;
        iter.reset() ;
        iter.toLast() ;
        uReturn = iter.insertAfterPoint(insertValues[ti]) ;
        TestUtilities::createMessage(3, &msg, prefix, Msgs[ti], suffix1) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)insertValues[ti], (void*)uReturn) ;
        iter.reset() ;
        // now move the cursor all the way to the penultimate position
        for (size_t i = 0 ; i < commonList.entries() - 1; i++)
        {
           uOrig = iter() ;
        }
        // verify original is still retained.
        TestUtilities::createMessage(3, &msg, prefix, Msgs[ti], suffix3) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)oldValues[ti], (void*)uOrig) ;
        uAppended = iter() ;
        TestUtilities::createMessage(3, &msg, prefix, Msgs[ti], suffix2) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( msg.data(), (void*)insertValues[ti], (void*)uAppended) ;

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

       UtlDList h;
       h.insert(&v1);
       h.insert(&v2);
       h.insert(&v3);
       h.insert(&v4);

       UtlDListIterator iter(h);

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
};

const int UtlDListIteratorTests::INDEX_NOT_EXIST = -1;
const int UtlDListIteratorTests::commonEntriesCount = 6;

const char* UtlDListIteratorTests::longAlphaNumString = \
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

const char* UtlDListIteratorTests::regularString = "This makes sense" ;

CPPUNIT_TEST_SUITE_REGISTRATION(UtlDListIteratorTests);
