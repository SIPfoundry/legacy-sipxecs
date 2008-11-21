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
   CPPUNIT_TEST(SubscribeWithEventListSupportAcceptedTest);
   CPPUNIT_TEST(SubscribeWithoutEventListSupportRejectedTest);
   CPPUNIT_TEST(SubscribeNothingSupportedRejectedTest);
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
   }
   
   void instantiateAllTestFixtures( UtlString resourceListFile,
                                    //< Name of the resource list to use for the test.  The filename
                                    //< specified mus texist in .../sipXrls/src/test/rlsdata/
                                    UtlString subscriptionDbName, 
                                    //< Specifies the subscription DB to use for the test.  The name 
                                    //< is used to select which of the files in .../sipXrls/src/test/rlsdata/
                                    //< will get used to preload the subscription IMDB.  The name provided here is the 
                                    //< xml file in .../sipXrls/src/test/rlsdata/ without the ".xml" extension.
                                    UtlString credentialDbName 
                                    //< Specifies the credential DB to use for the test.  The name 
                                    //< is used to select which of the files in .../sipXrls/src/test/rlsdata/
                                    //< will get used to preload the credential IMDB.  The name provided here is the 
                                    //< xml file in .../sipXrls/src/test/rlsdata/ without the ".xml" extension.
                                    )
   {
      mCredentialDbName = credentialDbName;
      // force copy of input files into the 'work' directory
      sipDbContext.inputFile( subscriptionDbName + ".xml" );
      sipDbContext.inputFile( credentialDbName + ".xml" );
      UtlString tempResourceListFile = UtlString(TEST_DATA_DIR) + "/" + resourceListFile;
      
      pResourceServerUnderTest = new ResourceListServer(
                                       "rlstest.test", // domain 
                                       "rlstest.test", // realm
                                       DIALOG_EVENT_TYPE, 
                                       DIALOG_EVENT_CONTENT_TYPE,
                                       45140, // TCP port
                                       45140, // UDP port
                                       45140, // TLS port
                                       "127.0.0.1", // Bind IP address
                                       &tempResourceListFile,
                                       (24 * 60 * 60), // Default subscription refresh interval
                                       (60 * 60), // Default subscription resubscribe interval.
                                       (40 * 60), // Default minimum subscription resubscribe interval.
                                       250,  // publishing delay? 
                                       20,   // The maximum number of reg subscriptions per resource.
                                       20,   // The maximum number of contacts per reg subscription.
                                       20,   // The maximum number of resource instances per contact
                                       20,    // The maximum number of dialogs per resource instance
                                       subscriptionDbName,
                                       credentialDbName );
   
      pUacOutputProcessor = new OutputProcessorFixture();
      pUasOutputProcessor = new OutputProcessorFixture();
      
      pResourceServerUnderTest->mClientUserAgent.addSipOutputProcessor( pUacOutputProcessor );
      pResourceServerUnderTest->mServerUserAgent.addSipOutputProcessor( pUasOutputProcessor );
 
      pSipUserAgent = new SipUserAgent( 45141, 45141, 45142
                                       ,"127.0.0.1"  // default publicAddress
                                       ,NULL         // default defaultUser
                                       ,"127.0.0.1"  // default defaultSipAddress
                                       ,"127.0.0.1:45140" ); // Rls Server
   }
   
   // The freeAllTestFixtures() has to be called for every call to instantiateAllTestFixtures()
   // after a test case is completed.
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
      if( pUasOutputProcessor->waitForMessages( 1, timeoutInSecs ) == true )
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
      if( pUacOutputProcessor->waitForMessages( 1, timeoutInSecs ) == true )
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
         if( response.getAuthenticationData( &dummy,
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
   //____________________________________________________
   // || || || || || || || || || || || || || || || || || 
   // \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ 
   // Acutal test cases start below.  The test case skeleton is:
   //
   // void someTestCase()
   // {
   //     instantiateAllTestFixtures( <resource list xml file to be used by Rls Server for test case>,
   //                                 <name of subscription DB to be used by Rls Server for test case>,
   //                                 <name of credential DB to be used by Rls Server for test case> 
   //                               );
   //     ...
   //     <add test case code here>
   //     ...
   //     freeAllTestFixtures();
   // }
   //____________________________________________________
   
   void SubscribeWithEventListSupportAcceptedTest()
   {
      instantiateAllTestFixtures( "resource-lists1.xml", "subscription1", "credential1" );

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

      freeAllTestFixtures();
   }

   void SubscribeWithoutEventListSupportRejectedTest()
   {
      instantiateAllTestFixtures( "resource-lists1.xml", "subscription1", "credential1" );

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
      ASSERT_STR_EQUAL( "eventlist", pRequireFieldValue );
      
      freeAllTestFixtures();
   }

   void SubscribeNothingSupportedRejectedTest()
   {
      instantiateAllTestFixtures( "resource-lists1.xml", "subscription1", "credential1" );

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
      ASSERT_STR_EQUAL( "eventlist", pRequireFieldValue );

      freeAllTestFixtures();
   }

};
   
CPPUNIT_TEST_SUITE_REGISTRATION(ResourceListServerTest);
