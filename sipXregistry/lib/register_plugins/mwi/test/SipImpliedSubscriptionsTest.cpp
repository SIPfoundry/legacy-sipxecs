//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <os/OsConfigDb.h>

#include <utl/PluginHooks.h>

#include <net/SipMessage.h>

#include <registry/RegisterPlugin.h>
#include "../SipImpliedSubscriptions.h"

#define PLUGIN_LIB_DIR TEST_DIR "/../.libs/"

/**
 * Unittest for SipMessage
 */
class SipImpliedSubscriptionsTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipImpliedSubscriptionsTest);
   CPPUNIT_TEST(testImpliedMwiSubscriptions);
   CPPUNIT_TEST_SUITE_END();

public:

   void testImpliedMwiSubscriptions()
      {
         PluginHooks regPlugin( "getRegisterPlugin", "SIP_REGISTER" );

         OsConfigDb configuration;
         configuration.set("SIP_REGISTER_HOOK_LIBRARY.Mwi",
                           PLUGIN_LIB_DIR "libRegistrarImpliedMWI.so" );
         configuration.set("SIP_REGISTER.Mwi.UA.DUMB", "^DUMB");
         configuration.set("SIP_REGISTER.Mwi.UA.DV1", "DV/1");

         regPlugin.readConfig(configuration);

         PluginIterator getPlugin(regPlugin);
         SipImpliedSubscriptions* plugin
            = (SipImpliedSubscriptions*)getPlugin.next();

         CPPUNIT_ASSERT(plugin);

         const char DumbMessage[] =
            "REGISTER sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP sipx.local:33855;branch=z9hG4bK-10cb6f9378a12d4218e10ef4dc78ea3d\r\n"
            "To: Sip User <sip:sipuser@pingtel.org>;tag=totag\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 REGISTER\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: DUMB/0.01\r\n"
            "Contact: sip:me@127.0.0.1\r\n"
            "Expires: 1000\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testDumbReg( DumbMessage, strlen( DumbMessage ) );

         CPPUNIT_ASSERT(plugin->needsImpliedSubscription(testDumbReg));

         {
            SipMessage subscribeRequest;
            UtlString  callId;
            UtlString  fromTag;
            UtlString  fromUri;

            plugin->buildSubscribeRequest( testDumbReg, 900
                                          ,subscribeRequest, callId, fromTag, fromUri
                                          );

#           if TEST_DEBUG
            {
               UtlString msg;
               int       len;
               subscribeRequest.getBytes(&msg, &len);

               printf( "Dumb Request:\n%s", msg.data());
            }
#           endif

            UtlString method;
            subscribeRequest.getRequestMethod(&method);
            ASSERT_STR_EQUAL("SUBSCRIBE", method.data());

            UtlString target;
            subscribeRequest.getRequestUri(&target);
            ASSERT_STR_EQUAL("sip:sipuser@pingtel.org", target.data());

            UtlString subscriber;
            subscribeRequest.getContactUri(0, &subscriber);
            ASSERT_STR_EQUAL("sip:me@127.0.0.1", subscriber.data());

            UtlString event;
            subscribeRequest.getEventFieldParts(&event);
            ASSERT_STR_EQUAL(SIP_EVENT_MESSAGE_SUMMARY, event.data());

            int expiration;
            subscribeRequest.getExpiresField(&expiration);
            CPPUNIT_ASSERT(expiration==900);

            ASSERT_STR_EQUAL("implied-mwi-f88dfabce84b6a2787ef024a7dbe8749",callId.data());
            ASSERT_STR_EQUAL("1d46019b940a0bfacc0ad3d138039b90",fromTag.data());
            ASSERT_STR_EQUAL("sip:sipsend@pingtel.org", fromUri.data());
         }

         const char NotDumbMessage[] =
            "REGISTER sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP sipx.local:33855;branch=z9hG4bK-10cb6f9378a12d4218e10ef4dc78ea3d\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 REGISTER\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: NOTDUMB/0.01\r\n"
            "Contact: sip:me@127.0.0.1\r\n"
            "Expires: 1000\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testNotDumbReg( NotDumbMessage, strlen( NotDumbMessage ) );

         CPPUNIT_ASSERT(!plugin->needsImpliedSubscription(testNotDumbReg));

         const char DV1Message[] =
            "REGISTER sip:sipx.local SIP/2.0\r\n"
            "Via: SIP/2.0/TCP sipx.local:33855;branch=z9hG4bK-10cb6f9378a12d4218e10ef4dc78ea3d\r\n"
            "To: sip:sipx.local\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 REGISTER\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: SOMEDV/1.01\r\n"
            "Contact: sip:me@127.0.0.1\r\n"
            "Expires: 1000\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testDV1Reg( DV1Message, strlen( DV1Message ) );

         CPPUNIT_ASSERT(plugin->needsImpliedSubscription(testDV1Reg));

      };

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipImpliedSubscriptionsTest);
