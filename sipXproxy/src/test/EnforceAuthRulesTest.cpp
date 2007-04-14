//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/TestCase.h"
#include "sipxunit/TestUtilities.h"
#include "testlib/SipDbTestContext.h"
#include "sipdb/AuthexceptionDB.h"
#include "sipdb/PermissionDB.h"
#include "sipdb/CredentialDB.h"
#include "os/OsDefs.h"
#include "os/OsConfigDb.h"
#include "utl/PluginHooks.h"
#include "net/SipMessage.h"

#include "sipauthproxy/AuthPlugin.h"
#include "sipauthproxy/EnforceAuthRules.h"

class EnforceAuthRulesTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(EnforceAuthRulesTest);

   CPPUNIT_TEST(isAuthorized);
   CPPUNIT_TEST(testNoPermNeededOut);
   CPPUNIT_TEST(testNoPermAck);
   CPPUNIT_TEST(testNoPermResponse);
   CPPUNIT_TEST(testForbidden);
   CPPUNIT_TEST(testException);
   CPPUNIT_TEST(testNoChallengeAuth);

   CPPUNIT_TEST_SUITE_END();

public:

   static EnforceAuthRules* enforcer;
   static SipDbTestContext  TestDbContext;

   void setUp()
      {
         RouteState::setSecret("fixed");
         TestDbContext.inputFile("authexception.xml");
         TestDbContext.inputFile("permission.xml");
         TestDbContext.inputFile("credential.xml");
      }

   void tearDown()
      {
         AuthexceptionDB::getInstance()->releaseInstance();
         PermissionDB::getInstance()->releaseInstance();
         CredentialDB::getInstance()->releaseInstance();
      }

   /* ****************************************************************
    * Note: all tests pass NULL for the SipAaa* sipAaa parameter.
    *       Since the EnforceAuthRules plugin does not use that
    *       parameter, this should not cause any problem.
    *       Should that change, see CallerAliasTest for how to set
    *       up a SipAaa instance for a test.
    * **************************************************************** */

   void isAuthorized()
      {

         // Authorization should succeed because all required permissions have been granted
         ResultSet huntingAndFishingPermissions;
         getPermissions(huntingAndFishingPermissions, true, true);
         UtlString matchedPermission;
         UtlString unmatchedPermissions;
         CPPUNIT_ASSERT(enforcer->isAuthorized(huntingAndFishingPermissions,    // requiredPermissions
                                               huntingAndFishingPermissions,    // grantedPermissions
                                               matchedPermission,
                                               unmatchedPermissions));

         // Authorization should fail, not all required permissions have been granted
         ResultSet huntingPermissionOnly;
         getPermissions(huntingPermissionOnly, true, false);
         CPPUNIT_ASSERT(enforcer->isAuthorized(huntingAndFishingPermissions,    // requiredPermissions
                                               huntingPermissionOnly,           // grantedPermissions
                                               matchedPermission,
                                               unmatchedPermissions));
      }

   // Test that an out-of-dialog request gets a Record-Route, even if it does not
   // require authorization/authentication, and test that the Record-Route has no
   // extraneous parameters applied.
   void testNoPermNeededOut()
      {
         OsConfigDb configuration;
         configuration.set("RULES", TEST_DATA_DIR "enforcerules.xml");

         enforcer->readConfig(configuration);

         UtlString identity; // no authenticated identity
         Url requestUri("sip:911@emergency-gw");

         const char* message =
            "INVITE sip:911@emergency-gw SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:911@emergency-gw\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 2 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         UtlSList noRemovedRoutes;
         RouteState routeState( testMsg, noRemovedRoutes );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         CPPUNIT_ASSERT(AuthPlugin::ALLOW_REQUEST
                        == enforcer->authorizeAndModify(NULL,
                                                        identity,
                                                        requestUri,
                                                        routeState,
                                                        testMsg,
                                                        rejectReason
                                                        ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         UtlString routeName("example.com");
         routeState.update(&testMsg, routeName);

         UtlString recordRoute;
         CPPUNIT_ASSERT(testMsg.getRecordRouteField(0, &recordRoute));
         ASSERT_STR_EQUAL( "<sip:example.com;lr;sipX-route=enforce%2Aauth%7E%21d1e296555015a54cb746fa7ac5695cf7>", recordRoute );

         // Only one Record-Route header.
         CPPUNIT_ASSERT(!testMsg.getRecordRouteField(1, &recordRoute));

         RouteState spiraledRouteState(testMsg, noRemovedRoutes);
         
         // now simulate a spiral with the same message
         CPPUNIT_ASSERT(AuthPlugin::ALLOW_REQUEST
                        == enforcer->authorizeAndModify(NULL,
                                                        identity,
                                                        requestUri,
                                                        spiraledRouteState,
                                                        testMsg,
                                                        rejectReason
                                                        ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         spiraledRouteState.update(&testMsg, routeName);

         CPPUNIT_ASSERT(testMsg.getRecordRouteField(0, &recordRoute));
         ASSERT_STR_EQUAL( "<sip:example.com;lr;sipX-route=enforce%2Aauth%7E%21d1e296555015a54cb746fa7ac5695cf7>", recordRoute );

         // Only one Record-Route header.
         CPPUNIT_ASSERT(!testMsg.getRecordRouteField(1, &recordRoute));
      }


   // Test that an ACK is not challenged and not RecordRouted
   void testNoPermAck()
      {
         OsConfigDb configuration;
         configuration.set("RULES", TEST_DATA_DIR "enforcerules.xml");

         enforcer->readConfig(configuration);

         UtlString identity; // no authenticated identity
         Url requestUri("sip:somewhere@forbidden");

         const char* message =
            "ACK sip:somewhere@forbidden SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:somewhere@forbidden\r\n"
            "From: Caller <sip:caller@example.org>; tag=99911983748\r\n"
            "Call-Id: b1373e736d7d359ead76fa5cd467d999\r\n"
            "Cseq: 2 ACK\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         UtlSList noRemovedRoutes;
         RouteState routeState( testMsg, noRemovedRoutes );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         CPPUNIT_ASSERT(AuthPlugin::ALLOW_REQUEST
                        == enforcer->authorizeAndModify(NULL,
                                                        identity,
                                                        requestUri,
                                                        routeState,
                                                        testMsg,
                                                        rejectReason
                                                        ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         UtlString routeName("example.com");
         routeState.update(&testMsg, routeName);

         UtlString recordRoute;
         CPPUNIT_ASSERT(!testMsg.getRecordRouteField(0, &recordRoute));

         // now simulate a spiral with the same message
         CPPUNIT_ASSERT(AuthPlugin::ALLOW_REQUEST
                        == enforcer->authorizeAndModify(NULL,
                                                        identity,
                                                        requestUri,
                                                        routeState,
                                                        testMsg,
                                                        rejectReason
                                                        ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         routeState.update(&testMsg, routeName);

         CPPUNIT_ASSERT(!testMsg.getRecordRouteField(0, &recordRoute));
      }
   

   // Test that a response message is allowed and is not modified
   void testNoPermResponse()
      {
         OsConfigDb configuration;
         configuration.set("RULES", TEST_DATA_DIR "enforcerules.xml");

         enforcer->readConfig(configuration);

         UtlString identity; // no authenticated identity
         Url requestUri("sip:somewhere@forbidden");

         const char* message =
            "SIP/2.0 200 Ok\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:somewhere@forbidden\r\n"
            "From: Caller <sip:caller@example.org>; tag=99911983748\r\n"
            "Call-Id: b1373e736d7d359ead76fa5cd467d999\r\n"
            "Cseq: 2 ACK\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "Record-Route: <sip:example.com;lr;sipX-route=enforce%2Aauth%7E%21d1e296555015a54cb746fa7ac5695cf7>\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         UtlSList noRemovedRoutes;
         RouteState routeState( testMsg, noRemovedRoutes );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         CPPUNIT_ASSERT(AuthPlugin::ALLOW_REQUEST
                        == enforcer->authorizeAndModify(NULL,
                                                        identity,
                                                        requestUri,
                                                        routeState,
                                                        testMsg,
                                                        rejectReason
                                                        ));
         ASSERT_STR_EQUAL(unmodifiedRejectReason, rejectReason.data());

         UtlString recordRoute;
         CPPUNIT_ASSERT(testMsg.getRecordRouteField(0, &recordRoute));
         ASSERT_STR_EQUAL( "<sip:example.com;lr;sipX-route=enforce%2Aauth%7E%21d1e296555015a54cb746fa7ac5695cf7>", recordRoute );
      }


   // Test that an out-of-dialog request gets an authentication challenge when
   // the auth rules require it.
   void testForbidden()
      {
         const char* message =
            "INVITE sip:user@forbidden SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@forbidden\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 2 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));


         UtlString identity; // no authenticated identity
         Url requestUri("sip:somewhere@forbidden");

         UtlSList noRemovedRoutes;
         RouteState routeState( testMsg, noRemovedRoutes );

         UtlString rejectReason;
         
         CPPUNIT_ASSERT(AuthPlugin::UNAUTHORIZED
                        == enforcer->authorizeAndModify(NULL,
                                                        identity,
                                                        requestUri,
                                                        routeState,
                                                        testMsg,
                                                        rejectReason
                                                        ));
         ASSERT_STR_EQUAL("", rejectReason.data());

         // now try the same request, but with an authenticated identity
         // so this time it should provide a reject reason
         identity = "user@example.com";
         
         CPPUNIT_ASSERT(AuthPlugin::UNAUTHORIZED
                        == enforcer->authorizeAndModify(NULL,
                                                        identity,
                                                        requestUri,
                                                        routeState,
                                                        testMsg,
                                                        rejectReason
                                                        ));
         ASSERT_STR_EQUAL("Requires NoAccess", rejectReason.data());
      }

   // Test that an otherwise fobidden request is allowed when the authexceptions 
   // say it is ok.
   void testException()
      {
         UtlString identity("supercaster@enforce.example.com"); // has only 'fishing' permission
         Url okRequestUri("sip:user@boat");

         UtlSList noRemovedRoutes;

         UtlString rejectReason;
         
         const char* okMessage =
            "INVITE sip:user@boat SIP/2.0\r\n" // 'lodge' requires 'hunting' permission
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@boat\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: exception-1\r\n"
            "Cseq: 2 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage okMsg(okMessage, strlen(okMessage));
         RouteState okRouteState( okMsg, noRemovedRoutes );

         // confirm that supercaster can call boat
         CPPUNIT_ASSERT(AuthPlugin::ALLOW_REQUEST
                        == enforcer->authorizeAndModify(NULL,
                                                        identity,
                                                        okRequestUri,
                                                        okRouteState,
                                                        okMsg,
                                                        rejectReason
                                                        ));
         CPPUNIT_ASSERT(rejectReason.isNull());

         UtlString routeName("example.com");
         okRouteState.update(&okMsg, routeName);

         UtlString recordRoute;
         CPPUNIT_ASSERT(okMsg.getRecordRouteField(0, &recordRoute));
         ASSERT_STR_EQUAL( "<sip:example.com;lr;sipX-route=enforce%2Aauth%7E%210083f7f42bdf4998911a18d41fb3aa01>", recordRoute );

         const char* noprivMessage =
            "INVITE sip:user@lodge SIP/2.0\r\n" // 'lodge' requires 'hunting' permission
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@lodge\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: exception-2\r\n"
            "Cseq: 2 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage noprivMsg(noprivMessage, strlen(noprivMessage));
         RouteState noprivRouteState( noprivMsg, noRemovedRoutes );
         Url noprivRequestUri("sip:user@lodge");

         // confirm that supercaster cannot call lodge
         CPPUNIT_ASSERT(AuthPlugin::UNAUTHORIZED
                        == enforcer->authorizeAndModify(NULL,
                                                        identity,
                                                        noprivRequestUri,
                                                        noprivRouteState,
                                                        noprivMsg,
                                                        rejectReason
                                                        ));
         ASSERT_STR_EQUAL("Requires hunting", rejectReason.data());

         // now try the same request, but to the identity that is in authexception, 
         // so this time it should work

         const char* allowedMessage =
            "INVITE sip:allowed@lodge SIP/2.0\r\n" // 'lodge' requires 'hunting' permission
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:allowed@lodge\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: exception-3\r\n"
            "Cseq: 2 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage allowedMsg(allowedMessage, strlen(allowedMessage));
         RouteState allowedRouteState( allowedMsg, noRemovedRoutes );

         Url allowedRequestUri("allowed@lodge");

         rejectReason.remove(0);
         
         CPPUNIT_ASSERT(AuthPlugin::ALLOW_REQUEST
                        == enforcer->authorizeAndModify(NULL,
                                                        identity,
                                                        allowedRequestUri,
                                                        allowedRouteState,
                                                        allowedMsg,
                                                        rejectReason
                                                        ));
         CPPUNIT_ASSERT(rejectReason.isNull());

         allowedRouteState.update(&allowedMsg, routeName);

         CPPUNIT_ASSERT(allowedMsg.getRecordRouteField(0, &recordRoute));
         ASSERT_STR_EQUAL( "<sip:example.com;lr;sipX-route=enforce%2Aauth%7E%2175da650843a06eee569f3c93b0f94ee5>", recordRoute );

      }

   // Test that an in-dialog message with an authorized route is not challenged.
   void testNoChallengeAuth()
      {
         // first, simulate the initial invite to generate the route

         UtlString identity("supercaster@enforce.example.com"); // has only 'fishing' permission
         Url okRequestUri("sip:user@boat");

         UtlSList noRemovedRoutes;

         UtlString rejectReason;
         
         const char* okMessage =
            "INVITE sip:user@boat SIP/2.0\r\n" // 'lodge' requires 'hunting' permission
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@boat\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: authorized-1\r\n"
            "Cseq: 2 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage okMsg(okMessage, strlen(okMessage));
         RouteState okRouteState( okMsg, noRemovedRoutes );

         // confirm that supercaster can call boat
         CPPUNIT_ASSERT(AuthPlugin::ALLOW_REQUEST
                        == enforcer->authorizeAndModify(NULL,
                                                        identity,
                                                        okRequestUri,
                                                        okRouteState,
                                                        okMsg,
                                                        rejectReason
                                                        ));
         CPPUNIT_ASSERT(rejectReason.isNull());

         UtlString routeName("example.com");
         okRouteState.update(&okMsg, routeName);

         UtlString recordRoute;
         CPPUNIT_ASSERT(okMsg.getRecordRouteField(0, &recordRoute));
         ASSERT_STR_EQUAL("<sip:example.com;lr;sipX-route=enforce%2Aauth%7E%21c2ce876a02a4f62e6a4ba3069bfb75b5>",
                          recordRoute );

         // put that route header into a popped route list
         UtlSList approvedRouteList;
         approvedRouteList.insert(&recordRoute);
         
         /*
          * Note that the request uri for this message is still 'boat' in order to
          * trigger the authrule.  This actually happens frequently when using gateways
          * because they are often configured using the IP address rather than a name,
          * and/or the IP address is still in the authrule to force authorization, even
          * for in-dialog requests.
          */
         const char* indialogForwardMessage =
            "INFO sip:user@boat SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@boat; tag=9873930447473\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: authorized-1\r\n"
            "Cseq: 2 INFO\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage indialogForwardMsg(indialogForwardMessage, strlen(indialogForwardMessage));
         RouteState indialogRouteState( indialogForwardMsg, approvedRouteList );
         Url indialogRequestUri("sip:user@boat");

         // confirm that a forward in-dialog message with that route is allowed
         // even though it is not authenticated.
         UtlString noIdentity;
         CPPUNIT_ASSERT(AuthPlugin::ALLOW_REQUEST
                        == enforcer->authorizeAndModify(NULL,
                                                        noIdentity,
                                                        indialogRequestUri,
                                                        indialogRouteState,
                                                        indialogForwardMsg,
                                                        rejectReason
                                                        ));
         CPPUNIT_ASSERT(rejectReason.isNull());
      }


private:
   // Return hunting and/or fishing permissions in the ResultSet.
   // Use a dummy identity.
   void getPermissions(ResultSet& permissions, bool hunting, bool fishing)
      {
         if (hunting)
         {
            addPermission(permissions, "hunting");
         }
         if (fishing)
         {
            addPermission(permissions, "fishing");
         }
      }

   void addPermission(ResultSet& permissions, const char* permissionValue)
      {
         UtlHashMap record;
         record.insertKeyAndValue(new UtlString("identity"),
                                  new UtlString("dummy identity"));
         record.insertKeyAndValue(new UtlString("permission"),
                                  new UtlString(permissionValue));
         permissions.addValue(record);
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(EnforceAuthRulesTest);

EnforceAuthRules* EnforceAuthRulesTest::enforcer = dynamic_cast<EnforceAuthRules*>(getAuthPlugin("enforce"));
SipDbTestContext  EnforceAuthRulesTest::TestDbContext(TEST_DATA_DIR, TEST_DIR "/enforceauthrules_context");
