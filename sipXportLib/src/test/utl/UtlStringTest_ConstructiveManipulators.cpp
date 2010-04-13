//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <utl/UtlStringTest.h>
#include <string.h>
#include <utl/UtlString.h>
#include <sipxunit/TestUtilities.h>

#ifdef WIN32
#   define MEMBER_CONST
#else
#   define MEMBER_CONST const
#endif

// The following variables have been defined in the base class
// and will be used in all the string tests
/**
const int UtlStringTest::commonTestSetLength ;
const char* UtlStringTest::longAlphaString ;
const char* UtlStringTest::splCharString ;
const BasicStringVerifier UtlStringTest::commonTestSet[];
const int UtlStringTest::commonTestSetLength  ;
*/


/**  This class is used to test the UtlString class.
*
*    PLEASE READ THE README FILE THAT CAN FOUND IN THE PARENT DIRECTORY.
*    The Readme describes the organization / flow of tests and
*    without reading this file, the following class (and all unit tests)
*    may not make a lot of sense and might be difficult to comprehend.
*/
class UtlStringTest_ConstructiveManipulators : public UtlStringTest
{
    CPPUNIT_TEST_SUITE(UtlStringTest_ConstructiveManipulators);
    CPPUNIT_TEST(testConstructor_1) ;
    CPPUNIT_TEST(testConstructor_3) ;
    CPPUNIT_TEST(testConstructor_4) ;
    CPPUNIT_TEST(testCapacity) ;
    CPPUNIT_TEST(testSetLength) ;
    CPPUNIT_TEST(testAssign_charstar) ;
    CPPUNIT_TEST(testCharstarCastOperator) ;
    CPPUNIT_TEST(testAssign_UtlString) ;
    CPPUNIT_TEST(testAppendWithInt);
    CPPUNIT_TEST(testAppendChar);
    CPPUNIT_TEST(testPlusEqualChar);
    CPPUNIT_TEST(testAppend_charstar_toEmptyString) ;
    CPPUNIT_TEST(testAppend_UtlString_toEmptyString) ;
    CPPUNIT_TEST(testAppend_charstar_specifyAllCharacters_toEmptyString) ;
    CPPUNIT_TEST(testAppend_charstar_toExistingString) ;
    CPPUNIT_TEST(testAppend_UtlString_toExistingString) ;
    CPPUNIT_TEST(testAppend_charstar_specifyAllCharacters_toExistingString) ;
    CPPUNIT_TEST(testAppend_charstar_specifiedSize_toEmptyString) ;
    CPPUNIT_TEST(testAppend_charstar_specifiedSize_toExistingString) ;
    CPPUNIT_TEST(testAppend_charstar_zero_bytes_toExistingString) ;
    CPPUNIT_TEST(testAppend_charstar_zero_bytes_toExistingString) ;
    CPPUNIT_TEST(testPlusEqual_charstar) ;
    CPPUNIT_TEST(testPlusEqual_UtlString) ;
    CPPUNIT_TEST(testPrepend) ;
    CPPUNIT_TEST(testInsert_AtFirstLocation) ;
    CPPUNIT_TEST(testInsert_AtMidLocation) ;
    CPPUNIT_TEST(testInsert_AtLastLocation) ;
    CPPUNIT_TEST(testInsert_AtVariousLocations) ;
    CPPUNIT_TEST(testReplace_zeroth_char) ;
    CPPUNIT_TEST(testReplace_last_char) ;
    CPPUNIT_TEST(testStringContainsNullNotAtEnd);
    CPPUNIT_TEST(testEfficientMemoryCopy);
    CPPUNIT_TEST(testSubstringAssign);
    CPPUNIT_TEST(testNull);
    CPPUNIT_TEST(testAppendNumber);
    CPPUNIT_TEST_SUITE_END();

public:

    UtlStringTest_ConstructiveManipulators()
    {
    }
    void setUp()
    {
    }

    void tearDown()
    {
    }

    ~UtlStringTest_ConstructiveManipulators()
    {
    }

    /** Sandbox method for experimenting with the API Under Test.
    *   This method MUST be empty when the test drivers are being
    *   checked in (final checkin) to the repository.
    */
    void DynaTest()
    {
    }


//// ========================= BEGIN TEST CASES ==============================

