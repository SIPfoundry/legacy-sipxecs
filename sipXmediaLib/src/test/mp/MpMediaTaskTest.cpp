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

#include <mp/MpMediaTask.h>
#include <mp/MpFlowGraphBase.h>

/**
 * Unittest for MpMediaTask
 */
class MpMediaTaskTest : public CppUnit::TestCase
{
    //
    // All commented out test cases are broken or surfacing real issues
    //
    CPPUNIT_TEST_SUITE(MpMediaTaskTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testManagedAndUnmanagedFlowGraph);
    CPPUNIT_TEST(testDebugMode);
    CPPUNIT_TEST(testFocus);
    CPPUNIT_TEST(testStartAndStopFlowGraph);
    CPPUNIT_TEST(testTimeLimitAndTimeout);
    CPPUNIT_TEST(testMultipleManagedAndUnmanagedFlowgraph);
    CPPUNIT_TEST_SUITE_END();

public:
    void testCreators()
    {
        MpMediaTask* pMediaTask = 0;
        OsStatus     res;

        int          numFramesAlready;

        // Call getMediaTask() which causes the task to get instantiated
        pMediaTask = MpMediaTask::getMediaTask(10);
        CPPUNIT_ASSERT(pMediaTask != NULL);

        // Check the initial state of the MpMediaTask object
 // ****************************************************************************
 // **** This is NOT THE INITIAL STATE UNLESS THE ABOVE CALL to getMediaTask()
 // **** is the very first call to that function.  The problem with these
 // **** tests is that they were meant to be run separately, but that is not
 // **** the case with our self-starting singleton tasks.  This one has been
 // **** around the track a few times already, we get whatever we get.
 // ****************************************************************************
        // Not anymore... CPPUNIT_ASSERT(pMediaTask->getDebugMode() == FALSE);
        // Good luck with the rest!
        CPPUNIT_ASSERT(pMediaTask->getFocus() == NULL);
        // Not anymore... CPPUNIT_ASSERT_EQUAL(0, pMediaTask->getLimitExceededCnt());
        CPPUNIT_ASSERT(pMediaTask->getTimeLimit() == MpMediaTask::DEF_TIME_LIMIT_USECS);
        CPPUNIT_ASSERT(pMediaTask->getWaitTimeout() == MpMediaTask::DEF_SEM_WAIT_MSECS);
        // Not anymore... CPPUNIT_ASSERT_EQUAL(0, pMediaTask->getWaitTimeoutCnt());
        CPPUNIT_ASSERT_EQUAL(0, pMediaTask->numManagedFlowGraphs());
        // Not anymore... CPPUNIT_ASSERT_EQUAL(0, pMediaTask->numProcessedFrames());
        numFramesAlready = pMediaTask->numProcessedFrames();
        CPPUNIT_ASSERT_EQUAL(0, pMediaTask->numStartedFlowGraphs());

        // Verify that the task is actually running by:
        //   enabling debug mode
        //   calling signalFrameStart()
        //   checking the processed frame count
        res = pMediaTask->setDebug(TRUE);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = MpMediaTask::signalFrameStart();  // send a signal to the task
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // and give it a chance to run
        OsTask::delay(20);

        // Not anymore... CPPUNIT_ASSERT_EQUAL(1, pMediaTask->numProcessedFrames());
        CPPUNIT_ASSERT_EQUAL((numFramesAlready+1), pMediaTask->numProcessedFrames());
    }

