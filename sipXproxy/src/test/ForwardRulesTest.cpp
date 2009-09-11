//
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

#include "os/OsDefs.h"
#include "net/Url.h"
#include "net/SipMessage.h"
#include "ForwardRules.h"

#define VM "VoIcEmAiL"
#define MS "MeDiAsErVeR"
#define LH "LoCaLhOsT"

class ForwardRulesTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(ForwardRulesTest);
      CPPUNIT_TEST(testNoMethodDefault);
      CPPUNIT_TEST(testSimpleMapMatchMethod);
      CPPUNIT_TEST(testSimpleMapMatchFieldExact);
      CPPUNIT_TEST(testSimpleMapMatchFieldLong);
      CPPUNIT_TEST(testSimpleMapNonMatchFieldPrefix);
      CPPUNIT_TEST(testSimpleMapNoField);
      CPPUNIT_TEST(testSimpleMapForeignSubnet);
      CPPUNIT_TEST(testSimpleMapForeignDNS);
      CPPUNIT_TEST(testSimpleMapAuthProxy);
      CPPUNIT_TEST_SUITE_END();


   public:
   void testNoMethodDefault()
      {
         ForwardRules theRules;
         UtlString     theRoute;
         UtlString     mappingType;
         bool          authRequired;
         UtlString     rulesFile(TEST_DATA_DIR "/rulesdata/simple.xml");

         CPPUNIT_ASSERT( theRules.loadMappings(rulesFile, MS, VM, LH )
                        == OS_SUCCESS
                        );

         CPPUNIT_ASSERT( theRules.getRoute(Url("sip:sipuaconfig.SIPXCHANGE_DOMAIN_NAME"),
                                           SipMessage("UNKNOWN sip:sipuaconfig.SIPXCHANGE_DOMAIN_NAME SIP/2.0\r\n"
                                                      "\r\n"
                                                      ),
                                           theRoute,
                                           mappingType,
                                           authRequired
                                           )
                        == OS_SUCCESS
                        );

         ASSERT_STR_EQUAL("CONFIG_SERVER", theRoute.data());
         ASSERT_STR_EQUAL("config", mappingType.data());
      }


      void testSimpleMapDefault()
      {
         ForwardRules theRules;
         UtlString     theRoute;
         UtlString     mappingType;
         bool          authRequired;
         UtlString     rulesFile(TEST_DATA_DIR "/rulesdata/simple.xml");

         CPPUNIT_ASSERT( theRules.loadMappings(rulesFile, MS, VM, LH )
                        == OS_SUCCESS
                        );

         CPPUNIT_ASSERT( theRules.getRoute(Url("sip:OTHER_DOMAIN_NAME"),
                                           SipMessage("UNKNOWN sip:SIPXCHANGE_DOMAIN_NAME SIP/2.0\r\n"
                                                      "\r\n"
                                                      ),
                                           theRoute,
                                           mappingType,
                                           authRequired
                                           )
                        != OS_SUCCESS
                        );
         //CPPUNIT_ASSERT( strcmp(theRoute.data(),"REGISTRAR_SERVER_DEFAULT") == 0 );
         //CPPUNIT_ASSERT( strcmp(mappingType.data(),"local") == 0 );
      }



      void testSimpleMapMatchMethod()
      {
         ForwardRules theRules;
         UtlString     theRoute;
         UtlString     mappingType;
         bool          authRequired;
         UtlString     rulesFile(TEST_DATA_DIR "/rulesdata/simple.xml");

         CPPUNIT_ASSERT( theRules.loadMappings(rulesFile, MS, VM, LH )
                        == OS_SUCCESS
                        );

         CPPUNIT_ASSERT( theRules.getRoute(Url("sip:SIPXCHANGE_DOMAIN_NAME"),
                                           SipMessage("REGISTER sip:SIPXCHANGE_DOMAIN_NAME SIP/2.0\r\n"
                                                      "\r\n"
                                                      ),
                                           theRoute,
                                           mappingType,
                                           authRequired
                                           )
                        == OS_SUCCESS
                        );
         CPPUNIT_ASSERT( strcmp(theRoute.data(),"REGISTRAR_SERVER_DEFAULT") == 0 );
         CPPUNIT_ASSERT( strcmp(mappingType.data(),"local") == 0 );
      }

      void testSimpleMapMatchFieldExact()
      {
         ForwardRules theRules;
         UtlString     theRoute;
         UtlString     mappingType;
         bool          authRequired;
         UtlString     rulesFile(TEST_DATA_DIR "/rulesdata/simple.xml");

         CPPUNIT_ASSERT( theRules.loadMappings(rulesFile, MS, VM, LH )
                        == OS_SUCCESS
                        );

         CPPUNIT_ASSERT( theRules.getRoute(Url("sip:SIPXCHANGE_DOMAIN_NAME"),
                                           SipMessage("SUBSCRIBE sip:SIPXCHANGE_DOMAIN_NAME SIP/2.0\r\n"
                                                      "Event: sip-config\r\n"
                                                      "\r\n"
                                                      ),
                                           theRoute,
                                           mappingType,
                                           authRequired
                                           )
                        == OS_SUCCESS
                        );
         CPPUNIT_ASSERT( strcmp(theRoute.data(),"CONFIG_SERVER_SUBSCRIBE") == 0 );
         CPPUNIT_ASSERT( strcmp(mappingType.data(),"local") == 0 );
      }

      void testSimpleMapMatchFieldLong()
      {
         ForwardRules theRules;
         UtlString     theRoute;
         UtlString     mappingType;
         bool          authRequired;
         UtlString     rulesFile(TEST_DATA_DIR "/rulesdata/simple.xml");

         CPPUNIT_ASSERT( theRules.loadMappings(rulesFile, MS, VM, LH )
                        == OS_SUCCESS
                        );

         CPPUNIT_ASSERT( theRules.getRoute(Url("sip:SIPXCHANGE_DOMAIN_NAME"),
                                           SipMessage("SUBSCRIBE sip:SIPXCHANGE_DOMAIN_NAME SIP/2.0\r\n"
                                                      "Event: sip-config-something\r\n"
                                                      "\r\n"
                                                      ),
                                           theRoute,
                                           mappingType,
                                           authRequired
                                           )
                        == OS_SUCCESS
                        );
         CPPUNIT_ASSERT( strcmp(theRoute.data(),"CONFIG_SERVER_SUBSCRIBE") == 0 );
         CPPUNIT_ASSERT( strcmp(mappingType.data(),"local") == 0 );
      }

      void testSimpleMapNonMatchFieldPrefix()
      {
         ForwardRules theRules;
         UtlString     theRoute;
         UtlString     mappingType;
         bool          authRequired;
         UtlString     rulesFile(TEST_DATA_DIR "/rulesdata/simple.xml");

         CPPUNIT_ASSERT( theRules.loadMappings(rulesFile, MS, VM, LH )
                        == OS_SUCCESS
                        );

         CPPUNIT_ASSERT( theRules.getRoute(Url("sip:SIPXCHANGE_DOMAIN_NAME"),
                                           SipMessage("SUBSCRIBE sip:SIPXCHANGE_DOMAIN_NAME SIP/2.0\r\n"
                                                      "Event: not-sip-config-something\r\n"
                                                      "\r\n"
                                                      ),
                                           theRoute,
                                           mappingType,
                                           authRequired
                                           )
                        == OS_SUCCESS
                        );
         CPPUNIT_ASSERT( strcmp(theRoute.data(),"REGISTRAR_SERVER_SUBSCRIBE") == 0 );
         CPPUNIT_ASSERT( strcmp(mappingType.data(),"local") == 0 );
      }

      void testSimpleMapNoField()
      {
         ForwardRules theRules;
         UtlString     theRoute;
         UtlString     mappingType;
         bool          authRequired;
         UtlString     rulesFile(TEST_DATA_DIR "/rulesdata/simple.xml");

         CPPUNIT_ASSERT( theRules.loadMappings(rulesFile, MS, VM, LH )
                        == OS_SUCCESS
                        );

         CPPUNIT_ASSERT( theRules.getRoute(Url("sip:SIPXCHANGE_DOMAIN_NAME"),
                                           SipMessage("UNKNOWN sip:SIPXCHANGE_DOMAIN_NAME SIP/2.0\r\n"
                                                      "\r\n"
                                                      ),
                                           theRoute,
                                           mappingType,
                                           authRequired
                                           )
                        == OS_SUCCESS
                        );
         CPPUNIT_ASSERT( strcmp(theRoute.data(),"REGISTRAR_SERVER_DEFAULT") == 0 );
         CPPUNIT_ASSERT( strcmp(mappingType.data(),"local") == 0 );
      }

      void testSimpleMapForeignSubnet()
      {
         ForwardRules theRules;
         UtlString     theRoute;
         UtlString     mappingType;
         bool          authRequired;
         UtlString     rulesFile(TEST_DATA_DIR "/rulesdata/simple.xml");

         CPPUNIT_ASSERT( theRules.loadMappings(rulesFile, MS, VM, LH )
                        == OS_SUCCESS
                        );

         CPPUNIT_ASSERT( theRules.getRoute(Url("sip:10.1.1.1:4242"),
                                           SipMessage("UNKNOWN sip:SIPXCHANGE_DOMAIN_NAME SIP/2.0\r\n"
                                                      "\r\n"
                                                      ),
                                           theRoute,
                                           mappingType,
                                           authRequired
                                           )
                        == OS_SUCCESS
                        );
         CPPUNIT_ASSERT( strcmp(theRoute.data(),"FOREIGN_SUBNET") == 0 );
         CPPUNIT_ASSERT( strcmp(mappingType.data(),"foreignSubnet") == 0 );
         CPPUNIT_ASSERT( authRequired == true );
      }

      void testSimpleMapForeignDNS()
      {
         ForwardRules theRules;
         UtlString     theRoute;
         UtlString     mappingType;
         bool          authRequired;
         UtlString     rulesFile(TEST_DATA_DIR "/rulesdata/simple.xml");

         CPPUNIT_ASSERT( theRules.loadMappings(rulesFile, MS, VM, LH )
                        == OS_SUCCESS
                        );

         CPPUNIT_ASSERT( theRules.getRoute(Url("sip:user@puppy.dog.woof.NeT"),
                                           SipMessage("UNKNOWN sip:SIPXCHANGE_DOMAIN_NAME SIP/2.0\r\n"
                                                      "\r\n"
                                                      ),
                                           theRoute,
                                           mappingType,
                                           authRequired
                                           )
                        == OS_SUCCESS
                        );
         CPPUNIT_ASSERT( strcmp(theRoute.data(),"FOREIGN_DNS") == 0 );
         CPPUNIT_ASSERT( strcmp(mappingType.data(),"foreignDns") == 0 );
         CPPUNIT_ASSERT( authRequired == true );

         CPPUNIT_ASSERT( theRules.getRoute(Url("sip:user@woof.net."),
                                           SipMessage("UNKNOWN sip:SIPXCHANGE_DOMAIN_NAME SIP/2.0\r\n"
                                                      "\r\n"
                                                      ),
                                           theRoute,
                                           mappingType,
                                           authRequired
                                           )
                        == OS_SUCCESS
                        );
         CPPUNIT_ASSERT( strcmp(theRoute.data(),"FOREIGN_DNS") == 0 );
         CPPUNIT_ASSERT( strcmp(mappingType.data(),"foreignDns") == 0 );
         CPPUNIT_ASSERT( authRequired == true );
      }

      void testSimpleMapAuthProxy()
      {
         ForwardRules theRules;
         UtlString     theRoute;
         UtlString     mappingType;
         bool          authRequired;
         UtlString     rulesFile(TEST_DATA_DIR "/rulesdata/simple.xml");

         CPPUNIT_ASSERT( theRules.loadMappings(rulesFile, MS, VM, LH )
                        == OS_SUCCESS
                        );

         CPPUNIT_ASSERT( theRules.getRoute(Url("sip:AUTHPROXY.GOOD"),
                                           SipMessage("UNKNOWN sip:SIPXCHANGE_DOMAIN_NAME SIP/2.0\r\n"
                                                      "\r\n"
                                                      ),
                                           theRoute,
                                           mappingType,
                                           authRequired
                                           )
                        == OS_SUCCESS
                        );
         CPPUNIT_ASSERT( theRoute.length() == 0 );
         CPPUNIT_ASSERT( strcmp(mappingType.data(),"authProxy good") == 0 );
         CPPUNIT_ASSERT( authRequired == true );

         CPPUNIT_ASSERT( theRules.getRoute(Url("sip:AUTHPROXY.BAD"),
                                           SipMessage("UNKNOWN sip:SIPXCHANGE_DOMAIN_NAME SIP/2.0\r\n"
                                                      "\r\n"
                                                      ),
                                           theRoute,
                                           mappingType,
                                           authRequired
                                           )
                        == OS_FAILED
                        );
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ForwardRulesTest);
