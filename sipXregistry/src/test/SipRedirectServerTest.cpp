// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <iostream>
#include <memory>
#include <sipxunit/TestUtilities.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include "registry/RedirectPlugin.h"
#include "registry/SipRedirectServer.h"

using namespace std;

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SipRedirectServerTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipRedirectServerTest);
   CPPUNIT_TEST( buildResponseFromRequestAndErrorDescriptorBaseTest );
   CPPUNIT_TEST( buildResponseFromRequestAndErrorDescriptorBaseWarningTest );
   CPPUNIT_TEST( buildResponseFromRequestAndErrorDescriptorKitchenSinkTest );
   CPPUNIT_TEST_SUITE_END();

public:
   SipRedirectServer* mpRedirectServer;   
   void setUp()
   {
      OsConfigDb* pConfigDb = new OsConfigDb();      
      mpRedirectServer      = new SipRedirectServer( pConfigDb, 0 );
      mpRedirectServer->mDefaultDomain = "example.com";
   }
   
   void buildResponseFromRequestAndErrorDescriptorBaseTest()
   {
      const char* request =
         "INVITE sip:601@192.168.1.11:5060 SIP/2.0\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=12345\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-\r\n"
         "Cseq: 1 INVITE\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=47.135.162.145;rport=14956\r\n"
         "\r\n";
      SipMessage requestMsg(request, strlen(request));
      
      ErrorDescriptor errorDescriptor;
      CPPUNIT_ASSERT(  errorDescriptor.setStatusLineData( 650, "there goes some error!" ) );
      CPPUNIT_ASSERT(  errorDescriptor.setWarningData( 350, "you have been warned!" ) );
      errorDescriptor.appendRequestToResponse();
      errorDescriptor.setAcceptFieldValue( "mastercard" );
      errorDescriptor.setAcceptEncodingFieldValue( "scrambled eggs");
      errorDescriptor.setAcceptLanguageFieldValue( "French" );
      errorDescriptor.setAllowFieldValue( "me" );
      errorDescriptor.setRequireFieldValue( "precision" );
      errorDescriptor.setRetryAfterFieldValue( "tomorrow" );
      errorDescriptor.setUnsupportedFieldValue( "feature" );

      SipMessage response;
      UtlString tmpString;
      const char* pTxt;
      mpRedirectServer->buildResponseFromRequestAndErrorDescriptor( response, requestMsg, errorDescriptor );

      // check the status line
      CPPUNIT_ASSERT( response.getResponseStatusCode() == 650 );
      response.getResponseStatusText( &tmpString );
      ASSERT_STR_EQUAL( "there goes some error!", tmpString.data() );
      
      // check warning header
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_WARNING_FIELD ) ) != 0 );
      ASSERT_STR_EQUAL( "350 example.com \"you have been warned!\"", pTxt );
      
      // check optional fields
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_ACCEPT_FIELD ) ) != 0 );
      ASSERT_STR_EQUAL( "mastercard", pTxt );
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_ACCEPT_ENCODING_FIELD ) ) != 0 );
      ASSERT_STR_EQUAL( "scrambled eggs", pTxt );
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_ACCEPT_LANGUAGE_FIELD ) ) != 0 );
      ASSERT_STR_EQUAL( "French", pTxt );
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_ALLOW_FIELD ) ) != 0 );
      ASSERT_STR_EQUAL( "me", pTxt );
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_REQUIRE_FIELD ) ) != 0 );
      ASSERT_STR_EQUAL( "precision", pTxt );
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_RETRY_AFTER_FIELD ) ) != 0 );
      ASSERT_STR_EQUAL( "tomorrow", pTxt );
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_UNSUPPORTED_FIELD ) ) != 0 );
      ASSERT_STR_EQUAL( "feature", pTxt );
      
      // check sipfrag body
      const HttpBody* pBody;
      int len;
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_CONTENT_TYPE_FIELD ) ) != 0 );
      ASSERT_STR_EQUAL( "message/sipfrag", pTxt );
      CPPUNIT_ASSERT( ( pBody = response.getBody() ) != 0 );
      pBody->getBytes( &tmpString, &len );
      CPPUNIT_ASSERT( strcmp( tmpString.data(), request ) == 0 );
   }

   void buildResponseFromRequestAndErrorDescriptorBaseWarningTest()
   {
      const char* request =
         "INVITE sip:601@192.168.1.11:5060 SIP/2.0\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=12345\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-\r\n"
         "Cseq: 1 INVITE\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=47.135.162.145;rport=14956\r\n"
         "\r\n";
      SipMessage requestMsg(request, strlen(request));
      
      ErrorDescriptor errorDescriptor;
      CPPUNIT_ASSERT(  errorDescriptor.setStatusLineData( 650, "there goes some error!" ) );

      SipMessage response;
      UtlString tmpString;
      const char* pTxt;
      mpRedirectServer->buildResponseFromRequestAndErrorDescriptor( response, requestMsg, errorDescriptor );

      // check the status line
      CPPUNIT_ASSERT( response.getResponseStatusCode() == 650 );
      response.getResponseStatusText( &tmpString );
      ASSERT_STR_EQUAL( "there goes some error!", tmpString.data() );
      
      // check warning header for absence
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_WARNING_FIELD ) ) == 0 );
      
      // check optional fields for absence
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_ACCEPT_FIELD ) ) == 0 );
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_ACCEPT_ENCODING_FIELD ) ) == 0 );
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_ACCEPT_LANGUAGE_FIELD ) ) == 0 );
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_ALLOW_FIELD ) ) == 0 );
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_REQUIRE_FIELD ) ) == 0 );
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_RETRY_AFTER_FIELD ) ) == 0 );
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_UNSUPPORTED_FIELD ) ) == 0 );
      
      // check sipfrag body for absence
      const HttpBody* pBody;
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_CONTENT_TYPE_FIELD ) ) == 0 );
      CPPUNIT_ASSERT( ( pBody = response.getBody() ) == 0 );
   }
   
   void buildResponseFromRequestAndErrorDescriptorKitchenSinkTest()
   {
      const char* request =
         "INVITE sip:601@192.168.1.11:5060 SIP/2.0\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=12345\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-\r\n"
         "Cseq: 1 INVITE\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=47.135.162.145;rport=14956\r\n"
         "\r\n";
      SipMessage requestMsg(request, strlen(request));
      
      ErrorDescriptor errorDescriptor;
      CPPUNIT_ASSERT(  errorDescriptor.setStatusLineData( 650, "there goes some error!" ) );
      CPPUNIT_ASSERT(  errorDescriptor.setWarningData( 350, "you have been warned!" ) );
      errorDescriptor.appendRequestToResponse();
      errorDescriptor.setAcceptFieldValue( "mastercard" );
      errorDescriptor.setAcceptEncodingFieldValue( "scrambled eggs");
      errorDescriptor.setAcceptLanguageFieldValue( "French" );
      errorDescriptor.setAllowFieldValue( "me" );
      errorDescriptor.setRequireFieldValue( "precision" );
      errorDescriptor.setRetryAfterFieldValue( "tomorrow" );
      errorDescriptor.setUnsupportedFieldValue( "feature" );

      SipMessage response;
      UtlString tmpString;
      const char* pTxt;
      mpRedirectServer->buildResponseFromRequestAndErrorDescriptor( response, requestMsg, errorDescriptor );

      // check the status line
      CPPUNIT_ASSERT( response.getResponseStatusCode() == 650 );
      response.getResponseStatusText( &tmpString );
      ASSERT_STR_EQUAL( "there goes some error!", tmpString.data() );
      
      // check warning header
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_WARNING_FIELD ) ) != 0 );
      ASSERT_STR_EQUAL( "350 example.com \"you have been warned!\"", pTxt );
      
      // check optional fields
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_ACCEPT_FIELD ) ) != 0 );
      ASSERT_STR_EQUAL( "mastercard", pTxt );
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_ACCEPT_ENCODING_FIELD ) ) != 0 );
      ASSERT_STR_EQUAL( "scrambled eggs", pTxt );
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_ACCEPT_LANGUAGE_FIELD ) ) != 0 );
      ASSERT_STR_EQUAL( "French", pTxt );
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_ALLOW_FIELD ) ) != 0 );
      ASSERT_STR_EQUAL( "me", pTxt );
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_REQUIRE_FIELD ) ) != 0 );
      ASSERT_STR_EQUAL( "precision", pTxt );
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_RETRY_AFTER_FIELD ) ) != 0 );
      ASSERT_STR_EQUAL( "tomorrow", pTxt );
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_UNSUPPORTED_FIELD ) ) != 0 );
      ASSERT_STR_EQUAL( "feature", pTxt );
      
      // check sipfrag body
      const HttpBody* pBody;
      int len;
      CPPUNIT_ASSERT( ( pTxt = response.getHeaderValue( 0, SIP_CONTENT_TYPE_FIELD ) ) != 0 );
      ASSERT_STR_EQUAL( "message/sipfrag", pTxt );
      CPPUNIT_ASSERT( ( pBody = response.getBody() ) != 0 );
      pBody->getBytes( &tmpString, &len );
      CPPUNIT_ASSERT( strcmp( tmpString.data(), request ) == 0 );
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipRedirectServerTest);
