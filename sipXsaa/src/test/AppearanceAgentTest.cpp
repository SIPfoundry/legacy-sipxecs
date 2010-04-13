//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>
#include <sys/time.h>
#include <utl/UtlSList.h>
#include <utl/UtlString.h>
#include <os/OsDefs.h>
#include <net/SipDialogEvent.h>
#include <net/SipMessage.h>
#include <net/SipOutputProcessor.h>
#include <net/SipUserAgent.h>
#include <string>
#include "os/OsSysLog.h"
#include "os/OsMutex.h"
#include "os/OsLock.h"
#include "testlib/SipDbTestContext.h"
#include "utl/UtlSList.h"
#include "../AppearanceAgent.h"
#include "../AppearanceGroup.h"
#include "../Appearance.h"
#include "RlsTestFixtures.h"

// GLOBALS
UtlBoolean    gShutdownFlag = FALSE;

class AppearanceAgentTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(AppearanceAgentTest);
//   CPPUNIT_TEST(AppearanceTest);
   CPPUNIT_TEST_SUITE_END();

private:
   OutputProcessorFixture* pUasOutputProcessor;
   AppearanceAgent*        pAppearanceAgentUnderTest;
   SipUserAgent*           pSipUserAgent;
   SipDbTestContext        sipDbContext;
   UtlString               mCredentialDbName;

