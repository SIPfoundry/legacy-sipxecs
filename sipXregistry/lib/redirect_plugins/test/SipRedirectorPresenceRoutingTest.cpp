// 
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.  
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
   CPPUNIT_TEST(routeToTelUriMiddle);
   CPPUNIT_TEST(routeToTelUriEnd);
   CPPUNIT_TEST(routeToUnvalidTelUris);
   CPPUNIT_TEST_SUITE_END();

public:
   PresenceLookupTask storage; 

   SipRedirectorPresenceRoutingTest() : 
      DummyPlugin("Dummy PlugIn"),
      storage( *(new Url("dummy")), 0, 0, *(new Url("dummy")) )
   {
      storage.mbPresenceInfoAvailable = true;
      storage.mSipUserIdentity = "sip:dummy@example.com";
      storage.mTelephonyPresence = "idle";
      storage.mXmppPresence = "offline";
      storage.mCustomPresenceMessage = "custom message";
   }

   void removeVoicemailContactTop()
   {
      SipMessage dummySipMessage;
      Url dummyRequestUri;
      ErrorDescriptor errorDescriptor;
      storage.mTelephonyPresence = "busy";
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
      SipRedirectorPrivateStorage *pStorage = &storage;
      rc = pluginUnderTest.lookUp(
            dummySipMessage,
            UtlString("DUMMY"),
            dummyRequestUri,
            UtlString("DUMMY"),
            contactList,
            0,
            0,
            pStorage,
            errorDescriptor );
      
      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );       
      CPPUNIT_ASSERT( contactList.entries() == 1 );        
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true );
      ASSERT_STR_EQUAL( "sip:~~vm~203@domain.com", tmpString.data() );
   }

   void removeVoicemailContactMiddle()
   {
      SipMessage dummySipMessage;
      Url dummyRequestUri;
      ErrorDescriptor errorDescriptor;
      storage.mTelephonyPresence = "busy";
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
      SipRedirectorPrivateStorage *pStorage = &storage;
      rc = pluginUnderTest.lookUp(
            dummySipMessage,
            UtlString("DUMMY"),
            dummyRequestUri,
            UtlString("DUMMY"),
            contactList,
            0,
            0,
            pStorage,
            errorDescriptor );
      
      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );       
      CPPUNIT_ASSERT( contactList.entries() == 1 );        
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true );
      ASSERT_STR_EQUAL( "sip:~~vm~203@domain.com", tmpString.data() );
   }

   void removeVoicemailContactBottom()
   {
      SipMessage dummySipMessage;
      Url dummyRequestUri;
      ErrorDescriptor errorDescriptor;
      storage.mTelephonyPresence = "busy";
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
      SipRedirectorPrivateStorage *pStorage = &storage;
      rc = pluginUnderTest.lookUp(
            dummySipMessage,
            UtlString("DUMMY"),
            dummyRequestUri,
            UtlString("DUMMY"),
            contactList,
            0,
            0,
            pStorage,
            errorDescriptor );
      
      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );       
      CPPUNIT_ASSERT( contactList.entries() == 1 );        
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true );
      ASSERT_STR_EQUAL( "sip:~~vm~203@domain.com", tmpString.data() );
   }

   void removeVoicemailContactButNotFound()
   {
      SipMessage dummySipMessage;
      Url dummyRequestUri;
      ErrorDescriptor errorDescriptor;
      storage.mTelephonyPresence = "busy";
      ContactList contactList("SipRedirectorPresenceRoutingTest");
      contactList.add(UtlString("sip:201@domain.com"), *this );
      contactList.add(UtlString("sip:202@domain.com"), *this );
      contactList.resetWasModifiedFlag();

      OsConfigDb configuration;
      configuration.set("VOICEMAIL_ON_BUSY", "Y" );
      SipRedirectorPresenceRouting pluginUnderTest("PIUT");
      pluginUnderTest.readConfig(configuration);

      RedirectPlugin::LookUpStatus rc;
      SipRedirectorPrivateStorage *pStorage = &storage;
      rc = pluginUnderTest.lookUp(
            dummySipMessage,
            UtlString("DUMMY"),
            dummyRequestUri,
            UtlString("DUMMY"),
            contactList,
            0,
            0,
            pStorage,
            errorDescriptor );
      
      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );       
      CPPUNIT_ASSERT( contactList.entries() == 2 );
      CPPUNIT_ASSERT( contactList.wasListModified() == false );
   }   
   
   void dontRemoveVoicemailContact()
   {
      SipMessage dummySipMessage;
      Url dummyRequestUri;
      ErrorDescriptor errorDescriptor;
      storage.mTelephonyPresence = "busy";
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
      SipRedirectorPrivateStorage *pStorage = &storage;
      rc = pluginUnderTest.lookUp(
            dummySipMessage,
            UtlString("DUMMY"),
            dummyRequestUri,
            UtlString("DUMMY"),
            contactList,
            0,
            0,
            pStorage,
            errorDescriptor );
      
      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );       
      CPPUNIT_ASSERT( contactList.entries() == 3 );  
      CPPUNIT_ASSERT( contactList.wasListModified() == false );
   }

   void routeToTelUriBeginning()
   {
      SipMessage dummySipMessage;
      Url dummyRequestUri;
      ErrorDescriptor errorDescriptor;
      storage.mCustomPresenceMessage = "tel:bob@bobnet.net is where I can be reached!";
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
      SipRedirectorPrivateStorage *pStorage = &storage;
      rc = pluginUnderTest.lookUp(
            dummySipMessage,
            UtlString("DUMMY"),
            dummyRequestUri,
            UtlString("DUMMY"),
            contactList,
            0,
            0,
            pStorage,
            errorDescriptor );
      
      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );       
      CPPUNIT_ASSERT( contactList.entries() == 4 );  
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true );
      ASSERT_STR_EQUAL( "sip:bob@bobnet.net", tmpString.data() );
   }

   void routeToTelUriMiddle()
   {
      SipMessage dummySipMessage;
      Url dummyRequestUri;
      ErrorDescriptor errorDescriptor;
      storage.mCustomPresenceMessage = "You can reach me at tel:bob@bobnet.net if this is an emergency!";
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
      SipRedirectorPrivateStorage *pStorage = &storage;
      rc = pluginUnderTest.lookUp(
            dummySipMessage,
            UtlString("DUMMY"),
            dummyRequestUri,
            UtlString("DUMMY"),
            contactList,
            0,
            0,
            pStorage,
            errorDescriptor );
      
      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );       
      CPPUNIT_ASSERT( contactList.entries() == 4 );  
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true );
      ASSERT_STR_EQUAL( "sip:bob@bobnet.net", tmpString.data() );
   }

   void routeToTelUriEnd()   
   {
      SipMessage dummySipMessage;
      Url dummyRequestUri;
      ErrorDescriptor errorDescriptor;
      storage.mCustomPresenceMessage = "You can reach me at tel:bob@bobnet.net";
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
      SipRedirectorPrivateStorage *pStorage = &storage;
      rc = pluginUnderTest.lookUp(
            dummySipMessage,
            UtlString("DUMMY"),
            dummyRequestUri,
            UtlString("DUMMY"),
            contactList,
            0,
            0,
            pStorage,
            errorDescriptor );
      
      UtlString tmpString;
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );       
      CPPUNIT_ASSERT( contactList.entries() == 4 );  
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true );
      ASSERT_STR_EQUAL( "sip:bob@bobnet.net", tmpString.data() );
   }
   
   void routeToUnvalidTelUris()
   {
      SipMessage dummySipMessage;
      Url dummyRequestUri;
      ErrorDescriptor errorDescriptor;
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

      
      SipRedirectorPrivateStorage *pStorage = &storage;
      UtlString tmpString;
      
      storage.mCustomPresenceMessage = "You can reach me at bob@bobnet.net";
      rc = pluginUnderTest.lookUp( dummySipMessage, UtlString("DUMMY"), dummyRequestUri, UtlString("DUMMY"), contactList, 0, 0, pStorage, errorDescriptor );
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );       
      CPPUNIT_ASSERT( contactList.wasListModified() == false );  

      storage.mCustomPresenceMessage = "You can reach me at tel:bobnet.net";
      rc = pluginUnderTest.lookUp( dummySipMessage, UtlString("DUMMY"), dummyRequestUri, UtlString("DUMMY"), contactList, 0, 0, pStorage, errorDescriptor );
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );       
      CPPUNIT_ASSERT( contactList.wasListModified() == false );  

      storage.mCustomPresenceMessage = "You can reach me at tel bob@bobnet.net";
      rc = pluginUnderTest.lookUp( dummySipMessage, UtlString("DUMMY"), dummyRequestUri, UtlString("DUMMY"), contactList, 0, 0, pStorage, errorDescriptor );
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );       
      CPPUNIT_ASSERT( contactList.wasListModified() == false );  

      storage.mCustomPresenceMessage = "You can reach me at http://bobnet.net";
      rc = pluginUnderTest.lookUp( dummySipMessage, UtlString("DUMMY"), dummyRequestUri, UtlString("DUMMY"), contactList, 0, 0, pStorage, errorDescriptor );
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );       
      CPPUNIT_ASSERT( contactList.wasListModified() == false );  

      storage.mCustomPresenceMessage = "You can reach me at mailto:bob@bobnet.net";
      rc = pluginUnderTest.lookUp( dummySipMessage, UtlString("DUMMY"), dummyRequestUri, UtlString("DUMMY"), contactList, 0, 0, pStorage, errorDescriptor );
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );       
      CPPUNIT_ASSERT( contactList.wasListModified() == false );  

      storage.mCustomPresenceMessage = "You can reach me at sip:bob@bobnet.net";
      rc = pluginUnderTest.lookUp( dummySipMessage, UtlString("DUMMY"), dummyRequestUri, UtlString("DUMMY"), contactList, 0, 0, pStorage, errorDescriptor );
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );       
      CPPUNIT_ASSERT( contactList.wasListModified() == false );  

      storage.mCustomPresenceMessage = "You can reach me at sip:tel@bobnet.net";
      rc = pluginUnderTest.lookUp( dummySipMessage, UtlString("DUMMY"), dummyRequestUri, UtlString("DUMMY"), contactList, 0, 0, pStorage, errorDescriptor );
      CPPUNIT_ASSERT( rc == RedirectPlugin::SUCCESS );       
      CPPUNIT_ASSERT( contactList.wasListModified() == false );  
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipRedirectorPresenceRoutingTest);
   
