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
#include <time.h>
#include <os/OsDefs.h>

#include <utl/UtlVoidPtr.h>
#include <utl/UtlString.h>
#include <utl/UtlInt.h>
#include <utl/UtlHashBag.h>
#include <utl/UtlContainableTestStub.h>
#include <sipxunit/TestUtilities.h>

using namespace std ;

class UtlHashBagTest : public  CppUnit::TestCase
{

    CPPUNIT_TEST_SUITE(UtlHashBagTest);
    CPPUNIT_TEST(checkSanity_Insert_Entries_And_At) ;
    CPPUNIT_TEST(testInsert) ;
    CPPUNIT_TEST(testFind) ;
    CPPUNIT_TEST(testContains) ;
    CPPUNIT_TEST(testRemove) ;
    CPPUNIT_TEST(testRemoveAndDestroy) ;
    CPPUNIT_TEST(testIsEmpty) ;
    CPPUNIT_TEST(testClear) ;
    CPPUNIT_TEST(testClearAndDestroy) ;
    CPPUNIT_TEST(testRemoveReference) ;
    CPPUNIT_TEST(testOneThousandInserts);
    CPPUNIT_TEST_SUITE_END();

private:

    static const int INDEX_NOT_EXIST ;
    static const int commonEntriesCount ;

    UtlHashBag commonList ;
    UtlHashBag emptyList ;
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

    enum FindOrContains { TEST_FIND, TEST_CONTAINS} ;
    enum TestInsertOrAppend {TEST_APPEND, TEST_INSERT} ;
    enum RemoveType {TEST_REMOVE, TEST_REMOVE_REF } ;

public:

    UtlHashBagTest()
    {
        commonContainables = new UtlContainable*[commonEntriesCount] ;
        commonContainables_Clone = new UtlContainable*[commonEntriesCount] ;
    }

