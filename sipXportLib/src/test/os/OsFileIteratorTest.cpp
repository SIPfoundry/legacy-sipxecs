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
#include <sipxunit/TestUtilities.h>

#include <os/OsFS.h>
#include <os/OsFS.h>
#include <os/OsTestUtilities.h>

// This prints the filter expressions and what they find
//#define TRACE_TEST(x) printf x
#define TRACE_TEST(x)

/**
 * Test Description
 */

/**
 * These files are created in the setUp routine; they are used for all trials
 * The indexes of this array MUST MATCH those for the expected element of the
 * struct trial below.
 */
static const char* const TestFiles[] =
{ "001.msg",
  "003.msg",
  "001.msgx",
  "x001.msg",
  "x001.msgx",
  "abc.msg",
  "abc.def",
  "001.not",
  "001xmsg",
  "002.msg"
};
static const int NumTestFiles = sizeof(TestFiles)/sizeof(char*);

/**
 * Each trial is a regular expression to be used as a filter, and an array
 * of booleans for whether or not the matching element of the TestFiles should
 * have been found by it.
 */
static const struct trial
{
      const char* regex;
      bool        expected[NumTestFiles];
} Trials[] =
{
   { "^[0-9]+\\.msg$", { true,  true,  false, false, false, false, false, false, false, true  } },
   { "[0-9]+\\.msg$",  { true,  true,  false,  true, false, false, false, false, false, true  } },
   { "^[a-z]+\\.msg$", { false, false, false, false, false, true,  false, false, false, false } }
};
static const int NumTrials = sizeof(Trials)/sizeof(struct trial);

class OsFileIteratorTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsFileIteratorTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testIterate);
    CPPUNIT_TEST_SUITE_END();

    /** where all tests should r/w data */
    OsPath mRootPath;

public:

    /**
     * Create the TestFiles
     */
    void setUp()
    {
        OsTestUtilities::createTestDir(mRootPath);
        int i;
        for ( i= 0; i < NumTestFiles; i++ )
        {
           OsTestUtilities::createDummyFile(mRootPath+OsPath::separator+TestFiles[i], 100);
        }
    }

    /**
     * clean up all files and directories for this test
     */
    void tearDown()
    {
       // comment out to save test output
       OsTestUtilities::removeTestDir(mRootPath);
    }

    /**
     * Find the element in TestFiles that was matched, and record that
     * fact in the results array.
     * We do it this way because the order in which the matches will
     * occur is undefined.
     */
    void recordMatch( bool results[NumTestFiles], OsPath& matched )
    {
       int file;
       bool found;

       TRACE_TEST(("\n ####### found '%s'", matched.data()));
       for ( file = 0, found = false; file < NumTestFiles && ! found; file++ )
       {
          if ( matched == TestFiles[file] )
          {
             CPPUNIT_ASSERT_MESSAGE( "Found the same file twice.", results );
             results[file] = true;
             found = true;
          }
       }
       CPPUNIT_ASSERT_MESSAGE( "Found a file not in the test set.", found );
    }


    void testCreation()
    {
        // Example of bug XPL-12
        OsFileIterator *i = new OsFileIterator(mRootPath);
        delete i;
    }

    void testIterate()
    {
       OsFileIterator fi(mRootPath);
       OsPath entry;
       int trial;

       //
       for ( trial = 0; trial < NumTrials; trial++ )
       {
          TRACE_TEST(("\n\n############### filter '%s'", Trials[trial].regex));
          bool results[NumTestFiles];
          int i;
          // set result for each test file to not found
          for ( i = 0; i < NumTestFiles; i++ )
          {
             results[i] = false;
          }

          OsStatus status;
          int numFound;

          // loop over the iterator, recording what gets found
          for( status = fi.findFirst(entry, Trials[trial].regex, OsFileIterator::FILES)
                  ,numFound = 0;
               status == OS_SUCCESS && numFound <= NumTestFiles;
               status = fi.findNext(entry)
             )
          {
             numFound++;
             recordMatch( results, entry );
          }
          CPPUNIT_ASSERT( numFound <= NumTestFiles ); // test integrity check

          bool passed = true;
          // compare the results with what the trial definition expected
          for ( i = 0; i < NumTestFiles; i++ )
          {
             if ( results[i] != Trials[trial].expected[i] )
             {
                passed = false;
                printf( Trials[trial].expected[i]
                        ? "\n! Expression '%s' did not find '%s'\n"
                        : "\n! Expression '%s' should not have found '%s'\n",
                        Trials[trial].regex, TestFiles[i]
                   );
             }
          }
          CPPUNIT_ASSERT( passed );
       }
       TRACE_TEST(("\n"));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsFileIteratorTest);
