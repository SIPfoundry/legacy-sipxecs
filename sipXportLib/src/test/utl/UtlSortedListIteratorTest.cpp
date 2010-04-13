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
#include <utl/UtlInt.h>
#include <utl/UtlString.h>
#include <utl/UtlSortedList.h>
#include <utl/UtlSortedListIterator.h>
#include <utl/UtlContainableTestStub.h>
#include <sipxunit/TestUtilities.h>

using namespace std ;


/**  This class is used to test the UtlInt utility class.
*
*    PLEASE READ THE README FILE THAT CAN FOUND IN THE PARENT OF THE DIRECTORY
*    OF THIS FILE. The Readme describes the organization / flow of tests and
*    without reading this file, the following class (and all unit tests)
*    may not make a lot of sense and might be difficult to comprehend. */
class UtlSortedListIteratorTest : public  CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(UtlSortedListIteratorTest);
    CPPUNIT_TEST(check_Advancing_Operator_StringList) ;
    CPPUNIT_TEST(check_Advancing_Operator_IntList) ;
    CPPUNIT_TEST(check_Reset) ;
    CPPUNIT_TEST(check_FindNext) ;
    CPPUNIT_TEST(removeItem) ;
    CPPUNIT_TEST_SUITE_END() ;

private:
    struct TestSortedListStruct
    {
        const char* testDescription ;
        UtlContainable* item ;
        size_t expectedIndex ;
    };

    static const int stringListCount ;
    static const int intListCount ;

    UtlSortedList emptyList ;
    UtlSortedList stringList ;
    UtlSortedList intList ;
    TestSortedListStruct* testDataForStringList ;
    TestSortedListStruct* testDataForIntList ;

public:

    UtlSortedListIteratorTest()
    {
        testDataForStringList = new TestSortedListStruct[stringListCount] ;
        testDataForIntList = new TestSortedListStruct[intListCount] ;
    }


    ~UtlSortedListIteratorTest()
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

        testDataForStringList[3].testDescription = \
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

    void check_Advancing_Operator_StringList()
    {
        UtlSortedListIterator slIter(stringList) ;

        for (int i = 0 ; i < stringListCount ; i++)
        {
            UtlContainable* uNext = slIter() ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(testDataForStringList[i].testDescription, \
               testDataForStringList[i].item, uNext) ;
        }
        // Verify that the iterator returns null after the last advancing has been called
        UtlContainable* uNext = slIter() ;

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Null is returned after all items have been " \
            "advanced ", (void*)NULL, (void*)uNext) ;
    }

    void check_Advancing_Operator_IntList()
    {
        UtlSortedListIterator slIter(intList) ;

        for (int i = 0 ; i < intListCount ; i++)
        {
            UtlContainable* uNext = slIter() ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(testDataForIntList[i].testDescription, \
               testDataForIntList[i].item, uNext) ;
        }
        // Verify that the iterator returns null after the last advancing has been called
        UtlContainable* uNext = slIter() ;

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Null is returned after all items have been " \
            "advanced ", (void*)NULL, (void*)uNext) ;
    }

    /** Verify that calling reset() pushes the iterator to
    *   the begining.
    */
    void check_Reset()
    {
        UtlSortedListIterator slIter(stringList) ;
        slIter() ;
        slIter() ;
        slIter.reset() ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Verify that calling reset on the iterator " \
            "positions it to the begining", (void*)testDataForStringList[0].item, \
            (void*)slIter()) ;
    }



    /** Test the findNext method. The various test case
    *   for this are :-
    *     a) Verify that all the elements can be found
             when the iterator has been reset.
    *     b) Verify that any element after the current iterator position can be searched.
    &     c)  Verify that any element before the current position is not returned in a search.
    *  Verify htat when there are two similar matches than
    *    either of the items is returned when searching.
    */
    void check_FindNext()
    {
        UtlSortedListIterator iter(stringList);
        const char* message;
        // Search for the first item
        message = "Search for the first item" ;
        UtlContainable* found = iter.findNext(testDataForStringList[0].item) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(message, testDataForStringList[0].item, found) ;

         message = "Search for the last item" ;
         iter.reset() ;
         found = iter.findNext(testDataForStringList[stringListCount-1].item) ;
         CPPUNIT_ASSERT_EQUAL_MESSAGE(message, testDataForStringList[stringListCount-1].item,  found) ;

         message = "Search for a middle item" ;
         iter.reset() ;
         found = iter.findNext(testDataForStringList[2].item) ;
         CPPUNIT_ASSERT_EQUAL_MESSAGE(message, testDataForStringList[2].item, found);

         message = "When the iterator has not been reset, search for an item that "\
                   "is not yet been read" ;
         iter.reset() ;
         iter() ;
         found = iter.findNext(testDataForStringList[2].item) ;
         CPPUNIT_ASSERT_EQUAL_MESSAGE(message, testDataForStringList[2].item, found)  ;

         UtlString dupString1("Mnop") ;
         UtlString dupString2("Mnop");
         stringList.insert(&dupString1) ;
         stringList.insert(&dupString2) ;

         iter.reset();
         found = iter.findNext(&dupString2) ;
         bool isSuccess = (found == &dupString1 || found == &dupString2) ;
         CPPUNIT_ASSERT_MESSAGE(message, isSuccess) ;
    }


    /*!a test case for the interaction of remove() and item().
    */
    void removeItem()
    {
       UtlString v1("a");
       UtlString v2("b");
       UtlString v3("c");
       UtlString v4("d");
       UtlContainable* e;

       UtlSortedList h;
       h.insert(&v1);
       h.insert(&v2);
       h.insert(&v3);
       h.insert(&v4);

       UtlSortedListIterator iter(h);

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

const int UtlSortedListIteratorTest::stringListCount = 4 ;
const int UtlSortedListIteratorTest::intListCount = 4 ;

CPPUNIT_TEST_SUITE_REGISTRATION(UtlSortedListIteratorTest);
