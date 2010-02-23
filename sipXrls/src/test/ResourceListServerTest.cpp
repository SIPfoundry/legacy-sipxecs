// 
// Copyright (C) 2008 Nortel, certain elements licensed under a Contributor Agreement.  
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
#include "../ResourceListServer.h"
#include "RlsTestFixtures.h"

// GLOBALS
UtlBoolean    gShutdownFlag = FALSE;

class ResourceListServerTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(ResourceListServerTest);
#if 0 // temporarily get around test problems
   CPPUNIT_TEST(SubscribeWithEventListSupportAcceptedTest);
   CPPUNIT_TEST(SubscribeWithoutEventListSupportRejectedTest);
   CPPUNIT_TEST(SubscribeNothingSupportedRejectedTest);
   CPPUNIT_TEST(regInfoSubscribeWithGruuAddressTest);
   CPPUNIT_TEST(regInfoSubscribeWithBadlyFormattedGruuTest);
   CPPUNIT_TEST(regInfoSubscribeWithPathHeaderTest);
   CPPUNIT_TEST(regInfoSubscribeWithJustUriTest);
   CPPUNIT_TEST(regInfoSubscribeWithMultiplePathHeadersTest);
   CPPUNIT_TEST(regInfoSubscribeWithMultiplePathHeaderElementsTest);
#endif
   CPPUNIT_TEST_SUITE_END();

private:
   OutputProcessorFixture* pUacOutputProcessor;
   OutputProcessorFixture* pUasOutputProcessor;
   ResourceListServer*     pResourceServerUnderTest;
   SipUserAgent*           pSipUserAgent;
   SipDbTestContext        sipDbContext;
   UtlString               mCredentialDbName;
   
