//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <os/OsConfigDb.h>

#include <utl/PluginHooks.h>
#include <os/OsDateTime.h>
#include <net/SipMessage.h>

#include <registry/RedirectPlugin.h>
#include "test/DummyPlugin.h"
#include "SipRedirectorPresenceRouting.h"

class SipRedirectorPresenceRoutingTest : public CppUnit::TestCase,  public DummyPlugin
{
   CPPUNIT_TEST_SUITE(SipRedirectorPresenceRoutingTest);
   CPPUNIT_TEST(removeVoicemailContactTop);
   CPPUNIT_TEST(removeVoicemailContactMiddle);
   CPPUNIT_TEST(removeVoicemailContactBottom);
   CPPUNIT_TEST(removeVoicemailContactButNotFound);
   CPPUNIT_TEST(dontRemoveVoicemailContact);
   CPPUNIT_TEST(routeToTelUriBeginning);
   CPPUNIT_TEST(routeToTelUriMiddle1);
   CPPUNIT_TEST(routeToTelUriMiddle2);
   CPPUNIT_TEST(routeToTelUriEnd);
   CPPUNIT_TEST(routeToInvalidTelUris);
   CPPUNIT_TEST(routeToSipUriBeginning);
   CPPUNIT_TEST(routeToSipUriMiddle1);
   CPPUNIT_TEST(routeToSipUriMiddle2);
   CPPUNIT_TEST(routeToSipUriEnd);
   CPPUNIT_TEST(routeToInvalidSipUris);
   CPPUNIT_TEST_SUITE_END();

public:
   UnifiedPresenceContainer* pPresenceContainer;
   UnifiedPresence* pUp;

   SipRedirectorPresenceRoutingTest() :
      DummyPlugin("Dummy PlugIn")
   {
   }

   UnifiedPresence* insertUnifiedPresenceEntry( void )
   {
       pPresenceContainer = UnifiedPresenceContainer::getInstance();
       pUp = new UnifiedPresence("dummy@example.com");
       pUp->setXmppPresence("OFFLINE");
       pUp->setXmppStatusMessage("custom message");
       pUp->setSipState("IDLE");
       pPresenceContainer->insert( new UtlString("dummy@example.com"), pUp );
       return pUp;
   }

