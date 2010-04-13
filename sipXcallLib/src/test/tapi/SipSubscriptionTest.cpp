//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlString.h>
#include <os/OsDefs.h>
#include <os/OsDateTime.h>
#include <tapi/sipXtapi.h>
#include <tapi/sipXtapiEvents.h>
#include <os/OsTask.h>
#include <net/Url.h>

#define UNIT_TEST_SIP_PORT 44450
#define UNIT_TEST_SIP_PORT2 44452
#define UNIT_TEST_RTP_PORT_START 45000
#define UNIT_TEST_RTP_PORT_START2 45100
#define UNIT_TEST_MAX_CALL_LEGS 10

#define ASSERT_STR_EQUAL(X, Y) (CPPUNIT_ASSERT(strcmp((X), (Y)) == 0))


/**
 * Unittest for sipXtapi SIP Subscribe/Notify client and server APIs
 */

class SipSubscriptionTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(SipSubscriptionTest);
    CPPUNIT_TEST(subscribeTest);
    CPPUNIT_TEST_SUITE_END();

    public:
        SIPX_CALLSTATE_EVENT mLastStateInst1;
        SIPX_CALLSTATE_EVENT mLastStateInst2;
        int mInst1NotifyCount;
        UtlString* mpInst1NotifyContent;
        UtlString mInst1NotifierUserAgent;
        SIPX_SUB mInst1NotifySubHandle;
        UtlBoolean mInst1GotSubscribe;
        SIPX_SUB mInst1SubHandle;
        SIPX_SUBSCRIPTION_STATE mInst1SubState;
        SIPX_SUBSCRIPTION_CAUSE mInst1SubCause;

    static void getSubStateString(SIPX_SUBSCRIPTION_STATE state, UtlString& stateString)
    {
        switch(state)
        {
        case SIPX_SUBSCRIPTION_PENDING:
            stateString = "SIPX_SUBSCRIPTION_PENDING";
            break;
        case SIPX_SUBSCRIPTION_ACTIVE:
            stateString = "SIPX_SUBSCRIPTION_ACTIVE";
            break;
        case SIPX_SUBSCRIPTION_FAILED:
            stateString = "SIPX_SUBSCRIPTION_FAILED";
            break;
        case SIPX_SUBSCRIPTION_EXPIRED:
            stateString = "SIPX_SUBSCRIPTION_EXPIRED";
            break;
        default:
            stateString = "unknown";
            break;
        }
    }

    static bool inst1Callback(SIPX_EVENT_CATEGORY category,
                               void* pInfo,
                               void* pUserData)
    {
        SipSubscriptionTest* testObj = (SipSubscriptionTest*) pUserData;

        if(category == EVENT_CATEGORY_CALLSTATE)
        {
            SIPX_CALLSTATE_INFO* state = (SIPX_CALLSTATE_INFO*)pInfo;
            if(state->event != CALLSTATE_AUDIO_EVENT)
            {
                testObj->mLastStateInst1 = state->event;
            }
#ifdef TEST_PRINT
            printf("inst1Callback EVENT_CATEGORY_CALLSTATE: %d\n", state->event);
#endif
        }
        // Subscribe state event
        else if(category == EVENT_CATEGORY_SUB_STATUS)
        {
            SIPX_SUBSTATUS_INFO* subInfo = (SIPX_SUBSTATUS_INFO*)pInfo;
            UtlString stateString;
            getSubStateString(subInfo->state, stateString);
#ifdef TEST_PRINT
            printf("inst1Callback EVENT_CATEGORY_SUB_STATUS state: %s user agent: %s\n",
                stateString.data(),
                subInfo->szSubServerUserAgent ? subInfo->szSubServerUserAgent : "<null>");
#endif
            testObj->mInst1SubHandle = subInfo->hSub;
            testObj->mInst1GotSubscribe = TRUE;
            testObj->mInst1SubState = subInfo->state;
            testObj->mInst1SubCause = subInfo->cause;
#ifdef TEST_PRINT
            printf("SIPX_SUBSTATUS_INFO size: %d\n", subInfo->nSize);
#endif

        }

        // Notify event body event
        else if(category == EVENT_CATEGORY_NOTIFY)
        {
#ifdef TEST_PRINT
            printf("inst1Callback EVENT_CATEGORY_NOTIFY\n");
#endif
            (testObj->mInst1NotifyCount)++;
            SIPX_NOTIFY_INFO* notifyInfo = (SIPX_NOTIFY_INFO*)pInfo;
            if(notifyInfo->nContentLength && notifyInfo->pContent)
            {
                testObj->mpInst1NotifyContent =
                    new UtlString((const char*)(notifyInfo->pContent),
                                  notifyInfo->nContentLength);
            }
            testObj->mInst1NotifySubHandle = notifyInfo->hSub;
            testObj->mInst1NotifierUserAgent = notifyInfo->szNotiferUserAgent;
        }
        else
        {
#ifdef TEST_PRINT
            printf("inst1Callback category: %d\n", category);
#endif
        }
        return(true);
    };

    static bool inst2Callback(SIPX_EVENT_CATEGORY category,
                               void* pInfo,
                               void* pUserData)
    {
        SipSubscriptionTest* testObj = (SipSubscriptionTest*) pUserData;
        // Answer the incoming call
        if(category == EVENT_CATEGORY_CALLSTATE)
        {
            SIPX_CALLSTATE_INFO* state = (SIPX_CALLSTATE_INFO*)pInfo;
            if(state->event != CALLSTATE_AUDIO_EVENT)
            {
                testObj->mLastStateInst2 = state->event;
            }
#ifdef TEST_PRINT
            printf("inst2Callback EVENT_CATEGORY_CALLSTATE: %d\n", state->event);
#endif
            if(testObj->mLastStateInst2 == CALLSTATE_OFFERING)
            {
#ifdef TEST_PRINT
                printf("Call2: CALLSTATE_OFFERING\n");
#endif
                sipxCallAccept(state->hCall);
                sipxCallAnswer(state->hCall);
            }
            else
            {
            }


        }

        // Subscribe state event
        else if(category == EVENT_CATEGORY_SUB_STATUS)
        {
#ifdef TEST_PRINT
            printf("inst2Callback EVENT_CATEGORY_SUB_STATUS\n");
#endif
        }

        // Notify event body event
        else if(category == EVENT_CATEGORY_NOTIFY)
        {
#ifdef TEST_PRINT
            printf("inst2Callback EVENT_CATEGORY_NOTIFY\n");
#endif
        }

        else
        {
#ifdef TEST_PRINT
            printf("inst2Callback category: %d\n", category);
#endif
        }

        return(true);
    };

    void subscribeTest()
    {
        mLastStateInst1 = CALLSTATE_UNKNOWN;
        mLastStateInst2 = CALLSTATE_UNKNOWN;
        mInst1NotifyCount = 0;
        mpInst1NotifyContent = NULL;
        mInst1NotifySubHandle = NULL;
        mInst1GotSubscribe = FALSE;
        mInst1SubHandle = NULL;
        mInst1SubState = SIPX_SUBSCRIPTION_PENDING;
        mInst1SubCause = SUBSCRIPTION_CAUSE_UNKNOWN;

        sipxConfigSetLogLevel(LOG_LEVEL_DEBUG);
        sipxConfigSetLogFile("SipSubscriptionTest.log");

        SIPX_INST sipxInstance1 = NULL;
        sipxInitialize(&sipxInstance1,
                       UNIT_TEST_SIP_PORT, UNIT_TEST_SIP_PORT, -1,
                       UNIT_TEST_RTP_PORT_START,
                       UNIT_TEST_MAX_CALL_LEGS);

        CPPUNIT_ASSERT(sipxInstance1);

        SIPX_INST sipxInstance2 = NULL;
        sipxInitialize(&sipxInstance2,
                       UNIT_TEST_SIP_PORT2, UNIT_TEST_SIP_PORT2, -1,
                       UNIT_TEST_RTP_PORT_START2,
                       UNIT_TEST_MAX_CALL_LEGS);

        CPPUNIT_ASSERT(sipxInstance2);

        // Register event listeners
        sipxEventListenerAdd(sipxInstance1,
                             inst1Callback,
                             this);
        sipxEventListenerAdd(sipxInstance2,
                             inst2Callback,
                             this);

        char numBuf[20];
        UtlString address1("sip:1@127.0.0.1:");
        sprintf(numBuf, "%d", UNIT_TEST_SIP_PORT);
        address1.append(numBuf);
        SIPX_LINE lineHandle1 = NULL;
        sipxLineAdd(sipxInstance1,
                    address1,
                    &lineHandle1);
        CPPUNIT_ASSERT(lineHandle1);

        // Inst1 calls Inst2
        SIPX_CALL callHandle1 = NULL;
        sipxCallCreate(sipxInstance1,
                       lineHandle1,
                       &callHandle1);
        CPPUNIT_ASSERT(callHandle1);
        UtlString address2("sip:2@127.0.0.1:");
        sprintf(numBuf, "%d", UNIT_TEST_SIP_PORT2);
        address2.append(numBuf);
        sipxCallConnect(callHandle1,
                        address2);

        int waitTime = 0;
        printf("waiting for call setup");
        while(mLastStateInst1 != CALLSTATE_CONNECTED)
        {
            OsTask::delay(100); // milliseconds

            waitTime++;
            if(waitTime % 10 == 0)
            {
                printf(".");
            }
            if(waitTime > 10000)
            {
                break;
            }
        }
        printf("\nstate: %d expecting: %d\n", mLastStateInst1, CALLSTATE_CONNECTED);

        CPPUNIT_ASSERT(mLastStateInst1 == CALLSTATE_CONNECTED);

        // Create some content, but do not publish it yet
        SIPX_PUB publishHandle2 = NULL;
        Url resourceUrl(address2);
        UtlString resourceId;
        resourceUrl.getIdentity(resourceId);
        UtlString eventType("foo");
        UtlString contentType("application/fooStuff");
        UtlString content("<fooStuff>\n\t<bar/>\n</fooStuff>");

        // Subscribe
#ifdef TEST_PRINT
        printf("subscribing\n");
#endif
        SIPX_SUB subHandle1 = NULL;
        SIPX_RESULT subResult =
            sipxCallSubscribe(callHandle1, "foo2", contentType, &subHandle1);
        CPPUNIT_ASSERT(subResult == SIPX_RESULT_SUCCESS);

        printf("waiting for subscribe state");
        int notifyWaits = 0;
        while(!mInst1GotSubscribe)
        {
            OsTask::delay(1000);
            notifyWaits++;
            printf(".");
            if(notifyWaits == 15)
            {
                break;
            }
        }
        printf("\n");

        mInst1GotSubscribe = FALSE;

        // Nothing has been published so we expect no NOTIFY
#ifdef TEST_PRINT
        printf("%d == %d\n", mInst1SubHandle, subHandle1);
#endif
        CPPUNIT_ASSERT_MESSAGE("Subscription event handle does not match expected handle",
            mInst1SubHandle == subHandle1);
        CPPUNIT_ASSERT_MESSAGE("Unexpected Notification content recieved",
            mpInst1NotifyContent == NULL);
        CPPUNIT_ASSERT_MESSAGE("subscription state not the expected: SIPX_SUBSCRIPTION_EXPIRED",
            mInst1SubState == SIPX_SUBSCRIPTION_EXPIRED);
        CPPUNIT_ASSERT_MESSAGE("subscription cause not the expected: SUBSCRIPTION_CAUSE_NORMAL",
            mInst1SubCause == SUBSCRIPTION_CAUSE_NORMAL);

        // Publish the content
        SIPX_RESULT pubResult = sipxPublisherCreate(sipxInstance2,
                                                    &publishHandle2,
                                                    resourceId,
                                                    eventType,
                                                    contentType,
                                                    content,
                                                    content.length());

        CPPUNIT_ASSERT(pubResult == SIPX_RESULT_SUCCESS);


        // Need to subscribe again as the prior subscription failed
        subResult =
            sipxCallSubscribe(callHandle1, eventType, contentType, &subHandle1);
        CPPUNIT_ASSERT(subResult == SIPX_RESULT_SUCCESS);

        printf("waiting for subscribe state");
        notifyWaits = 0;
        while(!mInst1GotSubscribe || mInst1NotifyCount == 0)
        {
            OsTask::delay(1000);
            notifyWaits++;
            printf(".");
            if(notifyWaits == 15)
            {
                break;
            }
        }
        printf("line 350\n");
        mInst1GotSubscribe = FALSE;

#ifdef TEST_PRINT
        printf("mInst1NotifyCount: %d\n", mInst1NotifyCount);
#endif
        if(mInst1NotifyCount != 1) printf("Did not recieve NOTIFY request");

        CPPUNIT_ASSERT_MESSAGE("Did not receive NOTIFY request",
            mInst1NotifyCount == 1);

        if(mpInst1NotifyContent == NULL) printf("Did not receive NOTIFY content");

        CPPUNIT_ASSERT_MESSAGE("Did not receive NOTIFY content",
            mpInst1NotifyContent);

        if(content.compareTo(mpInst1NotifyContent) != 0)
            printf("content differs.  got:\n%s\nexpected: \n%s\n",
            mpInst1NotifyContent->data(), content.data());

        ASSERT_STR_EQUAL(content.data(), mpInst1NotifyContent->data());
//#ifdef TEST_PRINT
        printf("%d == %d\n", mInst1NotifySubHandle, subHandle1);
//#endif
        CPPUNIT_ASSERT(mInst1NotifySubHandle == subHandle1);
        CPPUNIT_ASSERT(!mInst1NotifierUserAgent.isNull());
        UtlString userAgentPrefix = mInst1NotifierUserAgent;
        userAgentPrefix.remove(4);
        ASSERT_STR_EQUAL("sipX", userAgentPrefix.data());

        mpInst1NotifyContent = NULL;
        UtlString updateContent("<middle></middle>");
        pubResult = sipxPublisherUpdate(publishHandle2,
                                        contentType,
                                        updateContent,
                                        updateContent.length());

        printf("waiting for notify");
        notifyWaits = 0;
        while(mInst1NotifyCount == 1)
        {
            OsTask::delay(1000);
            notifyWaits++;
            printf(".");
            if(notifyWaits == 15)
            {
                break;
            }
        }
        printf("\n");
#ifdef TEST_PRINT
        printf("mInst1NotifyCount: %d\n", mInst1NotifyCount);
#endif
        CPPUNIT_ASSERT(mInst1NotifyCount == 2);
        CPPUNIT_ASSERT(mpInst1NotifyContent);
        ASSERT_STR_EQUAL(updateContent.data(), mpInst1NotifyContent->data());
#ifdef TEST_PRINT
        printf("%d == %d\n", mInst1NotifySubHandle, subHandle1);
#endif
        CPPUNIT_ASSERT(mInst1NotifySubHandle == subHandle1);
        CPPUNIT_ASSERT(!mInst1NotifierUserAgent.isNull());
        userAgentPrefix = mInst1NotifierUserAgent;
        userAgentPrefix.remove(4);
        ASSERT_STR_EQUAL("sipX", userAgentPrefix.data());


        mpInst1NotifyContent = NULL;
        UtlString endContent("<end></end>");
        pubResult = sipxPublisherDestroy(publishHandle2,
                                        contentType,
                                        endContent,
                                        endContent.length());

        printf("waiting for notify");
        notifyWaits = 0;
        while(mInst1NotifyCount == 2)
        {
            OsTask::delay(1000);
            notifyWaits++;
            printf(".");
            if(notifyWaits == 15)
            {
                break;
            }
        }
        printf("\n");
#ifdef TEST_PRINT
        printf("mInst1NotifyCount: %d\n", mInst1NotifyCount);
#endif
        CPPUNIT_ASSERT(mInst1NotifyCount == 3);
        CPPUNIT_ASSERT(mpInst1NotifyContent);
        ASSERT_STR_EQUAL(endContent.data(), mpInst1NotifyContent->data());
#ifdef TEST_PRINT
        printf("%d == %d\n", mInst1NotifySubHandle, subHandle1);
#endif
        CPPUNIT_ASSERT(mInst1NotifySubHandle == subHandle1);
        CPPUNIT_ASSERT(!mInst1NotifierUserAgent.isNull());
        userAgentPrefix = mInst1NotifierUserAgent;
        userAgentPrefix.remove(4);
        ASSERT_STR_EQUAL("sipX", userAgentPrefix.data());

    }
};



CPPUNIT_TEST_SUITE_REGISTRATION(SipSubscriptionTest);
