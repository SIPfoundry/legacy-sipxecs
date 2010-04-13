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
#include <utl/UtlHashBag.h>
#include <utl/UtlHashBagIterator.h>
#include <sipxunit/TestUtilities.h>

using namespace std ;

bool checkMask( int& foundMask, int maskBit )
{
   bool isOk;

   //printf("\ncheck %04x", maskBit);

   isOk = (0==(maskBit & foundMask));
   foundMask |= maskBit;
   return isOk;
}
#define CHECK_FOUND(previouslyFound,thisBit) \
        CPPUNIT_ASSERT_MESSAGE("Same key returned twice",checkMask(previouslyFound,thisBit))

class UtlHashBagIteratorTests : public CppUnit::TestCase
{

    CPPUNIT_TEST_SUITE(UtlHashBagIteratorTests);
    CPPUNIT_TEST(testAdvancingOperator_And_KeyMethod) ;
    CPPUNIT_TEST(testFiltered) ;
    CPPUNIT_TEST(testCollision) ;
    CPPUNIT_TEST(removeKey) ;
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

    UtlString commonString1_clone ;
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
    enum Operation { NOP, ADVANCE, RESET, REPOSITION } ;

public:

    UtlHashBagIteratorTests()
    {
        commonContainables = new UtlContainable*[commonEntriesCount] ;
        commonContainables_Clone = new UtlContainable*[commonEntriesCount] ;
    }

    ~UtlHashBagIteratorTests()
    {
        delete[] commonContainables ;
        delete[] commonContainables_Clone ;
    }

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

    /** Ignore this method. This is more like a sandbox
    */
    void DynaTest()
    {
    } //DynaTest


    /*!a Test case for the () operator and the key() method.
    *
    *    The test data for this test is :-
    *       1) The next entry is a UtlString
    *       2) The next entry is a UtlInt
    *       3) The next entry is the last entry
    *       4) All entries have been read
    */
    void testAdvancingOperator_And_KeyMethod()
    {
        const int testCount = 5;
        const char* prefix = "Verify the () operator " ;
        const char* prefix_for_key = "Verify the key() method " ;
        const char* Msgs[] = { \
                     "when the entry is the first of two value matches of UtlString type", \
                     "when the entry is the first of two reference matches of UtlInt type", \
                     "when the entry is a unique Containable", \
                     "when the entry is the second of two reference matches of UtlInt type", \
                     "when the entry is the second of two value matches of UtlString type" \
        } ;

        // Create a Hashtable such that it has one unique element (commonString2) ,
        // 2 elements that has value matches (commonString1 / commonString1_clone) ,
        // and 2 elements that *ARE* the same (commonInt1)
        UtlHashBag testList ;
        testList.insert(&commonString1) ;
        testList.insert(&commonInt1) ;
        testList.insert(&commonString1_clone) ;
        testList.insert(&commonString2) ;
        testList.insert(&commonInt1) ;

        UtlContainable* exp[] = { \
                          &commonString1 , &commonInt1, &commonString2, &commonInt1, &commonString1_clone
        } ;
        int expEntries = testCount;

        UtlHashBagIterator iter(testList) ;
        UtlContainable* act ;
        UtlContainable* expected_value_for_key_method ;
        string msg ;

        // Test the key method when the iterator has been reset.
        iter.reset() ;
        act = iter.key() ;
        TestUtilities::createMessage(2, &msg, prefix, "when the iterator has been reset") ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (void*)NULL, (void*)act) ;

        // Now iterate through the whole iterator and verify that all the items are
        // retreived. The () operator retreives the next item in the list. The
        // key item should retreive the item under the current position. (That is,
        // the key() method should always return what the previous () returned)
        for (int i = 0 ; i < testCount ; i++)
        {
             act = iter() ;
             expected_value_for_key_method = act;
             const char* curMessage = "";
             bool wasFound = false ;
             // We dont care about which item matches where. We only want to make sure
             // that each item *IS* retreived once and *ONLY* once.
             for (int j =0 ; j < testCount; j++)
             {
                 // If the item was already found during a previous iteration,
                 // we would have set the exp. value to NULL. so ignore all NULL
                 // expected values.
                 // The idea behind this is illustrated with the following example.
                 // Let us say that invoking foo.search(bar) can return either return
                 // either 90, 100 or 120 on the first search(either return is valid).
                 // Let's say that it returns 100. But if foo.search(bar) is invoked the
                 // second time, it should only return 90 or 120. So setting the element
                 // of the expected array that used to contain 100 to NULL means that
                 // that '100' is no longer a valid expected value.
                 if (exp[j] != NULL && act == exp[j])
                 {
                     wasFound = true ;
                     exp[j] = NULL ;
                     // Unlike traditional tests in which we know before hand the message
                     // for a particular iteration, in this case, the message is constructed
                     // based on the return value.
                     curMessage = Msgs[j] ;
                     break ;
                 }
              }
              if (!wasFound)
              {
                  curMessage = " :- One of the expected items was not retreived" \
                                  " using an indexing operator" ;
              }
              TestUtilities::createMessage(3, &msg, prefix, curMessage, " :- Verify return value") ;
              CPPUNIT_ASSERT_MESSAGE(msg.data(), wasFound) ;
              TestUtilities::createMessage(3, &msg, prefix, curMessage, " :- Verify that the number of entries has not changed") ;
              CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expEntries, (int)testList.entries()) ;
              // Verfiy that using the key() method retreives the value at the current position.
              // without repositioning the iterator.
              act = iter.key() ;
              TestUtilities::createMessage(2, &msg, prefix_for_key, curMessage) ;
              CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expected_value_for_key_method, act) ;
        }

        // Test the () operator when the whole list has been iterated
        act = iter() ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Verify that the indexing operator on a list that has been traversed fully returns NULL", \
                              (void*)NULL, (void*)act) ;


        // Test the () operator for an empty list
        UtlHashBagIterator emptyIter(emptyList) ;
        act = emptyIter() ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test the () operator for an empty list iterator" , (void*)NULL, (void*)act) ;

    } //testAdvancingOperator()



      void testFiltered()
      {
         UtlString key1a("key1");
         UtlString key1b("key1");
         UtlString key2("key2");
         UtlString key3("key3");

         UtlHashBag theBag;
         theBag.insert(&key1a);
         theBag.insert(&key1b);
         theBag.insert(&key2);
         theBag.insert(&key3);

         UtlString* content;

         {
            UtlString filter("key2");
            UtlHashBagIterator each(theBag,&filter);

            int foundMask;
            foundMask = 0;
            while ((content = (UtlString*)each()))
            {
               if ( &key1a == content )
               {
                  CHECK_FOUND(foundMask,0x0001);
               }
               else if ( &key1b == content )
               {
                  CHECK_FOUND(foundMask,0x0002);
               }
               else if ( &key2 == content )
               {
                  CHECK_FOUND(foundMask,0x0004);
               }
               else if ( &key3 == content )
               {
                  CHECK_FOUND(foundMask,0x0008);
               }
               else
               {
                  CPPUNIT_FAIL("Unknown element returned");
               }
            }
            CPPUNIT_ASSERT_MESSAGE("Expected element not found",0x0004==foundMask);
         }
         {
            UtlString filter("key1");
            UtlHashBagIterator each(theBag,&filter);

            int foundMask;
            foundMask = 0;
            while ((content = (UtlString*)each()))
            {
               if ( &key1a == content )
               {
                  CHECK_FOUND(foundMask,0x0001);
               }
               else if ( &key1b == content )
               {
                  CHECK_FOUND(foundMask,0x0002);
               }
               else if ( &key2 == content )
               {
                  CHECK_FOUND(foundMask,0x0004);
               }
               else if ( &key3 == content )
               {
                  CHECK_FOUND(foundMask,0x0008);
               }
               else
               {
                  CPPUNIT_FAIL("Unknown element returned");
               }
            }
            CPPUNIT_ASSERT_MESSAGE("Expected element not found",0x0003==foundMask);
         }
         {
            UtlString filter("key1");
            UtlHashBagIterator each(theBag,&filter);

            int foundMask;
            foundMask = 0;

            UtlString* removed = (UtlString*)theBag.remove(&filter);
            if ( &key1a == removed )
            {
               CHECK_FOUND(foundMask,0x0001);
            }
            else if ( &key1b == removed )
            {
               CHECK_FOUND(foundMask,0x0002);
            }
            else
            {
               CPPUNIT_FAIL("Unknown element removed");
            }

            while ((content = (UtlString*)each()))
            {
               if ( &key1a == content )
               {
                  CHECK_FOUND(foundMask,0x0001);
               }
               else if ( &key1b == content )
               {
                  CHECK_FOUND(foundMask,0x0002);
               }
               else if ( &key2 == content )
               {
                  CHECK_FOUND(foundMask,0x0004);
               }
               else if ( &key3 == content )
               {
                  CHECK_FOUND(foundMask,0x0008);
               }
               else
               {
                  CPPUNIT_FAIL("Unknown element returned");
               }
            }
            CPPUNIT_ASSERT_MESSAGE("Expected element not found",0x0003==foundMask);
         }
         {
            UtlString filter("key3");
            UtlHashBagIterator each(theBag,&filter);

            int foundMask;
            foundMask = 0;

            UtlString* removed = (UtlString*)theBag.remove(&filter);
            CPPUNIT_ASSERT(&key3==removed);

            while ((content = (UtlString*)each()))
            {
               if ( &key1a == content )
               {
                  CHECK_FOUND(foundMask,0x0001);
               }
               else if ( &key1b == content )
               {
                  CHECK_FOUND(foundMask,0x0002);
               }
               else if ( &key2 == content )
               {
                  CHECK_FOUND(foundMask,0x0004);
               }
               else if ( &key3 == content )
               {
                  CHECK_FOUND(foundMask,0x0008);
               }
               else
               {
                  CPPUNIT_FAIL("Unknown element returned");
               }
            }
            CPPUNIT_ASSERT_MESSAGE("Expected element not found",0x0000==foundMask);
         }
         {
            UtlString filter("key4");
            UtlHashBagIterator each(theBag,&filter);

            int foundMask;
            foundMask = 0;

            UtlString* removed = (UtlString*)theBag.remove(&filter);
            CPPUNIT_ASSERT(NULL==removed);

            while ((content = (UtlString*)each()))
            {
               if ( &key1a == content )
               {
                  CHECK_FOUND(foundMask,0x0001);
               }
               else if ( &key1b == content )
               {
                  CHECK_FOUND(foundMask,0x0002);
               }
               else if ( &key2 == content )
               {
                  CHECK_FOUND(foundMask,0x0004);
               }
               else if ( &key3 == content )
               {
                  CHECK_FOUND(foundMask,0x0008);
               }
               else
               {
                  CPPUNIT_FAIL("Unknown element returned");
               }
            }
            CPPUNIT_ASSERT_MESSAGE("Expected element not found",0x0000==foundMask);
         }
      }

   /* Check for a problem that turned up in insert():  If an object
    * was inserted that should have been put at the end of a chain,
    * it was instead inserted at the beginning.
    *
    * This test detects the problem by inserting two objects which we arrange
    * to fall into the same bucket.  The object with the largest hash is
    * inserted second, so it should go at the end of the list, but instead
    * is inserted at the beginning.  The result is that the small-hash
    * object can't be seen by a selective HashBagIterator.
    *
    * The objects we use to exercise this case have to be tuned to the hash
    * algorithm of the objects and the algorithm
    * (UtlHashBag::bucketNumber) for turning hashes into bucket numbers.
    */
   void testCollision()
      {
         UtlHashBag bag;

         // For UtlInt's, the hash is the value.
         UtlInt small_hash_object(1);
         bag.insert(&small_hash_object);

         // put in some other objects
         UtlInt other_hash_object3(3);
         bag.insert(&other_hash_object3);
         UtlInt other_hash_object4(4);
         bag.insert(&other_hash_object4);
         UtlInt other_hash_object5(5);
         bag.insert(&other_hash_object5);

         UtlContainable* found;
         {
            // check that a keyed iterator sees the small_hash_object
            UtlHashBagIterator itor(bag, new UtlInt(1));
            found = itor();
            CPPUNIT_ASSERT(found == &small_hash_object);

            // and nothing else
            found = itor();
            CPPUNIT_ASSERT(found == NULL);
         }

         // We depend on the fact that a HashBag initially has 16 buckets and
         // folds hashes with XOR.  Under these conditions, 1 & 16 collide and
         // land in the same bucket.
         UtlInt large_hash_object(16);
         bag.insert(&large_hash_object);

         {
            // check that a keyed iterator sees the small_hash_object
            UtlHashBagIterator itor(bag, new UtlInt(1));
            found = itor();
            CPPUNIT_ASSERT(found == &small_hash_object);

            // and nothing else
            found = itor();
            CPPUNIT_ASSERT(found == NULL);
         }

         {
            // check that a keyed iterator sees the large_hash_object
            UtlHashBagIterator itor(bag, new UtlInt(16));
            found = itor();
            CPPUNIT_ASSERT(found == &large_hash_object);

            // and nothing else
            found = itor();
            CPPUNIT_ASSERT(found == NULL);
         }
      } // testCollision


    /*!a test case for the interaction of remove() and key().
    */
    void removeKey()
    {
       UtlString v1("a");
       UtlString v2("b");
       UtlString v3("c");
       UtlString v4("d");
       UtlContainable* e;

       UtlHashBag h;
       h.insert(&v1);
       h.insert(&v2);
       h.insert(&v3);
       h.insert(&v4);

       UtlHashBagIterator iter(h);

       // Check that key() returns NULL in the initial state.
       CPPUNIT_ASSERT(iter.key() == NULL);

       // Step the iterator and check that key() returns non-NULL.
       e = iter();
       CPPUNIT_ASSERT(e != NULL);
       CPPUNIT_ASSERT(iter.key() != NULL);

       // Delete the element and check that key() returns NULL.
       h.remove(e);
       CPPUNIT_ASSERT(iter.key() == NULL);

       // Step the iterator and check that key() returns non-NULL.
       e = iter();
       CPPUNIT_ASSERT(e != NULL);
       CPPUNIT_ASSERT(iter.key() != NULL);

       // Step the iterator and check that key() returns non-NULL.
       e = iter();
       CPPUNIT_ASSERT(e != NULL);
       CPPUNIT_ASSERT(iter.key() != NULL);

       // Delete the element and check that key() returns NULL.
       h.remove(e);
       CPPUNIT_ASSERT(iter.key() == NULL);

       // Step the iterator and check that key() returns non-NULL.
       e = iter();
       CPPUNIT_ASSERT(e != NULL);
       CPPUNIT_ASSERT(iter.key() != NULL);

       // Step the iterator after the last element and check that
       // key() returns NULL.
       e = iter();
       CPPUNIT_ASSERT(e == NULL);
       CPPUNIT_ASSERT(iter.key() == NULL);

    } //removeKey()


};

const int UtlHashBagIteratorTests::INDEX_NOT_EXIST = -1;
const int UtlHashBagIteratorTests::commonEntriesCount = 6;
const char* UtlHashBagIteratorTests::longAlphaNumString = \
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

const char* UtlHashBagIteratorTests::regularString = "This makes sense" ;

CPPUNIT_TEST_SUITE_REGISTRATION(UtlHashBagIteratorTests);
