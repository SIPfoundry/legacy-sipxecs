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

#include <utl/UtlString.h>
#include <utl/UtlTokenizer.h>
#include <sipxunit/TestUtilities.h>

using namespace std ;

class UtlTokenizerTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(UtlTokenizerTest);
    CPPUNIT_TEST(testBasic);
    CPPUNIT_TEST(testDelimsAtVariousPositions);
    CPPUNIT_TEST_SUITE_END();


public:

    void testBasic()
    {
        struct TestBasicStructure
        {
            const char* testDescription ;
            const char* expectedString ;
            bool expectedReturnValue ;
        } ;

        const char* prefix = "Test the next() method for " ;
        const char* suffix1 = " :-Verify token" ;
        const char* suffix2 = " :-Verify return value" ;
        string Message ;

        UtlTokenizer tokens("one two three");
        const char* delim = " ";

        TestBasicStructure testData[] = { \
             { "1st token inaccurate", "one", true }, \
             { "2nd token inaccurate", "two", true }, \
             { "3rd token inaccurate", "three", true }, \
             { "Unexpected ending tokens", "", false } \
        } ;
        int testCount = sizeof(testData) / sizeof(testData[0]) ;
        for (int i = 0 ; i < testCount ; i++ )
        {
            UtlString token;
            UtlBoolean returnValue = tokens.next(token, delim);
            TestUtilities::createMessage(3, &Message, prefix, testData[i].testDescription, \
                suffix1) ;
            ASSERT_STR_EQUAL_MESSAGE(Message.data(), testData[i].expectedString, token.data());

            TestUtilities::createMessage(3, &Message, prefix, testData[i].testDescription, \
                suffix2) ;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(Message.data(), testData[i].expectedReturnValue, \
               (TRUE == returnValue)) ;

        } ;
    }

    void testDelimsAtVariousPositions()
    {
        struct TokenizerTestData
        {
            const char* testDescription ;
            const char* tokenizedString;
            int expectedCount;
        };

        char msg[1024];
        TokenizerTestData testSet[] = {
            { "string with multiple delims", \
              "---X---XX---",    3},
            { "string with one delim at the begining", \
              "X---",            1},
            { "string with no token", \
              "---",             1},
            { "string ending with delim", \
              "---X",            1},
            { "string begining with multiple delims", \
              "XXX---",          1},
            { "string with nothing but the delim", \
              "XXX",             0}
        };

        int n = sizeof(testSet) / sizeof(testSet[0]);
        for (int i = 0; i < n; i++)
        {
            UtlTokenizer toks(testSet[i].tokenizedString);
            int actualCount = 0;
            UtlBoolean done = FALSE;
            while (!done)
            {
                UtlString tok;
                UtlBoolean bSuccess = toks.next(tok, "X");
                done = tok.isNull();
                if (!done)
                {
                    memset(msg, 0, sizeof(msg));
                    sprintf(msg, "For %s, verify token number %d",
                        testSet[i].testDescription, actualCount);
                    ASSERT_STR_EQUAL_MESSAGE(msg, "---", tok.data());
                    actualCount++;

                    memset(msg, 0, sizeof(msg));
                    sprintf (msg, "For %s, verify return value for token number %d",
                        testSet[i].testDescription, actualCount);
                    CPPUNIT_ASSERT_MESSAGE(msg, (TRUE == bSuccess)) ;
                }
                else
                {
                    memset(msg, 0, sizeof(msg));
                    sprintf (msg, "For %s, verify return value after all tokens",
                        testSet[i].testDescription);
                    CPPUNIT_ASSERT_MESSAGE(msg, !((TRUE == bSuccess))) ;
                }
            }
            memset(msg, 0, sizeof(msg));
            sprintf(msg, "For %s, verify number of tokens", testSet[i].testDescription);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, testSet[i].expectedCount, actualCount);
        }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(UtlTokenizerTest);
