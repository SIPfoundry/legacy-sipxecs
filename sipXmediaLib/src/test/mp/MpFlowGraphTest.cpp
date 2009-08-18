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

#include <mp/MpFlowGraphBase.h>
#include <mp/MpTestResource.h>

/**
 * Unittest for MpFlowGraph
 */
class MpFlowGraphTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(MpFlowGraphTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testLinkAndResource);
    CPPUNIT_TEST(testDestroyResources);
    CPPUNIT_TEST(testEnableDisable);
    CPPUNIT_TEST(testSamplesPerFrameAndSec);
    CPPUNIT_TEST(testStartAndStop);
    CPPUNIT_TEST(testAccessors);
    CPPUNIT_TEST(testProcessNextFrame);
    CPPUNIT_TEST_SUITE_END();


public:
    void testCreators()
    {
        MpFlowGraphBase*    pFlowGraph = 0;
        MpTestResource* pResource1 = 0;
        MpTestResource* pResource2 = 0;
        OsStatus        res;

        // verify that we can create and delete MpFlowGraphBase objects
        pFlowGraph = new MpFlowGraphBase(80, 8000);

        // verify that the initial state information is sensible
        CPPUNIT_ASSERT(pFlowGraph->getState() == MpFlowGraphBase::STOPPED);
        CPPUNIT_ASSERT(pFlowGraph->numLinks() == 0);
        CPPUNIT_ASSERT(pFlowGraph->numFramesProcessed() == 0);
        CPPUNIT_ASSERT(pFlowGraph->numResources() == 0);
        CPPUNIT_ASSERT(!pFlowGraph->isStarted());

        delete pFlowGraph;

        // when we have a flow graph that contains resources and links,
        // verify that destroying the flow graph also gets rid of the resources
        // and links.
        pFlowGraph = new MpFlowGraphBase(80, 8000);
        pResource1 = new MpTestResource("test1", 1, 1, 1, 1);
        pResource2 = new MpTestResource("test2", 1, 1, 1, 1);

        res = pFlowGraph->addResource(*pResource1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->addResource(*pResource2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->addLink(*pResource1, 0, *pResource2, 0);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        delete pFlowGraph;
    }

    void testLinkAndResource()
    {
        MpFlowGraphBase*    pFlowGraph = 0;
        MpTestResource* pResource1 = 0;
        MpTestResource* pResource2 = 0;
        OsStatus        res;

        // ====== Test the addLink(), addResource(),
        //                 removeLink() and removeResource() methods ======
        pFlowGraph = new MpFlowGraphBase(80, 8000);
        pResource1 = new MpTestResource("test1", 2, 2, 2, 2);
        pResource2 = new MpTestResource("test2", 2, 2, 2, 2);

        res = pFlowGraph->addResource(*pResource1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->addResource(*pResource2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        // connect pResource1 output port 0 --> pResource2 input port 0
        res = pFlowGraph->addLink(*pResource1, 0, *pResource2, 0);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        // try connecting an invalid input port to a valid output port
        res = pFlowGraph->addLink(*pResource1, 2, *pResource2, 1);
        CPPUNIT_ASSERT(res == OS_INVALID_ARGUMENT);

        // try connecting a valid input port to an invalid output port
        res = pFlowGraph->addLink(*pResource1, 1, *pResource2, 2);
        CPPUNIT_ASSERT(res == OS_INVALID_ARGUMENT);

        // try removing a link on a port that doesn't exist
        res = pFlowGraph->removeLink(*pResource1, 2);
        CPPUNIT_ASSERT(res == OS_INVALID_ARGUMENT);

        // remove the link and resources
        res = pFlowGraph->removeLink(*pResource1, 0);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->removeResource(*pResource1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->removeResource(*pResource2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        delete pResource1;
        delete pResource2;
        delete pFlowGraph;
    }

    void testDestroyResources()
    {
        MpFlowGraphBase*    pFlowGraph = 0;
        MpTestResource* pResource1 = 0;
        MpTestResource* pResource2 = 0;
        OsStatus        res;

        pFlowGraph = new MpFlowGraphBase(80, 8000);
        pResource1 = new MpTestResource("test1", 2, 2, 2, 2);
        pResource2 = new MpTestResource("test2", 2, 2, 2, 2);

        res = pFlowGraph->addResource(*pResource1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->addResource(*pResource2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->addLink(*pResource1, 0, *pResource2, 0);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->destroyResources();
        CPPUNIT_ASSERT((res == OS_SUCCESS) &&
          (pFlowGraph->numResources() == 0) &&
          (pFlowGraph->numLinks() == 0));

        delete pFlowGraph;
    }

    void testEnableDisable()
    {
        MpFlowGraphBase*    pFlowGraph = 0;
        MpTestResource* pResource1 = 0;
        MpTestResource* pResource2 = 0;
        OsStatus        res;

        pFlowGraph = new MpFlowGraphBase(80, 8000);
        pResource1 = new MpTestResource("test1", 2, 2, 2, 2);
        pResource2 = new MpTestResource("test2", 2, 2, 2, 2);

        res = pFlowGraph->addResource(*pResource1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->addResource(*pResource2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        CPPUNIT_ASSERT((pResource1->isEnabled() == FALSE) &&
            (pResource2->isEnabled() == FALSE));

        // enable all of the resources in the flow graph
        res = pFlowGraph->enable();
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        CPPUNIT_ASSERT(pResource1->isEnabled() && pResource2->isEnabled());

        // now disable all of the the resources in the flow graph
        res = pFlowGraph->disable();
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        CPPUNIT_ASSERT((pResource1->isEnabled() == FALSE) &&
                       (pResource2->isEnabled() == FALSE));

        delete pFlowGraph;
    }

    /**
     * FAILS : Segmention fault
     */
    void testProcessNextFrame()
    {
       // Set up a flow graph with two resources (resource1 and resource2). Both
       // resources have 4 inputs and 4 outputs.  All four outputs of resource1
       // are connected to the corresponding inputs of resource2.  The resources
       // are further configured to behave as follows for each frame processing
       // interval.
       //
       // Resource 1:                   |   Resource 2:
       //   Creates output buffers on   |     Processes input buffers received on
       //   output ports 0, 2 and 3.    |     input ports 0, 1 and 2.
       //
       //   resource1   Output 0        -->   Input 0
       //   ignores     Output 1 (NULL) -->   Input 1
       //   its         Output 2        -->   Input 2
       //   inputs      Output 3        -->   Input 3 (not processed)
       //
       // The net result is that each frame time, resource2 should receive
       // non-NULL buffers on input ports 0, 2 and 3.  Since resource2 is not
       // processing input buffers on input port 3, for each frame, the old
       // buffer on input port 3 will be discarded to make way for a new buffer.
        MpFlowGraphBase*    pFlowGraph = 0;
        MpTestResource* pResource1 = 0;
        MpTestResource* pResource2 = 0;
        OsStatus        res;

        mpStartUp(8000, 80, 6*10, 0);

        pFlowGraph = new MpFlowGraphBase(80, 8000);
        pResource1 = new MpTestResource("resource1", 4, 4, 4, 4);
        pResource2 = new MpTestResource("resource2", 4, 4, 4, 4);

        res = pFlowGraph->addResource(*pResource1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->addResource(*pResource2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->addLink(*pResource1, 0, *pResource2, 0);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->addLink(*pResource1, 1, *pResource2, 1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->addLink(*pResource1, 2, *pResource2, 2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->addLink(*pResource1, 3, *pResource2, 3);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        // For resource1, create new buffers on output ports 0, 2 and 3 and
        // ignore all input buffers (Note: there shouldn't be any)
        pResource1->setGenOutBufMask(0xd);
        pResource1->setProcessInBufMask(0x0);

        // For resource2, process input buffers that arrive input ports 0, 1 and 2.
        pResource2->setGenOutBufMask(0x0);
        pResource2->setProcessInBufMask(0x7);

        CPPUNIT_ASSERT(pResource1->numFramesProcessed() == 0);
        CPPUNIT_ASSERT(pResource2->numFramesProcessed() == 0);

        // Enable the flow graph
        res = pFlowGraph->enable();
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        // Start the flow graph
        res = pFlowGraph->start();
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        // Process two frames
        res = pFlowGraph->processNextFrame();
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = pFlowGraph->processNextFrame();
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        for (int i = 0; i < 3; i++)
        {
            CPPUNIT_ASSERT((pResource1->mLastDoProcessArgs.inBufs[i]  == NULL) &&
                           (pResource1->mLastDoProcessArgs.outBufs[i] == NULL) &&
                           (pResource2->mLastDoProcessArgs.inBufs[i] == NULL) &&
                           (pResource2->mLastDoProcessArgs.outBufs[i] == NULL));
        }
        CPPUNIT_ASSERT((pResource1->numFramesProcessed() == 2) &&
                       (pResource1->mLastDoProcessArgs.inBufsSize == 4) &&
                       (pResource1->mLastDoProcessArgs.outBufsSize == 4) &&
                       (pResource2->numFramesProcessed() == 2));

        // Stop the flow graph
        res = pFlowGraph->stop();
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        // Request processing of another frame so that the STOP_FLOWGRAPH
        // message gets handled
        res = pFlowGraph->processNextFrame();
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        delete pFlowGraph;
    }

    void testSamplesPerFrameAndSec()
    {
        MpFlowGraphBase*    pFlowGraph = 0;
        MpTestResource* pResource1 = 0;
        OsStatus        res;

        pFlowGraph = new MpFlowGraphBase(80, 8000);
        pResource1 = new MpTestResource("test1", 2, 2, 2, 2);

        CPPUNIT_ASSERT_EQUAL(80, pFlowGraph->getSamplesPerFrame());
        CPPUNIT_ASSERT_EQUAL(8000, pFlowGraph->getSamplesPerSec());


        res = pFlowGraph->addResource(*pResource1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        // Call processFrame() on the resource and look at the initial settings
        pResource1->processFrame();
        CPPUNIT_ASSERT_EQUAL(80, pResource1->mLastDoProcessArgs.samplesPerFrame);

        CPPUNIT_ASSERT_EQUAL(8000, pResource1->mLastDoProcessArgs.samplesPerSecond);

        res = pFlowGraph->setSamplesPerFrame(160);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = pFlowGraph->setSamplesPerSec(32000);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        CPPUNIT_ASSERT_EQUAL(160, pFlowGraph->getSamplesPerFrame());
        CPPUNIT_ASSERT_EQUAL(32000, pFlowGraph->getSamplesPerSec());

        // Call processFrame() on the resource and look at the revised settings
        pResource1->processFrame();
        CPPUNIT_ASSERT_EQUAL(160, pResource1->mLastDoProcessArgs.samplesPerFrame);
        CPPUNIT_ASSERT_EQUAL(32000, pResource1->mLastDoProcessArgs.samplesPerSecond);

        delete pFlowGraph;
    }

    void testStartAndStop()
    {
        MpFlowGraphBase*    pFlowGraph = 0;
        OsStatus        res;

        pFlowGraph = new MpFlowGraphBase(80, 8000);

        CPPUNIT_ASSERT(!pFlowGraph->isStarted());  // verify the flow graph is not STARTED
        CPPUNIT_ASSERT(MpFlowGraphBase::STOPPED == pFlowGraph->getState());

        res = pFlowGraph->start();         // now start it
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = pFlowGraph->processNextFrame();
        CPPUNIT_ASSERT((res == OS_SUCCESS) && pFlowGraph->isStarted());
        CPPUNIT_ASSERT(MpFlowGraphBase::STARTED == pFlowGraph->getState());

        res = pFlowGraph->stop();          // now stop it again
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = pFlowGraph->processNextFrame();
        CPPUNIT_ASSERT((res == OS_SUCCESS) && !pFlowGraph->isStarted());
        CPPUNIT_ASSERT(MpFlowGraphBase::STOPPED == pFlowGraph->getState());

        delete pFlowGraph;
    }



    // lookupResource(), numLinks(), numFramesProcessed() and numResources()
    void testAccessors()
    {
        MpFlowGraphBase*    pFlowGraph = 0;
        MpTestResource* pResource1 = 0;
        MpTestResource* pResource2 = 0;
        MpResource*     pLookupRes = 0;
        OsStatus        res;


        pFlowGraph = new MpFlowGraphBase(80, 8000);
        CPPUNIT_ASSERT(pFlowGraph->numResources() == 0);
        CPPUNIT_ASSERT(pFlowGraph->numLinks() == 0);
        CPPUNIT_ASSERT(pFlowGraph->numFramesProcessed() == 0);

        pResource1 = new MpTestResource("resource1", 4, 4, 4, 4);
        pResource2 = new MpTestResource("resource2", 4, 4, 4, 4);

        res = pFlowGraph->addResource(*pResource1);
        CPPUNIT_ASSERT((res == OS_SUCCESS) && (pFlowGraph->numResources() == 1));

        res = pFlowGraph->addResource(*pResource2);
        CPPUNIT_ASSERT((res == OS_SUCCESS) && (pFlowGraph->numResources() == 2));

        res = pFlowGraph->lookupResource("resource1", pLookupRes);
        CPPUNIT_ASSERT((res == OS_SUCCESS) && (pLookupRes == pResource1));

        res = pFlowGraph->lookupResource("resource2", pLookupRes);
        CPPUNIT_ASSERT((res == OS_SUCCESS) && (pLookupRes == pResource2));

        res = pFlowGraph->lookupResource("unknown", pLookupRes);
        CPPUNIT_ASSERT(res == OS_NOT_FOUND);

        res = pFlowGraph->addLink(*pResource1, 0, *pResource2, 0);
        CPPUNIT_ASSERT((res == OS_SUCCESS) && (pFlowGraph->numLinks() == 1));

        res = pFlowGraph->addLink(*pResource1, 1, *pResource2, 1);
        CPPUNIT_ASSERT((res == OS_SUCCESS) && (pFlowGraph->numLinks() == 2));

        res = pFlowGraph->processNextFrame();
        CPPUNIT_ASSERT((res == OS_SUCCESS) && (pFlowGraph->numFramesProcessed() == 1));

        res = pFlowGraph->processNextFrame();
        CPPUNIT_ASSERT((res == OS_SUCCESS) && (pFlowGraph->numFramesProcessed() == 2));

        res = pFlowGraph->removeLink(*pResource1, 0);
        CPPUNIT_ASSERT((res == OS_SUCCESS) && (pFlowGraph->numLinks() == 1));

        res = pFlowGraph->removeLink(*pResource1, 1);
        CPPUNIT_ASSERT((res == OS_SUCCESS) && (pFlowGraph->numLinks() == 0));

        res = pFlowGraph->removeResource(*pResource2);
        CPPUNIT_ASSERT((res == OS_SUCCESS) && (pFlowGraph->numResources() == 1));

        res = pFlowGraph->removeResource(*pResource1);
        CPPUNIT_ASSERT((res == OS_SUCCESS) && (pFlowGraph->numResources() == 0));

        delete pResource1;
        delete pResource2;
        delete pFlowGraph;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpFlowGraphTest);
