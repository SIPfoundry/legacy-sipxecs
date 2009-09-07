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

#include <utl/UtlInt.h>
#include <utl/UtlString.h>
#include <sipxunit/TestUtilities.h>

using namespace std ;

/* PLEASE VERIFY WITH SCOTT */
/*
// -------------------Forward declarations------------------------------------
class UtlIntTests ;

// ------------------- Static constant initializers -------------------------
const int UtlIntTests::int_Zero = 0 ;
const int UtlIntTests::int_Positive = 101 ;
const int UtlIntTests::int_Negative = -51 ;
const int UtlIntTests::commonTestDataSet[] = { \
         int_Zero, int_Positive, int_Negative, \
         INT_MAX, INT_MIN \
    } ;

const char* UtlIntTests::commonTestDataSetMessages[] = { \
          "Zero" , \
          "Positive Integer", \
          "Negative Integer", \
          "MAX VALUE Positive Integer", \
          "MIN VALUE Negative Integer" \
          } ;
*/


/**  This class is used to test the UtlInt utility class.
*
*    PLEASE READ THE README FILE THAT CAN FOUND IN THE PARENT OF THE DIRECTORY
*    OF THIS FILE. The Readme describes the organization / flow of tests and
*    without reading this file, the following class (and all unit tests)
*    may not make a lot of sense and might be difficult to comprehend.
*/
class UtlIntTests : public CppUnit::TestCase
{

    CPPUNIT_TEST_SUITE(UtlIntTests);
    CPPUNIT_TEST(testConstructor) ;
    CPPUNIT_TEST(testCompareTo) ;
    CPPUNIT_TEST(testCompareTo_NonInt) ;
    CPPUNIT_TEST(testEquals) ;
    CPPUNIT_TEST(testEquals_NonInteger) ;
    CPPUNIT_TEST(testSetValue) ;
    CPPUNIT_TEST(testGetContainableType) ;
    CPPUNIT_TEST(testOperators) ;
    CPPUNIT_TEST_SUITE_END();

private:

    struct BasicIntVerifier
    {
        const char* message ;
        int input ;
        int expectedValue ;
    } ;

    static const int int_Zero ;
    static const int int_Positive ;
    static const int int_Negative ;
    //INT_MAX
    //INT_MIN
    static const int INDEX_NOT_FOUND  ;
    // An indication that the test should be ignored.
    static const int IGNORE_TEST ;

    static const BasicIntVerifier commonTestSet[] ;
    static const int commonTestSetLength ;

    enum EqualOrCompareTest { TEST_EQUAL, TEST_COMPARE } ;

public:
    UtlIntTests()
    {
    }

    void setUp()
    {
    }

    void tearDown()
    {
    }

