/*
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */


#include <sys/time.h>

#include <sipxunit/TestUtilities.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <os/OsMsgQ.h>
#include <os/OsLoggerHelper.h>

#include <time.h>
#include <string.h>

// msgSubType for the OsMsgTest implementation of OsMsg
int OS_MSGQS_EVENT_TEST = (OsMsg::USER_START);

class OsMsgQSharedTest : public CppUnit::TestCase
{

    // Implementation of a OsMsg for testing. This actually sends
    // a string which can be later retrieved from the queue and checked
    // as a validation that a timer fired.
    class OsMsgTest : public OsMsg
    {
       /* //////////////////////////// PUBLIC //////////////////////////////////// */
    public:

       /* ============================ CREATORS ================================== */

       OsMsgTest(const UtlString& str) :
           OsMsg(OS_EVENT, OS_MSGQS_EVENT_TEST),
           _str(str)
        {

        }

       virtual ~OsMsgTest() {}

       /* ============================ MANIPULATORS ============================== */

       virtual OsMsg* createCopy(void) const
        {
           return new OsMsgTest(_str);
        };
       //Create a copy of this msg object (which may be of a derived type)

       /* ============================ ACCESSORS ================================= */


       // Get pointer to the handle value.
       UtlString* getStr()
      {
         return &_str;
      }
       /* ============================ INQUIRY =================================== */

       /* //////////////////////////// PROTECTED ///////////////////////////////// */
    protected:

       /* //////////////////////////// PRIVATE /////////////////////////////////// */
    private:

       OsMsgTest(const OsMsgTest& rOsMsgTest);
       //Copy constructor (not implemented for this class)

       OsMsgTest& operator=(const OsMsgTest& rhs);
       //Assignment operator (not implemented for this class)

       UtlString _str;
    };

    CPPUNIT_TEST_SUITE(OsMsgQSharedTest);

    //All unit tests are listed here.

    CPPUNIT_TEST(testDefaultQueuePreference);
    CPPUNIT_TEST(testDefaultLimitedQueue);
    CPPUNIT_TEST(testCustomLimitedQueue);
    CPPUNIT_TEST(testDefaultUnLimitedQueue);
    CPPUNIT_TEST(testCustomUnLimitedQueue);

    CPPUNIT_TEST_SUITE_END();

public:

    OsMsgQSharedTest()
    {
    }

    void setUp()
    {
        //nothing to do
    }

    void tearDown()
    {
        //nothing to do
    }

    /// Verifies if a timer fired as expected
    void checkReceive(
            OsMsgQ* q, // queue where the message should have arrived
            const OsTime& t, // additional time to wait for the message
            const UtlString& expectedStr, // the expected contents of the message
            int expectedRet // the expected return code of this function
            )
    {
        OsMsg* recvMsg = NULL;
        OsStatus ret = OS_SUCCESS;

        ret = q->receive(recvMsg, t);
        CPPUNIT_ASSERT(expectedRet == ret);

        if (OS_SUCCESS == ret)
        {
            CPPUNIT_ASSERT(recvMsg);
        }

        if (OS_SUCCESS == ret && recvMsg)
        {
            ret = OS_FAILED;

            int msgType = recvMsg->getMsgType();
            int msgSubType = recvMsg->getMsgSubType();
            if(msgType == OsMsg::OS_EVENT &&
                    msgSubType == OS_MSGQS_EVENT_TEST)
            {
                UtlString *recvStr = ((OsMsgTest*)recvMsg)->getStr();
                CPPUNIT_ASSERT(expectedStr == *recvStr);

                if (expectedStr == *recvStr)
                {
                    ret = OS_SUCCESS;
                }
            }

            // Msg is owned by the caller of the recv so it should be deleted
            delete recvMsg;
        }

        CPPUNIT_ASSERT(ret == expectedRet);
    }

