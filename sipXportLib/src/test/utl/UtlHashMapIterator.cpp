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
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlContainableTestStub.h>
#include <sipxunit/TestUtilities.h>

using namespace std ;



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

static const char* regularString = "This makes sense" ;

static const int commonEntriesCount=4  ;
static const int INDEX_NOT_EXIST = -1;


/**  This class is used to test the UtlHashMapIterator utility class.
*
*    PLEASE READ THE README FILE THAT CAN FOUND IN THE PARENT OF THE DIRECTORY
*    OF THIS FILE. The Readme describes the organization / flow of tests and
*    without reading this file, the following class (and all unit tests)
*    may not make a lot of sense and might be difficult to comprehend.
*/
class UtlHashMapIteratorTests : public CppUnit::TestCase
{

    CPPUNIT_TEST_SUITE(UtlHashMapIteratorTests);
    CPPUNIT_TEST(checkSanity_Insert_Entries_And_At);
    CPPUNIT_TEST(testAdvancingOperator_And_KeyMethod) ;
    CPPUNIT_TEST(removeKeyValue) ;
    CPPUNIT_TEST_SUITE_END();

private:

    struct KeyValueStructure
    {
        const char* testDescription ;
        const UtlContainable* key ;
        const UtlContainable* value ;
    };

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


    KeyValueStructure* commonTestData ;
    KeyValueStructure* commonTestData_clone ;

    enum FindOrContains { TEST_FIND, TEST_CONTAINS} ;
    enum TestInsertOrAppend {TEST_APPEND, TEST_INSERT} ;
    enum RemoveType {TEST_REMOVE, TEST_REMOVE_KEY_AND_VALUE} ;

public:

    UtlHashMapIteratorTests()
    {
        commonTestData = new KeyValueStructure[commonEntriesCount] ;
        commonTestData_clone = new KeyValueStructure[commonEntriesCount] ;
    }


    ~UtlHashMapIteratorTests()
    {
        delete[] commonTestData ;
        delete[] commonTestData_clone ;
    }

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
        commonTestData[0].testDescription = "key = UtlString. value = UtlInt" ;
        commonTestData[0].key = &commonString1;
        commonTestData[0].value = &commonInt1 ;
        commonTestData[1].testDescription = "key = UtlInt. value = UtlString" ;
        commonTestData[1].key = &commonInt2 ;
        commonTestData[1].value = &commonString2 ;
        commonTestData[2].testDescription = "key = UtlString. value = UtlString" ;
        commonTestData[2].key = &commonString3 ;
        commonTestData[2].value = &commonString4 ;
        commonTestData[3].testDescription = "key = UtlVoidPtr. value = UtlInt" ;
        commonTestData[3].key = &commonInt3 ;
        commonTestData[3].value = &commonInt4 ;

        // Similarly create a clone of the list and the expected data.
        commonList_clone.insertKeyAndValue(&commonString1_clone, &commonInt1_clone) ;
        commonList_clone.insertKeyAndValue(&commonInt2_clone, &commonString2_clone) ;
        commonList_clone.insertKeyAndValue(&commonString3_clone, &commonString4_clone) ;
        commonList_clone.insertKeyAndValue(&commonInt3_clone, &commonInt4_clone) ;

