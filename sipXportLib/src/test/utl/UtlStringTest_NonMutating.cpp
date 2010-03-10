//
// Copyright (C) 2007, 2010 Avaya, Inc., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <string>
#include <utl/UtlString.h>
#include <utl/UtlInt.h>

#include <sipxunit/TestUtilities.h>
#include <utl/UtlStringTest.h>

#ifdef WIN32
#   define MEMBER_CONST
#else
#   define MEMBER_CONST const
#endif

// The following variables have been defined in the base class
// and will be used in all the string tests
/**
const int UtlStringTest::commonTestSetLength;
const char* UtlStringTest::longAlphaString;
const char* UtlStringTest::splCharString;
const BasicStringVerifier UtlStringTest::commonTestSet[];
const int UtlStringTest::commonTestSetLength ;
*/

/**  This class is used to test the UtlString class.
*
*    PLEASE READ THE README FILE THAT CAN FOUND IN THE PARENT OF THE DIRECTORY
*    OF THIS FILE. The Readme describes the organization / flow of tests and
*    without reading this file, the following class (and all unit tests)
*    may not make a lot of sense and might be difficult to comprehend.
*/
class UtlStringTest_NonMutating : public UtlStringTest
{

    CPPUNIT_TEST_SUITE(UtlStringTest_NonMutating);
    CPPUNIT_TEST(testSubstring);
    CPPUNIT_TEST(testCompareTo_exactMatch);
    CPPUNIT_TEST(testCompareTo_ignoreCase);
    CPPUNIT_TEST(testIsEquals);
    CPPUNIT_TEST(testEqualityOperator);
    CPPUNIT_TEST(testEqualityOperator2);
    CPPUNIT_TEST(testIndex_EmptyString);
    CPPUNIT_TEST(testIndex_SearchFromStart);
    CPPUNIT_TEST(testIndex_DontStartFromStart);
    CPPUNIT_TEST(testIndex_CaseInsensitiveMatch);
    CPPUNIT_TEST(testIndex_char_SearchFromStart);
    CPPUNIT_TEST(testIndex_char_DontStartFromStart);
    CPPUNIT_TEST(testIndexChar);
    CPPUNIT_TEST(testFirst_char);
    CPPUNIT_TEST(testFirst_charstar);
    CPPUNIT_TEST(testContains_charstar);
    CPPUNIT_TEST(testLast_char);
    CPPUNIT_TEST(testLength);
    CPPUNIT_TEST(testLength_NonNullTerminatedData);
    CPPUNIT_TEST(testIsNull);
    CPPUNIT_TEST(testLowerAndUpper);
    CPPUNIT_TEST(testPlusOperator);
    CPPUNIT_TEST(testIndexOperator);
    CPPUNIT_TEST(testCharAtOperator);
    CPPUNIT_TEST(testfindToken);
    CPPUNIT_TEST(testfindTokenSuffix);
    CPPUNIT_TEST_SUITE_END();

private :
    struct TestCompareDataStructure
    {
        const char* testDescription;
        const char* compareData1;
        const char* compareData2;
        int expectedValue;
    };

    struct TestCharstarIndexDataStructure
    {
        const char* testDescription;
        const char* searchString;
        int startPosition;  // Specify -1 to ignore this parameter
        ssize_t expectedValue;
    };

    struct TestCharIndexDataStructure
    {
        MEMBER_CONST char* testDescription;
        MEMBER_CONST char searchCharacter;
        int startPosition;  // Specify -1 to ignore this parameter
        ssize_t expectedValue;
    };
public:

    UtlStringTest_NonMutating()
    {
    }
    void setUp()
    {
    }

    void tearDown()
    {
    }

    ~UtlStringTest_NonMutating()
    {
    }


    /** Sandbox method for experimenting with the API Under Test.
    *   This method MUST be empty when the test drivers are being
    *   checked in (final checkin) to the repository.
    */
    void DynaTest()
    {
    }