public:
   AppearanceAgentTest() :
      pUasOutputProcessor( 0 ),
      pAppearanceAgentUnderTest( 0 ),
      pSipUserAgent( 0 ),
      sipDbContext( TEST_DATA_DIR, TEST_WORK_DIR )
   {
   }

   void setUp()
   {
   }

   void tearDown()
   {
      freeAllTestFixtures();
   }

   void instantiateAllTestFixtures( UtlString AppearanceGroupFile,
                                    //< Name of the appearance group file to use for the test.  The filename
                                    //< specified must exist in .../sipXrls/src/test/saadata/
                                    UtlString subscriptionDbName,
                                    //< Specifies the subscription DB to use for the test.  The name
                                    //< is used to select which of the files in .../sipXrls/src/test/saadata/
                                    //< will get used to preload the subscription IMDB.  The name provided here is the
                                    //< xml file in .../sipXrls/src/test/saadata/ without the ".xml" extension.
                                    UtlString credentialDbName,
                                    //< Specifies the credential DB to use for the test.  The name
                                    //< is used to select which of the files in .../sipXrls/src/test/saadata/
                                    //< will get used to preload the credential IMDB.  The name provided here is the
                                    //< xml file in .../sipXrls/src/test/saadata/ without the ".xml" extension.
                                    UtlString domainName
                                    //< Specify the domain name to use for the test
                                    )
   {
      mCredentialDbName = credentialDbName;
      // force copy of input files into the 'work' directory
      sipDbContext.inputFile( subscriptionDbName + ".xml" );
      sipDbContext.inputFile( credentialDbName + ".xml" );
      UtlString tempAppearanceGroupFile = UtlString(TEST_DATA_DIR) + "/" + AppearanceGroupFile;

      pSipUserAgent = new SipUserAgent( 45141, 45141, 45142
                                       ,"127.0.0.1"  // default publicAddress
                                       ,NULL         // default defaultUser
                                       ,"127.0.0.1" );  // default defaultSipAddress

      // Stop SipUserAgent from rejecting all SUBSCRIBEs
      pSipUserAgent->allowMethod(SIP_SUBSCRIBE_METHOD, true);

      pAppearanceAgentUnderTest = new AppearanceAgent(
                                       domainName, // domain
                                       "rlstest.test", // realm
                                       NULL,  // line manager
                                       45140, // TCP port
                                       45140, // UDP port
                                       45140, // TLS port
                                       "127.0.0.1", // Bind IP address
                                       &tempAppearanceGroupFile,
                                       (24 * 60 * 60), // Default subscription refresh interval
                                       (60 * 60), // Default subscription resubscribe interval.
                                       (40 * 60), // Default minimum subscription resubscribe interval.
                                       (20 * 60), // Default seized subscription resubscribe interval.
                                       250,  // publishing delay?
                                       20,   // The maximum number of reg subscriptions per group.
                                       32, 3600, 86400, // Subscribe server expiration parameters
                                       subscriptionDbName,
                                       credentialDbName );

      pUasOutputProcessor = new OutputProcessorFixture();

      pAppearanceAgentUnderTest->mServerUserAgent.addSipOutputProcessor( pUasOutputProcessor );

      UtlString proxyAddress;
      proxyAddress = "127.0.0.1:45140";

      pSipUserAgent->setProxyServers(proxyAddress.data());

      pAppearanceAgentUnderTest->start();
   }

   // The freeAllTestFixtures() is automatically called after every test case so
   // unit tests should not call this method.
   void freeAllTestFixtures()
   {
      if (pAppearanceAgentUnderTest)
      {
         pAppearanceAgentUnderTest->mServerUserAgent.removeSipOutputProcessor(pUasOutputProcessor);

         pAppearanceAgentUnderTest->shutdown();
         delete pAppearanceAgentUnderTest;
         pAppearanceAgentUnderTest = 0;

         delete pUasOutputProcessor;
         pUasOutputProcessor = 0;

         delete pSipUserAgent;
         pSipUserAgent = 0;
      }
   }

   // Primitive to send a SIP message to the AppearanceAgent server under test
   bool  sendToAppearanceAgentUnderTest( SipMessage& msg )
   {
      return pSipUserAgent->send( msg );
   }

   // Primitive to fetch the oldest unprocessed message sent by the 'server'
   // component of the AppearanceAgent server.
   bool getNextMessageFromAppearanceAgentUnderTest( SipMessage& message, int timeoutInSecs )
   {
      bool result = false;
      if( pUasOutputProcessor->waitForMessage((long) timeoutInSecs ) == true )
      {
         CallbackTrace trace;
         result = pUasOutputProcessor->popNextCallbackTrace( trace );
         if( result )
         {
            message = trace.getMessage();
         }
      }
      return result;
   }


   //____________________________________________________
   // || || || || || || || || || || || || || || || || ||
   // \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/
   // Actual test cases start below.  The test case skeletons are:
   //
   // void someTestCase()
   // {
   //     instantiateAllTestFixtures( <appearance group xml file to be used by SAA for test case>,
   //                                 <name of subscription DB to be used by SAA for test case>,
   //                                 <name of credential DB to be used by SAA for test case>,
   //                                 <domain name to be used for the test case>
   //                               );
   //     ...
   //     <add test case code here>
   //     ...
   // }
   //
   //____________________________________________________

   void AppearanceTest()
   {
      instantiateAllTestFixtures( "appearance-groups1.xml",
                                  "subscription1",
                                  "credential1",
                                  "177.0.0.1:54140");

      UtlString sharedUri = "sip:321@177.0.0.1:54140";
      UtlString app1uri = "sip:127.0.0.1:45141";
      UtlString dialogHandle;

      // receive the reg-info subscribe
      SipMessage request;
      UtlString b;
      ssize_t l;
      while(getNextMessageFromAppearanceAgentUnderTest( request, 5 ))
      {
         request.getBytes(&b, &l);
         OsSysLog::add(FAC_RLS, PRI_DEBUG, "got message %s", b.data());
         UtlString method;
         request.getRequestMethod(&method);
         if(!request.isResponse() &&
            0 == method.compareTo(SIP_SUBSCRIBE_METHOD) )
         {
            // Accept the Subscription, regardless of whether it for a 'dialog' or 'reg' event
            // in order to stop retransmissions
            SipMessage regResponse;
            regResponse.setResponseData(&request, 202, "Accepted", app1uri);
            SipMessage * dispatchedMessage = new SipMessage(regResponse);
            dispatchedMessage->getBytes(&b, &l);
            OsSysLog::add(FAC_RLS, PRI_DEBUG, "sent message %s", b.data());
            pAppearanceAgentUnderTest->mServerUserAgent.dispatch(dispatchedMessage);

            // Deal with the two events separately
            UtlString eventField;
            request.getEventField(eventField);
            if(0 == eventField.compareTo("reg"))
            {
               UtlString contactInfo;
               request.getContactUri(0, &contactInfo);
               UtlString callid;
               request.getCallIdField(&callid);
               int cseq;
               request.getCSeqField(&cseq, NULL);

               Url toField;
               regResponse.getToUrl(toField);

               SipMessage regNotify;
               regNotify.setNotifyData(&request, 1, "", "", "reg");
               UtlString regInfo ("<?xml version=\"1.0\"?>\r\n"
                                 "<reginfo xmlns=\"urn:ietf:params:xml:ns:reginfo\" "
                                 "xmlns:gr=\"urn:ietf:params:xml:ns:gruuinfo\" version=\"911\" state=\"full\">\r\n"
                                 "   <registration aor=\"sip:321@177.0.0.1:54140\" id=\"sip:321@177.0.0.1:54140\" state=\"active\">\r\n "
                                 "      <contact id=\"sip:321@177.0.0.1:54140@@&lt;");
               regInfo.append(contactInfo);
               regInfo.append("&gt;\" state=\"active\" event=\"registered\" q=\"1\" callid=\"");
               regInfo.append(callid);
               regInfo.append("\" cseq=\"");
               regInfo.appendNumber(cseq);
               regInfo.append("\">\r\n");
               regInfo.append("<uri>");
               regInfo.append(app1uri);
               regInfo.append("</uri>");
               regInfo.append("      </contact>\r\n"
                              "   </registration>\r\n"
                              "</reginfo>");
               HttpBody * newBody = new HttpBody (regInfo, strlen(regInfo), "application/reginfo+xml");
               regNotify.setContentType("application/reginfo+xml");
               regNotify.setBody(newBody);

               // Set the From field the same as the to field from the 202 response, as it
               // contains the dialog identifying to tags
               regNotify.setRawFromField(toField.toString().data());
               sendToAppearanceAgentUnderTest( regNotify );
               regNotify.getBytes(&b, &l);
               OsSysLog::add(FAC_RLS, PRI_DEBUG, "sent reg NOTIFY to AppAgent");
               OsSysLog::add(FAC_RLS, PRI_DEBUG, "sent message %s", b.data());
            }
            else if (0 == eventField.compareTo(SLA_EVENT_TYPE))
             {
                // should send empty NOTIFY, but no one will care
                // save dialogHandle for this subscription/Appearance (ignore retransmissions)
                if (dialogHandle.isNull())
                {
                   SipMessage fake(b);
                   fake.getDialogHandle(dialogHandle);
                   OsSysLog::add(FAC_RLS, PRI_DEBUG, "got SUBSCRIBE(sla) request: dialogHandle %s", dialogHandle.data());
                }
             }
         }
      }

      CPPUNIT_ASSERT( !dialogHandle.isNull() );
      OsSysLog::add(FAC_RLS, PRI_DEBUG, "we now have an Appearance - test it");
      AppearanceGroup* pAppGroup = pAppearanceAgentUnderTest->getAppearanceGroupSet().
         findAppearanceGroup(sharedUri);
      CPPUNIT_ASSERT( pAppGroup );

      Appearance* pApp = pAppGroup->findAppearance(dialogHandle);
      CPPUNIT_ASSERT( pApp );
      ASSERT_STR_EQUAL( app1uri.data(), pApp->getUri()->data() );

      // test adding a new dialog
      const char* dialogEventString =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" version=\"0\" state=\"partial\" entity=\"sip:321@177.0.0.1:54140\">\n"
            "<dialog id=\"1\" call-id=\"call-1116603513-890@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<local>\n"
            "<identity>moh@panther.pingtel.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\">\n"
            "<param pname=\"x-line-id\" pval=\"0\"/>\n"
            "<param pname=\"+sip.rendering\" pval=\"yes\"/>\n"
            "</target>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "</dialog-info>\n"
            ;
      SipDialogEvent dialogEvent(dialogEventString);
      bool bFullContentChanged = false;
      bool bPartialContentChanged = pApp->updateState(&dialogEvent, bFullContentChanged);
      CPPUNIT_ASSERT(bPartialContentChanged);
      CPPUNIT_ASSERT(bFullContentChanged);
      pApp->dumpState();
      CPPUNIT_ASSERT(pApp->appearanceIsBusy());
      CPPUNIT_ASSERT(pApp->appearanceIdIsSeized("0"));
      CPPUNIT_ASSERT(!pApp->appearanceIdIsSeized("1"));

      // simulate user putting the call on hold
      const char* dialogEventString2 =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" version=\"0\" state=\"partial\" entity=\"sip:321@177.0.0.1:54140\">\n"
            "<dialog id=\"1\" call-id=\"call-1116603513-890@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<local>\n"
            "<identity>moh@panther.pingtel.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\">\n"
            "<param pname=\"x-line-id\" pval=\"0\"/>\n"
            "<param pname=\"+sip.rendering\" pval=\"no\"/>\n"
            "</target>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "</dialog-info>\n"
            ;
      SipDialogEvent dialogEvent2(dialogEventString2);
      bPartialContentChanged = pApp->updateState(&dialogEvent2, bFullContentChanged);
      CPPUNIT_ASSERT(bPartialContentChanged);
      CPPUNIT_ASSERT(bFullContentChanged);
      pApp->dumpState();
      CPPUNIT_ASSERT(!pApp->appearanceIsBusy());
      CPPUNIT_ASSERT(pApp->appearanceIdIsSeized("0"));
      CPPUNIT_ASSERT(!pApp->appearanceIdIsSeized("1"));

      // test MESSAGE debug handling
      const char* message =
         "MESSAGE sip:~~sa~D~dumpstate@177.0.0.1:54140 SIP/2.0\r\n"
         "From: <sip:200@rlstest.test>;tag=17211757-9E4FBD78\r\n"
         "To: <sip:~~sa~D~dumpstate@177.0.0.1:54140>\r\n"
         "CSeq: 1 MESSAGE\r\n"
         "Call-ID: 51405734-b9be4835-dcd9d196\r\n"
         "Contact: <sip:331@10.10.10.1>\r\n"
         "Allow: INVITE, ACK, BYE, CANCEL, OPTIONS, INFO, MESSAGE, SUBSCRIBE, NOTIFY, PRACK, UPDATE, REFER\r\n"
         "Event: dialog\r\n"
         "User-Agent: UnitTest\r\n"
         "Accept-Language: en\r\n"
         "Accept: application/dialog-info+xml\r\n"
         "Max-Forwards: 70\r\n"
         "Expires: 3600\r\n"
         "Content-Length: 0\r\n"
         "\r\n";

      // send the MESSAGE
      SipMessage messageRequest( message, strlen( message ) );
      CPPUNIT_ASSERT( sendToAppearanceAgentUnderTest( messageRequest ) );

      // receive the 200 OK response
      SipMessage response;
      CPPUNIT_ASSERT( getNextMessageFromAppearanceAgentUnderTest( response, 5 ) );
      CPPUNIT_ASSERT( response.isResponse() );
      CPPUNIT_ASSERT( response.getResponseStatusCode() == SIP_OK_CODE );
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(AppearanceAgentTest);