    /**  =====================================================================
    *  Test UtlString(char* ) ;
    *  Test the Single argument constructor that accepts a char* argument.
    *  The test data being used for this test case are :-
    *      a) Zero length string
    *      b) A single character represented as a string
    *      c) A very large length string(257 characters) - that is alphanumeric
    *      (a), (b) and (c) are somewhat like boundry cases
    *      (altough there is no real boundry case for a string class)
    *      d) Any ordinary string
    *      e) A string with just numeric digits in it.
    *      f) A string that contains special characters in it.
    */
    void testConstructor_1()
    {
        const char* prefix = "For a string that is made of " ;
        const char* suffix = ":- One arg(char*) Constructor" ;
        string Message ;

        for (int i = 0 ; i < commonTestSetLength; i++)
        {
            UtlString testString(commonTestSet[i].input) ;
            TestUtilities::createMessage(3, &Message, prefix, \
                                         commonTestSet[i].testDescription, suffix) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(commonTestSet[i].input), \
                                      string(testString.data())) ;
        }
    }



     /**  =====================================================================
     *  Test the copy consturctor that accepts another UtlString object as the argument
     *  The test data being used for this test case are :-
     *      a) Zero length string
     *      b) A single character represented as a string
     *      c) A very large length string(257 characters) - alphanumeric
     *      (a), (b) and (c) are somewhat like boundry cases
     *      (altough there is no real boundry case for a string class)
     *      d) Any ordinary string
     *      e) A string with just numeric digits in it.
     *      f) A string that contains special characters in it.
     */
    void testConstructor_3()
    {

        const char* prefix = "Test copy constructor for " ;
        const char* suffix1 = ":- verified copied string" ;
        const char* suffix2 = ":- verify capacity";
        string Message ;

        for (int i = 0 ; i < commonTestSetLength; i++)
        {
            UtlString *tsOriginal = new UtlString(commonTestSet[i].input) ;
            UtlString tsCopied(*tsOriginal) ;
            size_t expectedCapacity = tsOriginal->capacity();
            delete tsOriginal;

            // Verify the copied string's data
            TestUtilities::createMessage(3, &Message, prefix, \
                           commonTestSet[i].testDescription, suffix1) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                           string(commonTestSet[i].input), string(tsCopied.data())) ;

            // Verify that the capacity is also copied.
            TestUtilities::createMessage(3, &Message, prefix, \
                           commonTestSet[i].testDescription, suffix2) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                                      expectedCapacity, tsCopied.capacity()) ;
        }
    } //testConstructor3


     /**  =====================================================================
     *  Test the copy consturctor that accepts another UtlString object
     *  and the length as the arguments
     *  The test data being used for this test case are :-
     *      a) Copy Zero bytes of a Zero length string
     *      b) Copy Zero bytes of a non-zero length string
     *      b) Copy 'n' bytes of a non zero length string, where n < strlen()
     *      c) Copy 'n' bytes of a non zero length string, where n = strlen()
     *      d) Copy 'n' bytes of a non zero length string, where n > strlen()
     */
    void testConstructor_4()
    {
        struct TestCopyConstructor2Struct
        {
            const char* testDescription ;
            const char* stringToCopy ;
            int lengthToCopy ;
            const char* expectedString ;
            int expectedLength ;
        } ;

        const char* prefix = "Test copy constructor(that specifies the number of bytes) for " ;
        const char* suffix1 = ":- verified copied string" ;
        const char* suffix2 = ":- verify length";
        string Message ;
        TestCopyConstructor2Struct testData[] = { \
            { "empty string. specify 0 as the length to copy", "", 0, "", 0 }, \
            { "a regular string. specify 0 as the length to copy", "Test String", \
              0, "", 0 }, \
            { "a regular string. specify n<strlen as length to copy", "Test String", \
              3, "Tes", 3 }, \
            { "a regular string. specify n=strlen as length to copy", "Test String", \
              11, "Test String", 11 }, \
            { "a regular string. specify n>strlen as length to copy", "Test String", \
              14, "Test String", 11 } \
        } ;
        int testCount = sizeof(testData)/sizeof(testData[0]) ;
        for (int i = 0 ; i < testCount; i++)
        {
            UtlString tsOriginal(testData[i].stringToCopy) ;
            KNOWN_EFENCE_BUG("Segmenation fault, unset EF_PROTECT_BELOW", "XPL-8") ;
            UtlString tsCopied(tsOriginal, testData[i].lengthToCopy) ;

            // Verify the copied string's data
            TestUtilities::createMessage(3, &Message, prefix, testData[i].testDescription, \
                suffix1) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                           string(testData[i].expectedString), string(tsCopied.data())) ;

            // Verify the copied string's length
            TestUtilities::createMessage(3, &Message, prefix, testData[i].testDescription, \
                suffix2) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), testData[i].expectedLength, \
                (int)tsCopied.length()) ;
        }
    } //testConstructor3

    /**
        This test case verifies the capacity(long) method.

    *   The data for the test case being used are
    *     a) Set the capacity to 0.
    *     b) Set the capacity to a large number.
    *     c) Set the capacity to 0 of a String whose capacity is non zero
    *     d) Set the capacity to a value greater than the current capacity.
    *     e) Set the capacity to a value less that the current capacity
    */
    void testCapacity()
    {
        // Structure that describes the test case.
        struct TestCapacityDataStructure
        {
            const char* testDescription ;
            const char* inputString ;
            size_t inputCapacity ;
            size_t minExpectedCapacity ;
        };
        const char* prefix = "Test capacity() -" ;
        const char* suffix1 = ":- Check returned value" ;
        const char* suffix2 =":- Check actual capacity" ;
        string Message ;

        const TestCapacityDataStructure testData[] = { \
              {"Set capacity of an empty string to 0", \
                    "", 0, 0 }, \
              {"Set capacity of an empty string to " \
                    "a large number", "", 2543, 2543 }, \
              {"Set capacity of an existing non-zero " \
                    "length string to 0","Some String", 0, 9 }, \
              {"Set capacity of an existing string " \
                    "to greater than  current", "Some String", 12, 12 }, \
              {"Set capacity of an existing string " \
                    "to less than current", "Some String", 2, 9 } \
        };
        const int testCount = sizeof(testData)/sizeof(testData[0]) ;

        for (int i =0 ; i<testCount ; i++)
        {
            UtlString testString(testData[i].inputString) ;
            size_t actualCapacity = testString.capacity(testData[i].inputCapacity) ;

            // Check return value ;
            TestUtilities::createMessage(3, &Message, prefix, \
                                         testData[i].testDescription, suffix1) ;
            CPPUNIT_ASSERT_MESSAGE(Message.data(), \
                        actualCapacity >= testData[i].minExpectedCapacity) ;

            // Check the new capacity of the string.
            actualCapacity = testString.capacity() ;
            TestUtilities::createMessage(3, &Message, prefix, \
                                         testData[i].testDescription, suffix2) ;
            CPPUNIT_ASSERT_MESSAGE(Message.data(), \
                        actualCapacity >= testData[i].minExpectedCapacity) ;

        }
    } //testCapacity

    void testSetLength()
    {
       UtlString buffer;
       char testdata[] = "abcdefghijklmnopqrstuvwxyz";
       char shortdata[] = "abcdefghij";

       buffer.append(testdata);
       ASSERT_STR_EQUAL(testdata, buffer.data());
       CPPUNIT_ASSERT_EQUAL((size_t)26,buffer.length());

       // force the length to a new value
       buffer.setLength(10U);
       // check that the data itself is not modified
       ASSERT_STR_EQUAL(shortdata, buffer.data());
       // but the length returned by the buffer has been
       CPPUNIT_ASSERT_EQUAL(strlen(shortdata),buffer.length());

       UtlString copy;
       copy.append(buffer);
       // check that the copy is the correct subset
       ASSERT_STR_EQUAL(shortdata, copy.data());
       // and the length returned by the copy has been
       CPPUNIT_ASSERT_EQUAL(strlen(shortdata),copy.length());
    }

    /**
     * Test to see how the compiler resolves the cases that gave Scott
     * trouble, viz. append(char*, int) and append(UtlString, int).
     * Also check append(UtlString, int, int)
     */
    void testAppendWithInt()
    {
       UtlString appendee;
       char* string = (char*) "abcde";
       UtlString source;

       appendee = "12345";
       /* Should append the characters of "string", with length 3,
        * viz., "abc". */
       appendee.append(string, 3);
       ASSERT_STR_EQUAL_MESSAGE("append(char*, int)", "12345abc",
                                appendee.data());

       appendee = "12345";
       source = string;
       appendee.append(source, 3);
       /* Should append the characters of "source", with length 3,
        * viz., "abc". */
       ASSERT_STR_EQUAL_MESSAGE("append(UtlString, int)", "12345abc",
                                appendee.data());

       appendee = "12345";
       source = string;
       appendee.append(source, 3, 1);
       /* Should append the characters of "source", at offset 3, with
        * length 1, viz., "d". */
       ASSERT_STR_EQUAL_MESSAGE("append(UtlString, int, int)",
                                "12345d", appendee.data());
    }

    /**
     * Test append to empty string and null character
     */
    void testAppendChar()
    {
        UtlString oneChar;
        ASSERT_STR_EQUAL_MESSAGE("Append single char to empty string", "x", oneChar.append('x').data());

        const char *szHasNull = "null at end\0";
        UtlString hasNull(szHasNull);
        CPPUNIT_ASSERT_MESSAGE("Append null to string",
            memcmp(szHasNull, hasNull.append('\0').data(), sizeof(szHasNull)) == 0);
    }

    /**
     * Test appending using the plus equals operator (to empty, to norm string,
     * adding null)
     */

    void testPlusEqualChar()
    {
        UtlString oneChar;
        oneChar += 'x' ;
        ASSERT_STR_EQUAL_MESSAGE("Append single char to empty string", "x", oneChar.data());

        UtlString manyChar("Random String") ;
        manyChar += '!' ;
        ASSERT_STR_EQUAL_MESSAGE("Append single char to norm string", "Random String!", manyChar.data());

        const char *szHasNull = "null at end\0";
        UtlString hasNull(szHasNull);
        hasNull += '\0' ;
        CPPUNIT_ASSERT_MESSAGE("Append null to string",
            memcmp(szHasNull, hasNull.data(), sizeof(szHasNull)) == 0);
    }

    /**
    *     This test case verifies the append(const char*) /
    *     append(const UtlString) method.
    *
    *     The test data for this test case is :-
    *       With an Empty (zero length) string
    *       a) Append an empty string
    *       b) Append a single character string
    *       c) Append a very long string(which also has alpha numeric chars)
    *       d) Append a meaningful string
    *       e) Append a string with only digits.
    *       f) Append a string with special characters.
    *       With a string that already has some text in it :-
    *       a) to f) are to be repeated.
    *
    *       With a string that is long and the (number of characters) == (capacity)
    *       a) Append another string that has alphanumeric / special characters
    */
    void testAppend_charstar_toEmptyString()
    {
        utlTestAppend_to_EmptyString(TYPE_CHARSTAR) ;
    }

    /** The above test case for appending UtlString instead of
    *   char*
    */
    void testAppend_UtlString_toEmptyString()
    {
        utlTestAppend_to_EmptyString(TYPE_UTLSTRING) ;
    }

    /**
    *   Test case to test the append(const char*, int size) method - Case a)
    *      Case a) - Copy to an empty string - All the bytes of the char* being passed
    *
    *   The test data for this test case are the same as that in the method,
    *   testAppend_charstar_toEmptyString
    *
    */
    void testAppend_charstar_specifyAllCharacters_toEmptyString()
    {
        utlTestAppend_to_EmptyString(TYPE_CHARSTAR, true) ;
    }

    /*! The above test case for appending UtlString instead of
    *   char*
    */
    void testAppend_UtlString_specifyAllCharacters_toEmptyString()
    {
        utlTestAppend_to_EmptyString(TYPE_UTLSTRING, true) ;
    }

    /** Utility to test the Append method to an empty string
    *   Using this utility, you can test both append(char*) &
    *   append(UtlString). By passing the appropriate value to the
    *   specifyCharLenght argument, you can also test the method that
    *   explicitly specifies the number of characters(equal to length)
    *   to be appended. That is, you can test append(char*, len) or
    *   append(UtlString, len)
    */
    void utlTestAppend_to_EmptyString(StringType type, bool specifyCharLength = false)
    {

        const char* prefix = "To a zero length string, test appending a" ;
        // The Messages array used is the commonDataSetMessages.
        const char* suffix1 = ":- verify if appended to original string" ;
        const char* suffix2 = ":- verify returned string" ;
        string Message ;

        for (int i = 0 ; i < commonTestSetLength ; i++)
        {
            UtlString baseString("") ;
            const char* returnValue ;
            UtlString stringToAppend(commonTestSet[i].input) ;
            if (type == TYPE_CHARSTAR)
            {
                // If specifyCharLength is passed as 'true', then invoke the
                // method as append(charstarString, size),
                // where size = strlen(charstarString).
                // Otherwise invoke as strlen(charstarString)
                if (specifyCharLength)
                {
                    UtlString &tsReturn = baseString.append( \
                           commonTestSet[i].input, commonTestSet[i].length) ;
                     returnValue = tsReturn.data() ;
                 }
                 else
                 {
                     UtlString &tsReturn = baseString.append( \
                                commonTestSet[i].input) ;
                     returnValue = tsReturn.data() ;
                 }
             }
             else
             {
                 if (specifyCharLength)
                 {
                     UtlString &tsReturn = baseString.append(stringToAppend, \
                                commonTestSet[i].length) ;
                     returnValue = tsReturn.data() ;
                 }
                 else
                 {
                     UtlString &tsReturn = baseString.append(stringToAppend) ;
                     returnValue = tsReturn.data() ;
                 }
             }

             TestUtilities::createMessage(3, &Message, prefix, commonTestSet[i].testDescription, \
                            suffix1) ;
             CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(commonTestSet[i].input), \
                            string(baseString.data())) ;

             TestUtilities::createMessage(3, &Message, prefix, commonTestSet[i].testDescription, \
                            suffix2) ;
             CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(commonTestSet[i].input), \
                            string(returnValue)) ;
         }
    } //utlTestAppend_to_EmptyString


    /**
    *     This test case verifies the append(const char*) method.
    *
    *     The test data for this test case is :-

    *         With a string that already has text in it:-
    *         a) Append an empty string
    *         b) Append a single character string
    *         c) Append a very long string(which also has alpha numeric chars)
    *         d) Append a meaningful string
    *         e) Append a string with only digits.
    *         f) Append a string with special characters.
    *
    *         With a long string and (number of characters) == (capacity)
    *         g) Append a string that has alphanumeric / special characters
    */
    void testAppend_charstar_toExistingString()
    {
        utlTestAppend_Charstar_to_ExistingString(false, TEST_APPEND) ;
        utlTestAppend_MaxCapacity(TYPE_CHARSTAR) ;
    }

    /**
    *   Test case to test the append(const char*, int size) method
    *   - Copy to a non-empty string - All the bytes of the char* being passed
    *
    *   The test data for this test case are :-
    *   all cases of the testAppend_char_toExistingString() test case
    */
    void testAppend_charstar_specifyAllCharacters_toExistingString()
    {
        utlTestAppend_Charstar_to_ExistingString(true, TEST_APPEND) ;
        utlTestAppend_MaxCapacity(TYPE_CHARSTAR, true) ;
    }

    /** Test the replace(len-1, len-1, char*) method
    *     The test data for this is the same as the testAppend()
    *     test case
    */
    void testReplace_last_char()
    {
        utlTestAppend_Charstar_to_ExistingString(false, TEST_REPLACELAST) ;
    }

    void testStringContainsNullNotAtEnd()
    {
        UtlString a("aa\0bb", 5);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("lost null char in string",
              strlen(a.data()), (size_t) 2);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect length with null char",
              a.length(), (size_t) 5);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("did not find null char",
              a.index('\0'), (ssize_t) 2);

        a.append("cc\0dd", 5);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("append with null char lost",
              a.length(), (size_t) 10);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to find last null char",
              a.last('\0'), (ssize_t) 7);

        UtlString b("\0", 1);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to create string with null char",
              b.length(), (size_t) 1);

        UtlString c(b);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to copy string with null char",
              c.length(), (size_t) 1);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("copied strings w/null char not equal",
              c, b);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to find null char w/string",
              a.index(b), (ssize_t) 2);

        a.remove(3);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("remove lost null char",
              a.length(), (size_t) 3);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("data after remove lost null char",
              strlen(a.data()), (size_t) 2);

        a.remove(2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to remove null char",
              strlen(a.data()), (size_t) 2);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong length after remove null char",
              a.length(), (size_t) 2);
    }

    void testEfficientMemoryCopy()
    {
        UtlString a;
        const void* dataPtr = a.data();
        const void* aPtr = NULL;

        a = "aaa";
        aPtr = a.data();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("char assign does not reuse memory",
               dataPtr, aPtr);

        dataPtr = a.data();
        a.append("bbbb");
        aPtr = a.data();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("char assign does not reuse memory",
              dataPtr, aPtr);

        dataPtr = a.data();
        a.insert(0, "zzz");
        aPtr = a.data();
        KNOWN_BUG("char assign does not reuse memory", "XPL-51");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("char assign does not reuse memory", dataPtr, aPtr);

        dataPtr = a.data();
        a.remove(0);
        aPtr = a.data();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("remove does not reuse memory", dataPtr, aPtr);

        UtlString b;
        b.append("ABC");
        b.capacity(20);
        dataPtr = b.data();
        b.append("34567890123456789");
        aPtr = b.data();
        KNOWN_BUG("char assign does not reuse memory", "XPL-51");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("char assign does not reuse memory", dataPtr, aPtr);
     }

    /** test the += char* operator. This operator is exactly the same as
    *    the append method and so is the test data.
    */
    void testPlusEqual_charstar()
    {
        utlTestAppend_Charstar_to_ExistingString(false, TEST_PLUSEQUAL) ;
    }

    /** Utility method to test the case where a char* is appended to a
    /   UtlString. By specifying the bool parameter as false, you can test
    //  the simple append case or if you set the param to true, you can
    //  test the case where the entire length of string being appended is
    //  passed as the second parameter when making the call
    */
    void utlTestAppend_Charstar_to_ExistingString(bool specifyCharLength, \
                                      AppendInsertReplaceOrPlusEqual optype)
    {
        const char* prefix = "To a string of " ;
        // Messages = common Test Data Set Messages.
        const char* suffix1 = " :- verify if appended" ;
        const char* suffix2 = " :- verify returned string" ;

        string strExp ;
        for (int i = 0 ; i < commonTestSetLength ; i++)
        {
            int len_baseString = strlen(commonTestSet[i].input) ;
            for(int j =0 ; j < commonTestSetLength; j++)
            {
                int len_stringToAppend = strlen(commonTestSet[j].input) ;
                UtlString baseString(commonTestSet[i].input) ;

                strExp.erase() ;
                strExp.append(commonTestSet[i].input) ;
                strExp.append(commonTestSet[j].input) ;

                const char* msgDataBeingAppended = "";
                const char* returnValue = "";

                if (specifyCharLength)
                {
                    msgDataBeingAppended = \
                          " test append(char* c, len), where c = " ;
                    UtlString &tsReturn = baseString.append( \
                               commonTestSet[j].input, len_stringToAppend) ;
                    returnValue = tsReturn.data() ;
                }
                else
                {
                    if (optype == TEST_APPEND)
                    {
                        msgDataBeingAppended = "test append(char*) char* = ";
                        UtlString &tsReturn  = \
                                     baseString.append(commonTestSet[j].input) ;
                        returnValue = tsReturn.data() ;
                    }
                    else if (optype == TEST_REPLACELAST)
                    {
                        msgDataBeingAppended = "test replace(len-1, len-1, " \
                                               "char* c) where c = " ;
                        UtlString &tsReturn = \
                                     baseString.replace(len_baseString, \
                                     len_baseString, commonTestSet[j].input) ;
                        returnValue = tsReturn.data() ;
                    }
                    else if (optype == TEST_PLUSEQUAL)
                    {
                        msgDataBeingAppended = "test operator += (char* c), " \
                                               "where c = " ;
                        UtlString &tsReturn = \
                                         (baseString += commonTestSet[j].input) ;
                        returnValue = tsReturn.data() ;
                    }
                }
                string compMessage ;

                // Check if the original string has now changed such that the
                // parameter passed *HAS* been appended .
                TestUtilities::createMessage(5, &compMessage, prefix, \
                               commonTestSet[i].testDescription, \
                               msgDataBeingAppended, \
                               commonTestSet[j].testDescription, suffix1);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(compMessage.data(), strExp, \
                    string(baseString.data())) ;

                // Check the return value of append()
                TestUtilities::createMessage(5, &compMessage, prefix, \
                               commonTestSet[i].testDescription, \
                               msgDataBeingAppended, \
                               commonTestSet[j].testDescription, suffix2);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(compMessage.data(), strExp, string(returnValue)) ;

            } // for int j = 0 .. (Types of strings to be appended)
        }// for int i = 0 to ...(Types of base strings)

    } //utlTestAppend_Charstar_to_ExistingString

    /** Utility method that is used to test the case where a string is at its
    *   maximum capacity and you try to append.
    *   The type parameter dictates what the right handside string's type
    *   is going to be . You can test both append(char*),
    *   append(UtlString) The second parameter decideds whether the
    *   number of characters being appended needs to be explicitly specified.
    */
    void utlTestAppend_MaxCapacity(StringType type, \
                                   bool specifyCharLength = false)

    {
         const char* prefix = "";
         const char* testTypeMsg = " for a string at max. capacity" ;
         const char* suffix1 = ":= verify appended" ;
         const char* suffix2 = ":= verify return value" ;

         int charLength = 257 ;
         const char* stringToAppend ="AnotherString123O-abz@B\")";
         int appendLength = strlen(stringToAppend) ;

         UtlString baseString(longAlphaString) ;
         baseString.capacity(charLength) ;
         const char* returnValue = "";
         UtlString utlStringToAppend(stringToAppend) ;

         // Whether the leng of the string to be appended needs to be specified
         // explicitly or not
         if (specifyCharLength)
         {
             // is the rhs for append a char* or another UtlString
             if (type == TYPE_CHARSTAR)
             {
                 prefix = "Test append(char*, len) " ;
                 UtlString &tsReturn = \
                        baseString.append(stringToAppend, appendLength) ;
                 returnValue = tsReturn.data() ;
             }
             else if(type == TYPE_UTLSTRING)
             {
                 prefix = "Test append(UtlString, len) " ;
                 UtlString &tsReturn = baseString.append( \
                            utlStringToAppend, appendLength) ;
                 returnValue = tsReturn.data() ;
             }
         }
         else
         {
             if (type == TYPE_CHARSTAR)
             {
                prefix = "Test append(char*) " ;
                UtlString &tsReturn = \
                             baseString.append(stringToAppend);
                returnValue = tsReturn.data() ;
             }
             else if (type == TYPE_UTLSTRING)
             {
                 prefix = "Test append(UtlString) " ;
                 UtlString &tsReturn = baseString.append( \
                            utlStringToAppend) ;
                 returnValue = tsReturn.data() ;
             }
         }

         string strExp ;
         strExp.append(longAlphaString) ;
         strExp.append(stringToAppend) ;

         string Msg ;
         // Verify if string has been appended.
         TestUtilities::createMessage(3, &Msg, prefix, testTypeMsg, suffix1) ;
         CPPUNIT_ASSERT_EQUAL_MESSAGE(Msg.data(), strExp, string(baseString.data())) ;

         // Verify return value
         TestUtilities::createMessage(3, &Msg, prefix, testTypeMsg, suffix2) ;
         CPPUNIT_ASSERT_EQUAL_MESSAGE(Msg.data(), strExp, string(returnValue)) ;
    }

    /** The above test case for appending UtlString instead of
    *   char*
    */
    void testAppend_UtlString_toExistingString()
    {
        utlTestAppend_to_ExistingString(TEST_APPEND) ;
        utlTestAppend_MaxCapacity(TYPE_UTLSTRING, false) ;
    }

    void testInsert_AtVariousLocations()
    {
        struct InsertTestData
        {
            MEMBER_CONST char* szSource ;
            MEMBER_CONST char* szInsert ;
            MEMBER_CONST int   iPosition ;
            MEMBER_CONST char* szResults ;
        } ;

        #define INSERT_TEST_DATA_LENGTH     7
        InsertTestData data[] =
        {
            {"", "INSERT", 0, "INSERT"},                // Insert at 0 w/ empty string
            {"XYZ123", "INSERT", 0, "INSERTXYZ123"},    // Insert at 0 w/ real string
            {"XYZ123", "INSERT", 6, "XYZ123INSERT"},    // Insert at very end
            {"XYZ123", "INSERT", 100, "XYZ123"},        // Insert past end
            {"XYZ123", "INSERT", -3, "XYZ123"},         // Insret before start
            {"XYZ123", NULL, 3, "XYZ123"},              // Bogus source pointer
            {"XYZ123", "INSERT", 3, "XYZINSERT123"},    // Insert in middle
        } ;

        for (int i=0; i<INSERT_TEST_DATA_LENGTH; i++)
        {
            UtlString source(data[i].szSource) ;
            source.insert(data[i].iPosition, data[i].szInsert) ;
            CPPUNIT_ASSERT_EQUAL(string(data[i].szResults), string(source.data()));
        }
    }


    /**  Test the insert method inserting at the last location
    *
    *     This test is exactly similar to the append method and
    *     and so is the test data.
    */
    void testInsert_AtLastLocation()
    {
        utlTestAppend_to_ExistingString(TEST_INSERTLAST) ;
    }

    /** test the += UtlString operator. This operator is
    *    exactly the same as the append method and so is the test data
    */
    void testPlusEqual_UtlString()
    {
        utlTestAppend_to_ExistingString(TEST_PLUSEQUAL) ;
    }


    /** Utiltity to test the append like methods to a non-empty string.
    *   The parameter passed as the 'to be appended string' must be a
    *   UtlString. The last parameter decides what target method
    *   is actually being tested. For example append(),insertAt(lastPosition)
    *   and the += operator do exactly the same thing. Instead of repeating
    *   all other steps, the test for these methods have been combined into
    *   this utility method.
    */
    void utlTestAppend_to_ExistingString( \
                  AppendInsertReplaceOrPlusEqual optype)
    {
        const char* prefix = "To a string made of " ;
        // Test Specific messages are constructed using the
        // common test data set messages.
        const char* suffix1 = " :- verify if appended" ;
        const char* suffix2 = " :- verify returned string" ;

        string strExp ;

        // Test appending each of the common types of string TO
        // every type (common) of string
        for (int i = 0 ; i < commonTestSetLength ; i++)
        {
            int len_baseString = strlen(commonTestSet[i].input) ;
            for(int j =0 ; j < commonTestSetLength; j++)
            {
                UtlString baseString(commonTestSet[i].input) ;
                UtlString stringToAppend(commonTestSet[j].input) ;

                // To calculate what the expected type of string is going
                // to be, use the standard string class
                strExp.erase() ;
                strExp.append(commonTestSet[i].input) ;
                strExp.append(commonTestSet[j].input) ;

                const char* tmpSuffix = "";

                const char* returnValue = "";
                if (optype == TEST_APPEND)
                {
                    tmpSuffix = " test append(UtlString s) where s = ";
                    UtlString &tsReturn = \
                                baseString.append(stringToAppend) ;
                    returnValue = tsReturn.data() ;
                }
                else if (optype == TEST_INSERTLAST)
                {
                    tmpSuffix = " test insert(len-1, UtlString s) where s = " ;
                    UtlString &tsReturn = baseString.insert( \
                               len_baseString, stringToAppend) ;
                    returnValue = tsReturn.data() ;
                }
                else if (optype == TEST_PLUSEQUAL)
                {
                    tmpSuffix = " test operator += (UtlString s) where s = " ;
                    UtlString &tsReturn = baseString \
                                         += stringToAppend ;
                    returnValue = tsReturn.data() ;
                }

                string compositeMessage ;
                // Verify if the string *has* been appended
                TestUtilities::createMessage(5, &compositeMessage, prefix, \
                                      commonTestSet[i].testDescription, tmpSuffix, \
                                      commonTestSet[j].testDescription, suffix1);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(compositeMessage.data(), strExp, \
                                          string(baseString.data())) ;

                // Verify the return value
                TestUtilities::createMessage(5, &compositeMessage, prefix, \
                                      commonTestSet[i].testDescription, tmpSuffix, \
                                      commonTestSet[j].testDescription, suffix2);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(compositeMessage.data(), strExp, string(returnValue)) ;
             } // end inner for loop
         } // end outer for loop

    } // End method - utlTestAppendToExistingString


   /**
    *   Test case to test the append(const char*, int size) method
    *      Test the case of copying to an empty string -
    *      a few of the bytes of the char* being passed
    *
    *   The test data for this test case are :-
    *      all the data of the testAppend_char_toEmptyString() test case;
    *      only in this case the test should try to copy only a few
    *      characters and verify that the resulting string has only
    *      'those many bytes' copied
    *
    */
    void testAppend_charstar_specifiedSize_toEmptyString()
    {
        utlTestAppend_specifiedSize_toEmptyString(TYPE_CHARSTAR) ;
    }


    /** Utility to test append(char*, size)/append(UtlString, size)
    *   to an empty string
    *   (This method was written keeping in mind that there may be an
    *    append(utlString, len). However such a method still doesn't exist)
    */
    void utlTestAppend_specifiedSize_toEmptyString(StringType type)
    {
        const char* prefix = "To a zero length string, test " \
                    "appending some of the characters of a " ;
        //Msgs = common test data set messages
        const char* suffix1 =":- verify if appended to original string" ;
        const char* suffix2 = ":- verify returned string" ;

        const char* eLong = \
                   "abcdefghijklmnopqrstuvwzyz"
                   "abcdefghijklmnopqrstuvwzyz"
                   "abcdefghijklmnopqrstuvwzyz"
                   "abcdefghijklmnopqrstuvwzyz"
                   "abcdefghijklmnopqrstuvwzyz"
                   "abcdefghijklmnopqrstuvwzyz"
                   "abcdefghijklmnopqrstuvwzyz"
                   "abcdefghijklmnopqrstuvwzyz"
                   "abcdefghijklmnopqrstuvwzyz"
                   "abcdefghijklmnopqrstuv" ;
        const char* eSpl = "Ð墀%$',+*\"?+*"   ;
        const char* exp[] = {  "", "", eLong,  "This makes sens", "1257", eSpl } ;

         for (int i = 0 ; i < commonTestSetLength ; i++)
         {
             UtlString baseString("") ;
             const char* returnValue ;
             UtlString stringToAppend(commonTestSet[i].input) ;

             int sizeToCopy = commonTestSet[i].length - 1 ;
             sizeToCopy = sizeToCopy < 0 ? 0 : sizeToCopy ;
             if (type == TYPE_CHARSTAR)
             {
                  UtlString &ts = baseString.append( \
                                  commonTestSet[i].input, sizeToCopy) ;
                  returnValue = ts.data() ;
             }
             else
             {
                  UtlString &ts = baseString.append( \
                                       stringToAppend, sizeToCopy) ;
                  returnValue = ts.data() ;
             }
             string Message ;

             // Verify if the target has been appended to the original string
             TestUtilities::createMessage(3, &Message, prefix, \
                            commonTestSet[i].testDescription, suffix1) ;
             CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(exp[i]), \
                                       string(baseString.data())) ;

             // Verify the return value
             TestUtilities::createMessage(3, &Message, prefix, \
                            commonTestSet[i].testDescription, suffix2) ;
             CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(exp[i]), \
                                       string(returnValue)) ;
         }
    }

    /**
    *   Test case to test the append(const char*, int size) method
    *      Test the case of copying to a non-zero string, a few of the
    *      bytes of the char* being passed
    *
    *   The test data for this test case are :-
    *      all the data of the testAppend_char_toExistingString() test case;
    *      only in this case the test should try to copy only a few characters
    *      and verify that the resulting string has only 'those many bytes'
    *      copied
    */
    void testAppend_charstar_specifiedSize_toExistingString()
    {
        utlTestAppend_specifiedSize_toExistingString(TYPE_CHARSTAR) ;
    }

    /** utility to test append(char*, size)/append(UtlString, size)
    *  to a non-empty string
    *  The append(UtlString is not yet implemented by utils library). However
    *  this method has the ability to test that when available.
    */
    void utlTestAppend_specifiedSize_toExistingString(StringType type)
    {
        struct LimitedSizeTestStructure
        {
            const char* testDescription ;
            const char* input ;
            int length ;
            int lengthToCopy ;
            const char* expectedValue ;
        };

        const char* prefix  = "To a non-zero length string that is made of" ;
        const char* suffix1 = ":- verify if appended to original string" ;
        const char* suffix2 = ":- verify returned string" ;
        string completeMessage("") ;

        const char* expectedForLong = \
                   "abcdefghijklmnopqrstuvwzyz"
                   "abcdefghijklmnopqrstuvwzyz"
                   "abcdefghijklmnopqrstuvwzyz"
                   "abcdefghijklmnopqrstuvwzyz"
                   "abcdefghijklmnopqrstuvwzyz"
                   "abcdefghijklmnopqrstuvwzyz"
                   "abcdefghijklmnopqrstuvwzyz"
                   "abcdefghijklmnopqrstuvwzyz"
                   "abcdefghijklmnopqrstuvwzyz"
                   "abcdefghijklmnopqrstu" ;

        const char* expectedForSplChars = "Ð墀%$',+*\"?+*"   ;

        const int lenLongString = strlen(longAlphaString) ;
        const int lenSplChars = strlen(splCharString) ;
        const LimitedSizeTestStructure testData[] = { \
            {"single character string", "g", 1, 0, ""}, \
            {"very long string", longAlphaString, lenLongString, \
                                 lenLongString - 2, expectedForLong }, \
            {"regular string", "This makes sense", 16, 11, "This makes "}, \
            {"string with only digits", "12567", 5, 3, "125" }, \
            {"string with special characters", splCharString, lenSplChars, \
                                  lenSplChars-1, expectedForSplChars } \
        } ;

        string expectedString; // Variable to create the expected appended string for each test.

        const int testCount = sizeof(testData)/sizeof(testData[0]) ;
        // We need to testing the different categories of string being
        // appended to each category of string.
        // To acheive this, iterate through 2 nested loops - Do not
        // test the case where the base string is empty.
        for (int i = 1 ; i < commonTestSetLength ; i++)
        {
            for (int j = 0 ; j < testCount; j++)
            {
                UtlString baseString(commonTestSet[i].input) ;
                UtlString stringToAppend(testData[j].input)  ;
                const char* returnValue ;
                const char* tmpSuffix ;

                if (type == TYPE_CHARSTAR)
                {
                    tmpSuffix = " test append(char* cs, n); where cs = " ;
                    UtlString &ts = baseString.append( \
                                       testData[j].input, testData[j].lengthToCopy) ;
                    returnValue = ts.data() ;
                }
                else
                {
                    tmpSuffix = " test append(UtilString us, n); where us = " ;
                    UtlString &ts = baseString.append( \
                                        stringToAppend, testData[j].lengthToCopy) ;
                    returnValue = ts.data() ;
                }

                // construct an expected string using the append operations on a
                // std::string instance. We are assuming here that we can trust the
                // std::string class.
                expectedString.erase() ;
                expectedString.append(commonTestSet[i].input) ;
                expectedString.append(testData[j].expectedValue) ;


                // Verify if the target has been appended to the original
                // string
                TestUtilities::createMessage( 5, &completeMessage, prefix, \
                                  commonTestSet[i].testDescription, tmpSuffix, \
                                  testData[j].testDescription, suffix1 );
                CPPUNIT_ASSERT_EQUAL_MESSAGE(completeMessage.data(), expectedString, \
                                          string(baseString.data())) ;

                // Verify the return value.
                TestUtilities::createMessage( 5, &completeMessage, prefix, \
                                  commonTestSet[i].testDescription, tmpSuffix, \
                                  testData[j].testDescription, suffix2 );
                CPPUNIT_ASSERT_EQUAL_MESSAGE(completeMessage.data(), \
                                          expectedString, string(returnValue)) ;

           } // end inner for loop
        } // end outer for loop
    }

    /**
    *   Test case to test the append(const char* cs, int size) method
    *      Case e) - Append zero bytes of a non-zero char* string to
    *        a zero length string
    *
    *   The test data for this test case are :-
    *      a) cs is Empty String
    *      b) cs is a reasonable length string
    *      c) cs is a special character.
    */
    void testAppend_charstar_zero_bytes_toEmptyString()
    {
        utlTestAppend_zero_bytes_toEmptyString(TYPE_CHARSTAR) ;
    }

    /** The above test case for appending UtlString instead of
    *   char*
    */
    void testAppend_UtlString_zero_bytes_toEmptyString()
    {
        utlTestAppend_zero_bytes_toEmptyString(TYPE_UTLSTRING) ;
    }

    /** Utility that is used to test either append(char*, int)
    *   or append(UtlString, int) when the the string to be tested
    *   is empty and the int parameter specified is zero bytes.
    */
    void utlTestAppend_zero_bytes_toEmptyString(StringType type)
    {
        const char* prefix = "To a zero length string, test appending"\
                       " zero characters of" ;
        const char* suffix1 = ":- verify if appended to original string" ;
        const char* suffix2 = ":- verify returned string" ;
        string Message ;

        const BasicStringVerifier testData[] = { \
            {"an Empty String", "", 0 }, \
            {"a reasonable length string","SomeString", 10}, \
            {"a string with special characters",  "12@#d*&", 7} \
        } ;

        const int testCount = sizeof(testData) / sizeof(testData[0]) ;

        for (int i = 0 ; i < testCount ; i++)
        {
             UtlString baseString("") ;
             UtlString stringToAppend(testData[i].input) ;
             const char* returnValue ;

             if (type == TYPE_CHARSTAR)
             {
                 UtlString &testString = baseString.append(testData[i].input,0) ;
                 returnValue = testString.data() ;
             }
             else
             {
                 UtlString &testString = baseString.append(stringToAppend, 0) ;
                 returnValue = testString.data() ;
             }

             // Verify if the string has been appended
             TestUtilities::createMessage(3, &Message, prefix, \
                                          testData[i].testDescription, suffix1) ;
             CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(""), string(baseString.data())) ;

             // Verify the return value
             TestUtilities::createMessage(3, &Message, prefix, \
                            testData[i].testDescription, suffix2) ;
             CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(""), string(returnValue)) ;

        }
    } //utlTestAppend_zero_bytes_toEmptyString

    /**
    *   Test case to test the append(const char*, int size) method
    *      Test the case where you append zero bytes of a
    *      non-zero char* string to a non-zero string
    *
    *   The test data for this test case are :-
    *      a) to f) of testAppend_char_toExistingString()
    */
    void testAppend_charstar_zero_bytes_toExistingString()
    {
        utlTestAppend_zero_bytes_toExistingString(TYPE_CHARSTAR) ;
    }


    /** The above test case for appending UtlString instead of
    *   char*
    */
    void testAppend_UtlString_zero_bytes_toExistingString()
    {
        utlTestAppend_zero_bytes_toExistingString(TYPE_UTLSTRING) ;
    }

    /** Utility that is used to test either append(char*, int)
    *   or append(UtlString, int) when the the string to be tested
    *   is empty and the int parameter specified is zero bytes.
    */
    void utlTestAppend_zero_bytes_toExistingString(StringType type)
    {
        const char* prefix = "To a string consisting of " ;
        const char* suffix1 = ":- verify if appended to original string" ;
        const char* suffix2 = ":- verify returned string"  ;
        string completeMsg("") ;

        const BasicStringVerifier testData[] = { \
            {"an Empty String", "", 0 }, \
            {"a reasonable length string","SomeString", 10}, \
            {"a string with special characters",  "12@#d*&", 7} \
        } ;
        const int testCount = sizeof(testData) / sizeof(testData[0]) ;

        for (int i = 0 ; i < commonTestSetLength ; i++)
        {
            for (int j = 0 ; j < testCount ; j++)
            {
                UtlString baseString(commonTestSet[i].input) ;
                UtlString stringToAppend(testData[j].input) ;
                const char* returnValue ;

                if (type == TYPE_CHARSTAR)
                {
                    UtlString &testString  = baseString.append( \
                                                        testData[j].input,0) ;
                    returnValue = testString.data() ;
                }
                else
                {
                    UtlString &testString  = baseString.append( \
                                                        stringToAppend, 0) ;
                    returnValue = testString.data() ;
                }

                const char* tmpSuffix = " append zero bytes of " ;

                // Verify that the target is appended to the original string
                TestUtilities::createMessage(5, &completeMsg, prefix, \
                               commonTestSet[i].testDescription, tmpSuffix, \
                               testData[j].testDescription, suffix1) ;
                CPPUNIT_ASSERT_EQUAL_MESSAGE(completeMsg.data(), \
                            string(commonTestSet[i].input), string(baseString.data())) ;

                // Verify the return value.
                TestUtilities::createMessage(5, &completeMsg, prefix, \
                               commonTestSet[i].testDescription, tmpSuffix, \
                               testData[j].testDescription, suffix2) ;
                CPPUNIT_ASSERT_EQUAL_MESSAGE(completeMsg.data(), string(commonTestSet[i].input), \
                                    string(returnValue)) ;
            }
        }
     } //utlTestAppend_zero_bytes_toExistingString

    /** Test the prepend method()

    *     The test data for this test case are
    *        For a stiing that is of each type as in the common test data set
    *        (check test_Constructor_1), append all types of strings in the
    *        common test data.
    */
    void testPrepend()
    {
        utlTestPrependInsertOrReplace(TEST_PREPEND) ;
    }

    /** Test the insert method when inserting at the 0th location
    *
    *        This test is exactly similar to the prepend method and
    *        and so is the test data
    */
    void testInsert_AtFirstLocation()
    {
        utlTestPrependInsertOrReplace(TEST_INSERT) ;
    }

    /**  Test the replace(0, 0, char*) method
    *       The test data for this is the same as the testPrepend()
    *       test case
    */
    void testReplace_zeroth_char()
    {
        utlTestPrependInsertOrReplace(TEST_REPLACEFIRST) ;
    }

    /** Utility method that tests the testPrepend, testInsert(0, ...) or
    *   testReplace(0, 0, ..) . Since all these methods are functionally
    *   equivalent, they have been abstracted into one single method.
    */
    void utlTestPrependInsertOrReplace( \
                PrependInsertOrReplace type = TEST_PREPEND)
    {
        const char* prefix = "To a string that is made of" ;
        const char* suffix1 = ":- verify if prepended to original string" ;
        const char* suffix2 = ":- verify returned string" ;
        string completeMessage("") ;

        string strExp; //variable to compute the expected result.

        // We need to be testing the different categories of string being
        // appended to each category of string.
        // To acheive this, iterate through 2 nested loops
        for (int i = 1 ; i < commonTestSetLength ; i++)
        {
            for (int j = 0 ; j < commonTestSetLength; j++)
            {
                UtlString baseString(commonTestSet[i].input) ;
                UtlString stringToPrepend(commonTestSet[j].input) ;
                const char* returnValue = "";

                strExp.erase() ;
                strExp.append(commonTestSet[j].input) ;
                strExp.append(commonTestSet[i].input) ;

                const char* tmpSuffix = "";
                if (type == TEST_PREPEND)
                {
                    UtlString &testString = baseString.prepend( \
                                         commonTestSet[j].input) ;
                    returnValue = testString.data() ;
                    tmpSuffix = " test prepending a " ;
                }
                else if (type == TEST_INSERT)
                {
                    UtlString &testString = baseString.insert(0, \
                                          stringToPrepend) ;
                    returnValue = testString.data() ;
                    tmpSuffix = " test inserting(at 0) a " ;
                }
                else if (type == TEST_REPLACEFIRST)
                {
                    UtlString &testString = baseString.replace(0, 0, \
                                         commonTestSet[j].input) ;
                    returnValue = testString.data() ;
                    tmpSuffix = " test replacing (at 0,0) a " ;
                }

                TestUtilities::createMessage(5, &completeMessage, prefix, \
                               commonTestSet[i].testDescription, tmpSuffix, \
                               commonTestSet[j].testDescription, suffix1 ) ;
                CPPUNIT_ASSERT_EQUAL_MESSAGE(completeMessage.data(), strExp, \
                                          string(baseString.data())) ;

                TestUtilities::createMessage(5, &completeMessage, prefix, \
                               commonTestSet[i].testDescription, tmpSuffix, \
                               commonTestSet[j].testDescription, suffix2 ) ;
                CPPUNIT_ASSERT_EQUAL_MESSAGE(completeMessage.data(), strExp, \
                                          string(returnValue)) ;
           } // for j
        } // for i
    } // testPrependInsertOrReplace


     /**  Test the insert method to insert a string somewhere in the middle
     *
     *     The test data for this are
     *       For any alphanumeric string
     *         a) insert a zero length string
     *         b) insert a reasonable nominal string
     *         c) insert an alphanumeric string
     *         (This method is not used very widely by us. That's the rationale
     *         behind testing only 3 cases - which btw, covers the different
     *         partitions)
     */
    void testInsert_AtMidLocation()
    {
        struct TestInsertDataStruct
        {
            const char* testDescription ;
            const char* input ;
            int length ;
            int insertLocation ;
            const char* expectedValue ;
        } ;

        const char* prefix = "To a regular string insert(n, char*), where char*=" ;
        const char* suffix1 = ":- verify if inserted" ;
        const char* suffix2 = ":- verify return value";
        string Message ;

        const char* baseString = "Original String" ;
        TestInsertDataStruct testData[] = { \
            {"empty string", "", 0, 3, baseString }, \
            {"regular string", "A test string", 13, 3, "OriA test stringginal String" }, \
            {"alpha-num string", "Ta12#45bc", 9, 3, "OriTa12#45bcginal String" } \
        } ;

        const int testCount = sizeof(testData)/sizeof(testData[0]) ;
        for (int i = 0 ; i < testCount; i++)
        {
            UtlString testString(baseString) ;
            UtlString returnString;

            returnString = testString.insert(testData[i].insertLocation, testData[i].input) ;

            TestUtilities::createMessage(3, &Message, prefix, \
                            testData[i].testDescription, suffix1) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(testData[i].expectedValue), \
                                      string(testString.data())) ;

            TestUtilities::createMessage(3, &Message, prefix, \
                            testData[i].testDescription, suffix2) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(testData[i].expectedValue), \
                                      string(returnString.data())) ;
        }
     } //testInsert_AtMidLocation()


    /**  Test the Assign(char*) operator.
    *   The test data for this case is the same as the char* constructor
    */
    void testAssign_charstar()
    {
        const char* prefix = " Test ( = char* ) when the old string is " ;
        string Message ;
        for (int i = 0 ; i < commonTestSetLength; i++)
        {
            UtlString testString("Old String...") ;
            testString = commonTestSet[i].input ;
            TestUtilities::createMessage(2, &Message, prefix, \
                           commonTestSet[i].testDescription) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                              string(commonTestSet[i].input), string(testString.data())) ;
        }
    }


    /** Testcase for the '= UtlString' assignment operator
    *
    *   The test data for this test case is the same as the copy constructor
    */
    void testAssign_UtlString()
    {
        const char* prefix = " ( = UtlString) when the old string is " ;
        const char* suffix1 = " :- verify original string is intact" ;
        const char* suffix2 = " :- verify copied string " ;
        const char* suffix3 = " :- verify capacity " ;
        string Message ;

        for (int i = 0 ; i < commonTestSetLength; i++)
        {
            UtlString ts_original(commonTestSet[i].input) ;
            UtlString ts_copied= ts_original ;

            // First verify that the original string is intact.
            TestUtilities::createMessage(3, &Message, prefix, commonTestSet[i].testDescription, \
                                         suffix1) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(commonTestSet[i].input), \
                                      string(ts_original.data())) ;

            // Verify the copied string
            TestUtilities::createMessage(3, &Message, prefix, commonTestSet[i].testDescription, \
                                         suffix2) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(commonTestSet[i].input), \
                                      string(ts_copied.data())) ;
            TestUtilities::createMessage(3, &Message, prefix, commonTestSet[i].testDescription, \
                                         suffix3) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), ts_original.capacity(), \
                                      ts_copied.capacity()) ;

       }
    }

    /** Test the const char*() operator
    *
    *       The data for this is the common data set
    */
    void testCharstarCastOperator()
    {
        const char* prefix = "Test the const char* cast operator for " ;
        string Message ;
        for (int i =0 ; i < commonTestSetLength; i++)
        {
            UtlString testString(commonTestSet[i].input) ;
            const char* actual = (const char*) testString ;
            TestUtilities::createMessage(2, &Message, prefix, commonTestSet[i].testDescription) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(commonTestSet[i].input), \
                                      string(actual) ) ;
        }
    }

   void testSubstringAssign()
      {
         UtlString source("0123456789");

         UtlString dest;

         dest = source(1,3);

         ASSERT_STR_EQUAL(dest.data(),"123");

         dest.append(source, 6, 2);

         ASSERT_STR_EQUAL(dest.data(),"12367");
      }

   // Test using NULL to represent the null string.
   void testNull()
      {
         UtlString t((const char *) NULL);
         ASSERT_STR_EQUAL_MESSAGE("Construct string from NULL",
                                  "", t.data());
         t.append("123");
         t.append((const char *) NULL);
         ASSERT_STR_EQUAL_MESSAGE("Append NULL",
                                  "123", t.data());
      }

   void testAppendNumber()
      {
         UtlString base("Number: ");

         UtlString t;
         t = base;
         t.appendNumber(12);
         ASSERT_STR_EQUAL("Number: 12", t.data());

         t = base;
         t.appendNumber(12, "%03d");
         ASSERT_STR_EQUAL("Number: 012", t.data());

         t = base;
         t.appendNumber(12, "%08x");
         ASSERT_STR_EQUAL("Number: 0000000c", t.data());

         t.remove(0);
         t.appendNumber(12);
         ASSERT_STR_EQUAL("12", t.data());

         t.remove(0);
         t.appendNumber(12, "%03d");
         ASSERT_STR_EQUAL("012", t.data());

         t.remove(0);
         t.appendNumber(12, "%08x");
         ASSERT_STR_EQUAL("0000000c", t.data());

         long long int value = 680531417658032128LL;
         t.remove(0);
         t.appendNumber(value);
         ASSERT_STR_EQUAL("680531417658032128", t.data());

         int value1 = 1234;
         t.remove(0);
         t.appendNumber(value1);
         ASSERT_STR_EQUAL("1234", t.data());

         ssize_t value2=1234;
         t.remove(0);
         t.appendNumber(value2);
         ASSERT_STR_EQUAL("1234", t.data());

         t = base;
         t.appendNumber(base.length(), "%x");
         ASSERT_STR_EQUAL("Number: 8", t.data());

      }

} ;

CPPUNIT_TEST_SUITE_REGISTRATION(UtlStringTest_ConstructiveManipulators);