    // Check status of a timer queue built with regular constructor
    void checkAfterConstructor(const OsMsgQShared* q,
            const char* name,
            int         maxMsgs,
            int         maxMsgLen,
            int         options,
            bool        reportFull,
            OsMsgQShared::QueuePreference queuePreference)
    {
        if (OsMsgQShared::QUEUE_LIMITED == q->_queuePreference)
        {
            CPPUNIT_ASSERT(NULL != q->_empty);
            CPPUNIT_ASSERT(dynamic_cast<OsMsgQShared::Semaphore*>(q->_empty) != NULL);
            CPPUNIT_ASSERT(NULL != q->_full);
            CPPUNIT_ASSERT(dynamic_cast<OsMsgQShared::Semaphore*>(q->_full) != NULL);
        }
        else
        {
            CPPUNIT_ASSERT(NULL == q->_empty);
            CPPUNIT_ASSERT(NULL != q->_full);
            CPPUNIT_ASSERT(dynamic_cast<OsMsgQShared::Semaphore*>(q->_full) != NULL);
        }

        CPPUNIT_ASSERT(0 == q->_queue.size());
        CPPUNIT_ASSERT(maxMsgLen == q->_maxMsgLen);
        CPPUNIT_ASSERT(options == q->_options);
        CPPUNIT_ASSERT(reportFull == q->_reportFull);
        CPPUNIT_ASSERT(queuePreference == q->_queuePreference);
    }

    void testDefaultQueuePreference()
    {
        OsMsgQShared* q = new OsMsgQShared("limited");
        //TEST: Queue is created with LIMITED preference
        checkAfterConstructor(q, "limited", OsMsgQBase::DEF_MAX_MSGS, OsMsgQBase::DEF_MAX_MSG_LEN, OsMsgQBase::Q_PRIORITY, true, OsMsgQShared::QUEUE_LIMITED);
        delete q;

        // change preference to unlimited
        OsMsgQShared::setQueuePreference(OsMsgQShared::QUEUE_UNLIMITED);
        q = new OsMsgQShared("unlimited");
        //TEST: Queue is created with UNLIMITED preference
        checkAfterConstructor(q, "unlimited", OsMsgQBase::DEF_MAX_MSGS, OsMsgQBase::DEF_MAX_MSG_LEN, OsMsgQBase::Q_PRIORITY, true, OsMsgQShared::QUEUE_UNLIMITED);
        delete q;
    }


    void checkSendOneRecvOneTimeout(OsMsgQShared* q, OsTime& t)
    {
        // send a message and receive it
        UtlString str("test");
        OsMsg* msgSent = new OsMsgTest(str);
        // TEST: send should work;
        CPPUNIT_ASSERT(OS_SUCCESS == q->send(*msgSent, t));
        // TEST: receive should work;
        checkReceive(q, t, str, OS_SUCCESS);

        delete msgSent;
    }

    void checkSendOneTimeout(OsMsgQShared* q, OsTime& t)
    {
        // send a message and receive it
        UtlString str("test");
        OsMsg* msgSent = new OsMsgTest(str);

        // TEST: send should give timeout
        CPPUNIT_ASSERT(OS_WAIT_TIMEOUT == q->send(*msgSent, t));

        delete msgSent;
    }

    void checkSendMany(OsMsgQShared* q, int count)
    {
        // send a message and receive it
        UtlString str("test");
        OsMsg* msgSent = new OsMsgTest(str);

        for (int i = 0; i < count; i++ )
        {
            // TEST: send should work;
            CPPUNIT_ASSERT(OS_SUCCESS == q->send(*msgSent, OsTime::OS_INFINITY));
        }

        delete msgSent;
    }

    void checkRecvMany(OsMsgQShared* q, int count)
    {
        // send a message and receive it
        UtlString str("test");

        for (int i = 0; i < count; i++ )
        {
            // TEST: send should work;
            checkReceive(q, OsTime::OS_INFINITY, str, OS_SUCCESS);
        }
    }


    void checkNoReceiveTimeout(OsMsgQShared* q, OsTime& t)
    {
        OsMsg* msgRecv = NULL;

        // TEST: Recv with timeout 1 sec should not retrieve anything because no message was sent yet
        CPPUNIT_ASSERT(OS_WAIT_TIMEOUT == q->receive(msgRecv, t));
        CPPUNIT_ASSERT(NULL == msgRecv);
    }

