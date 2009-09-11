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
#include <string>

#include "os/OsSysLog.h"

#include "utl/UtlSList.h"
#include "RouteState.h"

// CONSTANTS
static const char Value1[] = "value1";
static const char Value2[] = "value2";

/**
 * Unit test for RouteState::proxyMessage
 *
 *  These tests use only loose routes because the adjustment of a strict-routed message into
 *  a loose-routed message is tested in sipXtackLib/src/test/net/SipProxyMessageTest
 *  Similarly, the detailed testing of matching for forwarding rules is done in the
 *  ForwardRulesTest in this directory.
 */
class RouteStateTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(RouteStateTest);

   CPPUNIT_TEST(testIsMutable);
   CPPUNIT_TEST(testGetSetUnsetParameter);
   CPPUNIT_TEST(testSetUnsetUnMutable);
   CPPUNIT_TEST(testNameValidity);
   CPPUNIT_TEST(testTokenEncodeDecode);
   CPPUNIT_TEST(testRoutedRequestState);
   CPPUNIT_TEST(testAppendToExistingRecordRoute);
   CPPUNIT_TEST(testSpiraledState);
   CPPUNIT_TEST(testNewDialogState);
   CPPUNIT_TEST(testAppendedState);
   CPPUNIT_TEST(testAddCopy);
   CPPUNIT_TEST(testAuthorizedDialog);
   CPPUNIT_TEST(testOriginalFromTagValue);
   CPPUNIT_TEST(testDirectionIsCallerToCalled);

   CPPUNIT_TEST_SUITE_END();

private:

