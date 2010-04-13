//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include <string.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <os/OsStatus.h>
#include <sipdb/ResultSet.h>
#include <utl/UtlDefs.h>
#include "SipAaa.h"
#include <net/SipUserAgent.h>
#include <net/SipMessage.h>

class SipAaaTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipAaaTest);
   CPPUNIT_TEST(testIsAuthorized);
   CPPUNIT_TEST(testRRApplicationOut);
   CPPUNIT_TEST(testRRApplicationIn);
   CPPUNIT_TEST(testChallengeUnneededOut);
   CPPUNIT_TEST(testChallengeOut);
   CPPUNIT_TEST(testChallengeIn);
   CPPUNIT_TEST_SUITE_END();

public:

   SipUserAgent* mUserAgent;

   void setUp()
      {
         // Construct a SipUserAgent to provide the isMyHostAlias recognizer

         mUserAgent = new SipUserAgent(SIP_PORT, // udp port
                                       SIP_PORT, // tcp port
                                       -1,       // tls port
                                       "127.0.0.2" // public address
                                       );

         UtlString internalDomainAlias("example.com:5060");
         mUserAgent->setHostAliases(internalDomainAlias);

         UtlString internalHostAlias("internal.example.com:5060");
         mUserAgent->setHostAliases(internalHostAlias);

         UtlString externalAlias("external.example.net:5060");
         mUserAgent->setHostAliases(externalAlias);
      }

   void tearDown()
      {
         delete mUserAgent;
      }

   void testIsAuthorized()
      {
         // Construct a SipAaa using the empty constructor so that we don't have to fire
         // up a SipUserAgent, which isn't needed.
         SipAaa aaa;

         // Authorization should succeed because all required permissions have been granted
         ResultSet huntingAndFishingPermissions;
         getPermissions(huntingAndFishingPermissions, true, true);
         UtlString matchedPermission;
         UtlString unmatchedPermissions;
         CPPUNIT_ASSERT(aaa.isAuthorized(
                           huntingAndFishingPermissions,    // requiredPermissions
                           huntingAndFishingPermissions,    // grantedPermissions
                           matchedPermission,
                           unmatchedPermissions));

         // Authorization should fail, not all required permissions have been granted
         ResultSet huntingPermissionOnly;
         getPermissions(huntingPermissionOnly, true, false);
         CPPUNIT_ASSERT(aaa.isAuthorized(
                           huntingAndFishingPermissions,    // requiredPermissions
                           huntingPermissionOnly,           // grantedPermissions
                           matchedPermission,
                           unmatchedPermissions));
      }

   void testRRApplicationOut()
      {
         // Test that an out-of-dialog request gets a Record-Route, even if it does not
         // require authorization/authentication, and test that the Record-Route has no
         // extraneous parameters applied.

         // Construct a SipAaa using the test constructor so we can customize the auth rules.
         UtlString routeName("sip.example.com");
         // Empty auth rules, which means no authentication.
         const char authRules[] =
            "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>"
            "<mappings xmlns='http://www.sipfoundry.org/sipX/schema/xml/urlauth-00-00'>"
            "</mappings>";
         SipAaa aaa(*mUserAgent, "TestRealm", routeName, authRules);

         const char* message =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 2 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         CPPUNIT_ASSERT(aaa.proxyMessage(testMsg));

         UtlString rr;
         CPPUNIT_ASSERT(testMsg.getRecordRouteField(0, &rr));
         CPPUNIT_ASSERT(strcmp(rr.data(), "<sip:sip.example.com;lr>") == 0);
         // Only one Record-Route header.
         CPPUNIT_ASSERT(!testMsg.getRecordRouteField(1, &rr));
      }

   void testRRApplicationIn()
      {
         // Test that an in-dialog request does not get a Record-Route.

         // Construct a SipAaa using the test constructor so we can customize the auth rules.
         UtlString routeName("sip.example.com");
         // Auth rules forbidding everything.
         const char authRules[] =
            "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>"
            "<mappings xmlns='http://www.sipfoundry.org/sipX/schema/xml/urlauth-00-00'>"
               "<hostMatch>"
                 "<hostPattern>somewhere.com</hostPattern>"
                 "<userMatch>"
                   "<userPattern>.</userPattern>"
                   "<permissionMatch>"
                     "<permission>NoAccess</permission>"
                   "</permissionMatch>"
                 "</userMatch>"
               "</hostMatch>"
            "</mappings>";
         SipAaa aaa(*mUserAgent, "TestRealm", routeName, authRules);

         const char* message =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com;tag=onward\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         CPPUNIT_ASSERT(aaa.proxyMessage(testMsg));

         UtlString rr;
         // No Record-Route header.
         CPPUNIT_ASSERT(!testMsg.getRecordRouteField(0, &rr));
      }

   void testChallengeUnneededOut()
      {
         // Test that an out-of-dialog request does not get an authentication challenge when
         // the auth rules don't require it.

         // Construct a SipAaa using the test constructor so we can customize the auth rules.
         UtlString routeName("sip.example.com");
         // Empty auth rules, which means no authentication.
         const char authRules[] =
            "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>"
            "<mappings xmlns='http://www.sipfoundry.org/sipX/schema/xml/urlauth-00-00'>"
            "</mappings>";
         SipAaa aaa(*mUserAgent, "TestRealm", routeName, authRules);

         const char* message =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 2 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         CPPUNIT_ASSERT(aaa.proxyMessage(testMsg));

         // testMsg should not have been converted into a 407 response.
         CPPUNIT_ASSERT(!testMsg.isResponse());
         UtlString s;
         testMsg.getRequestMethod(&s);
         CPPUNIT_ASSERT(strcmp(s.data(), "INVITE") == 0);
         testMsg.getRequestUri(&s);
         CPPUNIT_ASSERT(strcmp(s.data(), "sip:user@somewhere.com") == 0);
      }

   void testChallengeOut()
      {
         // Test that an out-of-dialog request gets an authentication challenge when
         // the auth rules require it.

         // Construct a SipAaa using the test constructor so we can customize the auth rules.
         UtlString routeName("sip.example.com");
         // Auth rules forbidding everything.
         const char authRules[] =
            "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>"
            "<mappings xmlns='http://www.sipfoundry.org/sipX/schema/xml/urlauth-00-00'>"
               "<hostMatch>"
                 "<hostPattern>somewhere.com</hostPattern>"
                 "<userMatch>"
                   "<userPattern>.</userPattern>"
                   "<permissionMatch>"
                     "<permission>NoAccess</permission>"
                   "</permissionMatch>"
                 "</userMatch>"
               "</hostMatch>"
            "</mappings>";
         SipAaa aaa(*mUserAgent, "TestRealm", routeName, authRules);

         const char* message =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 2 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         CPPUNIT_ASSERT(aaa.proxyMessage(testMsg));

         // testMsg should have been converted into a 407 response.
         CPPUNIT_ASSERT(testMsg.isResponse());
         CPPUNIT_ASSERT_EQUAL(testMsg.getResponseStatusCode(), HTTP_PROXY_UNAUTHORIZED_CODE);
      }

   void testChallengeIn()
      {
         // Test that an in-dialog request does not get a Record-Route.

         // Construct a SipAaa using the test constructor so we can customize the auth rules.
         UtlString routeName("sip.example.com");
         // Auth rules forbidding everything.
         const char authRules[] =
            "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>"
            "<mappings xmlns='http://www.sipfoundry.org/sipX/schema/xml/urlauth-00-00'>"
               "<hostMatch>"
                 "<hostPattern>somewhere.com</hostPattern>"
                 "<userMatch>"
                   "<userPattern>.</userPattern>"
                   "<permissionMatch>"
                     "<permission>NoAccess</permission>"
                   "</permissionMatch>"
                 "</userMatch>"
               "</hostMatch>"
            "</mappings>";
         SipAaa aaa(*mUserAgent, "TestRealm", routeName, authRules);

         const char* message =
            "INVITE sip:user@somewhere.com SIP/2.0\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:user@somewhere.com;tag=onward\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         CPPUNIT_ASSERT(aaa.proxyMessage(testMsg));

         // testMsg should not have been converted into a 407 response.
         CPPUNIT_ASSERT(!testMsg.isResponse());
         UtlString s;
         testMsg.getRequestMethod(&s);
         CPPUNIT_ASSERT(strcmp(s.data(), "INVITE") == 0);
         testMsg.getRequestUri(&s);
         CPPUNIT_ASSERT(strcmp(s.data(), "sip:user@somewhere.com") == 0);
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

CPPUNIT_TEST_SUITE_REGISTRATION(SipAaaTest);