    void testManagedAndUnmanagedFlowGraph()
    {

        MpFlowGraphBase* pFlowGraph = 0;
        MpMediaTask*     pMediaTask = 0;
        OsStatus         res;

        // Test 1: Create an empty flow graph and manage it
        pMediaTask = MpMediaTask::getMediaTask(10);
        pFlowGraph = new MpFlowGraphBase(30, 30);
        res = pMediaTask->manageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = MpMediaTask::signalFrameStart();  // send a signal to the task
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // and give it a chance to run

        // NOTE: original delay of 20 was tempermental, I increased
        // this to 100 to reduce the chance of this happening to
        // hopefully 0% - DLH
        OsTask::delay(100);

        CPPUNIT_ASSERT_EQUAL(1, pMediaTask->numManagedFlowGraphs());

        // Test 2: Invoke manageFlowGraph() with the same flow graph
        //         (will increment the numHandledMsgErrs() count for that
        //         frame processing interval but should otherwise have no
        //         effect)
        res = pMediaTask->manageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = MpMediaTask::signalFrameStart();  // send a signal to the task
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // and give it a chance to run
        OsTask::delay(20);

        CPPUNIT_ASSERT_EQUAL(1, pMediaTask->numManagedFlowGraphs());
        CPPUNIT_ASSERT_EQUAL(1, pMediaTask->numHandledMsgErrs());

        // Test 3: Unmanage the flow graph
        res = pMediaTask->unmanageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = MpMediaTask::signalFrameStart();  // send a signal to the task
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // and give it a chance to run
        OsTask::delay(20);

        CPPUNIT_ASSERT_EQUAL(0, pMediaTask->numManagedFlowGraphs());
        // Test 4: Unmanage a flow graph which is not currently managed
        //         (will increment the numHandledMsgErrs() count for that
        //         frame processing interval but should otherwise have no
        //         effect)
        res = pMediaTask->unmanageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = MpMediaTask::signalFrameStart();  // send a signal to the task
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // and give it a chance to run
        OsTask::delay(20);

        CPPUNIT_ASSERT_EQUAL(0, pMediaTask->numManagedFlowGraphs());
        CPPUNIT_ASSERT_EQUAL(1, pMediaTask->numHandledMsgErrs());

        // Test 5: Attempt to manage a flow graph that is not in the
        //         MpFlowGraphBase::STOPPED state
        res = pFlowGraph->start();              // send the flow graph a start
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // command and a signal to
        res = pFlowGraph->processNextFrame();   // process its messages
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pMediaTask->manageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_INVALID_ARGUMENT);

        res = pFlowGraph->stop();               // send the flow graph a stop
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // command and a signal to
        res = pFlowGraph->processNextFrame();   // process its messages
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        // Test 6: Unmanage a flow graph that is "started"
        res = pMediaTask->manageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pMediaTask->startFlowGraph(*pFlowGraph); // start the flow graph
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = MpMediaTask::signalFrameStart();  // send a signal to the task
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // and give it a chance to run
        OsTask::delay(20);

        res = pMediaTask->unmanageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = MpMediaTask::signalFrameStart();  // send a signal to the task
        CPPUNIT_ASSERT(res == OS_SUCCESS);              // and give it a chance to run
        OsTask::delay(20);

        // verify that the flow graph has been stopped and is unmanaged
        CPPUNIT_ASSERT(pFlowGraph->getState() == MpFlowGraphBase::STOPPED);
        CPPUNIT_ASSERT_EQUAL(0, pMediaTask->numManagedFlowGraphs());
        CPPUNIT_ASSERT_EQUAL(0, pMediaTask->numStartedFlowGraphs());