        // The above values also become the name-value pair array
        // The first index of the 2nd dimension represents the key
        // while the second index represents the value expected.
        commonTestData_clone[0].key = &commonString1_clone;
        commonTestData_clone[1].value = &commonInt1_clone ;
        commonTestData_clone[1].key = &commonInt2_clone ;
        commonTestData_clone[1].value = &commonString2_clone ;
        commonTestData_clone[2].key = &commonString3_clone ;
        commonTestData_clone[2].value = &commonString4_clone ;
        commonTestData_clone[3].key = &commonInt3_clone ;
        commonTestData_clone[3].value = &commonInt4_clone ;
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
    *    by means of the insert() method.
    *
    */
    void checkSanity_Insert_Entries_And_At()
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Verify that the entries() for an empty HashMap \
                                  returns 0", (int)emptyList.entries(), 0) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Verify the entries() method for a HashMap", \
                                  commonEntriesCount, (int)commonList.entries()) ;
    }// checkSanity_Append_And_At()


    /*!a Test case for the () operator and the key() method.
    *
    *    The test data for this test is :-
    *       Verify that all items in the dictionary can be iteratated through.
    *       Verify that attempting to iterate when all items have been read returns a null
    */
    void testAdvancingOperator_And_KeyMethod()
    {
        const char* prefix1 = "Verify the () operator " ;
        const char* prefix2 = "Verify the key() method " ;
        const char* prefix3 = "Verify the value() method " ;
        string Message ;

        UtlHashMapIterator iter(commonList) ;
        iter() ;
        UtlContainable* actualKey ;
        UtlContainable* actualValue ;
        // Test the key/value methods when the iterator has been reset.
        iter.reset() ;
        actualKey = iter.key() ;
        actualValue = iter.value() ;

        TestUtilities::createMessage(2, &Message, prefix2, \
                                     "when the iterator has been reset") ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), (void*)NULL, (void*)actualKey) ;

        TestUtilities::createMessage(2, &Message, prefix3, \
                                     "when the iterator has been reset") ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), (void*)NULL, (void*)actualValue) ;

        // Now iterate through the whole iterator and verify that all the items are
        // retreived. The () operator retreives the key for next item in the list. The
        // key item should retreive the item under the current position. (That is,
        // the key() method should always return what the previous () returned). The
        // value() method should return the value corresponding to the key under the current
        // cursor.
        // Loop to iterate through all the entries in the Hashdictionary
        for (int i = 0 ; i < commonEntriesCount ; i++)
        {
            actualKey = iter() ;
            actualKey = iter.key() ;
            actualValue = iter.value() ;
            int foundIndex = -1 ;
            // Now iterate through the test data array and see if the key retreived
            // above matches any one of the array values. If it does, mark that as the
            // foundIndex
            for (int j = 0 ; j < commonEntriesCount; j++)
            {
                if (commonTestData[j].key && commonTestData[j].key == actualKey)
                {
                    foundIndex = j ;
                    break ;
                }
            }
            if (foundIndex != -1)
            {
                TestUtilities::createMessage(2, &Message, prefix1, \
                                     commonTestData[foundIndex].testDescription) ;
                // If we have come into this loop,it means that we have already
                // found the key using the () operator.
                CPPUNIT_ASSERT_MESSAGE(Message.data(), true) ;
                actualKey = iter.key() ;
                actualValue = iter.value() ;
                TestUtilities::createMessage(2, &Message, prefix2, \
                                     commonTestData[foundIndex].testDescription) ;
                CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                     (void*)commonTestData[foundIndex].key, (void*)actualKey) ;
                TestUtilities::createMessage(2, &Message, prefix3, \
                                     commonTestData[foundIndex].testDescription) ;
                CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                     (void*)commonTestData[foundIndex].value, (void*)actualValue) ;

                // Now set the key in the test data array for this index to NULL.
                // The idea behind this is that if a key is retreived once, it should never be
                // retreived again!
                commonTestData[foundIndex].key = NULL ;
            }
            else
            {
                TestUtilities::createMessage(2, &Message, prefix1, \
                                     "- Did not find one of the expected keys") ;
                CPPUNIT_ASSERT_MESSAGE(Message.data(), false) ;
            }
        }// End loop to iterate through all entries in the hashTable

        // Now verify that iterating through the iterator after all entries
        // have been read, returns NULL.
        actualKey = iter() ;
        const char* msgLast = "when all entries have been read" ;
        TestUtilities::createMessage(2, &Message, prefix1, msgLast) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), (void*)NULL, (void*)actualKey) ;

        actualKey = iter.key() ;
        actualValue = iter.value() ;
        TestUtilities::createMessage(2, &Message, prefix2, msgLast) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), (void*)NULL, (void*)actualKey) ;
        TestUtilities::createMessage(2, &Message, prefix3, msgLast) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), (void*)NULL, (void*)actualValue) ;
    } //testAdvancingOperator_And_KeyMethod()


    /*!a test case for the interaction of remove(), key(), and value()
    */
    void removeKeyValue()
    {
       UtlString v1("a");
       UtlString v2("b");
       UtlString v3("c");
       UtlString v4("d");
       UtlContainable* e;

       UtlHashMap h;
       h.insertKeyAndValue(&v1, &v1);
       h.insertKeyAndValue(&v2, &v2);
       h.insertKeyAndValue(&v3, &v3);
       h.insertKeyAndValue(&v4, &v4);

       UtlHashMapIterator iter(h);

       // Check that key() and value() return NULL in the initial state.
       CPPUNIT_ASSERT(iter.key() == NULL);
       CPPUNIT_ASSERT(iter.value() == NULL);

       // Step the iterator and check that key() and value() return non-NULL.
       e = iter();
       CPPUNIT_ASSERT(e != NULL);
       CPPUNIT_ASSERT(iter.key() != NULL);
       CPPUNIT_ASSERT(iter.value() != NULL);

       // Delete the element and check that key() and value() return NULL.
       h.remove(e);
       CPPUNIT_ASSERT(iter.key() == NULL);
       CPPUNIT_ASSERT(iter.value() == NULL);

       // Step the iterator and check that key() and value() return non-NULL.
       e = iter();
       CPPUNIT_ASSERT(e != NULL);
       CPPUNIT_ASSERT(iter.key() != NULL);
       CPPUNIT_ASSERT(iter.value() != NULL);

       // Step the iterator and check that key() and value() return non-NULL.
       e = iter();
       CPPUNIT_ASSERT(e != NULL);
       CPPUNIT_ASSERT(iter.key() != NULL);
       CPPUNIT_ASSERT(iter.value() != NULL);

       // Delete the element and check that key() and value() return NULL.
       h.remove(e);
       CPPUNIT_ASSERT(iter.key() == NULL);
       CPPUNIT_ASSERT(iter.value() == NULL);

       // Step the iterator and check that key() and value() return non-NULL.
       e = iter();
       CPPUNIT_ASSERT(e != NULL);
       CPPUNIT_ASSERT(iter.key() != NULL);
       CPPUNIT_ASSERT(iter.value() != NULL);

       // Step the iterator after the last element and check that
       // key() and value() return NULL.
       e = iter();
       CPPUNIT_ASSERT(e == NULL);
       CPPUNIT_ASSERT(iter.key() == NULL);
       CPPUNIT_ASSERT(iter.value() == NULL);

    } //removeKeyValue()


};

CPPUNIT_TEST_SUITE_REGISTRATION(UtlHashMapIteratorTests);
