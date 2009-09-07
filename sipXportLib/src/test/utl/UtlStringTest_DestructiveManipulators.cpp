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
*    PLEASE READ THE README FILE THAT CAN FOUND IN THE PARENT OF THE DIRECTORY
*    OF THIS FILE. The Readme describes the organization / flow of tests and
*    without reading this file, the following class (and all unit tests)
*    may not make a lot of sense and might be difficult to comprehend.
*/
class UtlStringTest_DestructiveManipulators : public  UtlStringTest
{

    CPPUNIT_TEST_SUITE(UtlStringTest_DestructiveManipulators);
    CPPUNIT_TEST(testReplace_Middle) ;
    CPPUNIT_TEST(testReplace_Middle_SpecifyLengthAsSize) ;
    CPPUNIT_TEST(testReplace_Middle_SpecifySize) ;
    CPPUNIT_TEST(testReplaceAt) ;
    CPPUNIT_TEST(testReplaceCharacter) ;
    CPPUNIT_TEST(testStrip_Default) ;
    CPPUNIT_TEST(testStrip_TrailingSpaces_SpecifyStripType) ;
    CPPUNIT_TEST(testStrip_LeadingSpaces) ;
    CPPUNIT_TEST(testStrip_AllSpaces) ;
    CPPUNIT_TEST(testStrip_Characters) ;
    CPPUNIT_TEST(testResize) ;
    CPPUNIT_TEST_SUITE_END();

private :
        struct TestStripDataStructure
        {
            const char* testDescription ;
            const char* input ;
            char characterToStrip ;
            const char* expectedForStripTrailing ;
            const char* expectedForStripLeading ;
            const char* expectedForStripBoth ;
        };

public:

    UtlStringTest_DestructiveManipulators()
    {
    }
    void setUp()
    {
    }

    void tearDown()
    {
    }

    ~UtlStringTest_DestructiveManipulators()
    {
    }

    /** Sandbox method for experimenting with the API Under Test.
    *   This method MUST be empty when the test drivers are being
    *   checked in (final checkin) to the repository.
    */
    void DynaTest()
    {
    }

      void testReplaceAt()
      {
         UtlString target("won");
         target.replaceAt(0,'t');
         ASSERT_STR_EQUAL("ton", target.data());
         target.replaceAt(1,'i');
         ASSERT_STR_EQUAL("tin", target.data());
         target.replaceAt(2,'e');
         ASSERT_STR_EQUAL("tie", target.data());
         target.replaceAt(3,'e');
         ASSERT_STR_EQUAL("tie", target.data());
         target.replaceAt(4,'e');
         ASSERT_STR_EQUAL("tie", target.data());
         target.replaceAt(5,'e');
         ASSERT_STR_EQUAL("tie", target.data());
      }