public:

   void setUp()
      {
      }

   void tearDown()
      {
      }

   void testIsMutable()
      {
         UtlSList removedHeaders;

         // test that an INVITE with no To tag and no routes or Record-Routes is mutable
         const char* mutableMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage mutableSipMessage(mutableMessage, strlen(mutableMessage));
         UtlString myRouteName("myhost.example.com");
         RouteState mutableRouteState(mutableSipMessage, removedHeaders, myRouteName);

         CPPUNIT_ASSERT(mutableRouteState.isMutable());
         CPPUNIT_ASSERT(!mutableRouteState.isFound());

         // test that an in-dialog request (has a To tag) is not mutable
         const char* indialogMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com; tag=MAKES_THIS_UNMUTABLE\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage indialogSipMessage(indialogMessage, strlen(indialogMessage));
         RouteState indialogRouteState(indialogSipMessage, removedHeaders, myRouteName);

         CPPUNIT_ASSERT(!indialogRouteState.isMutable());
         CPPUNIT_ASSERT(!indialogRouteState.isFound());

         // test that an ACK is not mutable
         const char* ackMessage =
            "ACK sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 ACK\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage ackSipMessage(ackMessage, strlen(indialogMessage));
         RouteState ackRouteState(ackSipMessage, removedHeaders, myRouteName);

         CPPUNIT_ASSERT(!ackRouteState.isMutable());
         CPPUNIT_ASSERT(!ackRouteState.isFound());

         // test that a response is not mutable
         const char* responseMessage =
            "SIP/2.0 200 Ok\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage responseSipMessage(responseMessage, strlen(responseMessage));
         RouteState responseRouteState(responseSipMessage, removedHeaders, myRouteName);

         CPPUNIT_ASSERT(!responseRouteState.isMutable());
         CPPUNIT_ASSERT(!responseRouteState.isFound());

      }

   void testGetSetUnsetParameter()
      {
         UtlSList removedHeaders;

         //
         const char* mutableMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage mutableSipMessage(mutableMessage, strlen(mutableMessage));
         UtlString myRouteName("myhost.example.com");
         RouteState mutableRouteState(mutableSipMessage, removedHeaders, myRouteName);

         // Check that no parameter is returned when non has been set
         UtlString value;
         const char* instanceName = "myinstance";
         const char* paramName    = "myparam";

         value.append("bad value"); // should be removed
         CPPUNIT_ASSERT(!mutableRouteState.getParameter(instanceName,paramName,value));
         CPPUNIT_ASSERT(value.isNull());

         // Now try to set that value
         UtlString inputValue("good value");
         mutableRouteState.setParameter(instanceName,paramName,inputValue);

         // ... and check that we can read it back
         value.append("bad value"); // should be replaced with "good value"
         CPPUNIT_ASSERT(mutableRouteState.getParameter(instanceName,paramName,value));
         CPPUNIT_ASSERT(value.compareTo(inputValue) == 0);

         // check that nothing is returned with a bad instance name
         const char* badInstanceName = "badinstance";
         value.append("bad value"); // should be removed
         CPPUNIT_ASSERT(!mutableRouteState.getParameter(badInstanceName,paramName,value));
         CPPUNIT_ASSERT(value.isNull());

         // check that nothing is returned with a bad parameter name
         const char* badParamName    = "badparam";
         value.append("bad value"); // should be removed
         CPPUNIT_ASSERT(!mutableRouteState.getParameter(instanceName,badParamName,value));
         CPPUNIT_ASSERT(value.isNull());

         // check that nothing is returned with both names bad
         value.append("bad value"); // should be removed
         CPPUNIT_ASSERT(!mutableRouteState.getParameter(badInstanceName,badParamName,value));
         CPPUNIT_ASSERT(value.isNull());

         // check that nothing is returned with a null instance name
         const char* nullInstanceName = "";
         value.append("bad value"); // should be removed
         CPPUNIT_ASSERT(!mutableRouteState.getParameter(nullInstanceName,paramName,value));
         CPPUNIT_ASSERT(value.isNull());

         // check that nothing is returned with a null parameter name
         const char* nullParamName    = "";
         value.append("bad value"); // should be removed
         CPPUNIT_ASSERT(!mutableRouteState.getParameter(instanceName,badParamName,value));
         CPPUNIT_ASSERT(value.isNull());

         // check that nothing is returned with both names null
         value.append("bad value"); // should be removed
         CPPUNIT_ASSERT(!mutableRouteState.getParameter(nullInstanceName,nullParamName,value));
         CPPUNIT_ASSERT(value.isNull());

         // recheck that the good value is still there
         value.append("bad value"); // should be replaced with "good value"
         CPPUNIT_ASSERT(mutableRouteState.getParameter(instanceName,paramName,value));
         CPPUNIT_ASSERT(value.compareTo(inputValue) == 0);

         // unset a non-existent value
         mutableRouteState.unsetParameter(badInstanceName,badParamName);

         // and confirm that value has not been set
         value.append("bad value"); // should be removed
         CPPUNIT_ASSERT(!mutableRouteState.getParameter(badInstanceName,badParamName,value));
         CPPUNIT_ASSERT(value.isNull());

         // unset the one valid value
         mutableRouteState.unsetParameter(instanceName,paramName);

         // and confirm that value is now not set
         value.append("bad value"); // should be removed
         CPPUNIT_ASSERT(!mutableRouteState.getParameter(instanceName,paramName,value));
         CPPUNIT_ASSERT(value.isNull());
      }

   void testSetUnsetUnMutable()
      {
         UtlSList removedHeaders;

         //
         const char* unMutableMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com; tag=MAKES_THIS_UNMUTABLE\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage unMutableSipMessage(unMutableMessage, strlen(unMutableMessage));
         UtlString myRouteName("myhost.example.com");
         RouteState unMutableRouteState(unMutableSipMessage, removedHeaders, myRouteName );

         // Check that no parameter is returned when non has been set
         UtlString value;
         const char* instanceName = "myinstance";
         const char* paramName    = "myparam";

         value.append("bad value"); // should be removed
         CPPUNIT_ASSERT(!unMutableRouteState.getParameter(instanceName,paramName,value));
         CPPUNIT_ASSERT(value.isNull());

         // Now try to set that value
         UtlString inputValue("good value");
         unMutableRouteState.setParameter(instanceName,paramName,inputValue);

         // ... and check that we can NOT read it back
         value.append("bad value"); // should be replaced with "good value"
         CPPUNIT_ASSERT(!unMutableRouteState.getParameter(instanceName,paramName,value));
         CPPUNIT_ASSERT(value.isNull());

         // check that nothing is returned with a bad instance name
         const char* badInstanceName = "badinstance";
         value.append("bad value"); // should be removed
         CPPUNIT_ASSERT(!unMutableRouteState.getParameter(badInstanceName,paramName,value));
         CPPUNIT_ASSERT(value.isNull());

         // check that nothing is returned with a bad parameter name
         const char* badParamName    = "badparam";
         value.append("bad value"); // should be removed
         CPPUNIT_ASSERT(!unMutableRouteState.getParameter(instanceName,badParamName,value));
         CPPUNIT_ASSERT(value.isNull());

         // check that nothing is returned with both names bad
         value.append("bad value"); // should be removed
         CPPUNIT_ASSERT(!unMutableRouteState.getParameter(badInstanceName,badParamName,value));
         CPPUNIT_ASSERT(value.isNull());

         // check that nothing is returned with a null instance name
         const char* nullInstanceName = "";
         value.append("bad value"); // should be removed
         CPPUNIT_ASSERT(!unMutableRouteState.getParameter(nullInstanceName,paramName,value));
         CPPUNIT_ASSERT(value.isNull());

         // check that nothing is returned with a null parameter name
         const char* nullParamName    = "";
         value.append("bad value"); // should be removed
         CPPUNIT_ASSERT(!unMutableRouteState.getParameter(instanceName,badParamName,value));
         CPPUNIT_ASSERT(value.isNull());

         // check that nothing is returned with both names null
         value.append("bad value"); // should be removed
         CPPUNIT_ASSERT(!unMutableRouteState.getParameter(nullInstanceName,nullParamName,value));
         CPPUNIT_ASSERT(value.isNull());

         // unset a non-existent value
         unMutableRouteState.unsetParameter(badInstanceName,badParamName);

         // and confirm that value has not been set
         value.append("bad value"); // should be removed
         CPPUNIT_ASSERT(!unMutableRouteState.getParameter(badInstanceName,badParamName,value));
         CPPUNIT_ASSERT(value.isNull());

         // unset the one valid value
         unMutableRouteState.unsetParameter(instanceName,paramName);

         // and confirm that value is now not set
         value.append("bad value"); // should be removed
         CPPUNIT_ASSERT(!unMutableRouteState.getParameter(instanceName,paramName,value));
         CPPUNIT_ASSERT(value.isNull());
      }

   void testNameValidity()
      {
         UtlSList removedHeaders;

         //
         const char* mutableMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage mutableSipMessage(mutableMessage, strlen(mutableMessage));
         UtlString myRouteName("myhost.example.com");
         RouteState mutableRouteState(mutableSipMessage, removedHeaders, myRouteName);

         // Check that no parameter is returned when non has been set
         UtlString value;
         const char* instanceName = "myinstance";
         const char* invalidParamName    = "para$meter";

         // Now try to set that value
         UtlString inputValue("good value");
         mutableRouteState.setParameter(instanceName,invalidParamName,inputValue);

         // ... and check that we cannot read it back
         value.append("bad value");
         CPPUNIT_ASSERT(!mutableRouteState.getParameter(instanceName,invalidParamName,value));
         CPPUNIT_ASSERT(value.isNull());
      }

   void testTokenEncodeDecode()
      {
         UtlSList removedHeaders;

         //
         const char* mutableMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage mutableSipMessage(mutableMessage, strlen(mutableMessage));
         UtlString myRouteName("myhost.example.com");
         RouteState mutableRouteState(mutableSipMessage, removedHeaders, myRouteName);

         const char* indialogMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com; tag=MAKES_THIS_UNMUTABLE\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage indialogSipMessage(indialogMessage, strlen(indialogMessage));
         RouteState indialogRouteState(indialogSipMessage, removedHeaders, myRouteName);

         UtlString signedToken;
         UtlString writeValue;

         writeValue = Value1;
         mutableRouteState.setParameter("plugin","param1",writeValue);

         writeValue = Value2;
         mutableRouteState.setParameter("plugin2","",writeValue);

         mutableRouteState.encode(signedToken);
         //printf("\n   signedToken: %s\n", signedToken.data());

         CPPUNIT_ASSERT(indialogRouteState.decode(signedToken));

         UtlString readValue;

         indialogRouteState.getParameter("plugin","param1",readValue);
         ASSERT_STR_EQUAL(Value1, readValue.data());

         indialogRouteState.getParameter("plugin2","",readValue);
         ASSERT_STR_EQUAL(Value2, readValue.data());
      }

   void testRoutedRequestState()
      {
         UtlSList removedHeaders;

         // test that no state is found when none exists
         const char* nostateMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage nostateSipMessage(nostateMessage, strlen(nostateMessage));
         UtlString myRouteName("myhost.example.com");
         RouteState nostateRouteState(nostateSipMessage, removedHeaders, myRouteName);

         CPPUNIT_ASSERT(nostateRouteState.isMutable());
         CPPUNIT_ASSERT(nostateRouteState.mValues.isEmpty()); // cheat - look inside

         const char* routedMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com; tag=MAKES_THIS_UNMUTABLE\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage routedSipMessage(routedMessage, strlen(routedMessage));

         const char* nostateRoute =
            "<sip:myhost.example.com:5080;lr>";
         removedHeaders.insert(new UtlString(nostateRoute));

         const char* routedStateValue =
            "<sip:myhost.example.com;lr;sipXecs-rs=plugin*param1~dmFsdWUx.plugin2*~dmFsdWUy!6cedf2bb36711d2e69b3012ab3d1e16d>";
         removedHeaders.insert(new UtlString(routedStateValue));

         RouteState routedState(routedSipMessage, removedHeaders, myRouteName);

         UtlString readValue;

         CPPUNIT_ASSERT(routedState.getParameter("plugin","param1",readValue));
         ASSERT_STR_EQUAL(Value1, readValue.data());

         CPPUNIT_ASSERT(routedState.getParameter("plugin2","",readValue));
         ASSERT_STR_EQUAL(Value2, readValue.data());

         // since the state was in a Route, not a Record-Route, this should be
         CPPUNIT_ASSERT( routedState.mRecordRouteIndices.empty() );

         removedHeaders.destroyAll();
      }

   void testAppendToExistingRecordRoute()
      {
         UtlSList removedRoutes;

         const char* message =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Record-Route: <sip:myhost.example.com;lr>\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage sipMessage(message, strlen(message));

         UtlString myRouteName("myhost.example.com");
         RouteState routeState(sipMessage, removedRoutes, myRouteName);

         CPPUNIT_ASSERT(routeState.isMutable());
         CPPUNIT_ASSERT(!routeState.isFound());

         CPPUNIT_ASSERT( routeState.mRecordRouteIndices.size() == 1 );
         CPPUNIT_ASSERT( routeState.mRecordRouteIndices[0] == 0 );

         routeState.setParameter("plugin","param1","dummyvalue");

         SipMessage outputSipMessage(sipMessage);

         routeState.update(&outputSipMessage);
         UtlString recordRoute, tempString, urlParmName;
         CPPUNIT_ASSERT( outputSipMessage.getRecordRouteUri(0, &recordRoute) );
         Url recordRouteUrl(recordRoute);
         recordRouteUrl.getUrlType( tempString );
         ASSERT_STR_EQUAL("sip", tempString.data());
         recordRouteUrl.getHostWithPort( tempString );
         ASSERT_STR_EQUAL("myhost.example.com", tempString.data());
         CPPUNIT_ASSERT( recordRouteUrl.getUrlParameter( 0, urlParmName, tempString ) );
         ASSERT_STR_EQUAL("lr", urlParmName.data());
         CPPUNIT_ASSERT( recordRouteUrl.getUrlParameter( 1, urlParmName, tempString ) );
         ASSERT_STR_EQUAL("sipXecs-rs", urlParmName.data());

         CPPUNIT_ASSERT( !outputSipMessage.getRecordRouteUri(1, &recordRoute) );

         removedRoutes.destroyAll();
      }

   void testSpiraledState()
      {
         UtlSList removedRoutes;

         removedRoutes.insert(new UtlString("<sip:myhost.example.com;lr>"));

         const char* spiraledMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Record-Route: <sip:other.example.com;lr>\r\n"
            "Record-Route: <sip:myhost.example.com;lr;sipXecs-rs=plugin*param1~dmFsdWUx.plugin2*~dmFsdWUy!6cedf2bb36711d2e69b3012ab3d1e16d>\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage spiraledSipMessage(spiraledMessage, strlen(spiraledMessage));

         UtlString myRouteName("myhost.example.com");
         RouteState spiraledState(spiraledSipMessage, removedRoutes, myRouteName);

         UtlString readValue;

         CPPUNIT_ASSERT(spiraledState.isMutable());
         CPPUNIT_ASSERT(spiraledState.isFound());

         CPPUNIT_ASSERT(spiraledState.getParameter("plugin","param1",readValue));
         ASSERT_STR_EQUAL(Value1, readValue.data());

         CPPUNIT_ASSERT(spiraledState.getParameter("plugin2","",readValue));
         ASSERT_STR_EQUAL(Value2, readValue.data());

         // since the state was in a Route, not a Record-Route, this should be
         CPPUNIT_ASSERT( spiraledState.mRecordRouteIndices.size() == 1 );
         CPPUNIT_ASSERT( spiraledState.mRecordRouteIndices[0] == 1 );

         removedRoutes.destroyAll();
      }


   void testNewDialogState()
      {
         UtlSList removedRoutes;
         removedRoutes.insert(new UtlString("<sip:myhost.example.com;lr>"));

         // test that no state is found when none exists
         const char* inputMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage inputSipMessage(inputMessage, strlen(inputMessage));
         UtlString myRouteName("myhost.example.com");
         RouteState routeState(inputSipMessage, removedRoutes, myRouteName);

         removedRoutes.destroyAll();

         CPPUNIT_ASSERT(routeState.isMutable());
         CPPUNIT_ASSERT(!routeState.isFound());
         CPPUNIT_ASSERT(routeState.mValues.isEmpty()); // cheat - look inside

//         const char* routedStateValue =
//            "<sip:myhost.example.com;lr;sipXecs-rs=plugin*param1~dmFsdWUx.plugin2*~dmFsdWUy!6cedf2bb36711d2e69b3012ab3d1e16d>";

         routeState.setParameter("plugin", "param1", Value1);
         routeState.setParameter("plugin2", "", Value2);

         SipMessage outputSipMessage(inputMessage);

         routeState.update(&outputSipMessage);

         UtlString outputMessage;
         ssize_t ignoreLength;
         outputSipMessage.getBytes(&outputMessage, &ignoreLength);

         const char* expectedOutput =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Record-Route: <sip:myhost.example.com;lr;sipXecs-rs=plugin%2Aparam1%7EdmFsdWUx.plugin2%2A%7EdmFsdWUy%216cedf2bb36711d2e69b3012ab3d1e16d>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         ASSERT_STR_EQUAL(expectedOutput, outputMessage.data());
      }

   void testAppendedState()
      {
         UtlSList removedRoutes;

         const char* recordRoutedMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Record-Route: <sip:other.example.com;lr>\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage recordRoutedSipMessage(recordRoutedMessage, strlen(recordRoutedMessage));
         UtlString myRouteName("myhost.example.com");
         RouteState state(recordRoutedSipMessage, removedRoutes, myRouteName);

         UtlString writeValue("write");

         CPPUNIT_ASSERT(state.isMutable());
         CPPUNIT_ASSERT(!state.isFound());

         state.setParameter("plugin","param1",writeValue);

         state.update(&recordRoutedSipMessage);

         UtlString outputMessage;
         ssize_t ignoreLength;
         recordRoutedSipMessage.getBytes(&outputMessage, &ignoreLength);

         const char* appendedRoutedMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Record-Route: <sip:myhost.example.com;lr;sipXecs-rs=plugin%2Aparam1%7Ed3JpdGU%60%21ede4982f328fc3611edb573f9044a45f>\r\n"
            "Record-Route: <sip:other.example.com;lr>\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         ASSERT_STR_EQUAL(appendedRoutedMessage, outputMessage.data());
      }

   void testAddCopy()
      {
         UtlSList removedRoutes;

         const char* spiraledMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Record-Route: <sip:other.example.com;lr>\r\n"
            "Record-Route: <sip:myhost.example.com;lr;sipXecs-rs=plugin*param1~dmFsdWUx.plugin2*~dmFsdWUy!6cedf2bb36711d2e69b3012ab3d1e16d>\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage spiraledSipMessage(spiraledMessage, strlen(spiraledMessage));

         UtlString myRouteName("myhost.example.com");
         RouteState routeState(spiraledSipMessage, removedRoutes, myRouteName);

         // request the addition of a copy and then update the route state
         routeState.addCopy();
         routeState.update( &spiraledSipMessage );

         // turn resultin message into a string for comparison
         UtlString msgBytes;
         ssize_t msgLen;
         spiraledSipMessage.getBytes(&msgBytes, &msgLen);

         UtlString expectedMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Record-Route: <sip:myhost.example.com;lr;sipXecs-rs=plugin%2Aparam1%7EdmFsdWUx.plugin2%2A%7EdmFsdWUy%216cedf2bb36711d2e69b3012ab3d1e16d>\r\n"
            "Record-Route: <sip:other.example.com;lr>\r\n"
            "Record-Route: <sip:myhost.example.com;lr;sipXecs-rs=plugin%2Aparam1%7EdmFsdWUx.plugin2%2A%7EdmFsdWUy%216cedf2bb36711d2e69b3012ab3d1e16d>\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         CPPUNIT_ASSERT(expectedMessage.compareTo( msgBytes ) == 0 );
      }

   void testAuthorizedDialog()
      {
         UtlSList removedHeaders;
         const char* mutableMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage mutableSipMessage(mutableMessage, strlen(mutableMessage));
         UtlString myRouteName("myhost.example.com");
         RouteState mutableRouteState(mutableSipMessage, removedHeaders, myRouteName);

         CPPUNIT_ASSERT( !mutableRouteState.isDialogAuthorized() );
         mutableRouteState.markDialogAsAuthorized();
         CPPUNIT_ASSERT( mutableRouteState.isDialogAuthorized() );
      }

   void testOriginalFromTagValue()
      {
         UtlSList removedRoutes;
         const char* instanceName = "testOriginalFromTagValue";

         const char* firstMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage firstSipMessage(firstMessage, strlen(firstMessage));
         UtlString myRouteName("myhost.example.com");
         RouteState firstRouteState(firstSipMessage, removedRoutes, myRouteName);

         UtlString returnedTag;
         CPPUNIT_ASSERT( firstRouteState.originalCallerFromTagValue(instanceName, returnedTag));
         ASSERT_STR_EQUAL("30543f3483e1cb11ecb40866edd3295b", returnedTag.data());

         firstRouteState.update(&firstSipMessage);

         UtlString outputMessage;
         ssize_t ignoreLength;
         firstSipMessage.getBytes(&outputMessage, &ignoreLength);

         const char* routedMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Record-Route: <sip:myhost.example.com;lr;sipXecs-rs=%2Afrom%7EMzA1NDNmMzQ4M2UxY2IxMWVjYjQwODY2ZWRkMzI5NWI%60%21210fe51fb5e55efdd09f617f9b1c07f4>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         ASSERT_STR_EQUAL(routedMessage, outputMessage.data());

         const char* returnedMessage =
            "SIP/2.0 200 Ok\r\n"
            "Record-Route: <sip:myhost.example.com;lr;sipXecs-rs=%2Afrom%7EMzA1NDNmMzQ4M2UxY2IxMWVjYjQwODY2ZWRkMzI5NWI%60%21210fe51fb5e55efdd09f617f9b1c07f4>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "From: Caller <sip:caller@example.org>; tag=xyzzy\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Contact: callee@127.0.0.2\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage returnedSipMessage(returnedMessage, strlen(returnedMessage));

         RouteState returnedRouteState(returnedSipMessage, removedRoutes, myRouteName);

         CPPUNIT_ASSERT( returnedRouteState.originalCallerFromTagValue(instanceName, returnedTag));
         ASSERT_STR_EQUAL("30543f3483e1cb11ecb40866edd3295b", returnedTag.data());

         UtlString inDialogRoute("<sip:myhost.example.com;lr;sipXecs-rs=%2Afrom%7EMzA1NDNmMzQ4M2UxY2IxMWVjYjQwODY2ZWRkMzI5NWI%60%21210fe51fb5e55efdd09f617f9b1c07f4>");
         removedRoutes.append(&inDialogRoute);

         const char* reverseMessage =
            "OPTIONS sip:caller@127.0.0.1.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "From: Caller <sip:caller@example.org>; tag=xyzzy\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 OPTIONS\r\n"
            "Contact: caller@127.0.0.2\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage reverseSipMessage(reverseMessage, strlen(reverseMessage));

         RouteState reverseRouteState(reverseSipMessage, removedRoutes, myRouteName);

         CPPUNIT_ASSERT( reverseRouteState.originalCallerFromTagValue(instanceName, returnedTag));
         ASSERT_STR_EQUAL("30543f3483e1cb11ecb40866edd3295b", returnedTag.data());

         const char* forwardMessage =
            "OPTIONS sip:callee@127.0.0.2 SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com; tag=xyzzy\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 3 OPTIONS\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage forwardSipMessage(forwardMessage, strlen(forwardMessage));

         RouteState forwardRouteState(forwardSipMessage, removedRoutes, myRouteName);

         CPPUNIT_ASSERT( forwardRouteState.originalCallerFromTagValue(instanceName, returnedTag));
         ASSERT_STR_EQUAL("30543f3483e1cb11ecb40866edd3295b", returnedTag.data());

      }

   void testDirectionIsCallerToCalled()
      {
         UtlSList removedRoutes;
         const char* instanceName = "testOriginalFromTagValue";

         const char* firstMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage firstSipMessage(firstMessage, strlen(firstMessage));
         UtlString myRouteName("myhost.example.com");
         RouteState firstRouteState(firstSipMessage, removedRoutes, myRouteName);

         CPPUNIT_ASSERT( firstRouteState.directionIsCallerToCalled(instanceName));

         firstRouteState.update(&firstSipMessage);

         UtlString outputMessage;
         ssize_t ignoreLength;
         firstSipMessage.getBytes(&outputMessage, &ignoreLength);

         const char* routedMessage =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Record-Route: <sip:myhost.example.com;lr;sipXecs-rs=%2Afrom%7EMzA1NDNmMzQ4M2UxY2IxMWVjYjQwODY2ZWRkMzI5NWI%60%21210fe51fb5e55efdd09f617f9b1c07f4>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         ASSERT_STR_EQUAL(routedMessage, outputMessage.data());

         const char* returnedMessage =
            "SIP/2.0 200 Ok\r\n"
            "Record-Route: <sip:myhost.example.com;lr;sipXecs-rs=%2Afrom%7EMzA1NDNmMzQ4M2UxY2IxMWVjYjQwODY2ZWRkMzI5NWI%60%21210fe51fb5e55efdd09f617f9b1c07f4>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "From: Caller <sip:caller@example.org>; tag=xyzzy\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Contact: callee@127.0.0.2\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage returnedSipMessage(returnedMessage, strlen(returnedMessage));

         RouteState returnedRouteState(returnedSipMessage, removedRoutes, myRouteName);

         CPPUNIT_ASSERT( ! returnedRouteState.directionIsCallerToCalled(instanceName));

         UtlString inDialogRoute("<sip:myhost.example.com;lr;sipXecs-rs=%2Afrom%7EMzA1NDNmMzQ4M2UxY2IxMWVjYjQwODY2ZWRkMzI5NWI%60%21210fe51fb5e55efdd09f617f9b1c07f4>");
         removedRoutes.append(&inDialogRoute);

         const char* reverseMessage =
            "OPTIONS sip:caller@127.0.0.1.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "From: Caller <sip:caller@example.org>; tag=xyzzy\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 OPTIONS\r\n"
            "Contact: caller@127.0.0.2\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage reverseSipMessage(reverseMessage, strlen(reverseMessage));

         RouteState reverseRouteState(reverseSipMessage, removedRoutes, myRouteName);

         CPPUNIT_ASSERT( ! reverseRouteState.directionIsCallerToCalled(instanceName));

         const char* forwardMessage =
            "OPTIONS sip:callee@127.0.0.2 SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com; tag=xyzzy\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 3 OPTIONS\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage forwardSipMessage(forwardMessage, strlen(forwardMessage));

         RouteState forwardRouteState(forwardSipMessage, removedRoutes, myRouteName);

         CPPUNIT_ASSERT( forwardRouteState.directionIsCallerToCalled(instanceName));
      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(RouteStateTest);
