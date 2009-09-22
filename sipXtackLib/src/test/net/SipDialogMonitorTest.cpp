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

#include <os/OsSysLog.h>
#include <net/SipDialogMonitor.h>
#include <net/SipMessage.h>

/**
 * State change notifier class for use by tests.
 */
class TestStateChangeNotifier : public StateChangeNotifier
{
public:

   /// Create a TestStateChangeNotifier.
   // When its setStatus is called, it will append to pLogString:
   // logPrefix, " ", "ON_HOOK" or "OFF_HOOK", "\n".
   TestStateChangeNotifier(const char* logPrefix,
                           UtlString* pLogString);

   bool setStatus(const Url& aor, const Status value);

protected:

   UtlString mLogPrefix;
   UtlString* mpLogString;
};

TestStateChangeNotifier::TestStateChangeNotifier(const char* logPrefix,
                                                 UtlString* pLogString) :
   mLogPrefix(logPrefix),
   mpLogString(pLogString)
{
}

bool TestStateChangeNotifier::setStatus(const Url& aor, const Status value)
{
   mpLogString->append(mLogPrefix);
   mpLogString->append(value == ON_HOOK ? " ON_HOOK\n" : " OFF_HOOK\n");
   return 1;
}


/**
 * Unit test for SipDialogMonitor
 */

class SipDialogMonitorTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipDialogMonitorTest);
   CPPUNIT_TEST(simpleTest);
   CPPUNIT_TEST(multipleMsgTest);
   CPPUNIT_TEST(multipleDialogTest);
   CPPUNIT_TEST(partialEventTest);
   CPPUNIT_TEST(implicitTerminationTest);
   CPPUNIT_TEST(emptyPartialTest);
   CPPUNIT_TEST(multipleSubscriptionTest);
   CPPUNIT_TEST(multipleSubscriptionFullTest);
   CPPUNIT_TEST(unknownDialog);
   CPPUNIT_TEST_SUITE_END();

public:

#ifdef LOGGING
   void setUp()
      {
         OsSysLog::initialize(0, "test");
         OsSysLog::setOutputFile(0, "log");
         OsSysLog::setLoggingPriority(PRI_DEBUG);
         OsSysLog::setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);

         system("pwd >/tmp/p");
      }