public:
   ResourceListServerTest() :
      pUacOutputProcessor( 0 ),
      pUasOutputProcessor( 0 ),
      pResourceServerUnderTest( 0 ),
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
   
   void instantiateAllTestFixtures( UtlString resourceListFile,
                                    //< Name of the resource list to use for the test.  The filename
                                    //< specified mus texist in .../sipXrls/src/test/rlsdata/
                                    UtlString subscriptionDbName, 
                                    //< Specifies the subscription DB to use for the test.  The name 
                                    //< is used to select which of the files in .../sipXrls/src/test/rlsdata/
                                    //< will get used to preload the subscription IMDB.  The name provided here is the 
                                    //< xml file in .../sipXrls/src/test/rlsdata/ without the ".xml" extension.
                                    UtlString credentialDbName,
                                    //< Specifies the credential DB to use for the test.  The name 
                                    //< is used to select which of the files in .../sipXrls/src/test/rlsdata/
                                    //< will get used to preload the credential IMDB.  The name provided here is the 
                                    //< xml file in .../sipXrls/src/test/rlsdata/ without the ".xml" extension.
                                    UtlString domianName,
                                    //< Specify the domain name to use for the test
                                    UtlBoolean routeAllRequestsToRlsServer
                                    //< Send all requests to either the RLS Server UA or the RLS Client UA
                                    //< The Client UA typically deals with the outgoing subscriptions and
                                    //< the Server UA deals witht the incoming subscriptions
                                    )
   {
      mCredentialDbName = credentialDbName;
      // force copy of input files into the 'work' directory
      sipDbContext.inputFile( subscriptionDbName + ".xml" );
      sipDbContext.inputFile( credentialDbName + ".xml" );
      UtlString tempResourceListFile = UtlString(TEST_DATA_DIR) + "/" + resourceListFile;

      pSipUserAgent = new SipUserAgent( 45141, 45141, 45142
                                       ,"127.0.0.1"  // default publicAddress
                                       ,NULL         // default defaultUser
                                       ,"127.0.0.1" );  // default defaultSipAddress

      // Stop SipUserAgent from rejecting all SUBSCRIBEs
      pSipUserAgent->allowMethod(SIP_SUBSCRIBE_METHOD, true);

      pResourceServerUnderTest = new ResourceListServer(
                                       domianName, // domain 
                                       "rlstest.test", // realm
                                       NULL, 
                                       DIALOG_EVENT_TYPE, 
                                      DIALOG_EVENT_CONTENT_TYPE,
                                       45140, // TCP port
                                       45140, // UDP port
                                       45140, // TLS port
                                       "127.0.0.1", // Bind IP address
                                       &tempResourceListFile,
                                       (60 * 60), // Default subscription resubscribe interval.
                                       (40 * 60), // Default minimum subscription resubscribe interval.
                                       250,  // publishing delay? 
                                       20,   // The maximum number of reg subscriptions per resource.
                                       20,   // The maximum number of contacts per reg subscription.
                                       20,   // The maximum number of resource instances per contact
                                       20,    // The maximum number of dialogs per resource instance
                                       32, 3600, 86400, // Subscribe server expiration parameters
                                       subscriptionDbName,
                                       credentialDbName );

      pUacOutputProcessor = new OutputProcessorFixture();
      pUasOutputProcessor = new OutputProcessorFixture();
      
      pResourceServerUnderTest->mClientUserAgent.addSipOutputProcessor( pUacOutputProcessor );
      pResourceServerUnderTest->mServerUserAgent.addSipOutputProcessor( pUasOutputProcessor );

      UtlString proxyAddress;
      if(routeAllRequestsToRlsServer)
      {
         proxyAddress = "127.0.0.1:45140";
      }
      else
      {
         int pPort;
         pResourceServerUnderTest->mClientUserAgent.getLocalAddress(&proxyAddress, &pPort);
         proxyAddress.append(':');
         proxyAddress.appendNumber(pPort);
      }

      pSipUserAgent->setProxyServers(proxyAddress.data());

      pResourceServerUnderTest->start();
   }
   
   // The freeAllTestFixtures() is automatically called after every test case so 
   // unit tests should not call this method.
   void freeAllTestFixtures()
   {
      pResourceServerUnderTest->mClientUserAgent.removeSipOutputProcessor( pUacOutputProcessor );     
      pResourceServerUnderTest->mServerUserAgent.removeSipOutputProcessor( pUasOutputProcessor );

      pResourceServerUnderTest->shutdown();
      delete pResourceServerUnderTest;
      pResourceServerUnderTest = 0;
         
      delete pUacOutputProcessor;
      pUacOutputProcessor = 0;
      
      delete pUasOutputProcessor;
      pUasOutputProcessor = 0;

      delete pSipUserAgent;
   }   
   
   // Primitive to send a SIP message to the RLS server under test
   bool  sendToRlsServerUnderTest( SipMessage& msg )
   {
      return pSipUserAgent->send( msg );
   }

   // Primitive to fetch the oldest unprocessed message sent by the 'server'
   // component of the RLS server.
   bool getNextMessageFromRlsServerUnderTest( SipMessage& message, int timeoutInSecs )
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
   
   // Primitive to fetch the oldest unprocessed message sent by the 'client'
   // component of the RLS server.
   bool getNextMessageFromRlsClientUnderTest( SipMessage& message, int timeoutInSecs )
   {
      bool result = false;
      if( pUacOutputProcessor->waitForMessage((long) timeoutInSecs ) == true )
      {
         CallbackTrace trace;
         result = pUacOutputProcessor->popNextCallbackTrace( trace );
         if( result )
         {
            message = trace.getMessage();
         }
      }
      return result;
   }
   
   // Adds an 'Authorization' header to the supplied request based on the challenge
   // contained in the supplied response.  In order to succeed, the SIP identity
   // designated in the From header of the request must have an entry in the 
   // credential database supplied to instantiateAllTestFixtures() 
   bool addCredentialsToRequest( SipMessage& request, const SipMessage& response )
   {
      bool result = false;
      CredentialDB* pCredDB = CredentialDB::getInstance( mCredentialDbName );
      if( pCredDB )
      {
         // retrieve information in the Www-authenticate: header
         UtlString dummy, nonce, realm;
         if( response.getAuthenticateData( &dummy,
                                           &realm,
                                           &nonce,
                                           &dummy,
                                           &dummy,
                                           &dummy,
                                           HttpMessage::SERVER,
                                           0 ) )
         {
            // look for credentials belonging to the requesting user.
            Url fromUrl;
            UtlString userId;
            UtlString authTypeDB;
            UtlString passTokenDB;
            UtlString fromUriAsString;
            UtlString user;

            request.getFromUrl(fromUrl); 
            fromUrl.getUserId( user );
            fromUrl.getUri( fromUriAsString );

            if( pCredDB->getCredential(fromUrl,
                                       realm,
                                       userId,
                                       passTokenDB,
                                       authTypeDB) )
            {
               // generate response hash
               // TBD - 25-jan-2010 work might be needed if these tests are re-enabled
               UtlString responseHash;
               UtlString method;

               request.getRequestMethod(&method);
               HttpMessage::buildMd5Digest(passTokenDB.data(),
                                           HTTP_MD5_ALGORITHM,
                                           nonce.data(),
                                           NULL, // client nonce
                                           1, // nonce count
                                           "",
                                           method.data(),
                                           fromUriAsString.data(),
                                           NULL,
                                           &responseHash
                                           );
               
               // add authorization header
               request.removeHeader( HTTP_AUTHORIZATION_FIELD, 0);
               request.setDigestAuthorizationData(         user.data(),
                                                           realm.data(),
                                                           nonce.data(),
                                                           fromUriAsString.data(),
                                                           responseHash.data(),
                                                           HTTP_MD5_ALGORITHM,
                                                           NULL,//clientNonce.data(),
                                                           NULL,
                                                           HTTP_QOP_AUTH,
                                                           1, // nonce count
                                                           HttpMessage::SERVER
                                                           );  
               result = true;
            }
         }
      }
      return result;
   }

   /// wrapper function for all reg-info event tests
   bool ContactSetTest(UtlString regContactxml, UtlString requestUri, UtlString route)
   {
      bool ret = FALSE;
      instantiateAllTestFixtures( "resource-lists2.xml", 
                                  "subscription1", 
                                  "credential1", 
                                  "sip:127.0.0.1:45141",
                                  FALSE);

      // receive the reg-info subscribe 
      SipMessage request;
      while(getNextMessageFromRlsClientUnderTest( request, 5 ) )
      {
         UtlString method;
         request.getRequestMethod(&method);
         if(!request.isResponse() &&
            0 == method.compareTo(SIP_SUBSCRIBE_METHOD) )
         {
            // Accept the Subscription, regardless of whether it for a 'dialog' or 'reg' event
            // in order to stop retransmissions
            SipMessage regResponse;
            regResponse.setResponseData(&request, 202, "Accepted", "sip:127.0.0.1:45141");
            SipMessage * dispatchedMessage = new SipMessage(regResponse);            
            pResourceServerUnderTest->mClientUserAgent.dispatch(dispatchedMessage);

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
                                 "   <registration aor=\"sip:332@rlstest.test\" id=\"sip:332@rlstest.test\" state=\"active\">\r\n "
                                 "      <contact id=\"sip:332@rlstest.test@@&lt;");
               regInfo.append(contactInfo);
               regInfo.append("&gt;\" state=\"active\" event=\"registered\" q=\"1\" callid=\"");
               regInfo.append(callid);
               regInfo.append("\" cseq=\"");
               regInfo.appendNumber(cseq);
               regInfo.append("\">\r\n");
               regInfo.append(regContactxml);
               regInfo.append("      </contact>\r\n"
                              "   </registration>\r\n"
                              "</reginfo>");
               HttpBody * newBody = new HttpBody (regInfo, strlen(regInfo), "application/reginfo+xml");
               regNotify.setContentType("application/reginfo+xml");
               regNotify.setBody(newBody);

               // Set the From field the same as the to field from the 202 response, as it
               // contains the dialog identifying to tags
               regNotify.setRawFromField(toField.toString().data());
               sendToRlsServerUnderTest( regNotify );
            }
            else if(0 == eventField.compareTo("dialog"))
            {
               // If we find a dialog event subscription with the request uri and route
               // that we are looking for, mark the test as passed

               UtlString uri;
               UtlString myRoute;
               request.getRequestUri(&uri);
               request.getRouteField(&myRoute); 
               if(0 == uri.compareTo(requestUri) &&
                  0 == route.compareTo(myRoute))
               {
                  ret = true;
               }
            }
         }
      }
      return ret;
   }

   //____________________________________________________
   // || || || || || || || || || || || || || || || || || 
   // \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ 
   // Actual test cases start below.  The test case skeletons are:
   //
   // void someTestCase()
   // {
   //     instantiateAllTestFixtures( <resource list xml file to be used by Rls Server for test case>,
   //                                 <name of subscription DB to be used by Rls Server for test case>,
   //                                 <name of credential DB to be used by Rls Server for test case>,
   //                                 <domain name to be used for the test case>,
   //                                 <should requests be sent to the SERVER or CLIENT UA>
   //                               );
   //     ...
   //     <add test case code here>
   //     ...
   // }
   //
   // void someRegEventTestCase()
   // {
   //      UtlString regInfoContact; < The contact info xml to be tested>
   //      ...
   //      CPPUNIT_ASSERT( ContactSetTest( <contact info xml with the uri, gruu, and path headers>, 
   //                                      <expected request uri, if the reg-info is parsed correctly>, 
   //                                      <expected route, if the reg-info is parsed correctly>
   //                                    );
   // }
   //
   //____________________________________________________

   void SubscribeWithEventListSupportAcceptedTest()
   {
      instantiateAllTestFixtures( "resource-lists1.xml", 
                                  "subscription1", 
                                  "credential1", 
                                  "rlstest.test",
                                  TRUE);

      const char* message = 
         "SUBSCRIBE sip:~~rl~F~331@177.0.0.1:54140 SIP/2.0\r\n"
         "From: <sip:200@rlstest.test>;tag=17211757-9E4FBD78\r\n"
         "To: <sip:~~rl~F~331@rlstest.test>\r\n"
         "CSeq: 1 SUBSCRIBE\r\n"
         "Call-ID: 51405734-b9be4835-dcd9d196\r\n"
         "Contact: <sip:331@10.10.10.1>\r\n"
         "Allow: INVITE, ACK, BYE, CANCEL, OPTIONS, INFO, MESSAGE, SUBSCRIBE, NOTIFY, PRACK, UPDATE, REFER\r\n"
         "Event: dialog\r\n"
         "User-Agent: UnitTest\r\n"
         "Accept-Language: en\r\n"
         "Supported: eventlist\r\n"
         "Accept: application/dialog-info+xml,application/rlmi+xml,multipart/related\r\n"
         "Max-Forwards: 70\r\n"
         "Expires: 3600\r\n"
         "Content-Length: 0\r\n"
         "\r\n";   

      // send the SUBSCRIBE
      SipMessage request( message, strlen( message ) );
      CPPUNIT_ASSERT( sendToRlsServerUnderTest( request ) );

      // receive the 401 Unauthorized 
      SipMessage response;
      CPPUNIT_ASSERT( getNextMessageFromRlsServerUnderTest( response, 5 ) );
      CPPUNIT_ASSERT( response.isResponse() );
      CPPUNIT_ASSERT( response.getResponseStatusCode() == HTTP_UNAUTHORIZED_CODE );

      // craft a new SUBSCRIBE with proper credentials and send it
      CPPUNIT_ASSERT( addCredentialsToRequest( request, response ) );
      CPPUNIT_ASSERT( sendToRlsServerUnderTest( request ) );

      // receive the 202 Accepted response 
      CPPUNIT_ASSERT( getNextMessageFromRlsServerUnderTest( response, 5 ) );
      CPPUNIT_ASSERT( response.isResponse() );
      CPPUNIT_ASSERT( response.getResponseStatusCode() == SIP_ACCEPTED_CODE );

      // requisite first NOTIFY request
      SipMessage notify_request;
      CPPUNIT_ASSERT( getNextMessageFromRlsServerUnderTest( notify_request, 15 ) );
      CPPUNIT_ASSERT( ! notify_request.isResponse() );
      UtlString tmp;
      notify_request.getRequestMethod(&tmp);
      CPPUNIT_ASSERT( 0 == tmp.compareTo( SIP_NOTIFY_METHOD ) );
      CPPUNIT_ASSERT( notify_request.isInRequireField(SIP_EVENTLIST_EXTENSION) );

      // no further NOTIFY request
/* XECS-2005
      CPPUNIT_ASSERT( ! getNextMessageFromRlsServerUnderTest( notify_request, 5 ) );
//*/
   }

   void SubscribeWithoutEventListSupportRejectedTest()
   {
      instantiateAllTestFixtures( "resource-lists1.xml", 
                                  "subscription1", 
                                  "credential1", 
                                  "rlstest.test",
                                  TRUE);

      const char* message = 
         "SUBSCRIBE sip:~~rl~F~331@177.0.0.1:54140 SIP/2.0\r\n"
         "From: <sip:200@rlstest.test>;tag=17211757-9E4FBD78\r\n"
         "To: <sip:~~rl~F~331@rlstest.test>\r\n"
         "CSeq: 1 SUBSCRIBE\r\n"
         "Call-ID: 51405734-b9be4835-dcd9d196\r\n"
         "Contact: <sip:331@10.10.10.1>\r\n"
         "Allow: INVITE, ACK, BYE, CANCEL, OPTIONS, INFO, MESSAGE, SUBSCRIBE, NOTIFY, PRACK, UPDATE, REFER\r\n"
         "Event: dialog\r\n"
         "User-Agent: UnitTest\r\n"
         "Accept-Language: en\r\n"
         "Supported: something else\r\n"
         "Accept: application/dialog-info+xml,application/rlmi+xml,multipart/related\r\n"
         "Max-Forwards: 70\r\n"
         "Expires: 3600\r\n"
         "Content-Length: 0\r\n"
         "\r\n";   

      // send the SUBSCRIBE
      SipMessage request( message, strlen( message ) );
      CPPUNIT_ASSERT( sendToRlsServerUnderTest( request ) );

      // receive the 401 Unauthorized 
      SipMessage response;
      CPPUNIT_ASSERT( getNextMessageFromRlsServerUnderTest( response, 5 ) );
      CPPUNIT_ASSERT( response.isResponse() );
      CPPUNIT_ASSERT( response.getResponseStatusCode() == HTTP_UNAUTHORIZED_CODE );

      // craft a new SUBSCRIBE with proper credentials and send it
      CPPUNIT_ASSERT( addCredentialsToRequest( request, response ) );
      CPPUNIT_ASSERT( sendToRlsServerUnderTest( request ) );

      // receive the 421 Extension Required response 
      CPPUNIT_ASSERT( getNextMessageFromRlsServerUnderTest( response, 5 ) );
      CPPUNIT_ASSERT( response.isResponse() );
      CPPUNIT_ASSERT( response.getResponseStatusCode() == SIP_EXTENSION_REQUIRED_CODE );
      const char *pRequireFieldValue = response.getHeaderValue( 0, SIP_REQUIRE_FIELD );
      CPPUNIT_ASSERT( pRequireFieldValue );
      ASSERT_STR_EQUAL( SIP_EVENTLIST_EXTENSION, pRequireFieldValue );
      
      // no requisite first NOTIFY request
      SipMessage notify_request;
      CPPUNIT_ASSERT( ! getNextMessageFromRlsServerUnderTest( notify_request, 5 ) );
   }

   void SubscribeNothingSupportedRejectedTest()
   {
      instantiateAllTestFixtures( "resource-lists1.xml", 
                                  "subscription1", 
                                  "credential1", 
                                  "rlstest.test",
                                  TRUE);

      const char* message = 
         "SUBSCRIBE sip:~~rl~F~331@177.0.0.1:54140 SIP/2.0\r\n"
         "From: <sip:200@rlstest.test>;tag=17211757-9E4FBD78\r\n"
         "To: <sip:~~rl~F~331@rlstest.test>\r\n"
         "CSeq: 1 SUBSCRIBE\r\n"
         "Call-ID: 51405734-b9be4835-dcd9d196\r\n"
         "Contact: <sip:331@10.10.10.1>\r\n"
         "Allow: INVITE, ACK, BYE, CANCEL, OPTIONS, INFO, MESSAGE, SUBSCRIBE, NOTIFY, PRACK, UPDATE, REFER\r\n"
         "Event: dialog\r\n"
         "User-Agent: UnitTest\r\n"
         "Accept-Language: en\r\n"
         "Accept: application/dialog-info+xml,application/rlmi+xml,multipart/related\r\n"
         "Max-Forwards: 70\r\n"
         "Expires: 3600\r\n"
         "Content-Length: 0\r\n"
         "\r\n";   

      // send the SUBSCRIBE
      SipMessage request( message, strlen( message ) );
      CPPUNIT_ASSERT( sendToRlsServerUnderTest( request ) );

      // receive the 401 Unauthorized 
      SipMessage response;
      CPPUNIT_ASSERT( getNextMessageFromRlsServerUnderTest( response, 5 ) );
      CPPUNIT_ASSERT( response.isResponse() );
      CPPUNIT_ASSERT( response.getResponseStatusCode() == HTTP_UNAUTHORIZED_CODE );

      // craft a new SUBSCRIBE with proper credentials and send it
      CPPUNIT_ASSERT( addCredentialsToRequest( request, response ) );
      CPPUNIT_ASSERT( sendToRlsServerUnderTest( request ) );

      // receive the 421 Extension Required response 
      CPPUNIT_ASSERT( getNextMessageFromRlsServerUnderTest( response, 5 ) );
      CPPUNIT_ASSERT( response.isResponse() );
      CPPUNIT_ASSERT( response.getResponseStatusCode() == SIP_EXTENSION_REQUIRED_CODE );
      const char *pRequireFieldValue = response.getHeaderValue( 0, SIP_REQUIRE_FIELD );
      CPPUNIT_ASSERT( pRequireFieldValue );
      ASSERT_STR_EQUAL( SIP_EVENTLIST_EXTENSION, pRequireFieldValue );

      // no requisite first NOTIFY request
      SipMessage notify_request;
      CPPUNIT_ASSERT( ! getNextMessageFromRlsServerUnderTest( notify_request, 5 ) );
   }

    // ================================================
    //  REG-INFO EVENT SPECIFIC TEST CASES START HERE:
    // ================================================


   void regInfoSubscribeWithGruuAddressTest()
   {
      UtlString regContactxml ("         <uri>sip:user@127.0.0.1:45141</uri>\r\n"
                               "         <unknown-param name=\"path\">&lt;sip:127.0.0.1:45141&gt;</unknown-param>\r\n"
                               "         <unknown-param name=\"+sip.instance\">\"&lt;urn:uuid:f81d4fae"
                               "-7dec-11d0-a765-00a0c91e6bf6&gt;\"</unknown-param>\r\n"
                               "         <gr:pub-gruu uri=\"sip:~~gr~hha9s8d-999@127.0.0.1:45141;gr\"/>\r\n");

      CPPUNIT_ASSERT(ContactSetTest(regContactxml, "sip:~~gr~hha9s8d-999@127.0.0.1:45141;gr", ""));
   }

   void regInfoSubscribeWithBadlyFormattedGruuTest()
   {
      UtlString regContactxml ("         <uri>sip:user@127.0.0.1:45141</uri>\r\n"
                               "         <unknown-param name=\"path\">&lt;sip:127.0.0.1:45141&gt;</unknown-param>\r\n"
                               "         <unknown-param name=\"+sip.instance\">\"&lt;urn:uuid:f81d4fae"
                               "-7dec-11d0-a765-00a0c91e6bf6&gt;\"</unknown-param>\r\n"
                               "         <gr:pub-gruu uri=\"~~gr~hha9s8d-999@127.0.0.1:45141;gr\"/>\r\n");

      CPPUNIT_ASSERT(ContactSetTest(regContactxml, "sip:~~gr~hha9s8d-999@127.0.0.1:45141;gr", ""));
   }

   void regInfoSubscribeWithPathHeaderTest()
   {
      UtlString regContactxml ("         <uri>sip:user@127.0.0.1:45141</uri>\r\n"
                               "         <unknown-param name=\"path\">&lt;sip:127.0.0.1:45141&gt;</unknown-param>\r\n");

      KNOWN_BUG("INTERMITTENT failures", "XX-6383");            
      CPPUNIT_ASSERT(ContactSetTest(regContactxml, "sip:user@127.0.0.1:45141", "<sip:127.0.0.1:45141;lr>"));
   }

   void regInfoSubscribeWithJustUriTest()
   {
      UtlString regContactxml ("         <uri>sip:user@127.0.0.1:45141</uri>\r\n"
                               "         <unknown-param name=\"+sip.instance\">\"&lt;urn:uuid:f81d4fae"
                                                              "-7dec-11d0-a765-00a0c91e6bf6&gt;\"</unknown-param>\r\n");
      KNOWN_BUG("INTERMITTENT failures", "XX-6383");            
      CPPUNIT_ASSERT(ContactSetTest(regContactxml, "sip:user@127.0.0.1:45141", ""));
   }

   void regInfoSubscribeWithMultiplePathHeadersTest()
   {
      UtlString regContactxml ("         <uri>sip:user@127.0.0.1:45141</uri>\r\n"
                               "         <unknown-param name=\"path\">&lt;sip:127.0.0.1:45141&gt;,&lt;sip:127.0.0.1:45142&gt;</unknown-param>\r\n");

      CPPUNIT_ASSERT(ContactSetTest(regContactxml, "sip:user@127.0.0.1:45141", "<sip:127.0.0.1:45141;lr>,<sip:127.0.0.1:45142;lr>"));
   }

   void regInfoSubscribeWithMultiplePathHeaderElementsTest()
   {
      UtlString regContactxml ("         <uri>sip:user@127.0.0.1:45141</uri>\r\n"
                               "         <unknown-param name=\"path\">&lt;sip:127.0.0.1:45142&gt;</unknown-param>\r\n"
                               "         <unknown-param name=\"path\">&lt;sip:127.0.0.1:45141&gt;</unknown-param>\r\n"
                               "         <unknown-param name=\"+sip.instance\">\"&lt;urn:uuid:f81d4fae"
                               "-7dec-11d0-a765-00a0c91e6bf6&gt;\"</unknown-param>\r\n");

      CPPUNIT_ASSERT(ContactSetTest(regContactxml, "sip:user@127.0.0.1:45141", "<sip:127.0.0.1:45141;lr>,<sip:127.0.0.1:45142;lr>"));
   }

};
   
CPPUNIT_TEST_SUITE_REGISTRATION(ResourceListServerTest);
