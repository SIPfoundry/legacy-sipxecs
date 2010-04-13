//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <net/CallId.h>
#include <utl/UtlString.h>

#if 0
#  define DEBUG_PRINT( format, data ) fprintf(stderr, format, data)
#else
#  define DEBUG_PRINT( format, data ) /* fprintf(stderr, format, data) */
#endif

/**
 * Unit tests for CallId.
 */
class CallIdTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(CallIdTest);

    CPPUNIT_TEST(testCallId);
    CPPUNIT_TEST(testGetNewTag);

    CPPUNIT_TEST_SUITE_END();

public:

    void testCallId()
    {
#define CASES 10
#define MIN_CALLID_DIFFS 7
#define CHARS_TO_COMPARE 10

// The test succeeds if at least 7 of the initial 10 characters differ
// between all pairs of Call-Ids we generate.  If the generated
// Call-Ids are random, the initial 10 pseudo-random characters will
// have at least 7 differences all but 0.00024% of the time.  Since
// there are 45 pairs of Call-Ids to compare, this test will fail
// about 0.00109% of the time for some pair.

       UtlString output[CASES];

       // Generate some call-Ids.
       for (unsigned int i = 0; i < sizeof (output) / sizeof (output[0]); i++)
       {
          CallId::getNewCallId(output[i]);
          // Enable this statement if you want to see some sample values
          // from getNewCallId.
          DEBUG_PRINT( "%s\n", output[i].data());
       }

       // Compare that they're different enough.
       for (unsigned int i = 0; i < sizeof (output) / sizeof (output[0]); i++)
       {
          for (unsigned int j = i+1; j < sizeof (output) / sizeof (output[0]); j++)
          {
             UtlString* s1 = &output[i];
             UtlString* s2 = &output[j];

             int differences = 0;
             for (size_t k = 0; k < CHARS_TO_COMPARE; k++)
             {
                if ((*s1)(k) != (*s2)(k))
                {
                   differences++;
                }
             }
             if (differences < MIN_CALLID_DIFFS)
             {
                char msg[200];
                sprintf(msg,
                        "Call-IDs '%s' and '%s' have %d different characters, "
                        "which is less than the minimum, %d",
                        s1->data(), s2->data(), differences, MIN_CALLID_DIFFS);
                CPPUNIT_ASSERT_MESSAGE(msg, FALSE);
             }
          }
       }
    }

   void testGetNewTag()
    {
#define CASES 10
// The test succeeds if at least 3 of the 8 digits differ between all
// pairs of tags we generate.  If the generated tags are random, this
// test will fail 0.00015% of the time for any one pair, or about 0.0044%
// of the time for one of the 45 pairs.
#define MIN_TAG_DIFFS 3
       UtlString output[CASES];

       // Generate some tags.
       for (unsigned int i = 0; i < sizeof (output) / sizeof (output[0]); i++)
       {
          CallId::getNewTag(output[i]);
          // Enable this statement if you want to see some sample values.
          #if 0
            DEBUG_PRINT( "%s\n", output[i].data());
          #endif
       }

       // Compare that they're different enough.
       for (unsigned int i = 0; i < sizeof (output) / sizeof (output[0]); i++)
       {
          for (unsigned int j = i+1; j < sizeof (output) / sizeof (output[0]); j++)
          {
             UtlString* s1 = &output[i];
             UtlString* s2 = &output[j];

             int differences = 0;
             for (size_t k = 0; k < s1->length() && k < s1->length(); k++)
             {
                if ((*s1)(k) != (*s2)(k))
                {
                   differences++;
                }
             }
             if (differences < MIN_TAG_DIFFS)
             {
                char msg[200];
                sprintf(msg,
                        "Tags '%s' and '%s' have %d different characters, "
                        "which is less than the minimum, %d",
                        s1->data(), s2->data(), differences, MIN_TAG_DIFFS);
                CPPUNIT_ASSERT_MESSAGE(msg, FALSE);
             }
          }
       }
#undef CASES
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CallIdTest);