   void removeVoicemailContactTop()
   {
       pUp = insertUnifiedPresenceEntry();
       const char* message =
          "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
          "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
          "From: bob <sip:601@example.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
          "To: joseph <sip:dummy@example.com>\r\n"
          "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
          "Cseq: 1 INVITE\r\n"
          "Max-Forwards: 19\r\n"
          "Supported: replaces\r\n"
          "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
          "Contact: <sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060>\r\n"
          "Content-Length: 0\r\n"
          "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=47.135.162.145;rport=14956\r\n"
          "\r\n";
      SipMessage dummySipMessage(message, strlen(message));
      Url toUrl;
      dummySipMessage.getToUrl(toUrl);

      pUp->setSipState("BUSY");
      pUp->setXmppStatusMessage("svsdvsdvsdvsvd");

      ContactList contactList("SipRedirectorPresenceRoutingTest");
      contactList.add(UtlString("sip:~~vm~203@domain.com"), *this );
      contactList.add(UtlString("sip:201@domain.com"), *this );
      contactList.add(UtlString("sip:202@domain.com"), *this );
      contactList.resetWasModifiedFlag();

      OsConfigDb configuration;
      configuration.set("VOICEMAIL_ON_BUSY", "Y" );
      SipRedirectorPresenceRouting pluginUnderTest("PIUT");
      pluginUnderTest.readConfig(configuration);

      RedirectPlugin::LookUpStatus rc;
      rc = pluginUnderTest.doLookUp(
            toUrl,
            contactList );

      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.entries() == 1 );
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true );
      ASSERT_STR_EQUAL( "sip:~~vm~203@domain.com", tmpString.data() );
   }

   void removeVoicemailContactMiddle()
   {
      pUp = insertUnifiedPresenceEntry();
      Url toUrl("sip:dummy@example.com");
      pUp->setSipState("BUSY");
      ContactList contactList("SipRedirectorPresenceRoutingTest");
      contactList.add(UtlString("sip:201@domain.com"), *this );
      contactList.add(UtlString("sip:~~vm~203@domain.com"), *this );
      contactList.add(UtlString("sip:202@domain.com"), *this );
      contactList.resetWasModifiedFlag();

      OsConfigDb configuration;
      configuration.set("VOICEMAIL_ON_BUSY", "Y" );
      SipRedirectorPresenceRouting pluginUnderTest("PIUT");
      pluginUnderTest.readConfig(configuration);

      RedirectPlugin::LookUpStatus rc;
      rc = pluginUnderTest.doLookUp(
            toUrl,
            contactList );

      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.entries() == 1 );
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true );
      ASSERT_STR_EQUAL( "sip:~~vm~203@domain.com", tmpString.data() );
   }

   void removeVoicemailContactBottom()
   {
      pUp = insertUnifiedPresenceEntry();
      Url toUrl("sip:dummy@example.com");
      pUp->setSipState("BUSY");
      ContactList contactList("SipRedirectorPresenceRoutingTest");
      contactList.add(UtlString("sip:201@domain.com"), *this );
      contactList.add(UtlString("sip:202@domain.com"), *this );
      contactList.add(UtlString("sip:~~vm~203@domain.com"), *this );
      contactList.resetWasModifiedFlag();

      OsConfigDb configuration;
      configuration.set("VOICEMAIL_ON_BUSY", "Y" );
      SipRedirectorPresenceRouting pluginUnderTest("PIUT");
      pluginUnderTest.readConfig(configuration);

      RedirectPlugin::LookUpStatus rc;
      rc = pluginUnderTest.doLookUp(
            toUrl,
            contactList );

      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.entries() == 1 );
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true );
      ASSERT_STR_EQUAL( "sip:~~vm~203@domain.com", tmpString.data() );
   }

   void removeVoicemailContactButNotFound()
   {
      pUp = insertUnifiedPresenceEntry();
      Url toUrl("sip:dummy@example.com");
      pUp->setSipState("BUSY");
      ContactList contactList("SipRedirectorPresenceRoutingTest");
      contactList.add(UtlString("sip:201@domain.com"), *this );
      contactList.add(UtlString("sip:202@domain.com"), *this );
      contactList.resetWasModifiedFlag();

      OsConfigDb configuration;
      configuration.set("VOICEMAIL_ON_BUSY", "Y" );
      SipRedirectorPresenceRouting pluginUnderTest("PIUT");
      pluginUnderTest.readConfig(configuration);

      RedirectPlugin::LookUpStatus rc;
      rc = pluginUnderTest.doLookUp(
            toUrl,
            contactList );

      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.entries() == 2 );
      CPPUNIT_ASSERT( contactList.wasListModified() == false );
   }

   void dontRemoveVoicemailContact()
   {
      pUp = insertUnifiedPresenceEntry();
      Url toUrl("sip:dummy@example.com");
      pUp->setSipState("BUSY");
      ContactList contactList("SipRedirectorPresenceRoutingTest");
      contactList.add(UtlString("sip:201@domain.com"), *this );
      contactList.add(UtlString("sip:202@domain.com"), *this );
      contactList.add(UtlString("sip:~~vm~203@domain.com"), *this );
      contactList.resetWasModifiedFlag();

      OsConfigDb configuration;
      configuration.set("VOICEMAIL_ON_BUSY", "N" );
      SipRedirectorPresenceRouting pluginUnderTest("PIUT");
      pluginUnderTest.readConfig(configuration);

      RedirectPlugin::LookUpStatus rc;
      rc = pluginUnderTest.doLookUp(
            toUrl,
            contactList );

      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.entries() == 3 );
      CPPUNIT_ASSERT( contactList.wasListModified() == false );
   }

   void routeToTelUriBeginning()
   {
      pUp = insertUnifiedPresenceEntry();
      Url toUrl("sip:dummy@example.com");
      pUp->setXmppStatusMessage("tel:12345 is where I can be reached!");
      ContactList contactList("SipRedirectorPresenceRoutingTest");
      contactList.add(UtlString("sip:201@domain.com"), *this );
      contactList.add(UtlString("sip:202@domain.com"), *this );
      contactList.add(UtlString("sip:~~vm~203@domain.com"), *this );
      contactList.resetWasModifiedFlag();

      OsConfigDb configuration;
      configuration.set("VOICEMAIL_ON_BUSY", "Y" );
      configuration.set("SIP_DOMAIN", "bobnet.net" );
      SipRedirectorPresenceRouting pluginUnderTest("PIUT");
      pluginUnderTest.readConfig(configuration);

      RedirectPlugin::LookUpStatus rc;
      rc = pluginUnderTest.doLookUp(
            toUrl,
            contactList );

      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.entries() == 4 );
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true );
      ASSERT_STR_EQUAL( "sip:12345@bobnet.net", tmpString.data() );
   }

   void routeToTelUriMiddle1()
   {
      pUp = insertUnifiedPresenceEntry();
      Url toUrl("sip:dummy@example.com");
      pUp->setXmppStatusMessage("You can reach me at tel:+1234567890 if this is an emergency 1234");
      ContactList contactList("SipRedirectorPresenceRoutingTest");
      contactList.add(UtlString("sip:201@domain.com"), *this );
      contactList.add(UtlString("sip:202@domain.com"), *this );
      contactList.add(UtlString("sip:~~vm~203@domain.com"), *this );
      contactList.resetWasModifiedFlag();

      OsConfigDb configuration;
      configuration.set("VOICEMAIL_ON_BUSY", "Y" );
      configuration.set("SIP_DOMAIN", "bobnet.net" );
      SipRedirectorPresenceRouting pluginUnderTest("PIUT");
      pluginUnderTest.readConfig(configuration);

      RedirectPlugin::LookUpStatus rc;
      rc = pluginUnderTest.doLookUp(
            toUrl,
            contactList );

      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.entries() == 4 );
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true );
      ASSERT_STR_EQUAL( "sip:+1234567890@bobnet.net", tmpString.data() );
   }

   void routeToTelUriMiddle2()
   {
      pUp = insertUnifiedPresenceEntry();
      Url toUrl("sip:dummy@example.com");
      pUp->setXmppStatusMessage("You can reach me at tel:+1234567890, if this is an emergency 1234");
      ContactList contactList("SipRedirectorPresenceRoutingTest");
      contactList.add(UtlString("sip:201@domain.com"), *this );
      contactList.add(UtlString("sip:202@domain.com"), *this );
      contactList.add(UtlString("sip:~~vm~203@domain.com"), *this );
      contactList.resetWasModifiedFlag();

      OsConfigDb configuration;
      configuration.set("VOICEMAIL_ON_BUSY", "Y" );
      configuration.set("SIP_DOMAIN", "bobnet.net" );
      SipRedirectorPresenceRouting pluginUnderTest("PIUT");
      pluginUnderTest.readConfig(configuration);

      RedirectPlugin::LookUpStatus rc;
      rc = pluginUnderTest.doLookUp(
            toUrl,
            contactList );

      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.entries() == 4 );
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true );
      ASSERT_STR_EQUAL( "sip:+1234567890@bobnet.net", tmpString.data() );
   }

   void routeToTelUriEnd()
   {
      pUp = insertUnifiedPresenceEntry();
      Url toUrl("sip:dummy@example.com");
      pUp->setXmppStatusMessage("You can reach me at tel:+(800)-555-1212.1234");
      ContactList contactList("SipRedirectorPresenceRoutingTest");
      contactList.add(UtlString("sip:201@domain.com"), *this );
      contactList.add(UtlString("sip:202@domain.com"), *this );
      contactList.add(UtlString("sip:~~vm~203@domain.com"), *this );
      contactList.resetWasModifiedFlag();

      OsConfigDb configuration;
      configuration.set("VOICEMAIL_ON_BUSY", "Y" );
      configuration.set("SIP_DOMAIN", "bobnet.net" );
      SipRedirectorPresenceRouting pluginUnderTest("PIUT");
      pluginUnderTest.readConfig(configuration);

      RedirectPlugin::LookUpStatus rc;
      rc = pluginUnderTest.doLookUp(
            toUrl,
            contactList );

      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.entries() == 4 );
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true );
      ASSERT_STR_EQUAL( "sip:+(800)-555-1212.1234@bobnet.net", tmpString.data() );
   }

   void routeToInvalidTelUris()
   {
      pUp = insertUnifiedPresenceEntry();
      Url toUrl("sip:dummy@example.com");
      ContactList contactList("SipRedirectorPresenceRoutingTest");
      contactList.add(UtlString("sip:201@domain.com"), *this );
      contactList.add(UtlString("sip:202@domain.com"), *this );
      contactList.add(UtlString("sip:~~vm~203@domain.com"), *this );
      contactList.resetWasModifiedFlag();

      OsConfigDb configuration;
      configuration.set("VOICEMAIL_ON_BUSY", "N" );
      configuration.set("SIP_DOMAIN", "bobnet.net" );
      SipRedirectorPresenceRouting pluginUnderTest("PIUT");
      pluginUnderTest.readConfig(configuration);

      RedirectPlugin::LookUpStatus rc;

      pUp->setXmppStatusMessage("You can reach me at tel:+(800)-555-1212.1234@bobnet.net");
      rc = pluginUnderTest.doLookUp( toUrl, contactList );
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.wasListModified() == false );

      pUp->setXmppStatusMessage("You can reach me at tel:+(800)-555-1212.1234#");
      rc = pluginUnderTest.doLookUp( toUrl, contactList );
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.wasListModified() == false );

      pUp->setXmppStatusMessage("You can reach me at tel 1234;bob");
      rc = pluginUnderTest.doLookUp( toUrl, contactList );
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.wasListModified() == false );

      pUp->setXmppStatusMessage("You can reach me at tel:1234/bob");
      rc = pluginUnderTest.doLookUp( toUrl, contactList );
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.wasListModified() == false );

      pUp->setXmppStatusMessage("You can reach me at tel:1234a");
      rc = pluginUnderTest.doLookUp( toUrl, contactList );
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.wasListModified() == false );

   }

   void routeToSipUriBeginning()
   {
      pUp = insertUnifiedPresenceEntry();
      Url toUrl("sip:dummy@example.com");
      pUp->setXmppStatusMessage("sip:bob@bobnet.net is where I can be reached!");
      ContactList contactList("SipRedirectorPresenceRoutingTest");
      contactList.add(UtlString("sip:201@domain.com"), *this );
      contactList.add(UtlString("sip:202@domain.com"), *this );
      contactList.add(UtlString("sip:~~vm~203@domain.com"), *this );
      contactList.resetWasModifiedFlag();

      OsConfigDb configuration;
      configuration.set("VOICEMAIL_ON_BUSY", "Y" );
      configuration.set("SIP_DOMAIN", "bobnet.net" );
      SipRedirectorPresenceRouting pluginUnderTest("PIUT");
      pluginUnderTest.readConfig(configuration);

      RedirectPlugin::LookUpStatus rc;
      rc = pluginUnderTest.doLookUp(
            toUrl,
            contactList );

      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.entries() == 4 );
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true );
      ASSERT_STR_EQUAL( "sip:bob@bobnet.net", tmpString.data() );
   }

   void routeToSipUriMiddle1()
   {
      pUp = insertUnifiedPresenceEntry();
      Url toUrl("sip:dummy@example.com");
      pUp->setXmppStatusMessage("You can reach me at sip:bob@bobnet.net if this is an emergency!");
      ContactList contactList("SipRedirectorPresenceRoutingTest");
      contactList.add(UtlString("sip:201@domain.com"), *this );
      contactList.add(UtlString("sip:202@domain.com"), *this );
      contactList.add(UtlString("sip:~~vm~203@domain.com"), *this );
      contactList.resetWasModifiedFlag();

      OsConfigDb configuration;
      configuration.set("VOICEMAIL_ON_BUSY", "Y" );
      configuration.set("SIP_DOMAIN", "bobnet.net" );
      SipRedirectorPresenceRouting pluginUnderTest("PIUT");
      pluginUnderTest.readConfig(configuration);

      RedirectPlugin::LookUpStatus rc;
      rc = pluginUnderTest.doLookUp(
            toUrl,
            contactList );

      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.entries() == 4 );
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true );
      ASSERT_STR_EQUAL( "sip:bob@bobnet.net", tmpString.data() );
   }

   void routeToSipUriMiddle2()
   {
      pUp = insertUnifiedPresenceEntry();
      Url toUrl("sip:dummy@example.com");
      pUp->setXmppStatusMessage("You can reach me at sip:bob@bobnet.net, if this is an emergency!");
      ContactList contactList("SipRedirectorPresenceRoutingTest");
      contactList.add(UtlString("sip:201@domain.com"), *this );
      contactList.add(UtlString("sip:202@domain.com"), *this );
      contactList.add(UtlString("sip:~~vm~203@domain.com"), *this );
      contactList.resetWasModifiedFlag();

      OsConfigDb configuration;
      configuration.set("VOICEMAIL_ON_BUSY", "Y" );
      configuration.set("SIP_DOMAIN", "bobnet.net" );
      SipRedirectorPresenceRouting pluginUnderTest("PIUT");
      pluginUnderTest.readConfig(configuration);

      RedirectPlugin::LookUpStatus rc;
      rc = pluginUnderTest.doLookUp(
            toUrl,
            contactList );

      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.entries() == 4 );
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true );
      ASSERT_STR_EQUAL( "sip:bob@bobnet.net", tmpString.data() );
   }

   void routeToSipUriEnd()
   {
      pUp = insertUnifiedPresenceEntry();
      Url toUrl("sip:dummy@example.com");
      pUp->setXmppStatusMessage("You can reach me at sip:bob@bobnet.net");
      ContactList contactList("SipRedirectorPresenceRoutingTest");
      contactList.add(UtlString("sip:201@domain.com"), *this );
      contactList.add(UtlString("sip:202@domain.com"), *this );
      contactList.add(UtlString("sip:~~vm~203@domain.com"), *this );
      contactList.resetWasModifiedFlag();

      OsConfigDb configuration;
      configuration.set("VOICEMAIL_ON_BUSY", "Y" );
      configuration.set("SIP_DOMAIN", "bobnet.net" );
      SipRedirectorPresenceRouting pluginUnderTest("PIUT");
      pluginUnderTest.readConfig(configuration);

      RedirectPlugin::LookUpStatus rc;
      rc = pluginUnderTest.doLookUp(
            toUrl,
            contactList );

      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.entries() == 4 );
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true );
      ASSERT_STR_EQUAL( "sip:bob@bobnet.net", tmpString.data() );
   }

   void routeToInvalidSipUris()
   {
      pUp = insertUnifiedPresenceEntry();
      Url toUrl("sip:dummy@example.com");
      ContactList contactList("SipRedirectorPresenceRoutingTest");
      contactList.add(UtlString("sip:201@domain.com"), *this );
      contactList.add(UtlString("sip:202@domain.com"), *this );
      contactList.add(UtlString("sip:~~vm~203@domain.com"), *this );
      contactList.resetWasModifiedFlag();

      OsConfigDb configuration;
      configuration.set("VOICEMAIL_ON_BUSY", "N" );
      configuration.set("SIP_DOMAIN", "bobnet.net" );
      SipRedirectorPresenceRouting pluginUnderTest("PIUT");
      pluginUnderTest.readConfig(configuration);

      RedirectPlugin::LookUpStatus rc;

      pUp->setXmppStatusMessage("You can reach me at bob@bobnet.net");
      rc = pluginUnderTest.doLookUp( toUrl, contactList );
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.wasListModified() == false );

      pUp->setXmppStatusMessage("You can reach me at sip:bobnet.net");
      rc = pluginUnderTest.doLookUp( toUrl, contactList );
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.wasListModified() == false );

      pUp->setXmppStatusMessage("You can reach me at sip:bob");
      rc = pluginUnderTest.doLookUp( toUrl, contactList );
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );
      CPPUNIT_ASSERT( contactList.wasListModified() == false );
   }

};


CPPUNIT_TEST_SUITE_REGISTRATION(SipRedirectorPresenceRoutingTest);

