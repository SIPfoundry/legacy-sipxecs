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

#include <utl/UtlInt.h>
#include <utl/UtlString.h>
#include <utl/UtlVoidPtr.h>
#include <sipxunit/TestUtilities.h>

using namespace std ;

/* PLEASE VERIFY WITH SCOTT */

/**  This class is used to test the UtlVoidPtr utility class.
*
*    PLEASE READ THE README FILE THAT CAN FOUND IN THE PARENT OF THE DIRECTORY
*    OF THIS FILE. The Readme describes the organization / flow of tests and
*    without reading this file, the following class (and all unit tests)
*    may not make a lot of sense and might be difficult to comprehend.
*/
class UtlVoidPtrTests : public CppUnit::TestCase
{

    CPPUNIT_TEST_SUITE(UtlVoidPtrTests);
    CPPUNIT_TEST(testConstructor) ;
    CPPUNIT_TEST(testGetContainableType) ;
    CPPUNIT_TEST(testIsEqual) ;
    CPPUNIT_TEST(testSetValue) ;
    CPPUNIT_TEST_SUITE_END();

private:

    struct BasicIntVerifier
    {
        char* message ;
        int input ;
        int expectedValue ;
    } ;

    static const int INDEX_NOT_FOUND  ;

public:
    UtlVoidPtrTests()
    {
    }

    void setUp()
    {
    }

    void tearDown()
    {
    }

    ~UtlVoidPtrTests()
    {
    }

    /** Sandbox method for experimenting with the API Under Test.
    *   This method MUST be empty when the test drivers are being
    *   checked in (final checkin) to the repository.
    */
    void DynaTest()
    {
    }

    /** Test the constructor
    *
    *   Test data for this test is :-
    *      a) test a default zero constructor
    *      b) Test by passing some arbitary pointer.
    *      c) Test by passing another containable object itself
    *
    */
    void testConstructor()
    {
        // test that the default constructor creates a void ptr
        // with 0 as its volume.
        UtlVoidPtr testVoidObj1 ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test the default constructor", \
            (void*)0, (void*) testVoidObj1.getValue()) ;

        // Test that you can pass any void* object as the argument
        const char* testCharstar = "Hello ganeshji" ;
        UtlVoidPtr testVoidObj2((void*)testCharstar) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test the one arg constructor by passing a char* " \
            "pointer as the argument", (void*)testCharstar, (void*)testVoidObj2.getValue()) ;

        // Test that passing another UtlContainable* object itself doesn't
        // cause any confusions
        UtlString testString("New String") ;
        UtlVoidPtr testVoidObj3(&testString) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test the one arg constructor by passing a UtlString* " \
            "as the argument", (void*)&testString, (void*)testVoidObj3.getValue()) ;
    }//testConstructor()

    /** Test the getContainableType method
    *
    */
    void testGetContainableType()
    {
        const char* message = "Test the getContainableType()" ;

        UtlVoidPtr testVoidObj((char*)"Hello ganeshji") ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(message, string("UtlVoidPtr"), \
            string(testVoidObj.getContainableType()));
    }

    /** Test the hash() method
    *
    */
    void testIsEqual()
    {
        UtlVoidPtr testVoidObj1 ;
        UtlVoidPtr testVoidObj1_clone ;
        UtlString testString = UtlString("Hello") ;
        UtlString testString_clone = UtlString("Hello") ;
        UtlVoidPtr testVoidObj2(&testString) ;
        UtlVoidPtr testVoidObj2_copy(&testString) ;
        UtlVoidPtr testVoidObj2_clone(&testString_clone) ;

        // For a UtlVoidPtr that has NULL as its value, any other VoidPtr with
        // NULL value should be Equal
        CPPUNIT_ASSERT_MESSAGE("Test the isEqual() method for a NULL pointer", \
            testVoidObj1.isEqual(&testVoidObj1_clone)) ;

        // For a UtlVoidPtr, another object that contains the same ponter is
        // considered to  be Equal
        CPPUNIT_ASSERT_MESSAGE("Test the isEqual() method for a regular voidPtr", \
            testVoidObj2.isEqual(&testVoidObj2_copy)) ;

       // For a UtlVoidPtr, another objec that contains a 'similar (not same) pointer
       // is NOT considered to be equal
        CPPUNIT_ASSERT_MESSAGE("Test the isEqual() method for 2 voidPtr whose " \
            "value is similar but not same", !testVoidObj2.isEqual(&testVoidObj2_clone)) ;
    }//testIsEqual()

    /** Test the setValue() method.
    *
    *   The test data for this method is
    *      a) For a NULL  pointer set a different type
    *      a) Set the value from one Non-NULL to another Non-NULL
    *      d) set the value from Non-NULL to NULL
    */
    void testSetValue()
    {
        const char* prefix ;
        const char* suffix1 = " : test return value" ;
        const char* suffix2 = " : test that value has been set" ;
        string Message ;

        UtlVoidPtr testVoidObj ;
        void* returnValue ;
        void* newValue  ;
        void* oldValue = 0 ;

        newValue = (void*)"Hello world" ;
        returnValue = testVoidObj.setValue(newValue) ;
        prefix = "For a VoidPtr object which is not NULL, test setValue(void* value) " \
            "where val is non NULL " ;
        TestUtilities::createMessage(2, &Message, prefix, suffix1) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), (void*)oldValue, (void*)returnValue) ;
        TestUtilities::createMessage(2, &Message, prefix, suffix2) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), (void*)newValue, \
            (void*)testVoidObj.getValue()) ;

        oldValue = testVoidObj.getValue() ;
        prefix = "Test setValue(void* value) where value is not NULL and the " \
           "existing object is not NULL" ;
        newValue = (void*)"Hello again world" ;
        returnValue = (void*)testVoidObj.setValue(newValue) ;
        TestUtilities::createMessage(2, &Message, prefix, suffix1) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), (void*)oldValue, (void*)returnValue) ;
        TestUtilities::createMessage(2, &Message, prefix, suffix2) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), (void*)newValue, \
            (void*)testVoidObj.getValue()) ;

        oldValue = testVoidObj.getValue() ;
        prefix = "Test setValue(void* value) where value = NULL and existing object is not" ;
        newValue = 0 ;
        returnValue = (void*)testVoidObj.setValue(newValue) ;
        TestUtilities::createMessage(2, &Message, prefix, suffix1) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), (void*)oldValue, (void*)returnValue);
        TestUtilities::createMessage(2, &Message, prefix, suffix1) ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), (void*)newValue, \
            (void*)testVoidObj.getValue()) ;

    } //testSetValue

};
const int UtlVoidPtrTests::INDEX_NOT_FOUND = -1 ;

CPPUNIT_TEST_SUITE_REGISTRATION(UtlVoidPtrTests);