    ~UtlIntTests()
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
    *    The test data for this test case is :-
    *     a) int = 0
    *     b) int = +ve integer
    *     c) int = -ve integer
    *     d) int = max +ve integer
    *     e) int = max -ve integer.
    *     The above set of data will be referred to as the
    *     Common Data Set henceforth! The common data set has been defined
    *     as a static const array of integers and a short description about
    *     the data is in a const array of char*
    *
    */
    void testConstructor()
    {
        // First test the default constructor
        UtlInt testInt ;
        const char* msg0 = "Test the default constructor" ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg0, (intptr_t)0, testInt.getValue()) ;

        //Now test the single argument constructor for each of
        // the common test data type
        const char* prefix = "Test UtlInt(int value), where value = " ;
        for (int i = 0  ;i < commonTestSetLength; i++)
        {
            string msg ;
            TestUtilities::createMessage(2, &msg, prefix, commonTestSet[i].message) ;
            UtlInt testInt(commonTestSet[i].input) ;
            int actualValue = testInt.getValue() ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), commonTestSet[i].expectedValue, actualValue) ;
        }
    }

    /** Test the compareTo method
    *
    *    The test data for this test is :-
    *        with the common data set, compare the data with
    *        another value that is a) equal to, b) greater than
    *        and c) less than itself
    */
    void testCompareTo()
    {
        utlTestCompareOrEquals(TEST_COMPARE) ;
    }

    /** Test the isEqual(UtlContainable*) method.
    *
    *   The test data for this test is the same as the compareTo method.
    */
    void testEquals()
    {
        utlTestCompareOrEquals(TEST_EQUAL) ;
    }

    /* Test the isEqual method or the compareTo method.
    */
    void utlTestCompareOrEquals(EqualOrCompareTest testType)
    {
        const char* prefix = "";
        if (testType == TEST_COMPARE)
        {
            prefix = "Test the compareTo method for an Integer whose value = " ;
        }
        else if (testType == TEST_EQUAL)
        {
            prefix = "Test the isEqual method for an Integer whose value = " ;
        }
        const char* suffix1 = " - when value == arg"  ;
        const char* suffix2 = " - when value > arg" ;
        const char* suffix3 = " - when value < arg" ;
        string msg ;

        struct compareToData
        {
            const char* message ; // Description about the type of test data
            int baseValue ; // The value of the test UtlInt
            int equalToBase ;  // An Integer that is equal to the base value
            int greaterThanBase ; // Integer that is greater than the base value
            int lessThanBase ;  // Integer that is less than the base value
        } ;
        const compareToData testData[] = { \
            { "zero", 0, 0, 10, -10 }, \
            { "positive integer", int_Positive, int_Positive, int_Positive+10, int_Positive-10 }, \
            { "negative integer", int_Negative, int_Negative, int_Negative+10, int_Negative-10 }, \
            { "integer at its maximum allowed value", INT_MAX, INT_MAX, IGNORE_TEST, INT_MAX-10 }, \
            { "integer at its minimum allowed value", INT_MIN, INT_MIN, INT_MIN+10, IGNORE_TEST }
        } ;
        const int testCount = sizeof(testData)/sizeof(testData[0]) ;
        int expectedForEquals = 0 ;
        int expectedForGreaterThan = 10 ;
        int expectedForLessThan = -10 ;

        // Loop to iterate through the array of test data.
        for (int i = 0 ; i < testCount ; i++)
        {

            UtlInt testInt(testData[i].baseValue) ;
            UtlInt intForCompare ;

            // -------------------------------------------------------------------------
            // first test the case when the rhs's int = base int.
            intForCompare = UtlInt(testData[i].equalToBase) ;
            TestUtilities::createMessage(3, &msg, prefix, \
                                             testData[i].message, suffix1) ;
            if (testType == TEST_COMPARE)
            {
                int actual = testInt.compareTo(&intForCompare) ;
                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedForEquals, actual) ;
            }
            else if (testType == TEST_EQUAL)
            {
                UtlBoolean actual = testInt.isEqual(&intForCompare) ;
                CPPUNIT_ASSERT_MESSAGE(msg.data(), actual) ;
            }
            // -------------------------------------------------------------------------

            // -------------------------------------------------------------------------
            // Now test the case where the integer's value is greater than the argument's value
            // In this case we SHOULD not try to compare for an integer which is at the
            // MINIMUM value that any integer can assume as such an integer cannot be greater
            // than anything else. This is bound to cause overruns thus yielding unexpected results.
            if (testData[i].baseValue != INT_MIN)
            {
                intForCompare = UtlInt(testData[i].lessThanBase) ;
                TestUtilities::createMessage(3, &msg, prefix, \
                                             testData[i].message, suffix2) ;
                if (testType == TEST_COMPARE)
                {
                    int actual = testInt.compareTo(&intForCompare) ;
                    CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedForGreaterThan, actual)  ;
                }
                else if (testType == TEST_EQUAL)
                {
                    UtlBoolean actual = testInt.isEqual(&intForCompare) ;
                    CPPUNIT_ASSERT_MESSAGE(msg.data(), !actual) ;
                }
            }
            // -------------------------------------------------------------------------

            // -------------------------------------------------------------------------
            // Now test the case where the integer's value is less than the argument's value
            // In this case we SHOULD not try to compare for an integer which is at the
            // MAXIMUM value that any integer can assume as such an integer cannot be less than
            // anything else. This is bound to cause overruns thus yielding unexpected results.
            if (testData[i].baseValue != INT_MAX)
            {
                intForCompare = UtlInt(testData[i].greaterThanBase) ;
                TestUtilities::createMessage(3, &msg, prefix, \
                                             testData[i].message, suffix3) ;
                if (testType == TEST_COMPARE)
                {
                    int actual = testInt.compareTo(&intForCompare) ;
                    CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedForLessThan, actual) ;
                }
                else if (testType == TEST_EQUAL)
                {
                    UtlBoolean actual = testInt.isEqual(&intForCompare) ;
                    CPPUNIT_ASSERT_MESSAGE(msg.data(), !actual) ;
                }
            }
            // -------------------------------------------------------------------------
        } // End of loop to iterate through test data.
    }

    /*!a Test the compareTo method when a non-UtlInt is passed.
    *
    */
    void testCompareTo_NonInt()
    {
        UtlString testUtlString("Test String") ;
        UtlInt testUtlInt(INT_MAX) ;
        int actual = testUtlInt.compareTo(&testUtlString) ;
        // If a collectable is *NOT* an Integer, then the only thing that is predictible is
        // that this is not equal to the argument
        CPPUNIT_ASSERT_MESSAGE( "Compare an Int with a String ", (actual != 0 )) ;
    }

    /*!a Test the Equals Method when the argument passed is not a UtlInt
    *
    */
    void testEquals_NonInteger()
    {
        const char* prefix = "Test the isEquals(UtlContainable other) for an Integer whose " \
                             "value is " ;
        const char* suffix = ", where other is a UtlContainable object that is not a " \
                             "UtlInt" ;
        UtlString testString("This is a test string") ;
        for (int i = 0 ; i < commonTestSetLength; i++)
        {
            UtlInt testInt(commonTestSet[i].input) ;
            string msg ;
            UtlBoolean isEqual =testInt.isEqual(&testString);
            TestUtilities::createMessage(3, &msg, prefix, commonTestSet[i].message, suffix) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (UtlBoolean)false, isEqual) ;
        }

    } //testEquals_NonInteger

    /*!a Test the setValue(int) method.
    *
    *    Test Data :-
    *        For each type in the common dataset as the base
    *        integer, add each type of common dataset integer
    */
    void testSetValue()
    {
        const char* prefix = "Test the setValue(int setter) method for a UtlInt " \
                             "whose value is " ;
        const char* suffix1 = " -- verify that the previous value is returned " ;
        const char* suffix2 = " -- verify that the new value has been set " ;
        string msg ;

        // For each type of UtlInt, verify that you can set
        // every common type of Integer.
        for (int i = 0 ; i < commonTestSetLength ; i++)
        {
            // Loop to test setting each common type of integer
            // for any given type of integer
            for (int j = 0 ; j < commonTestSetLength ; j++)
            {
                UtlInt baseInt(commonTestSet[i].input) ;

                int oldActualValue = baseInt.setValue(commonTestSet[j].input) ;
                int newActualValue = baseInt.getValue() ;

                // Verify that the return value = previous value
                TestUtilities::createMessage(5, &msg, prefix, commonTestSet[i].message, \
                                               " where setter = ", commonTestSet[j].message, \
                                               suffix1) ;
                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), commonTestSet[i].expectedValue, \
                                                       oldActualValue) ;

                // Verify that the value has been set.
                TestUtilities::createMessage(5, &msg, prefix, commonTestSet[i].message, \
                                               " where setter = ", commonTestSet[j].message, \
                                               suffix2) ;
                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), commonTestSet[j].expectedValue, \
                                                       newActualValue) ;

            }
        }
    } //testSetValue

    /*!a Test the getContainableType method
    *
    *    Test data = common test data set
    */
    void testGetContainableType()
    {
        const char* prefix = "Test the getContainableType() method for a UtlInt " \
                             "whose value is " ;
        string msg ;
        for (int i = 0 ; i < commonTestSetLength; i++)
        {
            UtlInt testInt(commonTestSet[i].input) ;
            UtlContainableType actual = testInt.getContainableType() ;
            TestUtilities::createMessage(2, &msg, prefix, commonTestSet[i].message) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), string("UtlInt"), \
                                      string(actual)) ;
        }
    } //testGetContainableType

    void testOperators()
    {
        // test prefix and postfix operators
        UtlInt testInt(1);
        CPPUNIT_ASSERT((++testInt).getValue() == 2);
        CPPUNIT_ASSERT((testInt++).getValue() == 2);
        CPPUNIT_ASSERT((--testInt).getValue() == 2);
        CPPUNIT_ASSERT((testInt--).getValue() == 2);

        // test conversion operator
        UtlInt testInt2(11);
        CPPUNIT_ASSERT(testInt2 == 11);
    } //testOperators
};


// ------------------- Static constant initializers -------------------------
const int UtlIntTests::int_Zero = 0 ;
const int UtlIntTests::int_Positive = 101 ;
const int UtlIntTests::int_Negative = -51 ;
const UtlIntTests::BasicIntVerifier \
      UtlIntTests::commonTestSet[]  = { \
         {"Zero", int_Zero, int_Zero},  \
         {"Positive Integer", int_Positive, int_Positive}, \
         {"Negative Integer", int_Negative, int_Negative}, \
         {"MAX VALUE Positive Integer", INT_MAX, INT_MAX}, \
         {"MIN VALUE Negative Integer", INT_MIN, INT_MIN} \
      } ;

const int UtlIntTests::commonTestSetLength = 5 ;
const int UtlIntTests::INDEX_NOT_FOUND = -1 ;
const int UtlIntTests::IGNORE_TEST=-1 ;
CPPUNIT_TEST_SUITE_REGISTRATION(UtlIntTests);