    void testDefaultLimitedQueue()
    {
        // change preference to limited
        OsMsgQShared::setQueuePreference(OsMsgQShared::QUEUE_LIMITED);
        OsTime t(1,0);

        // Create a queue with default values
        OsMsgQShared* q = new OsMsgQShared("limited");
        checkAfterConstructor(q, "limited", OsMsgQBase::DEF_MAX_MSGS, OsMsgQBase::DEF_MAX_MSG_LEN, OsMsgQBase::Q_PRIORITY, true, OsMsgQShared::QUEUE_LIMITED);

        // TEST: Initially queue is empty so recv should fail
        checkNoReceiveTimeout(q, t);
        // TEST: Send one recv one should work
        checkSendOneRecvOneTimeout(q, t);

        // TEST: Send as many as queue max limit should work
        checkSendMany(q, OsMsgQBase::DEF_MAX_MSGS);
        // TEST: Send over the queue max limit should not work
        checkSendOneTimeout(q, t);
        // TEST: Send as many as sent should work
        checkRecvMany(q, OsMsgQBase::DEF_MAX_MSGS);
        // TEST: Recv should fail after everything was received
        checkNoReceiveTimeout(q, t);

        delete q;
    }

    void testCustomLimitedQueue()
    {
        // change preference to limited
        OsMsgQShared::setQueuePreference(OsMsgQShared::QUEUE_LIMITED);

        OsTime t(1,0);

        // Create a queue with custom values
        OsMsgQShared* q = new OsMsgQShared("limited", 10, 10, OsMsgQBase::Q_PRIORITY, false);

        // TEST: Constructor initialized all fields with defaults
        checkAfterConstructor(q, "limited", 10, 10, OsMsgQBase::Q_PRIORITY, false, OsMsgQShared::QUEUE_LIMITED);

        // TEST: Initially queue is empty so recv should fail
        checkNoReceiveTimeout(q, t);
        // TEST: Send one recv one should work
        checkSendOneRecvOneTimeout(q, t);

        // TEST: Send as many as queue max limit should work
        checkSendMany(q, 10);
        // TEST: Send over the queue max limit should not work
        checkSendOneTimeout(q, t);
        // TEST: Send as many as sent should work
        checkRecvMany(q, 10);
        // TEST: Recv should fail after everything was received
        checkNoReceiveTimeout(q, t);

        delete q;
    }


    void testDefaultUnLimitedQueue()
    {
        // change preference to limited
        OsMsgQShared::setQueuePreference(OsMsgQShared::QUEUE_UNLIMITED);
        OsTime t(1,0);

        // Create a queue with default values
        OsMsgQShared* q = new OsMsgQShared("unlimited");
        checkAfterConstructor(q, "unlimited", OsMsgQBase::DEF_MAX_MSGS, OsMsgQBase::DEF_MAX_MSG_LEN, OsMsgQBase::Q_PRIORITY, true, OsMsgQShared::QUEUE_UNLIMITED);

        // TEST: Initially queue is empty so recv should fail
        checkNoReceiveTimeout(q, t);
        // TEST: Send one recv one should work
        checkSendOneRecvOneTimeout(q, t);

        // TEST: Send many  should work
        checkSendMany(q, 100000);
        // TEST: Send as many as sent should work
        checkRecvMany(q, 100000);
        // TEST: Recv should fail after everything was received
        checkNoReceiveTimeout(q, t);

        delete q;
    }

    void testCustomUnLimitedQueue()
    {
        // change preference to limited
        OsMsgQShared::setQueuePreference(OsMsgQShared::QUEUE_UNLIMITED);

        OsTime t(1,0);

        // Create a queue with custom values
        OsMsgQShared* q = new OsMsgQShared("unlimited", 10, 10, OsMsgQBase::Q_PRIORITY, false);

        // TEST: Constructor initialized all fields with defaults
        checkAfterConstructor(q, "unlimited", 10, 10, OsMsgQBase::Q_PRIORITY, false, OsMsgQShared::QUEUE_UNLIMITED);

        // TEST: Initially queue is empty so recv should fail
        checkNoReceiveTimeout(q, t);
        // TEST: Send one recv one should work
        checkSendOneRecvOneTimeout(q, t);

        // TEST: Send many  should work
        checkSendMany(q, 100000);
        // TEST: Send as many as sent should work
        checkRecvMany(q, 100000);
        // TEST: Recv should fail after everything was received
        checkNoReceiveTimeout(q, t);

        delete q;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsMsgQSharedTest);