#endif /* LOGGING */

   // String to record events in.
   UtlString eventLog;

   static SipMessage* makeMessage(const char* body)
      {
         // Buffers for composing the message.
         UtlString message;
         char header[1000];
         // Sequence number.
         static int seqNo = 1;

         // Compose the header.
         sprintf(header,
                 "NOTIFY sip:sipx.local SIP/2.0\r\n"
                 "To: sip:sipx.local\r\n"
                 "From: sip:sipx.local\r\n"
                 "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
                 "Cseq: %d NOTIFY\r\n"
                 "Content-Length: %zu\r\n"
                 "\r\n",
                 seqNo++,
                 strlen(body));
         message.append(header);
         message.append(body);

         return new SipMessage(message.data(), message.length());
      };

   void simpleTest()
      {
         // Create the SipUserAgent.
         SipUserAgent ua;

         // Create the SipDialogMonitor.
         UtlString domain("source-domain");
         SipDialogMonitor dm(&ua, domain, 5060, 100, FALSE);

         // Register a notifier.
         TestStateChangeNotifier notifier("notify", &eventLog);
         dm.addStateChangeNotifier("notify", &notifier);

         // Clear the event log.
         eventLog = "";

         // Start state-keeping for edh1.
         eventLog.append("createDialogState edh1\n");
         dm.createDialogState(new UtlString("edh1"));

         // Send a message.
         SipMessage* m1 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"1\" state=\"full\" entity=\"sip:333@source-domain\">"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m1\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m1);

         // Check that what we wanted happened.
         ASSERT_STR_EQUAL(eventLog.data(),
                          "createDialogState edh1\n"
                          "notifyEventCallback edh1 dh1 m1\n"
                          "notify ON_HOOK\n");
      }

   void multipleMsgTest()
      {
         // Create the SipUserAgent.
         SipUserAgent ua;

         // Create the SipDialogMonitor.
         UtlString domain("source-domain");
         SipDialogMonitor dm(&ua, domain, 5060, 100, FALSE);

         // Register a notifier.
         TestStateChangeNotifier notifier("notify", &eventLog);
         dm.addStateChangeNotifier("notify", &notifier);

         // Clear the event log.
         eventLog = "";

         // Start state-keeping for edh1.
         eventLog.append("createDialogState edh1\n");
         dm.createDialogState(new UtlString("edh1"));

         // Send a message with no dialog.
         SipMessage* m1 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"1\" state=\"full\" entity=\"sip:333@source-domain\">"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m1\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m1);

         // Send a message with a dialog.
         SipMessage* m2 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"1\" state=\"full\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"0\" call-id=\"s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>early</state>"
            "<duration>0</duration>"
            "<local>"
            "<identity>333@sample-domain</identity>"
            "<target uri=\"sip:333@sample-domain\"/>"
            "</local>"
            "<remote>"
            "<identity display=\"call-301.0\">301@sample-domain</identity>"
            "</remote>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m2\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m2);

         // Send the message with a dialog again.
         eventLog.append("notifyEventCallback edh1 dh1 m2\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m2);

         // Send a message with the dialog terminated.
         SipMessage* m3 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"2\" state=\"full\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"0\" call-id=\"s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>terminated</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m3\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m3);

         // Send the message with no dialog again.
         eventLog.append("notifyEventCallback edh1 dh1 m1\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m1);

         // Check that what we wanted happened.
         ASSERT_STR_EQUAL(eventLog.data(),
                          "createDialogState edh1\n"
                          "notifyEventCallback edh1 dh1 m1\n"
                          "notify ON_HOOK\n"
                          "notifyEventCallback edh1 dh1 m2\n"
                          "notify OFF_HOOK\n"
                          "notifyEventCallback edh1 dh1 m2\n"
                          "notify OFF_HOOK\n"
                          "notifyEventCallback edh1 dh1 m3\n"
                          "notify ON_HOOK\n"
                          "notifyEventCallback edh1 dh1 m1\n"
                          "notify ON_HOOK\n");
      }

   void multipleDialogTest()
      {
         // Create the SipUserAgent.
         SipUserAgent ua;

         // Create the SipDialogMonitor.
         UtlString domain("source-domain");
         SipDialogMonitor dm(&ua, domain, 5060, 100, FALSE);

         // Register a notifier.
         TestStateChangeNotifier notifier("notify", &eventLog);
         dm.addStateChangeNotifier("notify", &notifier);

         // Clear the event log.
         eventLog = "";

         // Start state-keeping for edh1.
         eventLog.append("createDialogState edh1\n");
         dm.createDialogState(new UtlString("edh1"));

         // Send a message with no dialog.
         SipMessage* m1 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"1\" state=\"full\" entity=\"sip:333@source-domain\">"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m1\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m1);

         // Send a message with a dialog.
         SipMessage* m2 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"2\" state=\"full\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"0\" call-id=\"1s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>committed</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m2\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m2);

         // Send a message with a second dialog.
         SipMessage* m3 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"3\" state=\"full\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"0\" call-id=\"1s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>committed</state>"
            "</dialog>"
            "<dialog id=\"1\" call-id=\"2s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>committed</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m3\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m3);

         // Send a message terminating the first dialog.
         SipMessage* m4 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"4\" state=\"full\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"0\" call-id=\"1s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>terminated</state>"
            "</dialog>"
            "<dialog id=\"1\" call-id=\"2s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>committed</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m4\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m4);

         // Send a message terminating the second dialog.
         SipMessage* m5 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"5\" state=\"full\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"0\" call-id=\"1s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>terminated</state>"
            "</dialog>"
            "<dialog id=\"1\" call-id=\"2s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>terminated</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m5\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m5);

         // Send a message that eliminates both messages.
         SipMessage* m6 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"6\" state=\"full\" entity=\"sip:333@source-domain\">"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m6\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m6);

         // Check that what we wanted happened.
         ASSERT_STR_EQUAL("createDialogState edh1\n"
                          "notifyEventCallback edh1 dh1 m1\n"
                          "notify ON_HOOK\n"
                          "notifyEventCallback edh1 dh1 m2\n"
                          "notify OFF_HOOK\n"
                          "notifyEventCallback edh1 dh1 m3\n"
                          "notify OFF_HOOK\n"
                          "notifyEventCallback edh1 dh1 m4\n"
                          "notify OFF_HOOK\n"
                          "notifyEventCallback edh1 dh1 m5\n"
                          "notify ON_HOOK\n"
                          "notifyEventCallback edh1 dh1 m6\n"
                          "notify ON_HOOK\n",
                          eventLog.data());
      }

   void partialEventTest()
      {
         // Create the SipUserAgent.
         SipUserAgent ua;

         // Create the SipDialogMonitor.
         UtlString domain("source-domain");
         SipDialogMonitor dm(&ua, domain, 5060, 100, FALSE);

         // Register a notifier.
         TestStateChangeNotifier notifier("notify", &eventLog);
         dm.addStateChangeNotifier("notify", &notifier);

         // Clear the event log.
         eventLog = "";

         // Start state-keeping for edh1.
         eventLog.append("createDialogState edh1\n");
         dm.createDialogState(new UtlString("edh1"));

         // Send a message with no dialog.
         SipMessage* m1 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"1\" state=\"full\" entity=\"sip:333@source-domain\">"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m1\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m1);

         // Send a message with a dialog.
         SipMessage* m2 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"2\" state=\"partial\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"0\" call-id=\"1s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>committed</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m2\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m2);

         // Send a message with a second dialog.
         SipMessage* m3 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"3\" state=\"partial\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"1\" call-id=\"2s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>committed</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m3\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m3);

         // Send a message terminating the first dialog.
         SipMessage* m4 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"4\" state=\"partial\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"0\" call-id=\"1s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>terminated</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m4\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m4);

         // Send a message terminating the second dialog.
         SipMessage* m5 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"5\" state=\"partial\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"1\" call-id=\"2s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>terminated</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m5\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m5);

         // Send a message that eliminates both messages.
         SipMessage* m6 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"6\" state=\"partial\" entity=\"sip:333@source-domain\">"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m6\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m6);

         // Check that what we wanted happened.
         ASSERT_STR_EQUAL("createDialogState edh1\n"
                          "notifyEventCallback edh1 dh1 m1\n"
                          "notify ON_HOOK\n"
                          "notifyEventCallback edh1 dh1 m2\n"
                          "notify OFF_HOOK\n"
                          "notifyEventCallback edh1 dh1 m3\n"
                          "notify OFF_HOOK\n"
                          "notifyEventCallback edh1 dh1 m4\n"
                          "notify OFF_HOOK\n"
                          "notifyEventCallback edh1 dh1 m5\n"
                          "notify ON_HOOK\n"
                          "notifyEventCallback edh1 dh1 m6\n"
                          "notify ON_HOOK\n",
                          eventLog.data());
      }

   void implicitTerminationTest()
      {
         // Create the SipUserAgent.
         SipUserAgent ua;

         // Create the SipDialogMonitor.
         UtlString domain("source-domain");
         SipDialogMonitor dm(&ua, domain, 5060, 100, FALSE);

         // Register a notifier.
         TestStateChangeNotifier notifier("notify", &eventLog);
         dm.addStateChangeNotifier("notify", &notifier);

         // Clear the event log.
         eventLog = "";

         // Start state-keeping for edh1.
         eventLog.append("createDialogState edh1\n");
         dm.createDialogState(new UtlString("edh1"));

         // Send a message with no dialog.
         SipMessage* m1 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"1\" state=\"full\" entity=\"sip:333@source-domain\">"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m1\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m1);

         // Send a message with a dialog.
         SipMessage* m2 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"2\" state=\"full\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"0\" call-id=\"1s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>committed</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m2\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m2);

         // Send a message that eliminates both messages.
         SipMessage* m3 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"3\" state=\"full\" entity=\"sip:333@source-domain\">"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m3\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m3);

         // Check that what we wanted happened.
         ASSERT_STR_EQUAL("createDialogState edh1\n"
                          "notifyEventCallback edh1 dh1 m1\n"
                          "notify ON_HOOK\n"
                          "notifyEventCallback edh1 dh1 m2\n"
                          "notify OFF_HOOK\n"
                          "notifyEventCallback edh1 dh1 m3\n"
                          "notify ON_HOOK\n",
                          eventLog.data());
      }

   void emptyPartialTest()
      {
         // Create the SipUserAgent.
         SipUserAgent ua;

         // Create the SipDialogMonitor.
         UtlString domain("source-domain");
         SipDialogMonitor dm(&ua, domain, 5060, 100, FALSE);

         // Register a notifier.
         TestStateChangeNotifier notifier("notify", &eventLog);
         dm.addStateChangeNotifier("notify", &notifier);

         // Clear the event log.
         eventLog = "";

         // Start state-keeping for edh1.
         eventLog.append("createDialogState edh1\n");
         dm.createDialogState(new UtlString("edh1"));

         // Send a message with no dialog.
         SipMessage* m1 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"1\" state=\"full\" entity=\"sip:333@source-domain\">"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m1\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m1);

         // Send a message with a dialog.
         SipMessage* m2 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"2\" state=\"partial\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"0\" call-id=\"1s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>committed</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m2\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m2);

         // Send a message that eliminates both messages, but is partial,
         // so it has no effect.
         SipMessage* m3 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"3\" state=\"partial\" entity=\"sip:333@source-domain\">"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m3\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m3);

         // Check that what we wanted happened.
         ASSERT_STR_EQUAL("createDialogState edh1\n"
                          "notifyEventCallback edh1 dh1 m1\n"
                          "notify ON_HOOK\n"
                          "notifyEventCallback edh1 dh1 m2\n"
                          "notify OFF_HOOK\n"
                          "notifyEventCallback edh1 dh1 m3\n"
                          "notify OFF_HOOK\n",
                          eventLog.data());
      }

   void multipleSubscriptionTest()
      {
         // Create the SipUserAgent.
         SipUserAgent ua;

         // Create the SipDialogMonitor.
         UtlString domain("source-domain");
         SipDialogMonitor dm(&ua, domain, 5060, 100, FALSE);

         // Register a notifier.
         TestStateChangeNotifier notifier("notify", &eventLog);
         dm.addStateChangeNotifier("notify", &notifier);

         // Clear the event log.
         eventLog = "";

         // Start state-keeping for edh1.
         eventLog.append("createDialogState edh1\n");
         dm.createDialogState(new UtlString("edh1"));

         // Send a message with no dialog.
         SipMessage* m1 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"1\" state=\"full\" entity=\"sip:333@source-domain\">"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m1\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m1);

         // Send a message with a dialog.
         SipMessage* m2 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"2\" state=\"partial\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"0\" call-id=\"1s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>committed</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m2\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m2);

         // Send a message with a second dialog to a second dialog handle.
         SipMessage* m101 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"101\" state=\"partial\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"1\" call-id=\"2s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>committed</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh2 m101\n");
         dm.notifyEventCallback("edh1", "dh2", (void*) &dm, m101);

         // Send a message terminating the first dialog.
         SipMessage* m3 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"3\" state=\"partial\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"0\" call-id=\"1s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>terminated</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m3\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m3);

         // Send a message terminating the second dialog.
         SipMessage* m102 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"102\" state=\"partial\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"1\" call-id=\"2s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>terminated</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh2 m102\n");
         dm.notifyEventCallback("edh1", "dh2", (void*) &dm, m102);

         // Check that what we wanted happened.
         ASSERT_STR_EQUAL("createDialogState edh1\n"
                          "notifyEventCallback edh1 dh1 m1\n"
                          "notify ON_HOOK\n"
                          "notifyEventCallback edh1 dh1 m2\n"
                          "notify OFF_HOOK\n"
                          "notifyEventCallback edh1 dh2 m101\n"
                          "notify OFF_HOOK\n"
                          "notifyEventCallback edh1 dh1 m3\n"
                          "notify OFF_HOOK\n"
                          "notifyEventCallback edh1 dh2 m102\n"
                          "notify ON_HOOK\n",
                          eventLog.data());
      }

   // Same as the preceding test, but using full updates, which gives
   // them a different meaning.
   void multipleSubscriptionFullTest()
      {
         // Create the SipUserAgent.
         SipUserAgent ua;

         // Create the SipDialogMonitor.
         UtlString domain("source-domain");
         SipDialogMonitor dm(&ua, domain, 5060, 100, FALSE);

         // Register a notifier.
         TestStateChangeNotifier notifier("notify", &eventLog);
         dm.addStateChangeNotifier("notify", &notifier);

         // Clear the event log.
         eventLog = "";

         // Start state-keeping for edh1.
         eventLog.append("createDialogState edh1\n");
         dm.createDialogState(new UtlString("edh1"));

         // Send a message with no dialog.
         SipMessage* m1 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"1\" state=\"full\" entity=\"sip:333@source-domain\">"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m1\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m1);

         // Send a message with a dialog.
         SipMessage* m2 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"2\" state=\"full\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"0\" call-id=\"1s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>committed</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m2\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m2);

         // Send a message with a second dialog to a second dialog handle.
         SipMessage* m101 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"101\" state=\"full\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"1\" call-id=\"2s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>committed</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh2 m101\n");
         dm.notifyEventCallback("edh1", "dh2", (void*) &dm, m101);

         // Send a message terminating the first dialog.
         SipMessage* m3 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"3\" state=\"full\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"0\" call-id=\"1s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>terminated</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m3\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m3);

         // Send a message terminating the second dialog.
         SipMessage* m102 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"102\" state=\"full\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"1\" call-id=\"2s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>terminated</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh2 m102\n");
         dm.notifyEventCallback("edh1", "dh2", (void*) &dm, m102);

         // Check that what we wanted happened.
         ASSERT_STR_EQUAL("createDialogState edh1\n"
                          "notifyEventCallback edh1 dh1 m1\n"
                          "notify ON_HOOK\n"
                          "notifyEventCallback edh1 dh1 m2\n"
                          "notify OFF_HOOK\n"
                          "notifyEventCallback edh1 dh2 m101\n"
                          "notify OFF_HOOK\n"
                          "notifyEventCallback edh1 dh1 m3\n"
                          "notify OFF_HOOK\n"
                          "notifyEventCallback edh1 dh2 m102\n"
                          "notify ON_HOOK\n",
                          eventLog.data());
      }

   // Report an event package for an unknown dialog, as if we had terminated
   // a subscription, and received the final NOTIFY.
   void unknownDialog()
      {
         // Create the SipUserAgent.
         SipUserAgent ua;

         // Create the SipDialogMonitor.
         UtlString domain("source-domain");
         SipDialogMonitor dm(&ua, domain, 5060, 100, FALSE);

         // Register a notifier.
         TestStateChangeNotifier notifier("notify", &eventLog);
         dm.addStateChangeNotifier("notify", &notifier);

         // Clear the event log.
         eventLog = "";

         // Start state-keeping for edh1.
         eventLog.append("createDialogState edh1\n");
         dm.createDialogState(new UtlString("edh1"));

         // Send a message with a dialog.
         SipMessage* m1 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"1\" state=\"full\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"0\" call-id=\"1s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>committed</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m1\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m1);

         // Stop state-keeping for edh1.
         eventLog.append("destroyDialogState edh1\n");
         dm.destroyDialogState(new UtlString("edh1"));

         // Un-register the notifier.
         // There should be no on/off-hook callbacks after this point.
         dm.removeStateChangeNotifier("notify");

         // Send a message to terminate the dialog.
         SipMessage* m2 = makeMessage(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "version=\"2\" state=\"full\" entity=\"sip:333@source-domain\">"
            "<dialog id=\"0\" call-id=\"1s_39120002_3e0fe794a52c322b\" local-tag=\"1570524184\" remote-tag=\"1c694990483\" direction=\"recipient\">"
            "<state>terminated</state>"
            "</dialog>"
            "</dialog-info>"
            );
         eventLog.append("notifyEventCallback edh1 dh1 m2\n");
         dm.notifyEventCallback("edh1", "dh1", (void*) &dm, m2);

         // Check that what we wanted happened.
         ASSERT_STR_EQUAL("createDialogState edh1\n"
                          "notifyEventCallback edh1 dh1 m1\n"
                          "notify OFF_HOOK\n"
                          "destroyDialogState edh1\n"
                          "notifyEventCallback edh1 dh1 m2\n",
                          eventLog.data());
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipDialogMonitorTest);
