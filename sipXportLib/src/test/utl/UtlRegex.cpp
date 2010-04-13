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
#include <stdlib.h>

#include <sipxunit/TestUtilities.h>

/**
 * Unit test for RegEx
 *
 * This is designed to test the encapsulation of PCRE, not PCRE itself.
 * These tests assume that the actual regular expression matching is
 * correct (although most will fail if it is not) because PCRE has its
 * own excellent unit tests.  If you are incorporating a modified version
 * of PCRE, you must use its unit tests to confirm the correctness of
 * the modifications before this test will be useful.
 */
#include "utl/UtlRegex.h"

class UtlRegExTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(UtlRegExTest);
   CPPUNIT_TEST(testInput1);
   CPPUNIT_TEST(testMatchInfo);
   CPPUNIT_TEST(testUnMatchInfo);
   CPPUNIT_TEST(testMatchStart);
   CPPUNIT_TEST(testMatchUtlInfo);
   CPPUNIT_TEST(testMatchUtlLookAhead);
   CPPUNIT_TEST(testMatchAfter);
   CPPUNIT_TEST(testMatchUtlLookBehind);
   CPPUNIT_TEST(testCopy1);
   CPPUNIT_TEST(testRecursionLimit);
   CPPUNIT_TEST(testQuotemeta);
   CPPUNIT_TEST_SUITE_END();

# define EXPRESSION( expression, options ) \
           { if ( TheRegEx ) { delete TheRegEx; } \
             TheRegEx = new RegEx( expression, options ); \
             CPPUNIT_ASSERT_MESSAGE( "compiling expression /" expression "/", TheRegEx ); \
           }

# define SHOULD_MATCH( matches, in ) \
  CPPUNIT_ASSERT_MESSAGE( "'" in "' should have matched" #matches, \
                          TheRegEx->Search( in ) && ( TheRegEx->SubStrings() == matches ) \
                         );

# define MATCH( n, matchVal ) \
  CPPUNIT_ASSERT_MESSAGE( "'" matchVal "' should have been a match", \
                        ( strcmp( TheRegEx->Match(n), matchVal ) == 0 ) \
                        );

# define SHOULD_NOT_MATCH( in ) \
  CPPUNIT_ASSERT_MESSAGE( "'" in "' should not have matched", \
                           ! TheRegEx->Search( in )   \
                         );

