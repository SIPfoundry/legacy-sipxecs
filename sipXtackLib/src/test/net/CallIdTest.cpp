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
    CPPUNIT_TEST(testGetNewCallId);
    CPPUNIT_TEST(testGetNewTag);

    CPPUNIT_TEST_SUITE_END();

public:

    void testCallId()
    {
#define CASES 10
// The test succeeds if at least 9 of the 16 digits differ between all
// pairs of Call-Ids we generate.  If the generated Call-Ids are random, this
// test will fail 0.00023% of the time for any one pair, or about 0.011%
// of the time for one of the 45 pairs.
#define MIN_DIFFS 9
       UtlString output[CASES];

       // Generate some call-Ids.
       for (unsigned int i = 0; i < sizeof (output) / sizeof (output[0]); i++)
       {
          CallId::getNewCallId("t", output[i]);
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
             for (unsigned int k = 0; k < s1->length() && k < s1->length(); k++)
             {
                if ((*s1)(k) != (*s2)(k))
                {
                   differences++;
                }
             }
             if (differences < MIN_DIFFS)
             {
                char msg[200];
                sprintf(msg,
                        "Call-IDs '%s' and '%s' have %d different characters, "
                        "which is less than the minimum, %d",
                        s1->data(), s2->data(), differences, MIN_DIFFS);
                CPPUNIT_ASSERT_MESSAGE(msg, FALSE);
             }
          }
       }
    }

    /* Support routine for testGetNewCallId to parse and validate a call ID.
     * The first argument is the call ID.
     * The second argument is the expected prefix.
     * The third argument receives the counter part of the call ID.
     */
    void testGetNewCallId_validate(UtlString &callId,
                                   const char* expected_prefix,
                                   UtlString* counter)
      {
         char actual_prefix[100], actual_counter[100], actual_suffix[100];
         char msg[1000];

         // The character that separates fields in call IDs.
         // This is a #define so it is easy to change.
         // Fields in generated call IDs must never contain this character.
         #define FIELD_SEPARATOR_CHAR "-"

         unsigned int chars_consumed = -1;
         sscanf(callId.data(),
                "%[^" FIELD_SEPARATOR_CHAR "]" FIELD_SEPARATOR_CHAR
                "%[^" FIELD_SEPARATOR_CHAR "]" FIELD_SEPARATOR_CHAR
                "%s%n",
                actual_prefix, actual_suffix, actual_counter,
                &chars_consumed);
         sprintf(msg, "Cannot parse call ID '%s'", callId.data());
         CPPUNIT_ASSERT_MESSAGE(msg,
                                chars_consumed == callId.length());
         sprintf(msg, "Actual prefix '%s' does not match expected prefix '%s' in call ID '%s'",
                 actual_prefix, expected_prefix, callId.data());
         CPPUNIT_ASSERT_MESSAGE(msg,
                                strcmp(actual_prefix, expected_prefix) == 0);
         *counter = actual_counter;
      }

    /* Support routine for testGetNewCallId to write over the stack to ensure
     * that a valid pointer does not appear in getNewCallId's stack by accident.
     */
     void testGetNewCallId_hose_stack()
      {
         int buffer[1024];
         // Access buffer through p, to confuse simple optimizers.
         int *p = &buffer[0];

         for (unsigned int i = 0; i < sizeof (buffer) / sizeof (int); i++)
         {
            p[i] = i;
         }
      }

    /* Some basic tests on the CallId::getNewCallId methods. */
    void testGetNewCallId()
    {
       // To hold the returned call IDs.
       UtlString callId1, callId2, callId3, callId4;
       // To hold counter fields.
       UtlString counter;

       testGetNewCallId_hose_stack();
       CallId::getNewCallId("prefix1", callId1);
       DEBUG_PRINT( "%s\n", callId1.data());
       testGetNewCallId_validate(callId1, "prefix1", &counter);

       testGetNewCallId_hose_stack();
       CallId::getNewCallId("prefix2", callId2);
       DEBUG_PRINT( "%s\n", callId2.data());
       testGetNewCallId_validate(callId2, "prefix2", &counter);

       testGetNewCallId_hose_stack();
       CallId::getNewCallId("prefix3", callId3);
       DEBUG_PRINT( "%s\n", callId3.data());
       testGetNewCallId_validate(callId3, "prefix3", &counter);
#undef CASES
#undef MIN_DIFFS
    }

    void testGetNewTag()
    {
#define CASES 10
// The test succeeds if at least 3 of the 8 digits differ between all
// pairs of tags we generate.  If the generated tags are random, this
// test will fail 0.00015% of the time for any one pair, or about 0.0044%
// of the time for one of the 45 pairs.
#define MIN_DIFFS 3
       UtlString output[CASES];

       // Generate some tags.
       for (unsigned int i = 0; i < sizeof (output) / sizeof (output[0]); i++)
       {
          CallId::getNewTag("", output[i]);
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
             for (unsigned int k = 0; k < s1->length() && k < s1->length(); k++)
             {
                if ((*s1)(k) != (*s2)(k))
                {
                   differences++;
                }
             }
             if (differences < MIN_DIFFS)
             {
                char msg[200];
                sprintf(msg,
                        "Tags '%s' and '%s' have %d different characters, "
                        "which is less than the minimum, %d",
                        s1->data(), s2->data(), differences, MIN_DIFFS);
                CPPUNIT_ASSERT_MESSAGE(msg, FALSE);
             }
          }
       }
#undef CASES
#undef MIN_DIFFS
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CallIdTest);