    /** Test the replace(src, tgt)
    *   The test case for this method are
    *     a) for an empty string replace 'a' with 'b'
    *     e) for a string with only one occurance of 'src' replace with 'tgt'
    *     c) for a string with multiple occurances of 'src' replace with 'tgt'
    *     d) for a string with one or more occurances of swapped case of 'src'
    *        replace with 'tgt'
    *     e) for a string with one or more occurances of 'src' replace with 'src
    *     f) for a string with zero occurances of 'src' replace with 'tgt'
    *     g) for any string replace '0' with 'tgt' (verify that bad things dont
             happen
    *     h) for a string with one or more occurances of 'src' replace with '0'
    */
    void testReplaceCharacter()
    {
        struct TestReplaceCharStructure
        {
            MEMBER_CONST char* testDescription ;
            MEMBER_CONST char* input ;
            MEMBER_CONST char replaceSrc ;
            MEMBER_CONST char replaceTgt ;
            MEMBER_CONST char* expectedResult ;
        };
        const char* prefix = "Test the replace(char src, char tgt) method " ;
        const char* suffix1 = ": Verify return value" ;
        const char* suffix2 = ": Verify original string" ;
        string Message ;

        TestReplaceCharStructure testData[] = { \
            { "for an empty string replace 'a' with 'b'", "", 'a', 'b', "" }, \
            { "for a string with only one occurance of 'src', replace with 'tgt'", \
              "Test string", 'e', 'o', "Tost string" }, \
            { "for a string with multiple occurances of 'src' replace with 'tgt'", \
              "Test string", 's', 'f', "Teft ftring" }, \
            { "for a string with occurance(s) of 'src', replace with swapped case of src", \
              "Test string", 'R', 'l', "Test string" }, \
            { "for a string with occurance(s) of 'src' replace with 'src'", \
              "Test string", 's', 's', "Test string" }, \
            { "for a string with no occurance of 'src' replace with 'tgt'", \
              "Test string", 'm', 'p', "Test string" }, \
            { "for any string replace '0' with 'tgt' ", \
              "Test string", (char)0, 'b', "Test string" }, \
            { "for a string with occurance(s) of 'src' replace 'src' with '0'", \
              "Test string", 's', (char)0, "Test string" }
        } ;
        int testCount = sizeof(testData) / sizeof(testData[0]) ;
        for (int i = 0 ; i < testCount ; i++)
        {
            UtlString testString(testData[i].input) ;
            UtlString returnValue = testString.replace(testData[i].replaceSrc, \
                                    testData[i].replaceTgt) ;
            TestUtilities::createMessage(3, &Message, prefix, testData[i].testDescription, \
                suffix1) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(testData[i].expectedResult), \
               string(testString.data())) ;
            TestUtilities::createMessage(3, &Message, prefix, testData[i].testDescription, \
               suffix2) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(testData[i].expectedResult), \
               string(returnValue.data())) ;
        }
    }


    /** Test the replace(pos, n, char*)
    *
    *    Test data for this method
    *      With a regular string, replace at nth suffixion (n < len)
    *           a) any regular string upto position m. m < len
    *           b) an alpha-num string upto position m. m < len
    *           c) any string upto position m. m = len
    *           d) any string upto position m. m > len
    *           e) an empty string. n = 0 . m = len
    */
    void testReplace_Middle()
    {
        struct TestReplaceDataStructure
        {
            const char* testDescription ;
            const char* input ;
            int startPosition ;
            int replaceLength ;
            const char* expectedValue;
        };

        const char* prefix = "test replace(start, end, char*), where char* = " ;
        const char* suffix1 = ":- verify if replaced" ;
        const char* suffix2 = ":- verify return value" ;
        string Message ;

        const char* baseString = "string rep" ;
        const TestReplaceDataStructure testData[] = { \
               {"an empty string. start = 0; last = 0 ", "", 0, 10, "" }, \
               {"a regular string. start > 0; last < len(baseString) ", "Test Str", \
                           2, 4, "stTest Str rep" }, \
               {"a alpha-num string. start > 0; last < len(baseString) ", "Te12 $tr", \
                           2, 4, "stTe12 $tr rep" }, \
               {"a regular string. start > 0; last = total no. of remaining characters ", \
                           "Test Str", 2, 8, "stTest Str" }, \
               {"a alpha-num string. start > 0 ; last > len(baseString)", "Te12 $tr", \
                           2, 11, "stTe12 $tr" }, \
        } ;

        const int testCount = sizeof(testData)/sizeof(testData[0])  ;
        // Using all the data that was created above, run the actual tests.
        for(int i = 0 ; i < testCount ; i++)
        {
            UtlString testString(baseString) ;
            UtlString returnString = testString.replace(testData[i].startPosition, \
                          testData[i].replaceLength, testData[i].input) ;

            // Verify if the selected text was replaced.
            TestUtilities::createMessage(3, &Message, prefix, testData[i].testDescription, \
                                        suffix1) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(testData[i].expectedValue), \
                                      string(testString.data())) ;

            // Verify return value
            TestUtilities::createMessage(3, &Message, prefix, testData[i].testDescription, \
                                        suffix2) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(testData[i].expectedValue), \
                                     string(returnString.data())) ;
        }
    } //testReplace_Middle

    /** Test the replace(pos, n, char*, len) where len = len(char*)
    *
    *  The test data for this test case is the same as the testReplace_Middle()
    *  test case
    */
    void testReplace_Middle_SpecifyLengthAsSize()
    {
        utlTestReplace(true) ;
    }

    /** Test the replace(pos, n, char*, len) where len < len(char*)
    *
    *    The test data for this case is the same as testReplace_Middle case
    */
    void testReplace_Middle_SpecifySize()
    {
        utlTestReplace(false) ;
    }


    // Utility to test the replace method.
    void utlTestReplace(bool replaceAllofTarget)
    {
        struct TestReplaceSelectedDataStructure
        {
            const char* testDescription ;
            const char* input ;
            int startPosition ;
            int replaceLength ;
            int inputLength ;
            int charactersToCopy ;
            const char* expectedForCopyAll ;
            const char* expectedForCopyLimited ;
        };

        const char* prefix = "test replace(start, end, char*, len), where char* = " ;
        const char* suffix1 ;
        const char* suffix2 ;
        if (replaceAllofTarget)
        {
            suffix1 = "size = len(targetString) :- verify if replaced" ;
            suffix2 = "size = len(targetString) :- verify return value";
        }
        else
        {
            suffix1 = "size < len(targetString) -2 :- verify if replaced" ;
            suffix2 = "size < len(targetString) -2 :- verify return value";
        }
        string Message ;

        const char* baseString = "string rep" ;
        TestReplaceSelectedDataStructure testData[] = { \
               { "an empty string. start = 0; last = 0 ", \
                     "", 0, 10, 0, 0, "", "" }, \
               { "a regular string. start > 0; last < len(baseString) ", \
                     "Test Str", 2, 4, 8, 4, "stTest Str rep", "stTest rep" }, \
               { "a alpha-num string. start > 0; last < len(baseString) ", \
                     "Te12 $tr", 2, 4, 8, 4, "stTe12 $tr rep", "stTe12 rep" }, \
               { "a regular string. start > 0; last = total no. of remaining characters", \
                     "Test Str", 2, 8, 8, 4, "stTest Str", "stTest" }, \
               { "a alpha-num string. start > 0 ; last > len(baseString) ", \
                     "Te12 $tr", 2, 11, 8, 4, "stTe12 $tr", "stTe12" } \
         } ;

        const int testCount = sizeof(testData)/sizeof(testData[0]) ;

        // Using all the data that was created above, run the actual tests.
        for(int i = 0 ; i < testCount ; i++)
        {
            UtlString ts1(baseString) ;
            UtlString ts2;
            const char* expected ;
            if (replaceAllofTarget)
            {
                ts2 = ts1.replace(testData[i].startPosition, testData[i].replaceLength, \
                                     testData[i].input, testData[i].inputLength) ;
                expected = testData[i].expectedForCopyAll ;
            }
            else
            {
                ts2 = ts1.replace(testData[i].startPosition, testData[i].replaceLength, \
                                     testData[i].input, testData[i].charactersToCopy) ;
                expected = testData[i].expectedForCopyLimited ;
            }

            // Verify if the selected text was replaced.
            TestUtilities::createMessage(3, &Message, prefix, testData[i].testDescription, \
                                        suffix1) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(expected), string(ts1.data())) ;

            // Verify return value
            TestUtilities::createMessage(3, &Message, prefix, testData[i].testDescription, \
                                        suffix2) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(expected), string(ts2.data())) ;
        }
   } //utlTestReplace


     /**  Test the strip() to strip off trailing spaces.
     *
     *     The different types of strings to be used for testing this case are
     *         a) An Empty String
     *         b) A regular string with trailing spaces
               c) A regular string with leading spaces
     *         d) A regular string without spaces
     *         e) An alphanumeric string with trailing spaces
     *         f) An alphanumeric string without trailing spaces
     *         g) An alphanumeric string with both spaces
     *         h) Space only string
     */
    void testStrip_Default()
    {
        KNOWN_EFENCE_BUG("Segmentation fault, EF_PROTECT_BELOW=1", "XPL-8");
        utlTestStripSpaces(UtlString::trailing, false, false) ;
    }

    /** Test the strip() method by specifying the strip type explicitly
    *
    *    This test is exactly similar to the testStrip_Default() one and
    *    hence the test data is the same
    *    Add another test here in which you explicitly specify that the
    *    space character is to be striped.
    */
    void testStrip_TrailingSpaces_SpecifyStripType()
    {
        KNOWN_EFENCE_BUG("Segmentation fault, EF_PROTECT_BELOW=1", "XPL-8");
        utlTestStripSpaces(UtlString::trailing, true, false) ;
        utlTestStripSpaces(UtlString::trailing, true, true) ;
    }

    /** Test the strip() method for stripping off the leading spaces
    *
    *     The different types of strings to be used for testing this case are
    *     the same as in the testStrip_Default() method.
    */
    void testStrip_LeadingSpaces()
    {
        utlTestStripSpaces(UtlString::leading, true, false) ;
        utlTestStripSpaces(UtlString::leading, true, true) ;
    }

    /** Test the strip() method for stripping off the leading spaces
    *
    *     The different types of strings to be used for testing this case are
    *     the same as in the testStrip_Default() method.
    */
    void testStrip_AllSpaces()
    {
        utlTestStripSpaces(UtlString::both, true, false) ;
        utlTestStripSpaces(UtlString::both, true, true) ;
    }

    /** Utility to test striping of space characters. By altering the parameters
    *  you can test the different strip flavours:
    *  The first parameter controls whether you want to 'strip' leading characters,
    *  trailing characters or both charactersr. The other two parameters are used to
    *  test variants of testing spaces - viz. a) test when the type
    *  of 'stripping' has been explicitly specified as trailing / nothing has specified.
    *  and b) test when the character to be specified has been explicitly specifed as ' ' or
    *  nothing has been specified.
    */
    void utlTestStripSpaces(UtlString::StripType sType, \
                   bool specifyStripType, bool specifyChar)
    {

        // We need a string which has both leading and
        // trailing spaces.
        string longStr("         ") ;
        longStr.append(longAlphaString) ;
        longStr.append("        ") ;

        // To evaluate the expected string resulting out of a strip(trail),
        // construct a string with only the longAlphaString and leading) spaces
        string tmpStringForTrailing("         ") ;
        tmpStringForTrailing.append(longAlphaString) ;

        // To evaluate the expected string resulting out of a strip(leading),
        // construct a string with only the longAlphaString and trailing spaces
        string tmpStringForLeading(longAlphaString) ;
        tmpStringForLeading.append("        ") ;

        const char* expectedForLeading = tmpStringForLeading.data() ;
        const char* expectedForTrailing = tmpStringForTrailing.data() ;
        // If both sides are striped then  the expected
        // string would just be longAlphaString
        const char* expectedForBoth = longAlphaString ;


        const TestStripDataStructure testData[] = { \
               { "empty char*", "", ' ', \
                     "", "", "" }, \
               { "regular char* w/o spaces", "A String", ' ', \
                     "A String", "A String", "A String" }, \
               { "regular char* w trailing spaces", "A String  ", ' ', \
                     "A String", "A String  ", "A String" }, \
               { "regular char* w leading spaces", "  A String", ' ', \
                     "  A String", "A String", "A String" }, \
               { "alpha-num char* w/o spaces", "String12#2A", ' ',\
                     "String12#2A", "String12#2A", "String12#2A" }, \
               { "alpha-num char* w trailing spaces", "String12#2A  ", ' ', \
                     "String12#2A", "String12#2A  ", "String12#2A" }, \
               { "alpha-num char* w both spaces", "  String12#2A  ", ' ', \
                     "  String12#2A", "String12#2A  ", "String12#2A" }, \
               { "space only char*", "     ", ' ', \
                     "", "", ""} , \
               { "Very long char*", longStr.data(), ' ', \
                     expectedForTrailing, expectedForLeading, expectedForBoth } \
        } ;

        // create the first part of the message based on the type of striping to be done!
        string prefix("Test the strip(")  ;
        if (specifyStripType)
        {
            switch(sType)
            {
                case UtlString::trailing :
                    prefix.append("trailing") ;
                    break ;
                case UtlString::leading :
                    prefix.append("leading") ;
                    break ;
                case UtlString::both :
                    prefix.append("both") ;
                    break ;
            }
        }
        if (specifyChar)
        {
            prefix.append(", ' '") ;
        }
        prefix.append(") for a string made of ") ;

        const int testCount = sizeof(testData) / sizeof(testData[0]) ;
        for (int i = 0 ; i < testCount; i++)
        {
            UtlString testString(testData[i].input) ;
            UtlString returnString ;
            string Message ;
            if (specifyStripType)
            {
                if(specifyChar)
                {
                    returnString = testString.strip(sType, ' ');
                }
                else
                {
                    returnString = testString.strip(sType) ;
                }
            }
            else
            {
                KNOWN_EFENCE_BUG("Segmentation fault w/efence", "XPL-9");
                returnString = testString.strip() ;
            }

            const char* expectedValue = "";
            switch(sType)
            {
                case UtlString::trailing:
                    expectedValue = testData[i].expectedForStripTrailing ;
                    break ;
                case UtlString::leading:
                    expectedValue = testData[i].expectedForStripLeading;
                    break ;
                case UtlString::both:
                    expectedValue = testData[i].expectedForStripBoth ;
                    break ;
            }
            TestUtilities::createMessage(2, &Message, prefix.data(), \
                            testData[i].testDescription) ;

            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                        string(expectedValue), \
                        string(returnString.data())) ;
        }
     } //utlTestStripSpaces


    /**  Test the strip() method for stripping of characters other than
    *    spaces
    *      Test data for this test case is :-
    *        a) Empty string
    *        b) String that has trailing 'c' s.
    *        c) String that has leading 'c'.
    *        d) String that has spaces on both ends
    *        e) String that has 'c' on both ends
    *        f) String w/o 'c' and w/o spaces.
    *        g) String w/o 'c' but with spaces.
    */
    void testStrip_Characters()
    {
        const char* prefix = "For a " ;
        const char* suffix1 = " :- test the strip(trailing, 'c') method" ;
        const char* suffix2 = " :- test the strip(leading, 'c') method" ;
        const char* suffix3 = " :- test the strip(both, 'c') method" ;
        string Message ;

       /*
       TestStripDataStructure :
            testDescription, input, characterToStrip, \
            expectedForStripTrailing, expectedForStripLeading, expectedForStripBoth \
      */

      const TestStripDataStructure testData[] = { \
               { "empty string", "", 'e',\
                 "", "", "" }, \

               { "string that has trailing 'c's", "Test Stree", 'e', \
                 "Test Str", "Test Stree", "Test Str" }, \

               { "string that has leading 'c's", "eeTest Str", 'e', \
                 "eeTest Str", "Test Str", "Test Str" }, \

               { "string that has 'c's on both sides", "eeeTest Stre", 'e', \
                 "eeeTest Str", "Test Stre", "Test Str" }, \

               { "string with no 'c's or spaces", "Test Str", 'e', \
                 "Test Str", "Test Str", "Test Str" }, \

               { "string with no 'c's but has spaces", "  Test Str  ", 'e', \
                 "  Test Str  ", "  Test Str  ", "  Test Str  " } \
              } ;

        const int testCount = sizeof(testData) / sizeof(testData[0])  ;

        // Test the case of striping trailing characters
        for (int i = 0 ; i < testCount ; i++)
        {

            UtlString::StripType sType = UtlString::trailing ;
            UtlString testString(testData[i].input) ;
            UtlString returnString = testString.strip(sType, \
                                 testData[i].characterToStrip) ;
            TestUtilities::createMessage(3, &Message, prefix, \
                                 testData[i].testDescription, suffix1) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                 string(testData[i].expectedForStripTrailing), string(returnString.data())) ;
        }
        // Test the case of striping leading characters
        for (int j = 0 ; j < testCount ; j++)
        {

            UtlString::StripType sType = UtlString::leading ;
            UtlString testString(testData[j].input) ;
            UtlString returnString = testString.strip(sType, \
                                 testData[j].characterToStrip) ;

            TestUtilities::createMessage(3, &Message, prefix, testData[j].testDescription, \
                                         suffix2) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                 string(testData[j].expectedForStripLeading), string(returnString.data())) ;
        }
        // Test the case of striping characters on both ends
        for (int k = 0 ; k < testCount ; k++)
        {

            UtlString::StripType sType = UtlString::both ;
            UtlString testString(testData[k].input) ;
            UtlString returnString = testString.strip(sType, \
                                                testData[k].characterToStrip) ;

            TestUtilities::createMessage(3, &Message, prefix, \
                           testData[k].testDescription, suffix3) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                 string(testData[k].expectedForStripBoth), string(returnString.data())) ;
        }
    }

    /** Test the resize() method for a string.
    *   The test data for this test are :-
    *    1) When the string is empty, resize to 0
    *    2) When the string is empty, resize to non-zero
    *    3) When the string is not empty, resize to n > current size
    *    4) When the string is not empty, resize to n = current size
    *    5) When the string is empty, resize to n < current size
    */
    void testResize()
    {
        struct TestResizeStruct
        {
            const char* testDescription ;
            const char* stringData ;
            int resizeLength ;
            const char* expectedString ;
            int expectedLength ;
        } ;

        const char* prefix = "Test the resize(n) method for " ;
        const char* suffix1 = " - Verify modified string data" ;
        const char* suffix2 = " - Verify modified string length" ;
        string Message ;

        TestResizeStruct testData[] = { \
            { "an empty string. Set n to 0", "", 0, "", 0 }, \
            { "an empty string. Set n to > 0", "", 5, "", 5 }, \
            { "a non-empty string. Set n > current size", "Test String", 14, \
              "Test String", 14 }, \
            { "a non-empty string. Set n = current size", "Test String", 11, \
              "Test String", 11 }, \
            { "a non-empty string. Set n < current size", "Test String", 6, \
              "Test S", 6 } \
        } ;

        int testCount = sizeof(testData)/sizeof(testData[0]) ;
        for (int i = 0 ; i < testCount; i++)
        {
            UtlString testString(testData[i].stringData) ;
            testString.resize(testData[i].resizeLength) ;
            //Verify target string's data
            TestUtilities::createMessage(3, &Message, prefix, testData[i].testDescription,\
                 suffix1) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), string(testData[i].expectedString), \
                string(testString.data())) ;
            //Verify target string's length
            TestUtilities::createMessage(3, &Message, prefix, testData[i].testDescription,\
                 suffix2) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), testData[i].expectedLength, \
                (int)testString.length()) ;

        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(UtlStringTest_DestructiveManipulators);