    ~UtlHashBagTest()
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
        commonList.insert(&commonString1) ;
        commonContainables[0] = &commonString1 ;
        commonContainables_Clone[0] = &commonString1_clone ;
        commonList.insert(&commonInt1) ;
        commonContainables[1] = &commonInt1 ;
        commonContainables_Clone[1] = &commonInt1_clone ;
        commonList.insert(&commonInt2) ;
        commonContainables[2] = &commonInt2 ;
        commonContainables_Clone[2] = &commonInt2_clone;
        commonList.insert(&commonString2) ;
        commonContainables[3] = &commonString2 ;
        commonContainables_Clone[3] = &commonString2_clone ;
        commonList.insert(&commonInt3) ;
        commonContainables[4] = &commonInt3 ;
        commonContainables_Clone[4] = &commonInt3_clone ;
        commonList.insert(&commonString3) ;
        commonContainables[5] = &commonString3 ;
        commonContainables_Clone[5] = &commonString3_clone ;
    }

    void tearDown()
    {
    }

    // Sandbox - Please ignore.
    void DynaTest()
    {
    } //DynaTest


    /*a! This test is more of a sanity check to verify that
    *    the basic insert(), entries() and at() methods work as expected.
    *    All future tests will depend heavily on the at() method
    *    and the most common way of having something in the list is
    *    by means of the insert() method.
    *
    */
    void checkSanity_Insert_Entries_And_At()
    {
        // Verify that the hashtables that were created and populated in the setup method
        // have their entries set to the right value.
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Verify that the entries() for an empty HashTable returns 0", (int)emptyList.entries(), 0) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Verify the entries() method for a HashTable", commonEntriesCount, (int)commonList.entries()) ;
    }// checkSanity_Append_And_At()


    /*a! Test the insert method for a list that is not empty. Add both unique and non-unique entries.
    *
    *    The test data for this test is :-
    *      a) Add a new UtlInt
    *      b) Add a new UtlString
    *      c) Add a new UtlVoidPtr
    *      d) Add a new Containable such that a similar one (value match) exists already
    *      e) Add a new Containable such that the same one (reference match) exists already
    */
    void testInsert()
    {
        struct testInsertStructure
        {
            const char* testDescription ;
            UtlContainable* itemToAdd ;
            UtlContainable* expectedReturnValue ;
            // In case of the hashtable, searching for a key can return *ANY* of the values
            // that matches and we dont care about which one. so we need two set of expected values
            UtlContainable* expectedFoundValue ;
            UtlContainable* altExpectedFoundValue ;
        } ;
        const char* prefix = "Test the insert(UtlContainable*) method for a non empty HashTable " ;
        const char* suffix1 = " :- Verify return value" ;
        const char* suffix2 = " :- Verify that the value is inserted" ;
        const char* suffix3 = " :- Verify that the number of entries is incremented by one" ;

        UtlInt uInt = UtlInt(1234) ;
        UtlString uString = UtlString("Test String") ;
        UtlVoidPtr uVoidPtr((char*)"Hello world") ;

        testInsertStructure testData[] = { \
            { "Add a new UtlInt ", &uInt, &uInt, &uInt, &uInt}, \
            { "Add a new UtlString ", &uString, &uString, &uString, &uString}, \
            { "Add a new UtlVoidPtr ", &uVoidPtr, &uVoidPtr, &uVoidPtr, &uVoidPtr}, \
            { "Add a Containable such that an identical one(value match) " \
              "exists in the table ", commonContainables_Clone[3], commonContainables_Clone[3], \
              commonContainables[3], commonContainables_Clone[3] }, \
            { "Add a Containable such that an exact duplicate " \
              "(including reference match) exists in the table ", commonContainables[4],
              commonContainables[4], commonContainables[4] } \
        } ;

        int expCount  = commonEntriesCount;

        string msg ;
        UtlContainable* uAct ;
        UtlContainable* uReturn ;
        /* :TODO: this part of the test is in error.
         *    when there is more than one of the same value in the bage,
         *    the find() method may return any of the objects with that value.
         */
        int testCount = sizeof(testData)/sizeof(testData[0]) ;
        for (int i = 0 ; i < testCount ; i++)
        {
            uReturn = commonList.insert(testData[i].itemToAdd) ;
            expCount++ ;
            uAct = commonList.find(testData[i].itemToAdd) ;
            TestUtilities::createMessage(3, &msg, prefix, \
                 testData[i].testDescription, suffix1) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), \
                testData[i].expectedReturnValue, uReturn) ;

            // when there is more than one of the same value in the bag,
            // the find() method may return any of the objects with that value.
            bool isFound = (uAct == testData[i].expectedFoundValue) || \
                 (uAct == testData[i].altExpectedFoundValue) ;
            TestUtilities::createMessage(3, &msg, prefix, \
                testData[i].testDescription, suffix2) ;
            CPPUNIT_ASSERT_MESSAGE(msg.data(), isFound) ;
            TestUtilities::createMessage(3, &msg, prefix, \
                testData[i].testDescription, suffix3) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expCount, (int)commonList.entries()) ;
        }
    } //testInsert



    /*!a Test case for the find() method.
    *
    *    The test data for this test case are:-
    *        a) is the first element
    *        b) is the last element
    *        c) is a mid element (unique match)
    *        d) has two value matches but a single ref match
    *        e) has two ref matches
    *        f) has a value match but no ref match
    *        g) has no match at all
    */
    void testFind()
    {
        utlTestFind_And_Contains(TEST_FIND) ;
    }

    /*!a Test case for the contains() method
    *
    *    The test data for this test case are the same as
    *    testFind() test case.
    */
    void testContains()
    {
        utlTestFind_And_Contains(TEST_CONTAINS) ;
    }

    void utlTestFind_And_Contains(FindOrContains type)
    {
        const int testCount = 7 ;
        const char* prefixFind = "Test the find() method when the match " ;
        const char* prefixContains = "Test the contains() method when the match " ;
        const char* Msgs[] = { \
               "is the first element ", \
               "is the last element ", \
               "is a mid element (unique match) ", \
               "has two value matches but a single ref match ", \
               "has two ref matches", \
               "has a value match but no ref match", \
               "has no match at all" \
        } ;

       // insert two elements to satisfy (d) and (e)
       commonList.insert( commonContainables_Clone[4]) ;
       commonList.insert( commonContainables[3]) ;

       UtlString noExist("This cannot and should not exist!!!") ;

       UtlContainable* searchValues[] = { \
                 commonContainables[0], commonContainables[5], commonContainables[2], \
                 commonContainables[4], commonContainables[3], \
                 commonContainables_Clone[2], &noExist \
       } ;

       bool expectedValues_Contains[]    = {true, true, true, true, true, true, false } ;

       UtlContainable* searchValuesForFind[] = { \
                 commonContainables[0], commonContainables[5], commonContainables[2], \
                 commonContainables[4], commonContainables[3], \
                 commonContainables_Clone[1], &noExist \
       } ;

       // In case of the hashtable, searching for a key can return *ANY* of the values
       // that matches and we dont care about which one. so we need two set of expected values
       UtlContainable* expValues_Find[][2] = { \
                 {commonContainables[0], commonContainables[0]}, \
                 {commonContainables[5], commonContainables[2]}, \
                 {commonContainables[2], commonContainables[2]}, \
                 {commonContainables_Clone[4], commonContainables[4]}, \
                 {commonContainables[3], commonContainables[3]}, \
                 {commonContainables[1], commonContainables[1]}, \
                 {NULL, NULL} \
       } ;

       for (int i = 0 ; i < testCount ; i++)
       {
           string msg ;
           if (type == TEST_FIND)
           {
               bool isFound = false ;
               UtlContainable* act = commonList.find(searchValuesForFind[i]) ;
               // If we are expecting either 'A' or 'B' for the search
               isFound = (act == expValues_Find[i][0]) || (act == expValues_Find[i][1]) ;
               TestUtilities::createMessage(2, &msg, prefixFind, Msgs[i]) ;
               CPPUNIT_ASSERT_MESSAGE(msg.data(), isFound) ;
           }
           else if (type == TEST_CONTAINS)
           {
               UtlBoolean act = commonList.contains(searchValues[i]) ;
               UtlBoolean exp = (UtlBoolean)expectedValues_Contains[i] ;
               TestUtilities::createMessage(2, &msg, prefixContains, Msgs[i]) ;
               CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), exp, act) ;
           }
       }

    }//utlTestIndex


    /*!a Test case to test the destroy()
    *    method.
    */
    void testRemoveAndDestroy()
    {
        const char* prefix  = "test the destroy() method " ;

        // The ContainableTestStub has been implemented such that a static
        // counter is incremented everytime an instance is created and
        // the counter is decremented everytime an instance is destroyed.
        UtlContainableTestStub* uStub = new UtlContainableTestStub(0) ;
        UtlContainableTestStub* uStubPtr = new UtlContainableTestStub(101) ;
        commonList.insert(uStub) ;
        commonList.insert(uStubPtr) ;

        string msg ;
        int cCountBefore = UtlContainableTestStub :: getCount() ;
        UtlBoolean retValue = commonList.destroy(uStubPtr) ;
        int cCountAfter  ;
        uStubPtr = NULL ;
        TestUtilities::createMessage(2, &msg, prefix, ":- Verify the return value") ;
        CPPUNIT_ASSERT_MESSAGE(msg.data(), retValue) ;

        // To verify that the object was destroyed, check to see if the static count has been
        // decremented. If yes, it means that the destructor was called.
        cCountAfter = UtlContainableTestStub :: getCount() ;
        TestUtilities::createMessage(2, &msg, prefix, ":- Verify that the object was deleted") ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), cCountBefore -1, cCountAfter) ;

        // THe way to verify if the object has been removed is to
        // create a new stub such that it has the same value as
        // the removed stub. Try to find the new stub
        UtlContainableTestStub uStubNew(101) ;
        UtlContainable* uSearch = commonList.find(&uStubNew) ;
        TestUtilities::createMessage(2, &msg, prefix, ":- Verify that the entry is removed") ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)NULL, (void*)uSearch) ;

        // Now test the case when you have added multiple deletable keys
        // and the first key inserted is deleted first.
        UtlContainableTestStub* uStubPtr2 = new UtlContainableTestStub(201) ;
        UtlContainableTestStub* uStubPtr3 = new UtlContainableTestStub(201) ;
        commonList.insert(uStubPtr2) ;
        commonList.insert(uStubPtr3) ;
        UtlInt uTestInt(2031) ;
        commonList.insert(&uTestInt) ;
        // after destroying, either uStubPtr or uStubPtr3 might have gotten deleted and
        // we have no way to find out which one. So create a new ContainableTestStub
        // and use that for search
        UtlContainableTestStub uStubTemp(201) ;


        cCountBefore = UtlContainableTestStub :: getCount() ;
        commonList.destroy(&uStubTemp) ;
        cCountAfter = UtlContainableTestStub :: getCount() ;

        uSearch = commonList.find(&uStubTemp) ;
        const char*  msgTemp = "Verify that doing a removeAndDestroy on " \
             "an item that has multiple matches removes only one entry " ;
        TestUtilities::createMessage(2, &msg, msgTemp, " :- Verify value is still found") ;
        CPPUNIT_ASSERT_MESSAGE("Verify that doing a removeAndDestroy on " \
             "an item that has multiple matches removes only one entry", \
             uSearch == uStubPtr2 || uSearch == uStubPtr3) ;
        TestUtilities::createMessage(2, &msg, msgTemp, " :- Verify value *was* destroyed") ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), cCountBefore -1, cCountAfter) ;

        msgTemp = "Verify that the remaining entry can also be deleted" ;
        cCountBefore = UtlContainableTestStub :: getCount() ;
        commonList.destroy(&uStubTemp) ;
        cCountAfter = UtlContainableTestStub :: getCount() ;
        TestUtilities::createMessage(2, &msg, msgTemp, " :- Verify value *was* destroyed") ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), cCountBefore -1, cCountAfter) ;

        // In all the above tests, a total of 5 entries were added
       // and 3 were removed.
        int finalCount = commonEntriesCount + 2 ;
        msgTemp = "Verify that the HashTable stil has the other entries" ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msgTemp, finalCount, (int)commonList.entries()) ;
    }

    /*!a Test case for testRemove()
    *
    *     The Test data for this test case is
    *          a) is an entry's reference
    *          b) is an entry's value(not reference)
    *          c) is the first of multiple matches and is the value match
    *          d) is the second of multiple matches.
    *          e) has no match at all
    */
    void testRemove()
    {
        int testCount = 4 ;
        const char* prefix = "test the remove(UtlContainable* c) method where c " ;
        const char* Msgs[] = { \
               "is an entry's reference ", \
               "is an entry's value(not reference) ", \
               "is first of multiple value matches ", \
               "has no match at all " \
        } ;
        const char* suffix1 = " :- Verify returned value" ;
        const char* suffix2 = " :- Verify total entries" ;

        // Insert a new value such that its value matches(isEqual to)
        // one of the existing items
        commonList.insert(commonContainables_Clone[4]) ;
        UtlString notExistContainable("This cannot and willnot exist");

        UtlContainable* dataForRemove[] = { \
                           commonContainables[0], \
                           commonContainables_Clone[2], \
                           commonContainables[4], \
                           &notExistContainable \
        } ;

        int totalEnt = commonEntriesCount + 1;

        UtlBoolean expectedReturnValues[] = { \
            true, true, true, false \
        } ;

        int entriesValue[] = { --totalEnt, --totalEnt, --totalEnt, totalEnt } ;

        totalEnt = commonEntriesCount + 1;


        for (int i = 0 ; i < testCount ; i++)
        {
            string msg ;

            TestUtilities::createMessage(3, &msg, prefix, Msgs[i], suffix1) ;
            UtlContainable* retValue ;
            retValue = commonList.remove(dataForRemove[i]) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (UtlBoolean)(retValue!=NULL), \
                           (UtlBoolean)expectedReturnValues[i]) ;

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

        UtlHashBag newList ;
        UtlHashBag secondNewList ;
        UtlHashBag commonList_Clone ;

        // Add a single entry to the list.
        newList.insert(commonContainables[0])  ;
        UtlString uS1("Tester string") ;

        // populate the second list and then removeAll entries.
        secondNewList.insert(&uS1) ;
        UtlInt uI1 = UtlInt(232) ;
        secondNewList.insert(&uI1) ;
        secondNewList.insert(commonContainables[3]) ;

        secondNewList.removeAll() ;
        UtlHashBag* testLists[] = { \
                 &emptyList, &newList, &commonList,  &secondNewList \
        } ;

        bool exp[] = { true, false, false, true } ;
        for (int i = 0 ; i < testCount; i++)
        {
            string msg ;
            TestUtilities::createMessage(2, &msg, prefix, Msgs[i]) ;
            UtlBoolean act = testLists[i] -> isEmpty() ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (UtlBoolean)exp[i], act) ;
        }
    } // testIsEmpty

    /*!a test the removeAll() method.
    *
    *    The test data for this method is
    *        a) When the list is empty
    *        b) When the list has one entry.
    *        c) When the list multiple entries
    *        d) When removeAll has been called and entries are added again
    *        e) When the removeAll is called twice on the list.
    *        f) When the removeAll is call on a list that has muliple entries
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

        UtlHashBag uSingleList ;
        UtlHashBag uAddAfterClear ;
        UtlHashBag uDoubleClear ;

        // populate the hashtable with the 'common' values
        UtlHashBag commonList_Clone ;
        for (int i = 0 ; i < commonEntriesCount ; i++)
        {
            commonList_Clone.insert(commonContainables_Clone[i]) ;
        }
        // Add two values such that one of them has a 'value'
        // match already in the table and the other one has
        // a ref match.
        commonList_Clone.insert(commonContainables_Clone[3]) ;
        commonList_Clone.insert(commonContainables[5]) ;

        // Add a single entry to the list that is to be made up
        // of just one element
        uSingleList.insert(&commonString1) ;

        // call removeAll() on a list and then add entries again.
        uAddAfterClear.insert(&commonInt1) ;
        uAddAfterClear.insert(&commonString1) ;
        uAddAfterClear.removeAll() ;
        uAddAfterClear.insert(&commonInt2) ;

        // call removeAll on a list twice.
        uDoubleClear.insert(&commonString3) ;
        uDoubleClear.insert(&commonInt3) ;
        uDoubleClear.removeAll() ;

        UtlHashBag* testLists[] = { \
                     &emptyList, &uSingleList, &commonList, &uAddAfterClear, &uDoubleClear, &commonList_Clone
        } ;
        int expEntries[] = { 0 , 0, 0, 1, 0, 0} ;

        // since we are not calling removeAll for all the lists, do it outside the for loop for those
        // that require to be cleared.
        emptyList.removeAll() ;
        uSingleList.removeAll() ;
        commonList.removeAll() ;
        commonList_Clone.removeAll() ;
        // no removeAll() for uAddAfterClear
        uDoubleClear.removeAll() ;

        for ( int j = 0 ; j < testCount ; j++)
        {
            string msg ;
            TestUtilities::createMessage(3, &msg, prefix, Msgs[j], suffix) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data() , expEntries[j], (int)testLists[j] -> entries()) ;

        }

    } //testClear()


    /*!a Test case to test the DestroyAll()
    *    method.
    */
    void testClearAndDestroy()
    {
        const int testCount = 3 ;
        int cCountBefore = UtlContainableTestStub :: getCount() ;
        const char* prefix  = "test the destroyAll() method " ;
        const char* suffix1  = " :- sanity check to double check that the counter was incremented for every new() call" ;
        const char* suffix2 = " :- Verify that all entries are removed and the objects are deleted" ;

        UtlContainableTestStub* uStub ;
        UtlContainableTestStub* uStubPtr ;
        UtlContainableTestStub* uStubPtr2 ;
        uStub = new UtlContainableTestStub(0) ;
        uStubPtr = new UtlContainableTestStub(201) ;
        uStubPtr2 = new UtlContainableTestStub(201) ;
        emptyList.insert(uStub) ;
        emptyList.insert(uStubPtr) ;
        emptyList.insert(uStubPtr2) ;

        cCountBefore = UtlContainableTestStub :: getCount() - cCountBefore ;
        emptyList.destroyAll() ;
        int cCountAfter = UtlContainableTestStub :: getCount() ;

        string msg ;
        // Make sure that static count was incremented for every instance
        // of the TestStub that was created.
        TestUtilities::createMessage(2, &msg, prefix, suffix1) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(),  testCount, cCountBefore) ;

        // Verify that the list has no entries left after destroyAll()
        // and also ensure that all the TestStub instances were deleted.
        TestUtilities::createMessage(2, &msg, prefix, suffix2) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), 0, cCountAfter) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), 0, (int)emptyList.entries()) ;
    } //testClearAndDestroy

   void testRemoveReference()
      {
         // the following two entries collide if the initial bucket size is 16
         UtlInt int1(1);
         UtlInt int16(16);

         UtlInt int2a(2);
         UtlInt int2b(2);
         UtlInt int3(3);

         UtlHashBag bag;

         CPPUNIT_ASSERT( bag.numberOfBuckets() == 16 ); // check assumption of collision

         // Load all the test objects
         CPPUNIT_ASSERT( bag.insert(&int1) == &int1 );
         CPPUNIT_ASSERT( bag.insert(&int16) == &int16 );
         CPPUNIT_ASSERT( bag.insert(&int2a) == &int2a );
         CPPUNIT_ASSERT( bag.insert(&int2b) == &int2b );
         CPPUNIT_ASSERT( bag.insert(&int3) == &int3 );

         // Check that everything is there
         CPPUNIT_ASSERT( bag.entries() == 5 );
         CPPUNIT_ASSERT( bag.contains(&int1) );
         CPPUNIT_ASSERT( bag.contains(&int16) );
         CPPUNIT_ASSERT( bag.contains(&int2a) ); // cannot test for 2a and 2b independently
         CPPUNIT_ASSERT( bag.contains(&int3) );

         // Take entry 1 out (might collide w/ 16)
         CPPUNIT_ASSERT( bag.removeReference(&int1) == &int1 );

         // Check that everything except entry 1 is still there, and that 1 is gone
         CPPUNIT_ASSERT( bag.entries() == 4 );
         CPPUNIT_ASSERT( ! bag.contains(&int1) );
         CPPUNIT_ASSERT( bag.contains(&int16) );
         CPPUNIT_ASSERT( bag.contains(&int2a) );// cannot test for 2a and 2b independently
         CPPUNIT_ASSERT( bag.contains(&int3) );

         // Put entry 1 back in (so that 16 will collide w/ it again)
         CPPUNIT_ASSERT( bag.insert(&int1) == &int1 );

         // Check that everything is there
         CPPUNIT_ASSERT( bag.entries() == 5 );
         CPPUNIT_ASSERT( bag.contains(&int1) );
         CPPUNIT_ASSERT( bag.contains(&int16) );
         CPPUNIT_ASSERT( bag.contains(&int2a) );
         CPPUNIT_ASSERT( bag.contains(&int3) );

         // Take entry 16 out (might collide w/ 1)
         CPPUNIT_ASSERT( bag.removeReference(&int16) == &int16 );

         // Check that everything except entry 16 is still there, and that 16 is gone
         CPPUNIT_ASSERT( bag.entries() == 4 );
         CPPUNIT_ASSERT( bag.contains(&int1) );
         CPPUNIT_ASSERT( ! bag.contains(&int16) );
         CPPUNIT_ASSERT( bag.contains(&int2a) );// cannot test for 2a and 2b independently
         CPPUNIT_ASSERT( bag.contains(&int3) );

         // remove 2a (and ensure that you don't get back 2b)
         CPPUNIT_ASSERT( bag.removeReference(&int2a) == &int2a );

         // Check that everything that should be is still there
         CPPUNIT_ASSERT( bag.entries() == 3 );
         CPPUNIT_ASSERT( bag.contains(&int1) );
         CPPUNIT_ASSERT( ! bag.contains(&int16) );
         CPPUNIT_ASSERT( bag.find(&int2a) == &int2b ); // equal values, but now there's only one
         CPPUNIT_ASSERT( bag.contains(&int3) );

         // remove 3 (no collision for this one)
         CPPUNIT_ASSERT( bag.removeReference(&int3) == &int3 );

         // Check that everything that should be is still there
         CPPUNIT_ASSERT( bag.entries() == 2 );
         CPPUNIT_ASSERT( bag.contains(&int1) );
         CPPUNIT_ASSERT( ! bag.contains(&int16) );
         CPPUNIT_ASSERT( bag.find(&int2a) == &int2b ); // equal values, but now there's only one
         CPPUNIT_ASSERT( ! bag.contains(&int3) );

         // remove 3 again - should fail this time
         CPPUNIT_ASSERT( bag.removeReference(&int3) == NULL );

         // Check that everything that should be is still there
         CPPUNIT_ASSERT( bag.entries() == 2 );
         CPPUNIT_ASSERT( bag.contains(&int1) );
         CPPUNIT_ASSERT( ! bag.contains(&int16) );
         CPPUNIT_ASSERT( bag.find(&int2a) == &int2b ); // equal values, but now there's only one
         CPPUNIT_ASSERT( ! bag.contains(&int3) );

      }

      void testOneThousandInserts()
      {
         // Test case used to validate fix to issue XPL-169
         const int COUNT = 1000;
         const char stringPrefix[] = "Apofur81Kb";
         UtlHashBag bag;
         char tmpString[20];

         for( int i = 0; i < COUNT; i++)
         {
            sprintf(tmpString, "%s%d", stringPrefix, i);
            UtlString *stringToInsert = new UtlString();
            *stringToInsert = tmpString;

            CPPUNIT_ASSERT( ! bag.contains( stringToInsert ) );
            bag.insert( stringToInsert );
            CPPUNIT_ASSERT_EQUAL( i+1, (int)bag.entries() );

            for( unsigned int j = 0; j < bag.entries(); j++ )
            {
               // verify that all entries are indeed in the bag
               sprintf( tmpString, "%s%d", stringPrefix, j );
               UtlString stringTolookUp( tmpString );
               CPPUNIT_ASSERT_MESSAGE( tmpString, bag.contains( &stringTolookUp ) );
            }
         }
      }
};

const char* UtlHashBagTest::longAlphaNumString = \
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

const char* UtlHashBagTest::regularString = "This makes sense" ;


// letting the compiler know that such a variable exists.
const int UtlHashBagTest::commonEntriesCount = 6 ;
const int UtlHashBagTest::INDEX_NOT_EXIST = -1;

CPPUNIT_TEST_SUITE_REGISTRATION(UtlHashBagTest);
