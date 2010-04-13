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

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <cstdarg>

#include <utl/UtlLongLongInt.h>
#include <utl/UtlString.h>
#include <sipxunit/TestUtilities.h>

using namespace std ;

/**  This class is used to test the UtlLongLongInt utility class.
*
*    PLEASE READ THE README FILE THAT CAN FOUND IN THE PARENT OF THE DIRECTORY
*    OF THIS FILE. The Readme describes the organization / flow of tests and
*    without reading this file, the following class (and all unit tests)
*    may not make a lot of sense and might be difficult to comprehend.
*/
class UtlLongLongIntTests : public CppUnit::TestCase
{

    CPPUNIT_TEST_SUITE(UtlLongLongIntTests);
    CPPUNIT_TEST(testConstructor) ;
    CPPUNIT_TEST(testCompareTo) ;
    CPPUNIT_TEST(testCompareTo_NonIntll) ;
    CPPUNIT_TEST(testEquals) ;
    CPPUNIT_TEST(testEquals_NonIntll) ;
    CPPUNIT_TEST(testSetValue) ;
    CPPUNIT_TEST(testGetContainableType) ;
    CPPUNIT_TEST(testOperators) ;
    CPPUNIT_TEST_SUITE_END();

private:

    struct BasicIntllVerifier
    {
        const char* message ;
        Int64 input ;
        Int64 expectedValue ;
    } ;

    static const Int64 llint_Zero ;
    static const Int64 llint_Positive ;
    static const Int64 llint_Negative ;
    //LLONG_MAX
    //LLONG_MIN
    static const ssize_t INDEX_NOT_FOUND  ;
    // An indication that the test should be ignored.
    static const int IGNORE_TEST ;

    static const BasicIntllVerifier commonTestSet[] ;
    static const int commonTestSetLength ;

    enum EqualOrCompareTest { TEST_EQUAL, TEST_COMPARE } ;

public:
    UtlLongLongIntTests()
    {
    }

    void setUp()
    {
    }

    void tearDown()
    {
    }