public:

   void testInput1()
      {
         RegEx* TheRegEx = 0;

         EXPRESSION( "the quick brown fox", 0 );
         SHOULD_MATCH( 1, "the quick brown fox" );
         MATCH( 0, "the quick brown fox" );
         SHOULD_NOT_MATCH("The quick brown FOX");
         SHOULD_MATCH( 1, "What do you know about the quick brown fox?" );
         MATCH( 0, "the quick brown fox" );
         SHOULD_NOT_MATCH("What do you know about THE QUICK BROWN FOX?");

         delete TheRegEx;

      }

   void testCopy1()
      {
         static const RegEx ConstRegEx("^[a-z]+([0-9]+)$");
         RegEx* TheRegEx = new RegEx(ConstRegEx);

         SHOULD_MATCH(2,"foo35");
         MATCH(0,"foo35");
         MATCH(1,"35");

         SHOULD_NOT_MATCH( "abc" );
         SHOULD_NOT_MATCH( "Foo35" );
         SHOULD_NOT_MATCH( "12Foo35" );
         SHOULD_NOT_MATCH( "foo35 " );

         delete TheRegEx;
      }

   void testCopy2()
      {
         static const RegEx ConstRegEx("^[a-z]+([0-9]+)$");
         RegEx TheCopiedRegEx(ConstRegEx);
         RegEx* TheRegEx = &TheCopiedRegEx;

         SHOULD_MATCH(2,"foo35");
         MATCH(0,"foo35");
         MATCH(1,"35");

         SHOULD_NOT_MATCH( "abc" );
         SHOULD_NOT_MATCH( "Foo35" );
         SHOULD_NOT_MATCH( "12Foo35" );
         SHOULD_NOT_MATCH( "foo35 " );
      }

   void testMatchInfo()
      {
         RegEx matchABCs("A+(B+)(C+)");

         CPPUNIT_ASSERT(matchABCs.Search("xxxAABBBBC"));

         CPPUNIT_ASSERT( matchABCs.Matches() == 3 );

         int start;
         int length;

         CPPUNIT_ASSERT( matchABCs.Match(0, start, length));
         CPPUNIT_ASSERT( start == 3 );
         CPPUNIT_ASSERT( length == 7 );

         CPPUNIT_ASSERT( matchABCs.Match(1, start, length));
         CPPUNIT_ASSERT( start == 5 );
         CPPUNIT_ASSERT( length == 4 );

         CPPUNIT_ASSERT( matchABCs.Match(2, start, length));
         CPPUNIT_ASSERT( start == 9 );
         CPPUNIT_ASSERT( length == 1 );
      }

   void testUnMatchInfo()
      {
         RegEx matchABCs("(A+|(B+))(C+)");

         CPPUNIT_ASSERT(matchABCs.Search("xxxAACC"));

         CPPUNIT_ASSERT( matchABCs.Matches() == 4 );

         int start;
         int length;

         CPPUNIT_ASSERT( matchABCs.Match(0, start, length));
         CPPUNIT_ASSERT( start == 3 );
         CPPUNIT_ASSERT( length == 4 );

         CPPUNIT_ASSERT( matchABCs.Match(3, start, length));
         CPPUNIT_ASSERT( start == 5 );
         CPPUNIT_ASSERT( length == 2 );

         CPPUNIT_ASSERT( matchABCs.Match(1, start, length));
         CPPUNIT_ASSERT( start == 3 );
         CPPUNIT_ASSERT( length == 2 );

         CPPUNIT_ASSERT( !matchABCs.Match(2, start, length));
         CPPUNIT_ASSERT( start == -1 );
         CPPUNIT_ASSERT( length == 0 );
      }


   void testMatchStart()
      {
         RegEx matchABCs("A+(B+)(C+)");

         CPPUNIT_ASSERT(matchABCs.Search("xxxAABBBBC"));

         CPPUNIT_ASSERT( matchABCs.Matches() == 3 );

         CPPUNIT_ASSERT( matchABCs.MatchStart(0) == 3 );
         CPPUNIT_ASSERT( matchABCs.MatchStart(1) == 5 );
         CPPUNIT_ASSERT( matchABCs.MatchStart(2) == 9 );
      }


   void testMatchUtlInfo()
      {
         RegEx matchABCs("A+(B+)(C+)");
         const char* subject = "xxxAABBBBC";

         CPPUNIT_ASSERT(matchABCs.Search(subject));

         CPPUNIT_ASSERT( matchABCs.Matches() == 3 );

         int start;
         int length;

         CPPUNIT_ASSERT( matchABCs.Match(0, start, length));
         CPPUNIT_ASSERT( start == 3 );
         CPPUNIT_ASSERT( length == 7 );

         CPPUNIT_ASSERT( matchABCs.Match(1, start, length));
         CPPUNIT_ASSERT( start == 5 );
         CPPUNIT_ASSERT( length == 4 );

         CPPUNIT_ASSERT( matchABCs.Match(2, start, length));
         CPPUNIT_ASSERT( start == 9 );
         CPPUNIT_ASSERT( length == 1 );

         CPPUNIT_ASSERT( matchABCs.SearchAt(subject, 2));

         CPPUNIT_ASSERT( matchABCs.Match(0, start, length));
         CPPUNIT_ASSERT( start == 3 );
         CPPUNIT_ASSERT( length == 7 );

         CPPUNIT_ASSERT( matchABCs.Match(1, start, length));
         CPPUNIT_ASSERT( start == 5 );
         CPPUNIT_ASSERT( length == 4 );

         CPPUNIT_ASSERT( matchABCs.Match(2, start, length));
         CPPUNIT_ASSERT( start == 9 );
         CPPUNIT_ASSERT( length == 1 );
      }

   void testMatchUtlLookAhead()
      {
         RegEx matchAs("A+(?=:)");
         const char* subject = "xxxAA:x";

         int start;
         int length;

         CPPUNIT_ASSERT(matchAs.Search(subject));

         CPPUNIT_ASSERT( matchAs.Matches() == 1 );

         CPPUNIT_ASSERT( matchAs.Match(0, start, length));
         CPPUNIT_ASSERT( start == 3 );
         CPPUNIT_ASSERT( length == 2 );

         UtlString after;
         matchAs.AfterMatchString(&after);

         CPPUNIT_ASSERT( after.compareTo(":x") == 0 );
      }

   void testMatchAfter()
      {
         RegEx matchBseq("A+(B+)C+");
         CPPUNIT_ASSERT(matchBseq.Search("xxAABBBCCCyy"));

         CPPUNIT_ASSERT(matchBseq.AfterMatch(1) == 7);
         CPPUNIT_ASSERT(matchBseq.AfterMatch(0) == 10);
      }

   void testMatchUtlLookBehind()
      {
         RegEx matchAs("(?<=:)A+");
         const char* subject = "xxx:AAyyyyy";

         CPPUNIT_ASSERT(matchAs.Search(subject));

         CPPUNIT_ASSERT( matchAs.Matches() == 1 );

         int start;
         int length;

         CPPUNIT_ASSERT( matchAs.Match(0, start, length));
         CPPUNIT_ASSERT( start == 4 );
         CPPUNIT_ASSERT( length == 2 );

         UtlString before;
         matchAs.BeforeMatchString(&before);

         CPPUNIT_ASSERT( before.compareTo("xxx:") == 0 );

         CPPUNIT_ASSERT(matchAs.SearchAt(subject, 4));

         CPPUNIT_ASSERT( matchAs.Matches() == 1 );

         CPPUNIT_ASSERT( matchAs.Match(0, start, length));
         CPPUNIT_ASSERT( start == 4 );
         CPPUNIT_ASSERT( length == 2 );
      }


   void testRecursionLimit()
      {
         // this pattern recurses for every character
         RegEx matchAs("([^<]|<(?!inet))+", 0, 100 );

         /*
          * Because we are using a version of PCRE that does not support a limit on
          * recursion, this string must be much shorter than the limit.  See the
          * cautionary note in UtlRegEx.h on MAX_RECURSION.
          */
         UtlString okSubject( /* 45 x 'a' */
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa<foo"
                              );
         UtlString bigSubject( /* 100 x 'a' */
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa<foo"
                              );
         CPPUNIT_ASSERT(matchAs.Search(okSubject));

//         CPPUNIT_ASSERT(matchAs.Matches() == 3);

//         int start;
//         int length;

//         CPPUNIT_ASSERT( matchAs.Match(0, start, length));
//         CPPUNIT_ASSERT_EQUAL( 0, start );
//         CPPUNIT_ASSERT_EQUAL( 99, length );

         CPPUNIT_ASSERT(!matchAs.Search(bigSubject));
      }

   void testQuotemeta()
      {
        struct test
        {
            const char* literal;
            const char* expected_regexp;
        } tests[] = {
           { "", "" },
           { "1", "1" },
           { "a", "a" },
           { "!", "\\!" },
           { " ", "\\ " },
           { "a1", "a1" },
           { "a! ", "a\\!\\ " },
           { "this is", "this\\ is" },
           { "this_is", "this\\_is" },
        };

        for (unsigned int i = 0; i < sizeof (tests) / sizeof (tests[0]); i++)
        {
           char message[100];
           sprintf(message, "Test %d: '%s'", i, tests[i].literal);

           UtlString literal(tests[i].literal);
           UtlString regexp;
           RegEx::Quotemeta(literal, regexp);

           ASSERT_STR_EQUAL_MESSAGE(message,
                                    tests[i].expected_regexp, regexp.data());
        }
      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(UtlRegExTest);
