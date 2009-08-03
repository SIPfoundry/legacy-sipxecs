//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include "sipXtapiTest.h"
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "EventRecorder.h"
#include "EventValidator.h"
#include "os/OsBSem.h"

#ifdef PURIFY
    #include "pure.h"
#endif
SIPX_INST g_hInst = NULL ;
EventRecorder g_recorder ;
EventRecorder g_lineRecorder;

SIPX_INST g_hInst2 = NULL ;
EventRecorder g_recorder2 ;

SIPX_INST g_hInst3 = NULL ;
EventRecorder g_recorder3 ;

SIPX_INST g_hInst4 = NULL ;
EventRecorder g_recorder4 ;

SIPX_INST g_hInst5 = NULL ;


SIPX_CALL ghCallHangup = 0;


int main(int argc, char* argv[])
{
    enableConsoleOutput(FALSE) ;

    // Get the top level suite from the registry
    CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

    // Adds the test to the list of tests to run
    CppUnit::TextUi::TestRunner runner ;
    runner.addTest(suite) ;

    // Change the default outputter to a compiler error format outputter
    runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cerr)) ;

    // Run the tests.
    bool wasSuccessful = runner.run() ;

    // Return error code 1 if one of the tests failed.
    return wasSuccessful ? 0 : 1 ;
}


CPPUNIT_TEST_SUITE_REGISTRATION( sipXtapiTestSuite );

void sipXtapiTestSuite::testNothing()
{
    // Effectively does nothing; however, prime memory leak
    // detection.
    checkForLeaks() ;
}


sipXtapiTestSuite::sipXtapiTestSuite()
{

}

OsBSem suiteLock(OsBSem::Q_PRIORITY, OsBSem::FULL);
void sipXtapiTestSuite::setUp()
{
#ifdef _WIN32
#ifdef SIPX_TEST_FOR_MEMORY_LEAKS
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT);

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF) ;
    _CrtMemCheckpoint( &msBeforeTest );
#endif
#endif

    suiteLock.acquire();
    sipxConfigSetLogLevel(LOG_LEVEL_DEBUG);
    sipxConfigSetLogFile("sipXtapiTests.txt") ;

    if (g_hInst == NULL)
    {
	    sipxInitialize(&g_hInst, 8000, 8000, 8001, 8050, 32, NULL, "127.0.0.1") ;
    }

    if (g_hInst2 == NULL)
    {
	    sipxInitialize(&g_hInst2, 9100, 9100, 9101, 9050, 32, NULL, "127.0.0.1") ;
    }

    if (g_hInst3 == NULL)
    {
	    sipxInitialize(&g_hInst3, 10000, 10000, 10001, 10050, 32, NULL, "127.0.0.1") ;
    }

    if (g_hInst4 == NULL)
    {
	    sipxInitialize(&g_hInst4, 12070, 12070, 12071, 12050, 32, NULL, "127.0.0.1") ;
    }


    //if (g_hInst5 == NULL)
    //{
        //sipxInitialize(&g_hInst5, 5060, 5060, 5061, 13000, 32) ;
    //}

#ifdef PURIFY
    PurifyClearLeaks();
#endif
    suiteLock.release();
}

void sipXtapiTestSuite::tearDown()
{
#ifdef _WIN32
#ifdef SIPX_TEST_FOR_MEMORY_LEAKS
    static bool bFirstRun = true ;
#endif
#endif
    SIPX_RESULT rc ;

    suiteLock.acquire();

#ifdef PURIFY
    Sleep(250000);
    PurifyNewLeaks();
#endif

    if (g_hInst != NULL)
    {
        rc = sipxUnInitialize(g_hInst);
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        g_hInst = NULL;
    }

    if (g_hInst2 != NULL)
    {
        rc = sipxUnInitialize(g_hInst2);
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        g_hInst2 = NULL;
    }

    if (g_hInst3 != NULL)
    {
        rc = sipxUnInitialize(g_hInst3);
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        g_hInst3 = NULL;
    }

    if (g_hInst4 != NULL)
    {
        rc = sipxUnInitialize(g_hInst4);
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        g_hInst4 = NULL;
    }

    if (g_hInst5 != NULL)
    {
        rc = sipxUnInitialize(g_hInst5);
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        g_hInst5 = NULL;
    }


    suiteLock.release();

#ifdef _WIN32
#ifdef SIPX_TEST_FOR_MEMORY_LEAKS
    if (bFirstRun == false)
    {
        _CrtMemCheckpoint( &msAfterTest );
        _CrtMemState diff ;

        if (_CrtMemDifference( &diff, &msBeforeTest, &msAfterTest))
        {
            _CrtMemDumpStatistics( &diff );
            _CrtMemDumpAllObjectsSince(&msBeforeTest) ;
        }
    }
    else
    {
        bFirstRun = false ;
    }
#endif
#endif

}

void sipXtapiTestSuite::checkForLeaks()
{
    checkForCallLeaks(g_hInst) ;
    checkForCallLeaks(g_hInst2) ;
    checkForCallLeaks(g_hInst3) ;
    sipxCheckForHandleLeaks() ;
}

#define MAX_CALL_CHECK  16
void sipXtapiTestSuite::checkForCallLeaks(SIPX_INST hInst)
{
    SIPX_RESULT rc ;
    UtlString callIds[MAX_CALL_CHECK] ;
    int numCalls ;

    rc = sipxGetActiveCallIds(hInst, MAX_CALL_CHECK, numCalls, callIds) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
    if (rc == SIPX_RESULT_SUCCESS)
    {
        if (numCalls != 0)
        {
            printf("Call leak(s) detected (%d):\n", numCalls) ;
            for (int i=0; i<numCalls; i++)
            {
                printf("\tCallId=%s\n", callIds[i].data()) ;
            }

            CPPUNIT_ASSERT_EQUAL(numCalls, 0) ;
        }
    }
}