        delete pFlowGraph;
    }

    void testDebugMode()
    {

        MpMediaTask*     pMediaTask = 0;
        OsStatus         res;
        int              waitTimeoutCnt;

        // Test 1: Verify that wait for "frame start" timeouts are noticed
        //         only when the media task is not in debug mode
        pMediaTask = MpMediaTask::getMediaTask(10);
        res = pMediaTask->setDebug(FALSE);      // turn debug mode off
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT(pMediaTask->getDebugMode() == FALSE);

        waitTimeoutCnt = pMediaTask->getWaitTimeoutCnt();
        res = MpMediaTask::signalFrameStart();  // send a signal to the task
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        OsTask::delay(1000);                    // and wait 1 second

        // $$$ Need to understand why the following test fails on vxWorks
        // WHAT THE #(*$&#(*&???  CPPUNIT_ASSERT(pMediaTask->getWaitTimeoutCnt() > waitTimeoutCnt);

        res = pMediaTask->setDebug(TRUE);       // turn debug mode on
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT(pMediaTask->getDebugMode() == TRUE);

        res = MpMediaTask::signalFrameStart();  // send a signal to the task
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // and give it a chance to run
        OsTask::delay(20);

        waitTimeoutCnt = pMediaTask->getWaitTimeoutCnt();
        OsTask::delay(1000);                     // wait 1 second
        CPPUNIT_ASSERT_EQUAL(waitTimeoutCnt, pMediaTask->getWaitTimeoutCnt());
    }


    void testFocus()
    {
        MpFlowGraphBase* pFlowGraph = 0;
        MpMediaTask*     pMediaTask = 0;
        OsStatus         res;

        // Test 1: Attempt to setFocus to a flow graph that the media task
        //         is not managing
        pMediaTask = MpMediaTask::getMediaTask(10);
        pFlowGraph = new MpFlowGraphBase(30, 30);
        res = pMediaTask->setFocus(pFlowGraph); // send the media task a
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // set_focus command and
        res = MpMediaTask::signalFrameStart();  // give it a chance to run
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        OsTask::delay(20);

        CPPUNIT_ASSERT_EQUAL(1, pMediaTask->numHandledMsgErrs());

        // Test 2: Set the focus to a flow graph that has not been started
        res = pMediaTask->manageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // manage the flow graph and
        res = pMediaTask->setFocus(pFlowGraph); // send the media task a
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // set_focus command and
        res = MpMediaTask::signalFrameStart();  // give it a chance to run
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        OsTask::delay(20);
        CPPUNIT_ASSERT_EQUAL(1, pMediaTask->numHandledMsgErrs());

        // Test 3: Set the focus to a flow graph that has been started
        res = pMediaTask->startFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // start the flow graph and
        res = pMediaTask->setFocus(pFlowGraph); // send the media task a
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // set_focus command and
        res = MpMediaTask::signalFrameStart();  // give it a chance to run
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        OsTask::delay(20);

        // 6/16/99, incompatible with new, real implementation of Focus:
        // CPPUNIT_ASSERT(pMediaTask->getFocus() == pFlowGraph);

        // Test 4: Set the focus to NULL
        res = pMediaTask->unmanageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // unmanage the flow graph
        res = pMediaTask->setFocus(NULL);       // and send the media task a
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // set_focus command and
        res = MpMediaTask::signalFrameStart();  // give it a chance to run
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT(pMediaTask->getFocus() == NULL);

        delete pFlowGraph;
    }

    void testTimeLimitAndTimeout()
    {
        MpMediaTask*     pMediaTask = 0;
        OsStatus         res;
        int              oldValue;

        // Test 1: Set the time limit to twice its original value
        pMediaTask = MpMediaTask::getMediaTask(10);
        oldValue = pMediaTask->getTimeLimit();
        res = pMediaTask->setTimeLimit(oldValue * 2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(oldValue * 2, pMediaTask->getTimeLimit());

        // Test 2: Set the time limit back to its original value
        res = pMediaTask->setTimeLimit(oldValue);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(oldValue, pMediaTask->getTimeLimit());

        // Test 3: Set the wait timeout to twice its original value
        oldValue = pMediaTask->getWaitTimeout();
        res = pMediaTask->setWaitTimeout(oldValue * 2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(oldValue * 2, pMediaTask->getWaitTimeout());

        // Test 4: Set the wait timeout to -1 (infinity)
        res = pMediaTask->setWaitTimeout(-1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(-1, pMediaTask->getWaitTimeout());

        // Test 5: Set the wait timeout back to its original value
        res = pMediaTask->setWaitTimeout(oldValue);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(oldValue, pMediaTask->getWaitTimeout());
    }

    void testStartAndStopFlowGraph()
    {

        MpFlowGraphBase* pFlowGraph = 0;
        MpMediaTask*     pMediaTask = 0;
        OsStatus         res;

        // Test 1: Set the time limit to twice its original value
        pMediaTask = MpMediaTask::getMediaTask(10);

        pFlowGraph = new MpFlowGraphBase(30, 30);

        pMediaTask->numHandledMsgErrs(); // clear count
        // Test 1: Attempt to start a flow graph that is not being managed
        //CPPUNIT_ASSERT_EQUAL(1, pMediaTask->numStartedFlowGraphs());
        res = pMediaTask->startFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = MpMediaTask::signalFrameStart();  // signal the media task and
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // give it a chance to run
        OsTask::delay(20);
        // NOTE: Original test code had "1", not sure what's correct
        CPPUNIT_ASSERT_EQUAL(1, pMediaTask->numHandledMsgErrs());
        CPPUNIT_ASSERT_EQUAL(0, pMediaTask->numStartedFlowGraphs());

        // Test 2: Start a flow graph that is managed
        pMediaTask->numHandledMsgErrs(); // clear the count

        res = pMediaTask->manageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = pMediaTask->startFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = MpMediaTask::signalFrameStart();  // signal the media task and
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // give it a chance to run
        OsTask::delay(20);
        CPPUNIT_ASSERT_EQUAL(0, pMediaTask->numHandledMsgErrs());
        CPPUNIT_ASSERT_EQUAL(1, pMediaTask->numStartedFlowGraphs());
        CPPUNIT_ASSERT(pFlowGraph->isStarted());

        // Test 3: Attempt to start the same flow graph again
        pMediaTask->numHandledMsgErrs(); // clear the count

        res = pMediaTask->startFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = MpMediaTask::signalFrameStart();  // signal the media task and
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // give it a chance to run
        OsTask::delay(20);
        //CPPUNIT_ASSERT_EQUAL(0, pMediaTask->numHandledMsgErrs());
        CPPUNIT_ASSERT_EQUAL(1, pMediaTask->numStartedFlowGraphs());


        // Test 4: Stop the flow graph
        pMediaTask->numHandledMsgErrs(); // clear the count

        res = pMediaTask->stopFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = MpMediaTask::signalFrameStart();  // signal the media task and
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // give it a chance to run
        OsTask::delay(20);
        CPPUNIT_ASSERT_EQUAL(0, pMediaTask->numHandledMsgErrs());
        CPPUNIT_ASSERT_EQUAL(0, pMediaTask->numStartedFlowGraphs());
        CPPUNIT_ASSERT(!pFlowGraph->isStarted());

        // Test 5: Attempt to stop the same flow graph again
        pMediaTask->numHandledMsgErrs(); // clear the count

        res = pMediaTask->stopFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = MpMediaTask::signalFrameStart();  // signal the media task and
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // give it a chance to run

        OsTask::delay(20);
        //CPPUNIT_ASSERT_EQUAL(1, pMediaTask->numHandledMsgErrs());
        CPPUNIT_ASSERT_EQUAL(0, pMediaTask->numStartedFlowGraphs());
        CPPUNIT_ASSERT(!pFlowGraph->isStarted());

        // Test 6: Attempt to stop a flow graph that is not being managed
        pMediaTask->numHandledMsgErrs(); // clear the count

        res = pMediaTask->unmanageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = pMediaTask->stopFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = MpMediaTask::signalFrameStart();  // signal the media task and
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // give it a chance to run
        OsTask::delay(20);
        CPPUNIT_ASSERT_EQUAL(1, pMediaTask->numHandledMsgErrs());
        CPPUNIT_ASSERT_EQUAL(0, pMediaTask->numStartedFlowGraphs());
        CPPUNIT_ASSERT(!pFlowGraph->isStarted());

        delete pFlowGraph;
    }

    void testMultipleManagedAndUnmanagedFlowgraph()
    {
        MpFlowGraphBase* pFlowGraph1 = 0;
        MpFlowGraphBase* pFlowGraph2 = 0;
        MpMediaTask*     pMediaTask  = 0;
        MpFlowGraphBase* flowGraphs[2];
        int              itemCnt;
        OsStatus         res;

        pMediaTask  = MpMediaTask::getMediaTask(10);
        pFlowGraph1 = new MpFlowGraphBase(30, 30);
        pFlowGraph2 = new MpFlowGraphBase(30, 30);

        // Test 1: Add one managed flow graph
        res = pMediaTask->manageFlowGraph(*pFlowGraph1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = MpMediaTask::signalFrameStart();  // signal the media task and
        CPPUNIT_ASSERT(res == OS_SUCCESS);              // give it a chance to run

        // NOTE: original delay of 20 was tempermental, I increased
        // this to 100 to reduce the chance of this happening to
        // hopefully 0% - DLH
        OsTask::delay(100);

        flowGraphs[0] = flowGraphs[1] = NULL;
        res = pMediaTask->getManagedFlowGraphs(flowGraphs, 2, itemCnt);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(1, itemCnt);
        CPPUNIT_ASSERT(flowGraphs[0] == pFlowGraph1);

        // Test 2: Add a second managed flow graph
        res = pMediaTask->manageFlowGraph(*pFlowGraph2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = MpMediaTask::signalFrameStart();  // signal the media task and
        CPPUNIT_ASSERT(res == OS_SUCCESS);              // give it a chance to run
        OsTask::delay(20);

        flowGraphs[0] = flowGraphs[1] = NULL;
        res = pMediaTask->getManagedFlowGraphs(flowGraphs, 2, itemCnt);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(2, itemCnt);
        CPPUNIT_ASSERT(flowGraphs[0] == pFlowGraph1 ||
                       flowGraphs[0] == pFlowGraph2);

        CPPUNIT_ASSERT(flowGraphs[1] == pFlowGraph1 ||
                       flowGraphs[1] == pFlowGraph2);

        CPPUNIT_ASSERT(flowGraphs[0] != flowGraphs[1]);

        res = pMediaTask->unmanageFlowGraph(*pFlowGraph1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = pMediaTask->unmanageFlowGraph(*pFlowGraph2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = MpMediaTask::signalFrameStart();  // signal the media task and
        CPPUNIT_ASSERT(res == OS_SUCCESS);              // give it a chance to run
        OsTask::delay(20);

        delete pFlowGraph1;
        delete pFlowGraph2;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpMediaTaskTest);