    /*!a Test the (start, length) operator.
    */
    void testSubstring()
    {
       // Note that the start and length are size_t's, though we
       // construct some of their values by casting negative numbers.
       struct TestSubstringStructure
       {
          const char* string;
          size_t start;
          size_t length;
          const char* expectedValue;
       };

       const char* prefix = "Test the (start, len) operator, with ";
       string Message;

       TestSubstringStructure testData[] =
          {
             // String of 0 characters.
             { "", 0, 0, "" },
             { "", 0, 1, "" },
             { "", 0, 10, "" },
             { "", 0, UtlString::UTLSTRING_TO_END, "" },
             { "", 0, (size_t) (-2), "" },
             { "", 1, 0, "" },
             { "", 1, 1, "" },
             { "", 1, 10, "" },
             { "", 1, UtlString::UTLSTRING_TO_END, "" },
             { "", 1, (size_t) (-2), "" },
             { "", UtlString::UTLSTRING_TO_END, 0, "" },
             { "", UtlString::UTLSTRING_TO_END, 1, "" },
             { "", UtlString::UTLSTRING_TO_END, 10, "" },
             { "", UtlString::UTLSTRING_TO_END, UtlString::UTLSTRING_TO_END, "" },
             { "", UtlString::UTLSTRING_TO_END, (size_t) (-2), "" },
             // Test (-2) as well as UtlString::UTLSTRING_TO_END, since -1 is a special value, (-2)
             // might be handled differently.
             { "", (size_t) (-2), 0, "" },
             { "", (size_t) (-2), 1, "" },
             { "", (size_t) (-2), 10, "" },
             { "", (size_t) (-2), UtlString::UTLSTRING_TO_END, "" },
             { "", (size_t) (-2), (size_t) (-2), "" },
             // String of 1 character.
             { "a", 0, 0, "" },
             { "b", 0, 1, "b" },
             { "c", 0, 2, "" },
             { "d", 0, 10, "" },
             { "e", 0, UtlString::UTLSTRING_TO_END, "e" },
             { "f", 0, (size_t) (-2), "" },
             { "g", 1, 0, "" },
             { "h", 1, 1, "" },
             { "i", 1, 2, "" },
             { "j", 1, 10, "" },
             { "k", 1, UtlString::UTLSTRING_TO_END, "" },
             { "l", 1, (size_t) (-2), "" },
             { "g", 2, 0, "" },
             { "h", 2, 1, "" },
             { "i", 2, 2, "" },
             { "j", 2, 10, "" },
             { "k", 2, UtlString::UTLSTRING_TO_END, "" },
             { "l", 2, (size_t) (-2), "" },
             { "m", UtlString::UTLSTRING_TO_END, 0, "" },
             { "n", UtlString::UTLSTRING_TO_END, 1, "" },
             { "o", UtlString::UTLSTRING_TO_END, 2, "" },
             { "p", UtlString::UTLSTRING_TO_END, 3, "" },
             { "q", UtlString::UTLSTRING_TO_END, 10, "" },
             { "r", UtlString::UTLSTRING_TO_END, UtlString::UTLSTRING_TO_END, "" },
             { "s", UtlString::UTLSTRING_TO_END, (size_t) (-2), "" },
             { "t", (size_t) (-2), 0, "" },
             { "u", (size_t) (-2), 1, "" },
             { "v", (size_t) (-2), 2, "" },
             { "w", (size_t) (-2), 3, "" },
             { "x", (size_t) (-2), 4, "" },
             { "y", (size_t) (-2), 10, "" },
             { "z", (size_t) (-2), UtlString::UTLSTRING_TO_END, "" },
             { "A", (size_t) (-2), (size_t) (-2), "" },
             // String of 2 characters.
             { "aQ", 0, 0, "" },
             { "bW", 0, 1, "b" },
             { "cE", 0, 2, "cE" },
             { "cE", 0, 3, "" },
             { "dR", 0, 10, "" },
             { "eT", 0, UtlString::UTLSTRING_TO_END, "eT" },
             { "fY", 0, (size_t) (-2), "" },
             { "gU", 1, 0, "" },
             { "hI", 1, 1, "I" },
             { "iO", 1, 2, "" },
             { "iO", 1, 3, "" },
             { "jP", 1, 10, "" },
             { "kA", 1, UtlString::UTLSTRING_TO_END, "A" },
             { "lS", 1, (size_t) (-2), "" },
             { "gU", 2, 0, "" },
             { "hI", 2, 1, "" },
             { "iO", 2, 2, "" },
             { "jP", 2, 10, "" },
             { "kA", 2, UtlString::UTLSTRING_TO_END, "" },
             { "lS", 2, (size_t) (-2), "" },
             { "gU", 3, 0, "" },
             { "hI", 3, 1, "" },
             { "iO", 3, 2, "" },
             { "iO", 3, 3, "" },
             { "jP", 3, 10, "" },
             { "kA", 3, UtlString::UTLSTRING_TO_END, "" },
             { "lS", 3, (size_t) (-2), "" },
             { "mD", UtlString::UTLSTRING_TO_END, 0, "" },
             { "nF", UtlString::UTLSTRING_TO_END, 1, "" },
             { "oG", UtlString::UTLSTRING_TO_END, 2, "" },
             { "pH", UtlString::UTLSTRING_TO_END, 3, "" },
             { "pH", UtlString::UTLSTRING_TO_END, 4, "" },
             { "qJ", UtlString::UTLSTRING_TO_END, 10, "" },
             { "rK", UtlString::UTLSTRING_TO_END, UtlString::UTLSTRING_TO_END, "" },
             { "sL", UtlString::UTLSTRING_TO_END, (size_t) (-2), "" },
             { "tZ", (size_t) (-2), 0, "" },
             { "uX", (size_t) (-2), 1, "" },
             { "vC", (size_t) (-2), 2, "" },
             { "wV", (size_t) (-2), 3, "" },
             { "xB", (size_t) (-2), 4, "" },
             { "xB", (size_t) (-2), 5, "" },
             { "yN", (size_t) (-2), 10, "" },
             { "zM", (size_t) (-2), UtlString::UTLSTRING_TO_END, "" },
             { "Aq", (size_t) (-2), (size_t) (-2), "" },
             // String of 3 characters.
             { "aQ0", 0, 0, "" },
             { "bW1", 0, 1, "b" },
             { "cE2", 0, 2, "cE" },
             { "cE3", 0, 3, "cE3" },
             { "cE3", 0, 4, "" },
             { "dR4", 0, 10, "" },
             { "eT5", 0, UtlString::UTLSTRING_TO_END, "eT5" },
             { "fY6", 0, (size_t) (-2), "" },
             { "gU7", 1, 0, "" },
             { "hI8", 1, 1, "I" },
             { "iO9", 1, 2, "O9" },
             { "iO0", 1, 3, "" },
             { "jP1", 1, 10, "" },
             { "kA2", 1, UtlString::UTLSTRING_TO_END, "A2" },
             { "lS3", 1, (size_t) (-2), "" },
             { "gU7", 2, 0, "" },
             { "hI8", 2, 1, "8" },
             { "iO9", 2, 2, "" },
             { "iO0", 2, 3, "" },
             { "jP1", 2, 10, "" },
             { "kA2", 2, UtlString::UTLSTRING_TO_END, "2" },
             { "lS3", 2, (size_t) (-2), "" },
             { "gU7", 3, 0, "" },
             { "hI8", 3, 1, "" },
             { "iO9", 3, 2, "" },
             { "iO0", 3, 3, "" },
             { "jP1", 3, 10, "" },
             { "kA2", 3, UtlString::UTLSTRING_TO_END, "" },
             { "lS3", 3, (size_t) (-2), "" },
             { "gU7", 4, 0, "" },
             { "hI8", 4, 1, "" },
             { "iO9", 4, 2, "" },
             { "iO0", 4, 3, "" },
             { "jP1", 4, 10, "" },
             { "kA2", 4, UtlString::UTLSTRING_TO_END, "" },
             { "lS3", 4, (size_t) (-2), "" },
             { "mD4", UtlString::UTLSTRING_TO_END, 0, "" },
             { "nF5", UtlString::UTLSTRING_TO_END, 1, "" },
             { "oG6", UtlString::UTLSTRING_TO_END, 2, "" },
             { "pH7", UtlString::UTLSTRING_TO_END, 3, "" },
             { "pH8", UtlString::UTLSTRING_TO_END, 4, "" },
             { "pH8", UtlString::UTLSTRING_TO_END, 5, "" },
             { "qJ9", UtlString::UTLSTRING_TO_END, 10, "" },
             { "rK0", UtlString::UTLSTRING_TO_END, UtlString::UTLSTRING_TO_END, "" },
             { "sL1", UtlString::UTLSTRING_TO_END, (size_t) (-2), "" },
             { "tZ2", (size_t) (-2), 0, "" },
             { "uX3", (size_t) (-2), 1, "" },
             { "vC4", (size_t) (-2), 2, "" },
             { "wV5", (size_t) (-2), 3, "" },
             { "xB6", (size_t) (-2), 4, "" },
             { "xB7", (size_t) (-2), 5, "" },
             { "xB7", (size_t) (-2), 6, "" },
             { "yN8", (size_t) (-2), 10, "" },
             { "zM9", (size_t) (-2), UtlString::UTLSTRING_TO_END, "" },
             { "Aq0", (size_t) (-2), (size_t) (-2), "" },
             // String of 10 characters.
             { "pinesitaQ0", 0, 0, "" },
             { "nssae <bW1", 0, 1, "n" },
             { "* C/*e cE2", 0, 2, "* " },
             { "saCeon*cE3", 0, 3, "saC" },
             { " arsi} cE3", 0, 4, " ars" },
             { " arsi} cE3", 0, 9, " arsi} cE" },
             { "/ oCmp/dR4", 0, 10, "/ oCmp/dR4" },
             { "/ oCmp/dR4", 0, 11, "" },
             { "/ =====eT5", 0, UtlString::UTLSTRING_TO_END, "/ =====eT5" },
             { "arCe;esfY6", 0, (size_t) (-2), "" },
             { "a /**aCgU7", 1, 0, "" },
             { " <hCtcshI8", 1, 1, "<" },
             { "ae,{ amiO9", 1, 2, "e," },
             { "sae arCiO0", 1, 3, "ae " },
             { "sae arCiO0", 1, 8, "ae arCiO" },
             { "sae arCiO0", 1, 9, "ae arCiO0" },
             { "eoCmpenjP1", 1, 10, "" },
             { "ises svkA2", 1, UtlString::UTLSTRING_TO_END, "ses svkA2" },
             { "eti cmolS3", 1, (size_t) (-2), "" },
             { "a /**aCgU7", 5, 0, "" },
             { " <hCtcshI8", 5, 1, "c" },
             { "ae,{ amiO9", 5, 2, "am" },
             { "sae arCiO0", 5, 3, "rCi" },
             { "sae arCiO0", 5, 4, "rCiO" },
             { "sae arCiO0", 5, 5, "rCiO0" },
             { "sae arCiO0", 5, 6, "" },
             { "sae arCiO0", 5, 9, "" },
             { "eoCmpenjP1", 5, 10, "" },
             { "ises svkA2", 5, UtlString::UTLSTRING_TO_END, "svkA2" },
             { "a /**aCgU7", 8, 0, "" },
             { " <hCtcshI8", 8, 1, "I" },
             { "ae,{ amiO9", 8, 2, "O9" },
             { "sae arCiO0", 8, 3, "" },
             { "ises svkA2", 8, UtlString::UTLSTRING_TO_END, "A2" },
             { "eti cmolS3", 8, (size_t) (-2), "" },
             { "a /**aCgU7", 9, 0, "" },
             { " <hCtcshI8", 9, 1, "8" },
             { "ae,{ amiO9", 9, 2, "" },
             { "sae arCiO0", 9, 3, "" },
             { "ises svkA2", 9, UtlString::UTLSTRING_TO_END, "2" },
             { "eti cmolS3", 9, (size_t) (-2), "" },
             { "a /**aCgU7", 10, 0, "" },
             { " <hCtcshI8", 10, 1, "" },
             { "ae,{ amiO9", 10, 2, "" },
             { "sae arCiO0", 10, 3, "" },
             { "ises svkA2", 10, UtlString::UTLSTRING_TO_END, "" },
             { "eti cmolS3", 10, (size_t) (-2), "" },
             { "a /**aCgU7", 11, 0, "" },
             { " <hCtcshI8", 11, 1, "" },
             { "ae,{ amiO9", 11, 2, "" },
             { "ises svkA2", 11, UtlString::UTLSTRING_TO_END, "" },
             { "eti cmolS3", 11, (size_t) (-2), "" },
             { "orgn i*mD4", UtlString::UTLSTRING_TO_END, 0, "" },
             { "/ nosrinF5", UtlString::UTLSTRING_TO_END, 1, "" },
             { "patanstoG6", UtlString::UTLSTRING_TO_END, 2, "" },
             { "ns tnropH7", UtlString::UTLSTRING_TO_END, 3, "" },
             { "oc l*/tpH8", UtlString::UTLSTRING_TO_END, 4, "" },
             { " edypm pH8", UtlString::UTLSTRING_TO_END, 5, "" },
             { "nu eef;qJ9", UtlString::UTLSTRING_TO_END, 10, "" },
             { "e /TpyprK0", UtlString::UTLSTRING_TO_END, UtlString::UTLSTRING_TO_END, "" },
             { "* ** CssL1", UtlString::UTLSTRING_TO_END, (size_t) (-2), "" },
             { "aitivy tZ2", (size_t) (-2), 0, "" },
             { "oce esiuX3", (size_t) (-2), 1, "" },
             { "tnsristvC4", (size_t) (-2), 2, "" },
             { "ng* riSwV5", (size_t) (-2), 3, "" },
             { "t} / otxB6", (size_t) (-2), 4, "" },
             { " hdsenhxB7", (size_t) (-2), 5, "" },
             { "te  o fxB7", (size_t) (-2), 6, "" },
             { "b morf yN8", (size_t) (-2), 10, "" },
             { "prist <zM9", (size_t) (-2), UtlString::UTLSTRING_TO_END, "" },
             { "ng* cutAq0", (size_t) (-2), (size_t) (-2), "" },
             // A special test to ensure that it isn't relying on
             // "start + len <= size" to determine validity -- that addition
             // can overflow.
             { "abcdefghijkl",
               // Construct start == len == MAXINT/2 + 1.
               // Then start + len == 2!
               ~(~((size_t) 0) >> 1) + 1,
               ~(~((size_t) 0) >> 1) + 1,
               "" },
          };

       // Go through the table of tests.
       for (unsigned i=0; i < sizeof(testData)/sizeof(testData[0]); i++)
       {
          // Get the test string as a UtlString.
          UtlString testString(testData[i].string);
          // Get the expected value as a UtlString.
          UtlString expectedValue(testData[i].expectedValue);
          // Assemble the test description.
          char description[80];
          sprintf(description, "\"%s\"(%zu, %zu)", testData[i].string,
                  testData[i].start, testData[i].length);

          // Execute the (start, length) operator.
          UtlString actualValue = testString((size_t) (testData[i].start),
                                             (size_t) (testData[i].length));

          TestUtilities::createMessage(2, &Message, prefix,
                                       description);
          CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(),
                                       expectedValue,
                                       actualValue);
       }
    }//testSubstring

    /** testCase for comparing the compareTo(char* ) / compareTo(UtlString) method
    *    This test case checks the compareTo() method for a caseSensitive comparision
    *
    *    The test data for this test case is :-
    *      a)  "lhs is empty. rhs is not",
    *      b)  "rhs is empty. lhs is not",
    *      c)  "lhs = rhs",
    *      d)  "lhs =~ rhs. The first character of lhs is lower case",
    *      e)  "lhs =~ rhs. The first character of lhs is upper case",
    *      f)  "lhs =~ rhs. The last character of lhs is lower case",
    *      g)  "lhs =~ rhs. The last character of lhs is upper case",
    *      h)  "lhs != rhs. The first character of the lhs is alphabetically first",
    *      i)  "lhs != rhs. The first character of the lhs is alphabetically second",
    *      j)  "lhs != rhs. After initial similarity in characters, the character(s)
                in the lhs are before those in the rhs" ,
    *      k)  "lhs != rhs. After initial similarity in characters, the character(s)
                in the lhs are after those in the rhs",
    *      l)  "lhs != rhs. Text is alphanumeric. After initial similarity,
                the digit(s) in the lhs are before that in the rhs",
    *      m)  "lhs != rhs. Text is alphanumeric. After initial similarity, the digit(s)
                in the lhs are after that in the rhs",
    *      n)  "lhs != rhs. The text has special characters"
    *
    *      Each of the above cases is verified for
    *      1) compareTo(char*)
    *      2) compareTo(char*, caseMatch=exactMatch)
    *      3) compareTo(UtlString)
    *      4) compareTo(UtlString, caseMatch=exactMatch)
    */
    void testCompareTo_exactMatch()
    {
        const char* prefix = "Test the compareTo method when ";
        /*
        Composition of TestCompareDataStructure:
            testDescription, compareData1, compareData2, expectedValue;
        */
        const TestCompareDataStructure testData[] = { \
          { "lhs is empty. rhs is not", "", "test string", -1 },\
          { "rhs is empty. lhs is not", "test string", "", 1 },\
          { "rhs is empty. lhs is empty", "", "", 0 }, \
          { "lhs == rhs", "test string", "test string", 0 },\
          { "lhs =~ rhs. The first character of lhs is lower case", \
                   "Test string", "test string", -1 },\
          { "lhs =~ rhs. The first character of lhs is upper case", \
                   "test string", "Test string", 1 },\
          { "lhs =~ rhs. The last character of lhs is lower case", \
                   "test strinG", "test string", -1 }, \
          { "lhs =~ rhs. The last character of lhs is upper case", \
                   "test string", "test strinG", 1 },\
          { "lhs != rhs. The first character of the lhs is alphabetically first", \
                   "Hey dude", "No way", -1 },\
          { "lhs != rhs. The first character of the lhs is alphabetically second", \
                   "Why is this", "May be not", 1 }, \
          { "lhs != rhs. After initial similarity in characters, the character(s)" \
                   " in the lhs are before those in the rhs", \
                   "This is good", "This is sick", -1 },\
          { "lhs != rhs. After initial similarity in characters, the character(s) in the lhs " \
                 "are after those in the rhs", \
                   "Are we right?", "Are we not?", 1 },  \
          { "lhs != rhs. Text is alphanumeric. After initial similarity, the digit(s) in the lhs " \
                 "are before that in the rhs", \
                 "Number891", "Number93", -1 },\
          { "lhs != rhs. Text is alphanumeric. After initial similarity, the digit(s) in the lhs " \
                 "are after that in the rhs", \
                 "Number765", "Number1234", 1 },\
          { "lhs != rhs. The text has special characters", \
                 "T#ter", "T&ter", -1 }, // lhs < rhs
          { "lhs == rhs. The text has special characters", \
                 "TÜter", "TÜter", 0 } \
        };
       const int testCount = sizeof(testData)/sizeof(testData[0]);

       // Test the compare to method when the matchCase is not specified
       // and is assumed as default, The parameter passed is a char*
       utlTestCompareTo(prefix, testData, testCount, false, \
                        TYPE_CHARSTAR, UtlString::matchCase);

       // Test the compare to method when the matchCase is specified explicitly
       // The parameter passed is a char*
       utlTestCompareTo(prefix, testData, testCount, true, \
                        TYPE_CHARSTAR, UtlString::matchCase);

       // Test the compare to method when the matchCase is specified explicitly
       // The parameter passed is a UtlString
       utlTestCompareTo(prefix, testData, testCount, true, \
                        TYPE_UTLSTRING, UtlString::matchCase);
    }


    /**  TestCase for comparing the compareTo(char*, CompareCase=ignoreCase) / compareTo
         (UtlString) This test case checks the compareTo() methods for a
         caseInsensitive comparision
    *
    *    The test data for this test case is :-
    *        a) "lhs is empty. rhs is not",
    *        b) "rhs is empty. lhs is not",
    *        c) "lhs == rhs",
    *        d) "lhs =~ rhs. All the characters in the two strings have their case flipped",
    *        e) "lhs =~ rhs. The characters are such that some have the same case,
                 while others have 'em flipped",
    *        f) "lhs != rhs. The first non-matching character is alphabetically first in the lhs
                 (case doesnt matter)",
    *        g) "lhs != rhs. The first non-matching character is alphabetically first in the rhs
                (case doesn't matter)",
    *        h) "lhs != rhs. The strings are alpha-numeric",
    *        i) "lhs != rhs. The strings have special characters",
    *        j) "lhs == rhs. The strings have special characters"
    *
    */
    void testCompareTo_ignoreCase()
    {
        const char* prefix = "Test the compareTo method when ";
        /*
        Composition of TestCompareDataStructure:
            testDescription, compareData1, compareData2, expectedValue;
        */

        TestCompareDataStructure testData[] = { \
            { "lhs is empty. rhs is not", "", "test string", -1}, \
            { "rhs is empty. lhs is not", "test string", "", 1}, \
            { "lhs == rhs", "test string", "test string", 0}, \
            { "lhs =~ rhs. All the characters in the two strings have their case flipped", \
                   "TesT sTriNg", "tESt StRInG", 0}, \
            { "lhs =~ rhs. The characters are such that some have the same case, while " \
                   "others have 'em flipped", "Test String", "TesT STring", 0}, \
            { "lhs != rhs. The first non-matching character would make the lhs to appear " \
                     "before (case doesnt matter)", "Test String", "Test ttring", -1}, \
            { "lhs != rhs. The first non-matching character would make the lhs to appear later " \
                     "(case doesn't matter)", "Test String", "Test bring", 1}, \
            { "lhs != rhs. The strings are alpha-numeric", "Test234", "tEst1234", 1}, \
            { "lhs != rhs. The strings have special characters", "Test^#", "test#^", 1}, \
            { "lhs == rhs. The strings have special characters", "Test^#", "test^#", 0} \
        };
        const int testCount = sizeof(testData)/sizeof(testData[0]) ;
        utlTestCompareTo(prefix, testData, testCount, true, TYPE_CHARSTAR, \
                         UtlString::ignoreCase);

        utlTestCompareTo(prefix, testData, testCount, true, TYPE_UTLSTRING, \
                         UtlString::ignoreCase);
    }

    /**
    *  utility used for testing the compareTo method. The type of test is controlled by the last
    *  (optional) parameters. If specifyEnum is set to false, then the compareTo() function is
    *  called without the CompareCase parameter (amounts to string.compareTo(char*) or
    *  string.compareTo(string)). If set to true, then the method is called with the
    *  argument(amoutns to string.compareTo(char*, enum) or string.compareTo(string, enum).

    *  If the type parameter is set to 0, then the compareTo(char* ,..) is called. If set to 1,
    *  then compareTo(UtlString, ..)  is called
     */
    void utlTestCompareTo(const char* prefix, const TestCompareDataStructure testData[], \
                                const int testCount, bool specifyEnum, StringType type, \
                                UtlString::CompareCase caseCompare)
    {
        const char* suffix;
        string Message;
        if (type == TYPE_CHARSTAR )
        {
            if (specifyEnum)
            {
                if (caseCompare == UtlString::ignoreCase)
                {
                    suffix = " :- compareTo(char*, CompareCase=ignore) ";
                }
                else
                {
                    suffix = " :- compareTo(char*, CompareCase=exact) ";
                }
            }
            else
            {
                suffix = " :- compareTo(char*)";
            }
        }
        // else type is to compareTo UtlString
        else
        {
            if (specifyEnum)
            {
                if (caseCompare == UtlString::ignoreCase)
                {
                    suffix = " :- compareTo(UtlString, CompareCase=ignore) ";
                }
                else
                {
                    suffix = " :- compareTo(UtlString, CompareCase=exact) ";
                }
            }
            else
            {
                suffix = " :- compareTo(UtlString) " ;
            }
        }

        for ( int i = 0; i < testCount; i++)
        {
            UtlString compareString1(testData[i].compareData1);
            UtlString compareString2(testData[i].compareData2);
            int cmpResult;
            if (type == TYPE_CHARSTAR)
            {
                if (specifyEnum)
                {
                    cmpResult = compareString1.compareTo(testData[i].compareData2, caseCompare);
                }
                else
                {
                    cmpResult = compareString1.compareTo(testData[i].compareData2);
                }
            }
            else
            {
                cmpResult = compareString1.compareTo(&compareString2, caseCompare);
            }

            // When string 'a' comes before string 'b', we are not concerned about
            // what is the integer returned when you call a.compareTo(b). We only want
           //  to check if it returns any negative number and so on.
            bool testResult = false;
            if (testData[i].expectedValue == 0)
            {
                testResult = cmpResult == 0;
            }
            else if (testData[i].expectedValue == -1)
            {
                testResult = cmpResult < 0;
            }
            else
            {
                testResult = cmpResult > 0;
            }

            TestUtilities::createMessage(3, &Message, prefix, testData[i].testDescription, \
                                         suffix);
            CPPUNIT_ASSERT_MESSAGE(Message.data(), testResult);
        }
    } //utlTestCompareTo


    /*! Test the isEquals method.
    *
    *     The test data for this test is :-
    *      a) Test two strings that*ARE* equal
    *      b) Test two strings that would have been equal if the search were case sensitive.
    *      c) Test two strings that are not equal.
    *      d) Check an empty string with a non empty string
    *      e) Check an empty string with an empty String
    *      6) Check a String with an Integer
    */
    void testIsEquals()
    {
        const char* prefix = "Test the isEquals method for a string such that ";
        string Message;

        //Since the equals / compareTo methods are almost similar, use
        // the same data structure
        /*
        Composition of TestCompareDataStructure:
            testDescription, compareData1, compareData2, expectedValue;
        */
        const TestCompareDataStructure testData[] = { \
               { "the two strings *ARE* equal" , \
                      "Hello World", "Hello World", 1}, \
               { "the two strings match for a case-insensitive search", \
                      "Hello World", "hello world", 0}, \
               { "the two strings are not equal", \
                      "Hello World", "Different Universe", 0}, \
               { "lhs is an empty string and rhs is not" , \
                      "", "Hello World", 0}, \
               { "both the strings are empty strings" ,
                      "", "", 1 }, \
        };

        const int testCount = sizeof(testData)/sizeof(testData[0]);
        for(int i = 0; i < testCount; i++)
        {
            UtlString testString(testData[i].compareData1);
            UtlString compareString(testData[i].compareData2);

            bool actual = (TRUE == testString.isEqual(&compareString));
            bool expected = (testData[i].expectedValue == 1);

            TestUtilities::createMessage(2, &Message, prefix, testData[i].testDescription);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), expected, actual);
        }

        // Now check the case where a different type of collectable is compared to
        // a string
        UtlString testString("123");
        UtlInt testInt(123);
        bool actual = (TRUE == testString.isEqual(&testInt));
        TestUtilities::createMessage(2, &Message, prefix, "the target is not a UtlString");
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), false, actual);

    } // testIsEqual();


    /**
     * Test [not]equality operator on strings, left-hand and
     * right-hand variations.
     */
    void testEqualityOperator()
    {
        struct EqualityOperatorTestData
        {
            MEMBER_CONST char rhs[128];
            MEMBER_CONST char lhs[128];
            bool isequal;
        };

        const EqualityOperatorTestData data[] = {
            {"",        "",        true},
            {"test",    "test",    true},
            {"test",    "tEst",    false},
            {"good",    "bad",     false},
            {"abcd",    "abcf",    false}
        };


        char msg[1024];
        UtlString utlstr;
        int n = sizeof(data) / sizeof(data[0]);
        for (int i = 0; i < n; i++)
        {
            utlstr = data[i].rhs;
            sprintf(msg, "(UtlString)\"%s\" == (char *)\"%s\"", data[i].rhs, data[i].lhs);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, (int)data[i].isequal,
                    (int)(utlstr == data[i].lhs));

            sprintf(msg, "(UtlString)\"%s\" != (char *)\"%s\"", data[i].rhs, data[i].lhs);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, (int)!data[i].isequal,
                    (int)(utlstr != data[i].lhs));

            utlstr = data[i].lhs;
            sprintf(msg, "(char *)\"%s\" == (UtlString)\"%s\"", data[i].rhs, data[i].lhs);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, (int)data[i].isequal,
                    (int)(data[i].rhs == utlstr));

            sprintf(msg, "(char *)\"%s\" != (UtlString)\"%s\"", data[i].rhs, data[i].lhs);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, (int)!data[i].isequal,
                    (int)(data[i].rhs != utlstr));
        }
    }


    /**
     * Test == and != on a series of cases with a variety of type combinations.
     */
    void testEqualityOperator2()
    {
        // Construct one-character test strings.
        const char* s_a1 = "a";
        const char* s_a2 = "a";
        const char* s_b = "b";

        // Construct UtlStrings.
        const UtlString u_a1 = "a";
        const UtlString u_a2 = "a";
        const UtlString u_b = "b";

        // Compare (const UtlString) to (const UtlString).
        CPPUNIT_ASSERT_MESSAGE("u_a1 == u_a1", (u_a1 == u_a1) == 1);
        CPPUNIT_ASSERT_MESSAGE("u_a1 == u_a2", (u_a1 == u_a2) == 1);
        CPPUNIT_ASSERT_MESSAGE("u_a1 == u_b", (u_a1 == u_b) == 0);
        CPPUNIT_ASSERT_MESSAGE("u_a1 != u_a1", (u_a1 != u_a1) == 0);
        CPPUNIT_ASSERT_MESSAGE("u_a1 != u_a2", (u_a1 != u_a2) == 0);
        CPPUNIT_ASSERT_MESSAGE("u_a1 != u_b", (u_a1 != u_b) == 1);

        // Compare (const UtlString) to (const char*).
        CPPUNIT_ASSERT_MESSAGE("u_a1 == s_a1", (u_a1 == s_a1) == 1);
        CPPUNIT_ASSERT_MESSAGE("u_a1 == s_a2", (u_a1 == s_a2) == 1);
        CPPUNIT_ASSERT_MESSAGE("u_a1 == s_b", (u_a1 == s_b) == 0);
        CPPUNIT_ASSERT_MESSAGE("u_a1 != s_a1", (u_a1 != s_a1) == 0);
        CPPUNIT_ASSERT_MESSAGE("u_a1 != s_a2", (u_a1 != s_a2) == 0);
        CPPUNIT_ASSERT_MESSAGE("u_a1 != s_b", (u_a1 != s_b) == 1);

        // Compare (const char*) to (const UtlString).
        CPPUNIT_ASSERT_MESSAGE("s_a1 == u_a1", (s_a1 == u_a1) == 1);
        CPPUNIT_ASSERT_MESSAGE("s_a1 == u_a2", (s_a1 == u_a2) == 1);
        CPPUNIT_ASSERT_MESSAGE("s_a1 == u_b", (s_a1 == u_b) == 0);
        CPPUNIT_ASSERT_MESSAGE("s_a1 != u_a1", (s_a1 != u_a1) == 0);
        CPPUNIT_ASSERT_MESSAGE("s_a1 != u_a2", (s_a1 != u_a2) == 0);
        CPPUNIT_ASSERT_MESSAGE("s_a1 != u_b", (s_a1 != u_b) == 1);

        // Compare (const UtlString) to (const char).
        // These operators do not exist.

        // Compare (const char) to (const UtlString).
        CPPUNIT_ASSERT_MESSAGE("u_a1[0] == u_a1", (u_a1[0] == u_a1) == 1);
        CPPUNIT_ASSERT_MESSAGE("u_a1[0] == u_a2", (u_a1[0] == u_a2) == 1);
        CPPUNIT_ASSERT_MESSAGE("u_a1[0] == u_b", (u_a1[0] == u_b) == 0);
        CPPUNIT_ASSERT_MESSAGE("u_a1[0] != u_a1", (u_a1[0] != u_a1) == 0);
        CPPUNIT_ASSERT_MESSAGE("u_a1[0] != u_a2", (u_a1[0] != u_a2) == 0);
        CPPUNIT_ASSERT_MESSAGE("u_a1[0] != u_b", (u_a1[0] != u_b) == 1);

        // Compare (const UtlString*) to (const UtlString*).
        // This is a pointer comparison, not a string comparison.
        CPPUNIT_ASSERT_MESSAGE("&u_a1 == &u_a1", (&u_a1 == &u_a1) == 1);
        CPPUNIT_ASSERT_MESSAGE("&u_a1 == &u_a2", (&u_a1 == &u_a2) == 0);
        CPPUNIT_ASSERT_MESSAGE("&u_a1 == &u_b", (&u_a1 == &u_b) == 0);
        CPPUNIT_ASSERT_MESSAGE("&u_a1 != &u_a1", (&u_a1 != &u_a1) == 0);
        CPPUNIT_ASSERT_MESSAGE("&u_a1 != &u_a2", (&u_a1 != &u_a2) == 1);
        CPPUNIT_ASSERT_MESSAGE("&u_a1 != &u_b", (&u_a1 != &u_b) == 1);
    }


    /*a! Test case for comparing the index(char*, ...)  methods for an empty string
    *
    *    The test data for this test case is :-
    *        1. When the String is empty
    *            a) Test trying to find another empty string
    *            b) Test trying to find a non empty string
    *            // Verify with bob if we are hanlding the next case.
    *            c) Test trying to find, starting off from a non-zero position, for a
    *               non empty string
    *            For (a) to (b) verify by both specifying position explicitly
    *            as 0 / not providing at all
    */
    void testIndex_EmptyString()
    {
        const char* prefix = "Test index method for an empty string. Find ";
        string Message;

        const TestCharstarIndexDataStructure testData[] = { \
               { "Empty char* string", "", -1, (ssize_t)0 }, \
               { "Non empty char* string", "Some", -1, INDEX_NOT_FOUND} \
        };

        UtlString testString("");
        const int testCount = sizeof(testData) / sizeof(testData[0]);
        for (int i =0; i <testCount; i++)
        {
            ssize_t result = testString.index(testData[i].searchString);

            TestUtilities::createMessage(2, &Message, prefix, testData[i].testDescription);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), testData[i].expectedValue, result);
        }
    }


    /*a! Test case for comparing the index(char*, ...)  methods
    *
    *    The test data for this test case is :-
    *        2) With the string with data (The data is "Teststring test and test" )
    *            Test a case sensitive search for
    *            a) Find the index of  "Test" - starting characters
    *            b) Find the index of "test" - only the text with the correct case is returned
    *            c) Find the index of "s" -character as a string that matches
    *            d) Find the index of "string" - a string that only matches once
    *            e) Find the index of "bad"  - a string that is not prefix
    *               sent
    *               For (a) to (e) verify by both specifying position explicitly
    *               as 0 / not providing at all
    */
    void testIndex_SearchFromStart()
    {
        utlTestIndexFirstOrContains_charstar(TEST_INDEX);
    }

   /*a! Some simple tests to ensure that UtlString::index(char, size_t)
    *   searches correctly.  (This is to ensure that my amendment of its
    *   documentation is correct.)
    */
    void testIndexChar()
    {
       UtlString s1 = "";
       CPPUNIT_ASSERT_EQUAL((ssize_t)UTL_NOT_FOUND, s1.index('.', 0));

       UtlString s2 = ".";
       CPPUNIT_ASSERT_EQUAL((ssize_t) 0, s2.index('.', 0));
       CPPUNIT_ASSERT_EQUAL((ssize_t)UTL_NOT_FOUND, s2.index('.', 1));
       CPPUNIT_ASSERT_EQUAL((ssize_t)UTL_NOT_FOUND, s2.index('x', 0));
       CPPUNIT_ASSERT_EQUAL((ssize_t)UTL_NOT_FOUND, s2.index('x', 1));

       UtlString s3 = ".x.";
       CPPUNIT_ASSERT_EQUAL((ssize_t) 0, s3.index('.', 0));
       CPPUNIT_ASSERT_EQUAL((ssize_t) 2, s3.index('.', 1));
       CPPUNIT_ASSERT_EQUAL((ssize_t) 2, s3.index('.', 2));
       CPPUNIT_ASSERT_EQUAL((ssize_t)UTL_NOT_FOUND, s3.index('.', 3));
       CPPUNIT_ASSERT_EQUAL((ssize_t) 1, s3.index('x', 0));
       CPPUNIT_ASSERT_EQUAL((ssize_t) 1, s3.index('x', 1));
       CPPUNIT_ASSERT_EQUAL((ssize_t)UTL_NOT_FOUND, s3.index('x', 2));
       CPPUNIT_ASSERT_EQUAL((ssize_t)UTL_NOT_FOUND, s3.index('x', 3));

       UtlString s4 = "...";
       CPPUNIT_ASSERT_EQUAL((ssize_t) 0, s4.index('.', 0));
       CPPUNIT_ASSERT_EQUAL((ssize_t) 1, s4.index('.', 1));
       CPPUNIT_ASSERT_EQUAL((ssize_t) 2, s4.index('.', 2));
       CPPUNIT_ASSERT_EQUAL((ssize_t)UTL_NOT_FOUND, s4.index('.', 3));
       CPPUNIT_ASSERT_EQUAL((ssize_t)UTL_NOT_FOUND, s4.index('x', 0));
       CPPUNIT_ASSERT_EQUAL((ssize_t)UTL_NOT_FOUND, s4.index('x', 1));
       CPPUNIT_ASSERT_EQUAL((ssize_t)UTL_NOT_FOUND, s4.index('x', 2));
       CPPUNIT_ASSERT_EQUAL((ssize_t)UTL_NOT_FOUND, s4.index('x', 3));
    }

    /*a! Test case for the first(char*) method.  This method is
    *    exactly similar to the index(char*) method and so is the test data
    *    Refer to the testIndex_charstar_SearchFromStart() method for details.
    */
    void testFirst_charstar()
    {
        utlTestIndexFirstOrContains_charstar(TEST_FIRST);
    }

    /*a! Test case for comparing the contains(char*)  methods
    *
    *    The test data for this test case is :-
    *        2) With the string with data (The data is "Teststring test and test" )
    *            Test a case sensitive search for
    *            a) Find the index of  "Test" - starting characters
    *            b) Find the index of "test" - only the text with the correct case is returned
    *            c) Find the index of "s" -character as a string that matches
    *            d) Find the index of "string" - a string that only matches once
    *            e) Find the index of "bad"  - a string that is not prefixsent
    *               For (a) to (e) verify by both specifying position explicitly
    *               as 0 / not providing at all
    */
    void testContains_charstar()
    {
        utlTestIndexFirstOrContains_charstar(TEST_CONTAINS);
    }


   /** Utility to test the index, first or contains method that accepts a char*
    *   argument.
    */
    void utlTestIndexFirstOrContains_charstar(IndexFirstOrContainsType type)
    {
        const char* prefix = "";
        string Message;
        if (type == TEST_INDEX)
        {
            prefix = "Test index starting search from the first location. Find the index of" ;
        }
        else if (type == TEST_CONTAINS)
        {
            prefix = "Test contains(char*) for";
        }
        else if (type == TEST_FIRST)
        {
            prefix = "Test first(char*) for";
        }

        const TestCharstarIndexDataStructure testData[] = { \
           { "a multi-character char* string" , "Test", -1, 0 }, \
           { "a string if the search were case-insensitive", "test", -1, 11 },  \
           { "a single-character char* string", "s", -1, 2 }, \
           { "a string that only matches once", "string", -1, 4 },  \
           { "a string that doesn't match", "bzk", -1, INDEX_NOT_FOUND } \
        };

        const int testCount = sizeof(testData)/sizeof(testData[0]);
        UtlString testString("Teststring test and test");

        bool specifyPosition = false;

        int loopCount;
        // if the type is to test index(), then we need to test both the
        // cases :- a) position is explicitly specified as 0
        // b) position is not specified at all.
        // However the first() method takes only one argument and we dont
        // need to worry about the position.
        loopCount = type == TEST_INDEX ? 2 : 1;
        for (int i =0;  i < loopCount; i++)
        {
            const char* suffix = "";
            if (specifyPosition)
            {
                suffix = ":- index(char*, 0)";
            }
            else
            {
               if (type == TEST_INDEX)
               {
                   suffix = ":- index(char*)";
               }
               else if(type == TEST_CONTAINS)
               {
                   suffix = ":- contains()";
               }
               else if(type == TEST_FIRST)
               {
                   suffix = ":- first()";
               }
            }

            ssize_t result;
            for(int j =0; j < testCount; j++)
            {
                TestUtilities::createMessage(3, &Message, prefix, \
                       testData[j].testDescription, suffix);
                if (specifyPosition)
                {
                    result = testString.index(testData[j].searchString, 0);
                    CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                                              testData[j].expectedValue, result);
                }
                else
                {
                    if (type == TEST_INDEX)
                    {
                        result = testString.index(testData[j].searchString);
                        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                                          testData[j].expectedValue, result);
                    }
                    else if (type == TEST_FIRST)
                    {
                        result = testString.first(testData[j].searchString);
                        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                                    testData[j].expectedValue, result);
                    }
                    else if (type == TEST_CONTAINS)
                    {
                        bool isContained = (TRUE == testString.contains(testData[j].searchString));
                        bool wasExpectedToContain = (testData[j].expectedValue \
                                                    != INDEX_NOT_FOUND);
                        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                                          wasExpectedToContain, isContained);
                    }
                }

            } //for int j=0 to
            // Set the stage ready for the next test where the
            // position is explicitly specified as 0.
            specifyPosition = !specifyPosition;
        }
    } //utlTestIndexFirstOrContains_charstar


    /*a! Test case for comparing the index(char*, pos, ...)  methods
    *
    *    The test data for this test case is :- ((The main string is "Teststring test and test" )
    *        1. Find the index, starting off from a position other than the start`
    *            a) Find the index of "test" starting from position greater than the first index
    *            b) Find the index of "string" starting from the exact position of match
    *            c) Find the index of "string" starting of from position greater than match index
    *            d) Find the index of "bad"  - a string that is not prefixsent
    *            // Verify with bob if we are hanlding the next case.
    *            e) Find the index of "string" start from aa position greater than the length
    */
    void testIndex_DontStartFromStart()
    {
        const char* prefix = "Test index starting search from a different (not start)location. "\
                       "Find the index of a " ;
        string Message;

        const TestCharstarIndexDataStructure testData[] = { \
           { "string that matches twice, such that the position specified " \
                "is after the first", "test", 12, 20 }, \
           { "string that matches, exactly at the position specified", \
                "string",  4, 4 }, \
           { "string  that is found, but before the specified position", \
                "string", 5, INDEX_NOT_FOUND}, \
           { "string that doesn't match", "bad", 2, INDEX_NOT_FOUND }, \
           { "string that matches, exactly at the position specified and " \
                "is the last position", "test", 11, 11 } \
        };

        const int testCount = sizeof(testData)  / sizeof(testData[0]);
        UtlString testString("Teststring test and test");
        // Test both the case where the caseType is specified
        // and is not.
        bool specifyCase = false;
        for (int i =0;  i < 2; i++)
        {
            const char* suffix;
            if (specifyCase)
            {
                suffix = ":- index(char*, pos, CaseType)";
            }
            else
            {
               suffix = ":- index(char*, pos)";
            }
            ssize_t result;
            for(int j =0; j < testCount; j++)
            {
                if (specifyCase)
                {
                    result = testString.index(testData[j].searchString, \
                             testData[j].startPosition, UtlString::matchCase);
                }
                else
                {
                    result = testString.index(testData[j].searchString, \
                             testData[j].startPosition);
                }

                TestUtilities::createMessage(3, &Message, prefix, \
                                     testData[j].testDescription, suffix);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), testData[j].expectedValue, result);
            }
            specifyCase = !specifyCase;
        }
    }


    /**  Test case for comparing the index(char*, ...)  methods using a case-insensitive match
    *
    *    The test data for this test case is :- ((The main string is "Teststring test and
    *    test an#232 end" )
    *        For all the tests, the CompareCase argument must be set to ignoreCase
    *        1. Find the index of an empty string
    *        2. Find the index of a char* string that matches including the case
    *        3. Find the index of a char* string that matches both 'case insensitive
    *           and case sensitive'.
    *        4. Find the index of a char* string that matches 'case-insenistive' and once only
    *        5. Find the index of a alpha-numeric char* string that matches in its numeric position
    */
    void testIndex_CaseInsensitiveMatch()
    {
        const char* prefix = "Test the index(, , CompareCase=ignoreCase) method for";
        string Message;

        const TestCharstarIndexDataStructure testData[] = { \
              { "an Empty string", "", 0, 0 }, \
              { "a char* string that is an exact match", "string", 0, 4}, \
              { "a char* string that matches both case-sensitive "
                  "and case-insensitive", "test", 0, 0 }, \
              { "a char* string that matches case-insensitive "
                  "and once only", "AnD", 0, 16}, \
              { "an alphanumeric char* string that matches", "an#232", 0, 25}, \
              { "a char* string that matches case-insensitive after position " \
                  "n. Specify position", "and", 4, 16}, \
              { "a char* string that doesn't match", "bad", 0, INDEX_NOT_FOUND } \
        };

        UtlString testString("Teststring test and test an#232 end");
        ssize_t result;
        const int testCount = sizeof(testData) / sizeof(testData[0]) ;
        for (int i =0; i < testCount; i++)
        {
            result = testString.index(testData[i].searchString, \
                     testData[i].startPosition, UtlString::ignoreCase);

            TestUtilities::createMessage(2, &Message, prefix, testData[i].testDescription);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), testData[i].expectedValue, result);
        }
    } //testIndex_CaseInsensitiveMatch


    /*a! Test case for comparing the index(char, pos, .)  methods for an empty string
    *
    *    The test data for this test case is :- ((The main string is "wednesDay")
    *        1. Find the index, starting off from a position other than start`
    *            a) Find the index of 'e' staring from position greater than the first index
    *            b) Find the index of 'n' exactly at the position where it matches
    *            b) Find the index of 'n' starting of from position greater than match index
    *            c) Find the index of 'z'  - a char that is not prefixsent
    *            // Verify with bob if we are hanlding the next case.
    *            i) Find the index of 'n' start from aa position greater than the length
    */
    void testIndex_char_DontStartFromStart()
    {
        const char* prefix = "Test index starting search from a different " \
                       "(not start)location. Find the index of a " ;
        const char* suffix = ":- index(char, pos)";
        string Message;

        UtlString ts("wednesDay");
        // Structure of TestCharIndexDataStructure =
        // { testDescription, searchChar, startPosition, expectedValue }
        TestCharIndexDataStructure testData[] = { \
           { "character that matches twice, such that the position specified " \
             "is after the first", 'e', 2, 4 }, \
           { "character that matches exactly at the position specified", \
             'n', 3, 3 }, \
           { "character's index is before the specified position", \
             'n', 4, INDEX_NOT_FOUND }, \
           { "character is not found in the string", 'z', 0, INDEX_NOT_FOUND } \

        };

        const int testCount = sizeof(testData)/sizeof(testData[0]);
        for(int j =0; j < testCount; j++)
        {
            ssize_t result = ts.index(testData[j].searchCharacter, testData[j].startPosition);
            TestUtilities::createMessage(3, &Message, prefix, \
                testData[j].testDescription, suffix);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                testData[j].expectedValue, result);
        }
    }


    /*a! Test case for comparing the index(char, ...)  methods
    *
    *    The test data for this test case is :-
    *        2) With the string with data ("wednesDay" )
    *            Test a case sensitive search for
    *            a) Find the index of  'w' - starting character
    *            b) Find the index of 'D' - only the index with the correct case is returned
    *            c) Find the index of 'y' - last character
    *            d) Find the index of 'z'  - a character that is not found
    *               For (a) to (d) verify by both specifying position explicitly
    *               as 0 / not providing at all
    */
    void testIndex_char_SearchFromStart()
    {
        utlTestIndexOrFirstMethod_char(TEST_INDEX);
    }

    /*a! Test case for the first(char) method.  This method is
    *    exactly similar to the index(char) method and so is the test data
    *    Refer to the testIndex_char_SearchFromStart() method for details.
    */
    void testFirst_char()
    {
        utlTestIndexOrFirstMethod_char(TEST_FIRST);
    }


    /** common utility method that can be used to drive either the index()
    *   method, the first() method or the contains method. (since the pattern
    *   of all the three are almost same)
    */
    void utlTestIndexOrFirstMethod_char(IndexFirstOrContainsType type)
    {
        const char* prefix = "";
        string Message;
        if (type == TEST_CONTAINS)
        {
            prefix = "Test the contains() method for";
        }
        else if (type == TEST_INDEX)
        {
            prefix = "Test the index method starting search from the first location, " \
                     "find the index of " ;
        }
        else if (type == TEST_FIRST)
        {
            prefix = "Test the first() method for";
        }
        UtlString testString("wednesDay");
        // Structure of TestCharIndexDataStructure =
        // { testDescription, searchChar, startPosition, expectedValue }
        TestCharIndexDataStructure testData[] = { \
            { "the first characater", 'w', 0, 0 }, \
            { "a character that would have matched at 0 location if the search were " \
              "case-insensitive",  'D', 0, 6 }, \
            { "the last character", 'y', 0, 8}, \
            { "a character that is not found in the string", 'z', 0, INDEX_NOT_FOUND } \
        };

        bool specifyPosition = false;
        const int testCount = sizeof(testData) / sizeof(testData[0]);
        int loopCount;
        // if the type is to test index(), then we need to test both the
        // cases :- a) position is explicitly specified as 0
        // b) position is not specified at all.
        // However the first() method takes only one argument and we dont
        // need to worry about the position.
        loopCount = type == TEST_INDEX ? 2 : 1;
        for (int i =0;  i < loopCount; i++)
        {
            const char* suffix = "";
            if (specifyPosition)
            {
                suffix = ":- index(char, 0)";
            }
            else
            {
               if (type == TEST_INDEX)
               {
                   suffix = ":- index(char)";
               }
               else if(type == TEST_FIRST)
               {
                   suffix = ":- first()";
               }
               else if(type == TEST_CONTAINS)
               {
                   suffix = ":- contains()";
               }
            }
            ssize_t result;
            for(int j =0; j < testCount; j++)
            {
                TestUtilities::createMessage(3, &Message, prefix, \
                    testData[j].testDescription, suffix);
                if (specifyPosition)
                {
                    result = testString.index(testData[j].searchCharacter, 0);
                }
                else
                {
                    if (type == TEST_INDEX)
                    {
                        result = testString.index(testData[j].searchCharacter);
                    }
                    else if (type == TEST_FIRST)
                    {
                        result = testString.first(testData[j].searchCharacter);
                    }
                }
                CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                    testData[j].expectedValue, result);
            }
            specifyPosition = !specifyPosition;
       }
    } //utlTestIndexOrFirstMethod_char


    /** Test case for verifying the last(char) method.
    *    The test data for this test case is :- ((The main string is "wednesDay elf")
    *        1. Find the index, starting off from a position other than start`
    *            a) Find the index of 'e' - a character that has multiple matches
    *            b) Find the index of 'n' - a character that matches in the middle and only once
    *            c) Find the index of 'z'  - a character that is not prefixsent
    *            d) Find the index of 'w' - the first character
    *            e) Find the index of 'f' - the last character
    *            f) Find the index of 'd' - a character that matches multiple times, "
    *               if case-insensitive.
    *            g) Find the index of ' ' - a spl. character
    *            // Verify with bob if we are hanlding the next case.
    *            i) Find the index of 'n' start from aa position greater than the length
    */
    void testLast_char()
    {
        const char* prefix = "Test the last() method with a character that";
        string Message;
        UtlString testString("wednesDay elf");
        // Structure of TestCharIndexDataStructure =
        // { testDescription, searchChar, startPosition, expectedValue }

        TestCharIndexDataStructure testData[] = { \
           /* Since the position to be specified has no significance for this
              test, give it as -1 */
           { "has multiple matches", 'e', -1, 10 }, \
           { "has a single match and is a mid-character", 'n', -1, 3 }, \
           { "is the first character", 'w', -1, 0 }, \
           { "is the last character", 'f', -1, 12 }, \
           { "a character that would have matched multiple times, if " \
             "the search were case-insensitive", 'd', -1, 2 }, \
           { "a non alpha character", ' ', -1, 9 }, \
           { "a character that is not present", 'z', -1, INDEX_NOT_FOUND } \
        };

        const int testCount = sizeof(testData)/sizeof(testData[0]);
        for (int i = 0; i < testCount; i++)
        {
            ssize_t result = testString.last(testData[i].searchCharacter);
            TestUtilities::createMessage(2, &Message, prefix, \
                testData[i].testDescription);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                testData[i].expectedValue, result);
        }
    }//testLast_char()


    /**  Test the length() method .
    *
    *     The test data for this test are the common set of test data.
    *     Refer to the test_Constructor_1() method (or any other test case
    *     that uses the common test data
    */
    void testLength()
    {
        const char* prefix = "Test the length() method for a string that is made up of ";
        const char* suffix1 = " :- initial length";
        const char* suffix2 = " :- length after mutating";

        for (int i = 0; i < commonTestSetLength; i++)
        {
            UtlString ts(commonTestSet[i].input);
            int originalLength = (int)ts.length();
            // Do some operations which modifies the string's size
            ts.prepend("123");
            ts.append("45");
            ts.insert(3, UtlString("687"));
            ts.replace(5, 1, "89");

            int expectedMutate = commonTestSet[i].length + 9;
            int actualMutate = (int)ts.length();

            string Message;
            TestUtilities::createMessage(3, &Message, prefix, \
                commonTestSet[i].testDescription, suffix1);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), commonTestSet[i].length, \
                originalLength);

            TestUtilities::createMessage(3, &Message, prefix, \
                commonTestSet[i].testDescription, suffix2);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                expectedMutate, actualMutate);
        }
    } //testLength()


    /** Test case for testing the length when the capacity of the string is
    *   is large and when a non null-terminated string has been appended to the string
    */
    void testLength_NonNullTerminatedData()
    {
        const char* prefix = "Test the length() method when";
        string Message;

        UtlString testString1("Test1234");
        UtlString testString2("Test");
        testString1.capacity(256);
        testString2.append("12", 4);

        int actualLength;

        actualLength = (int)testString1.length();
        TestUtilities::createMessage(2, &Message, prefix, \
            "When the capacity as been explicitly set to a large value");
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), 8, actualLength);

        actualLength = testString2.length();
        TestUtilities::createMessage(2, &Message, prefix, \
            "When a non-null terminated string has been appended");
        CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), 8, actualLength);
    }


    /*!a Test the isNull() method.
    *
    *    Test data for this test is :-
    *       a) Empty String
    *       b) String that has only 'space' characters
    *       c) String that has a single 'special' character
    *       d) String that has only digits.
    */
    void testIsNull()
    {
        struct TestIsNullStructure
        {
            const char* testDescription;
            const char* input;
            UtlBoolean expectedValue;
        };
        const char* prefix = "Test the isNull() method for";
        string Message;

        TestIsNullStructure testData[] = { \
            { "an empty string", "", true }, \
            { "a space-only string", "    ", false}, \
            { "a single char string", "þ", false}, \
            { "a digit-only string", "239", false } \
        };
        const int testCount = sizeof(testData)/sizeof(testData[0]);
        for (int i = 0; i < testCount; i++)
        {
            UtlString testString(testData[i].input);
            UtlBoolean isNull = testString.isNull();

            TestUtilities::createMessage(2, &Message, prefix, \
                testData[i].testDescription);
            CPPUNIT_ASSERT_EQUAL_MESSAGE (Message.data(),  \
                (bool)(TRUE == testData[i].expectedValue), (bool)(TRUE == isNull));
        }

        // Verify the isNull method after 'nulliying' a non-null string
        UtlString tsNotNull("abc");
        tsNotNull.replace(0, 3,"");
        CPPUNIT_ASSERT_MESSAGE(\
            "Test the isNull() method after nullifying a non-null string", \
            tsNotNull.isNull() );
    }


    /*!a Test the toLower() and toUpper() method.
    *
    *   The test data for this method is:-
    *       a) Empty string
    *       b) String with all UPPER characters
    *       c) String with all LOWER characters
    *       d) String with mixedcase characters
    *       e) alphanumeric string with mixedcase characters.
    */
    void testLowerAndUpper()
    {
        struct TestLowerAndUpperStructure
        {
            const char* testDescription;
            const char* input;
            const char* expectedValueForLower;
            const char* expectedValueForUpper;
        };

        const char* prefix = "For a ";
        const char* suffix1 = " :- test the toLower() method";
        const char* suffix2 = " :- test the toUpper() method";
        string Message;

        TestLowerAndUpperStructure testData[] = { \
            { "empty string", "", "", "" }, \
            { "all uppercase string", "TEST", "test", "TEST" }, \
            { "all lowercase string", "test", "test", "TEST" }, \
            { "mixed case string", "TeSt", "test", "TEST" }, \
            { "alpha-num mixed case", "Te#2 4st", "te#2 4st", "TE#2 4ST" } \
        };

        const int testCount = sizeof(testData)/sizeof(testData[0]);
        for (int i = 0; i < testCount; i++)
        {
            UtlString testString1(testData[i].input);
            UtlString testString2(testData[i].input);

            testString1.toLower();
            testString2.toUpper();

            TestUtilities::createMessage(3, &Message, prefix, \
                testData[i].testDescription, suffix1);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                string(testData[i].expectedValueForLower), string(testString1.data()));

            TestUtilities::createMessage(3, &Message, prefix, \
                testData[i].testDescription, suffix2);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                string(testData[i].expectedValueForUpper), string(testString2.data()));
        }
    } //testLowerAndUpper


    /** Test the + operator.
    *        The test data for this case is:-
    *         The normal range of data.
    */
    void testPlusOperator()
    {
        const char* suffix1 = " lhs = UtlString, rhs = UtlString " ;
        const char* suffix2 = " lhs = UtlString, rhs = charstar ";
        const char* suffix3 = " lhs = charstar, rhs = UtlString ";

        for (int i = 0; i < commonTestSetLength; i++)
        {
            const char* baseString = commonTestSet[i].testDescription;
            const char* baseMsg = commonTestSet[i].testDescription;
            UtlString tsBase(baseString);

            for (int j = 0; j < commonTestSetLength; j++)
            {
                const char* stringToAppend = commonTestSet[j].input;
                const char* appendMsg = commonTestSet[j].testDescription;
                UtlString tsAppend(stringToAppend);

                string Msg("To a ");
                Msg.append(baseMsg);
                Msg.append(" add a ");
                Msg.append(appendMsg);

                string Msg2;

                string expString(baseString);
                expString.append(stringToAppend);

                UtlString testString;
                // Test UtlString + UtlString
                testString = tsBase + tsAppend;
                TestUtilities::createMessage(2, &Msg2, Msg.data(), suffix1);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(Msg2.data(), expString, \
                    string(testString.data()));

                // Test UtlString + char*
                testString = tsBase + stringToAppend;
                TestUtilities::createMessage(2, &Msg2, Msg.data(), suffix2);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(Msg2.data(), expString, \
                    string(testString.data()));

                // Test char* + UtlString
                testString = baseString + tsAppend;
                TestUtilities::createMessage(2, &Msg2, Msg.data(), suffix3);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(Msg2.data(), expString, \
                    string(testString.data()));
            }
        }
    }//testPlusOperator()


    /*a! Test the indexing operator (m, n)
    *
    *    The test data for this test is :-
    *        a) Find (0, 0) for an empty string
    *        b) Find (0, 0) for a non-empty string
    *        c) Find (len-1, len-1) for a non-empty string
    *        d) Find (start, end) for a string. start>0; end<len-1
    *        e) Find (start, end) for a string. start=0; end=len-1
    */
    void testIndexOperator()
    {
        struct TestIndexOperatorStructure
        {
            const char* testDescription;
            const char* input;
            int startPosition;
            int endPosition;
            const char* expectedValue;
        };

        const char* suffix1 = " :- verify return string";
        string Message;

        TestIndexOperatorStructure testData[] = { \
            { "For an empty string, find string(0,0)", "", \
              0, 0, "" }, \
            { "For a string, find string(0, 0)", "TestStr", \
              0, 0, "" }, \
            { "For a string, find string(len-1, len-1)", "TestStr", \
              7, 7, "" }, \
            { "For a string, find string(>0, <len)", "Tes#23t", \
              2, 4, "s#23" }, \
            { "For a string, find string(0,  len-1)", "TestStr", \
              0, 7, "TestStr" } \
        };

        const int testCount = sizeof(testData)/sizeof(testData[0]);
        for (int i = 0; i < testCount; i++)
        {
            UtlString testString1(testData[i].input);
            UtlString testString2 = testString1(testData[i].startPosition, \
                testData[i].endPosition);

            TestUtilities::createMessage(2, &Message, testData[i].testDescription, \
                suffix1);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), \
                string(testData[i].expectedValue), string(testString2.data()));
        }
    } //testIndexOperator


    /*!a Test the (n) operator.
    *
    *    With an alphanumeric , get the character at
    *        a) The 0th position
    *        b) The last position
    *        c) 'n' suffixion such that the character is an 'alpha' character
    *        d) 'n' position such that the character is a 'numeric' character
    */
    void testCharAtOperator()
    {
        struct TestCharAtStructure
        {
            const char* testDescription;
            int index;
            char expectedValue;
        };

        const char* prefix = "Test the (n) operator, when";
        string Message;

        TestCharAtStructure testData[] = { \
            { "n = 0", 0, 'S' }, \
            { "n = len(string)", 9, 'x' }, \
            { "character at n is an alpha character", 3, 'e' }, \
            { "character at n is a numeric character", 5, '2' } \
        };

        const char* baseString = "Some12$Text";
        UtlString testString(baseString);

        const int testCount = sizeof(testData)/sizeof(testData[0]);
        for (int i=0; i < testCount; i++)
        {
            char actualValue = testString(testData[i].index);

            TestUtilities::createMessage(2, &Message, prefix, testData[i].testDescription);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), testData[i].expectedValue, \
                actualValue);
        }
    }//testCharAtOperator


    /*!a Test findToken.
    *
    *    look for string as
    *        a) The first token
    *        b) The last token
    *        c) nowhere in the string
    *        d) somewhere in the string
    *        d) substring exists in the string but doesn't meet token criteria
    */
    void testfindToken()
    {
        UtlBoolean result;
        enum Mode
        {
           literalAndRegexp,
           literalOnly,
           regexpOnly
        };
        struct TestCharAtStructure
        {
           const char* testDescription;
           const char* testNeedle;
           const char* testHaystack;
           const char* testDelimit;
           const char* testSuffix;
           enum Mode mode;
           UtlBoolean expectedResult;
        };

        const char* prefix = "Test findRegEx, when ";
        string Message;

        TestCharAtStructure testData[] = {
            { "regEx first and last in string",
              "testWord", " testWord", ",", NULL,
              literalAndRegexp, TRUE },
            { "regEx NOT in string",
              "testWord", "no, such ,word", ",", NULL,
              literalAndRegexp, FALSE },
            { "regEx not first in string",
              "testWord", "not , the , first,testWord, or, last", ",", NULL,
              literalAndRegexp, TRUE  },
            { "regEx last in string",
              "testWord", "not , the , first,testWord", ",", NULL,
              literalAndRegexp, TRUE  },
            { "regEx NOT(exactly) in string",
              "testWord", "no such, testWord-sim, word", ",", NULL,
              literalAndRegexp, FALSE },
            { "regEx has suffix",
              "testWord", "suffix , is, semicolon,testWord;dale, seen, here", ",", ";",
              literalAndRegexp, TRUE  },
            { "regEx has no suffix",
              "testWord", "suffix , is, semicolon,testWord, not, here", ",", ";",
              literalAndRegexp, TRUE  },
            { "search string contains regexp meta chars 1",
              "text/dialoginfo+xml", "text/plain,text/dialoginfo+xml", ",", NULL,
              literalOnly, TRUE },
            { "search string contains regexp meta chars 2",
              "text/dialoginfo+xml", "text/plain,text/dialoginfo+xml", ",", NULL,
              regexpOnly, FALSE },
            { "search for regexp",
              "ab+c*", "a, ab, d", ",", NULL,
              regexpOnly, TRUE },
            // Check case insensitivity.
            { "regEx first and last in string",
              "testword", " TestWord", ",", NULL,
              literalAndRegexp, TRUE },
            { "regEx not first in string",
              "TESTWORD", "not , the , first,testword, or, last", ",", NULL,
              literalAndRegexp, TRUE  },
            { "search for regexp",
              "Ab+c*", "a, aB, d", ",", NULL,
              regexpOnly, TRUE },
        };

        UtlString *ptestHaystack;

        const int testCount = sizeof(testData)/sizeof(testData[0]);
        for (int i=0; i < testCount; i++)
        {
            ptestHaystack = new UtlString(testData[i].testHaystack);
            TestUtilities::createMessage(2, &Message, prefix,
                                         testData[i].testDescription);
            if (testData[i].mode != regexpOnly)
            {
               // Execute the test as a literal string.
               result =
                  ptestHaystack->findToken(testData[i].testNeedle,
                                           testData[i].testDelimit,
                                           testData[i].testSuffix);
               CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(),
                                            testData[i].expectedResult,
                                            result);
            }

            if (testData[i].mode != literalOnly)
            {
               // Execute the test as a regexp string.
               result =
                  ptestHaystack->findToken(testData[i].testNeedle,
                                           testData[i].testDelimit,
                                           testData[i].testSuffix,
                                           true);
               CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(),
                                            testData[i].expectedResult,
                                            result);
            }

            delete ptestHaystack;
        }
    }//testfindToken

    void testfindTokenSuffix()
    {
       UtlString string("word1,word2;parameter,word3");

       CPPUNIT_ASSERT(!string.findToken("word2", ","));
       CPPUNIT_ASSERT(string.findToken("word2", ",", ";"));
    }//testfindTokenSuffix

};

CPPUNIT_TEST_SUITE_REGISTRATION(UtlStringTest_NonMutating);