    ~UtlLongLongIntTests()
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
    *     a) llint = 0
    *     b) llint = +ve long long integer
    *     c) llint = -ve long long integer
    *     d) llint = max +ve long long integer
    *     e) llint = max -ve long long integer.
    *     The above set of data will be referred to as the
    *     Common Data Set henceforth! The common data set has been defined
    *     as a static const array of long long integers and a short description about
    *     the data is in a const array of char*
    */
    void testConstructor()
    {
        // First test the default constructor
        UtlLongLongInt testIntll ;
        const char* msg0 = "Test the default constructor" ;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg0, 0LL, testIntll.getValue()) ;

        // Now test the single argument constructor for each of
        // the common test data type
        const char* prefix = "Test UtlLongLongInt(int value), where value = " ;
        for (int i = 0  ;i < commonTestSetLength; i++)
        {
            string msg ;
            TestUtilities::createMessage(2, &msg, prefix, commonTestSet[i].message) ;
            UtlLongLongInt testIntll(commonTestSet[i].input) ;
            Int64 actualValue = testIntll.getValue() ;
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
            prefix = "Test the compareTo method for a Long Long Integer whose value = " ;
        }
        else if (testType == TEST_EQUAL)
        {
            prefix = "Test the isEqual method for a Long Long Integer whose value = " ;
        }
        const char* suffix1 = " - when value == arg"  ;
        const char* suffix2 = " - when value > arg" ;
        const char* suffix3 = " - when value < arg" ;
        string msg ;

        struct compareToData
        {
            const char* message ; // Description about the type of test data
            Int64 baseValue ; // The value of the test UtlLongLongInt
            Int64 equalToBase ;  // A Long Long Integer that is equal to the base value
            Int64 greaterThanBase ; // Long Long Integer that is greater than the base value
            Int64 lessThanBase ;  // Long Long Integer that is less than the base value
        } ;
        const compareToData testData[] = { \
            { "zero", 0, 0, 10, -10 }, \
            { "positive integer", llint_Positive, llint_Positive, llint_Positive+10, llint_Positive-10 }, \
            { "negative integer", llint_Negative, llint_Negative, llint_Negative+10, llint_Negative-10 }, \
            { "integer at its maximum allowed value", LLONG_MAX, LLONG_MAX, IGNORE_TEST, LLONG_MAX-10 }, \
            { "integer at its minimum allowed value", LLONG_MIN, LLONG_MIN, LLONG_MIN+10, IGNORE_TEST }
        } ;
        const int testCount = sizeof(testData)/sizeof(testData[0]) ;
        Int64 expectedForEquals = 0 ;
        Int64 expectedForGreaterThan = 1 ;
        Int64 expectedForLessThan = -1 ;

        // Loop to iterate through the array of test data.
        for (int i = 0 ; i < testCount ; i++)
        {
            UtlLongLongInt testIntll(testData[i].baseValue) ;
            UtlLongLongInt llintForCompare ;

            // -------------------------------------------------------------------------
            // first test the case when the rhs's llint = base llint.
            llintForCompare = UtlLongLongInt(testData[i].equalToBase) ;
            TestUtilities::createMessage(3, &msg, prefix, \
                                             testData[i].message, suffix1) ;
            if (testType == TEST_COMPARE)
            {
                Int64 actual = testIntll.compareTo(&llintForCompare) ;
                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedForEquals, actual) ;
            }
            else if (testType == TEST_EQUAL)
            {
                UtlBoolean actual = testIntll.isEqual(&llintForCompare) ;
                CPPUNIT_ASSERT_MESSAGE(msg.data(), actual) ;
            }
            // -------------------------------------------------------------------------

            // -------------------------------------------------------------------------
            // Now test the case where the llint's value is greater than the argument's value
            // In this case we SHOULD not try to compare for a llint which is at the
            // MINIMUM value that any llint can assume as such a llint cannot be greater
            // than anything else. This is bound to cause overruns thus yielding unexpected results.
            if (testData[i].baseValue != LLONG_MIN)
            {
                llintForCompare = UtlLongLongInt(testData[i].lessThanBase) ;
                TestUtilities::createMessage(3, &msg, prefix, \
                                             testData[i].message, suffix2) ;
                if (testType == TEST_COMPARE)
                {
                    Int64 actual = testIntll.compareTo(&llintForCompare) ;
                    CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedForGreaterThan, actual)  ;
                }
                else if (testType == TEST_EQUAL)
                {
                    UtlBoolean actual = testIntll.isEqual(&llintForCompare) ;
                    CPPUNIT_ASSERT_MESSAGE(msg.data(), !actual) ;
                }
            }
            // -------------------------------------------------------------------------

            // -------------------------------------------------------------------------
            // Now test the case where the integer's value is less than the argument's value
            // In this case we SHOULD not try to compare for an integer which is at the
            // MAXIMUM value that any integer can assume as such an integer cannot be less than
            // anything else. This is bound to cause overruns thus yielding unexpected results.
            if (testData[i].baseValue != LLONG_MAX)
            {
                llintForCompare = UtlLongLongInt(testData[i].greaterThanBase) ;
                TestUtilities::createMessage(3, &msg, prefix, \
                                             testData[i].message, suffix3) ;
                if (testType == TEST_COMPARE)
                {
                    Int64 actual = testIntll.compareTo(&llintForCompare) ;
                    CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), expectedForLessThan, actual) ;
                }
                else if (testType == TEST_EQUAL)
                {
                    UtlBoolean actual = testIntll.isEqual(&llintForCompare) ;
                    CPPUNIT_ASSERT_MESSAGE(msg.data(), !actual) ;
                }
            }
            // -------------------------------------------------------------------------
        } // End of loop to iterate through test data.
    }

    /*!a Test the compareTo method when a non-UtlLongLongInt is passed.
    *
    */
    void testCompareTo_NonIntll()
    {
        UtlString testUtlString("Test String") ;
        UtlLongLongInt testUtlLongLongInt(LLONG_MAX) ;
        Int64 actual = testUtlLongLongInt.compareTo(&testUtlString) ;
        // If a collectable is *NOT* an Integer, then the only thing that is predictible is
        // that this is not equal to the argument
        CPPUNIT_ASSERT_MESSAGE( "Compare a Long Long Integer with a String ", (actual != 0 )) ;
    }

    /*!a Test the Equals Method when the argument passed is not a UtlLongLongInt
    *
    */
    void testEquals_NonIntll()
    {
        const char* prefix = "Test the isEquals(UtlContainable other) for an Integer whose " \
                             "value is " ;
        const char* suffix = ", where other is a UtlContainable object that is not a " \
                             "UtlLongLongInt" ;
        UtlString testString("This is a test string") ;
        for (int i = 0 ; i < commonTestSetLength; i++)
        {
            UtlLongLongInt testIntll(commonTestSet[i].input) ;
            string msg ;
            UtlBoolean isEqual = testIntll.isEqual(&testString);
            TestUtilities::createMessage(3, &msg, prefix, commonTestSet[i].message, suffix) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), (UtlBoolean)false, isEqual) ;
        }

    } //testEquals_NonIntll

    /*!a Test the setValue(llint) method.
    *
    *    Test Data :-
    *        For each type in the common dataset as the base
    *        integer, add each type of common dataset llint
    */
    void testSetValue()
    {
        const char* prefix = "Test the setValue(llint setter) method for a UtlLongLongInt " \
                             "whose value is " ;
        const char* suffix1 = " -- verify that the previous value is returned " ;
        const char* suffix2 = " -- verify that the new value has been set " ;
        string msg ;

        // For each type of UtlLongLongInt, verify that you can set
        // every common type of Long Long Integer.
        for (int i = 0 ; i < commonTestSetLength ; i++)
        {
            // Loop to test setting each common type of llint
            // for any given type of llint
            for (int j = 0 ; j < commonTestSetLength ; j++)
            {
                UtlLongLongInt baseIntll(commonTestSet[i].input) ;

                Int64 oldActualValue = baseIntll.setValue(commonTestSet[j].input) ;
                Int64 newActualValue = baseIntll.getValue() ;

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
        const char* prefix = "Test the getContainableType() method for a UtlLongLongInt " \
                             "whose value is " ;
        string msg ;
        for (int i = 0 ; i < commonTestSetLength; i++)
        {
            UtlLongLongInt testIntll(commonTestSet[i].input) ;
            UtlContainableType actual = testIntll.getContainableType() ;
            TestUtilities::createMessage(2, &msg, prefix, commonTestSet[i].message) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(), string("UtlLongLongInt"), \
                                      string(actual)) ;
        }
    } //testGetContainableType

    void testOperators()
    {
        UtlLongLongInt testIntll(1);
        CPPUNIT_ASSERT((++testIntll).getValue() == 2);
        CPPUNIT_ASSERT((testIntll++).getValue() == 2);
        CPPUNIT_ASSERT((--testIntll).getValue() == 2);
        CPPUNIT_ASSERT((testIntll--).getValue() == 2);

        // test conversion operator
        UtlLongLongInt testIntll2(LLONG_MAX);
        CPPUNIT_ASSERT(testIntll2 == LLONG_MAX);
    } //testOperators
};


// ------------------- Static constant initializers -------------------------
const Int64 UtlLongLongIntTests::llint_Zero = 0LL ;
const Int64 UtlLongLongIntTests::llint_Positive = 101LL ;
const Int64 UtlLongLongIntTests::llint_Negative = -51LL ;
const UtlLongLongIntTests::BasicIntllVerifier \
      UtlLongLongIntTests::commonTestSet[]  = { \
         {"Zero", llint_Zero, llint_Zero},  \
         {"Positive Long Long Integer", llint_Positive, llint_Positive}, \
         {"Negative Long Long Integer", llint_Negative, llint_Negative}, \
         {"MAX VALUE Positive Long Long Integer", LLONG_MAX, LLONG_MAX}, \
         {"MIN VALUE Negative Long Long Integer", LLONG_MIN, LLONG_MIN} \
      } ;

const int UtlLongLongIntTests::commonTestSetLength = 5 ;
const ssize_t UtlLongLongIntTests::INDEX_NOT_FOUND = -1 ;
const int UtlLongLongIntTests::IGNORE_TEST=-1 ;
CPPUNIT_TEST_SUITE_REGISTRATION(UtlLongLongIntTests);
