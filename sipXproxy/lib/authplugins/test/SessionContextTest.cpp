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
#include "SessionContext.h"
#include "NatTraversalRules.h"

class SessionContextTest : public CppUnit::TestCase, public CallTrackerInterfaceForSessionContext
{
      CPPUNIT_TEST_SUITE(SessionContextTest);
      CPPUNIT_TEST( LocationImposesMediaRelayTest_LocalNATed2LocalNATed );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_LocalNATed2Public );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_LocalNATed2RemoteNATed );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_Public2Public );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_Public2RemoteNATed );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_RemoteNATed2RemoteNATed );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_LocalNATed2OutsideUnknown );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_LocalNATed2LocalUnknown );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_Public2OutsideUnknown );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_Public2LocalUnknown );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_RemoteNATed2OutsideUnknown );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_RemoteNATed2LocalUnknown );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_Public2LocalNATed );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_RemoteNATed2LocalNATed );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_RemoteNATed2Public );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_OutsideUnknown2LocalNATed );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_LocalUnknown2LocalNATed );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_OutsideUnknown2Public );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_LocalUnknown2Public );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_OutsideUnknown2RemoteNATed );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_LocalUnknown2RemoteNATed );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_RemoteNATed2RemoteNATedConservative_SameIp );
      CPPUNIT_TEST( LocationImposesMediaRelayTest_RemoteNATed2RemoteNATedConservative_DifferentIp );
      CPPUNIT_TEST( getDiscriminatingTagValueWithDirectionTest );
      CPPUNIT_TEST( getDiscriminatingTagValueWithoutDirectionTest_CallerToCallee );
      CPPUNIT_TEST( getDiscriminatingTagValueWithoutDirectionTest_CalleeToCaller );
      CPPUNIT_TEST( downstreamForkHandlingTest );
      CPPUNIT_TEST_SUITE_END();

   public:
      SessionContext* mpSessionContext;
      NatTraversalRules mRules;
      MediaRelay mMediaRelay;

      SessionContextTest()
      {
         UtlString rulesFile(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules3.xml");
         CPPUNIT_ASSERT( mRules.loadRules( rulesFile ) == OS_SUCCESS );
      }

      virtual void reportSessionContextReadyForDeletion( const UtlString& handle )
      {

      }


      void setUp()
      {
         const char* message =
            "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         mpSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
      }

      void tearDown()
      {
         delete mpSessionContext;
      }

      void LocationImposesMediaRelayTest_LocalNATed2LocalNATed()
      {
         const char* message =
            "INVITE sip:601@192.168.0.11:5060;x-sipX-nonat SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:602@192.168.0.101:5060;x-sipX-nonat>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == false );
      }

      void LocationImposesMediaRelayTest_LocalNATed2Public()
      {
         const char* message =
            "INVITE sip:601@47.135.152.111:5060;x-sipX-nonat SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:602@192.168.0.101:5060;x-sipX-nonat>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == true );
      }

      void LocationImposesMediaRelayTest_LocalNATed2RemoteNATed()
      {
         const char* message =
            "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:602@192.168.0.101:5060;x-sipX-nonat>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == true );
      }

      void LocationImposesMediaRelayTest_Public2Public()
      {
         const char* message =
             "INVITE sip:602@47.135.152.112:5060;x-sipX-nonat SIP/2.0\r\n"
             "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
             "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
             "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
             "Cseq: 1 INVITE\r\n"
             "Max-Forwards: 19\r\n"
             "Supported: replaces\r\n"
             "Contact: <sip:602@47.135.152.111:5060;x-sipX-nonat>\r\n"
             "Content-Length: 0\r\n"
             "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
             "\r\n";
          SipMessage testMsg(message, strlen(message) );

          SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
          CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == false );
      }

      void LocationImposesMediaRelayTest_Public2RemoteNATed()
      {
         const char* message =
            "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:602@47.135.152.111:5060;x-sipX-nonat>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == true );
      }

      void LocationImposesMediaRelayTest_RemoteNATed2RemoteNATed()
      {
         const char* message =
            "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == true );
      }

      void LocationImposesMediaRelayTest_LocalNATed2OutsideUnknown()
      {
         const char* message =
            "INVITE sip:601@192.168.1.11:5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:602@192.168.0.101:5060;x-sipX-nonat>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == true );
      }

      void LocationImposesMediaRelayTest_LocalNATed2LocalUnknown()
      {
         const char* message =
            "INVITE sip:601@192.168.0.11:5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:602@192.168.0.101:5060;x-sipX-nonat>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == false );
      }

      void LocationImposesMediaRelayTest_Public2OutsideUnknown()
      {
         const char* message =
            "INVITE sip:601@192.168.1.11:5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:602@47.135.162.10:5060;x-sipX-nonat>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == true );
      }

      void LocationImposesMediaRelayTest_Public2LocalUnknown()
      {
         const char* message =
            "INVITE sip:601@192.168.0.11:5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:602@47.135.162.10:5060;x-sipX-nonat>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == true );
      }

      void LocationImposesMediaRelayTest_RemoteNATed2OutsideUnknown()
      {
         const char* message =
            "INVITE sip:601@192.168.1.11:5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == true );
      }

      void LocationImposesMediaRelayTest_RemoteNATed2LocalUnknown()
      {
         const char* message =
            "INVITE sip:601@192.168.0.11:5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == true );
      }

      void LocationImposesMediaRelayTest_Public2LocalNATed()
      {
         const char* message =
            "INVITE sip:602@192.168.0.101:5060;x-sipX-nonat SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@47.135.152.111:5060;x-sipX-nonat>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == true );
      }

      void LocationImposesMediaRelayTest_RemoteNATed2LocalNATed()
      {
         const char* message =
            "INVITE sip:602@192.168.0.101:5060;x-sipX-nonat SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == true );
      }

      void LocationImposesMediaRelayTest_RemoteNATed2Public()
      {
         const char* message =
            "INVITE sip:602@47.135.152.111:5060;x-sipX-nonat SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == true );
      }

      void LocationImposesMediaRelayTest_OutsideUnknown2LocalNATed()
      {
         const char* message =
            "INVITE sip:602@192.168.0.101:5060;x-sipX-nonat SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.1.11:5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == true );
      }

      void LocationImposesMediaRelayTest_LocalUnknown2LocalNATed()
      {
         const char* message =
            "INVITE sip:602@192.168.0.101:5060;x-sipX-nonat SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.0.11:5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == false );
      }

      void LocationImposesMediaRelayTest_OutsideUnknown2Public()
      {
         const char* message =
            "INVITE sip:602@47.135.162.10:5060;x-sipX-nonat SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.1.11:5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == true );
      }

      void LocationImposesMediaRelayTest_LocalUnknown2Public()
      {
         const char* message =
            "INVITE sip:602@47.135.162.10:5060;x-sipX-nonat SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.0.11:5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == true );
      }

      void LocationImposesMediaRelayTest_OutsideUnknown2RemoteNATed()
      {
         const char* message =
            "INVITE sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.1.11:5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == true );
      }

      void LocationImposesMediaRelayTest_LocalUnknown2RemoteNATed()
      {
         const char* message =
            "INVITE sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.0.11:5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == true );
      }

      void LocationImposesMediaRelayTest_RemoteNATed2RemoteNATedConservative_SameIp()
      {
         NatTraversalRules tempRules;
         UtlString rulesFile(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules4.xml");
         CPPUNIT_ASSERT( tempRules.loadRules( rulesFile ) == OS_SUCCESS );

         const char* message =
            "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &tempRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == false );
      }

      void LocationImposesMediaRelayTest_RemoteNATed2RemoteNATedConservative_DifferentIp()
      {
         NatTraversalRules tempRules;
         UtlString rulesFile(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules4.xml");
         CPPUNIT_ASSERT( tempRules.loadRules( rulesFile ) == OS_SUCCESS );

         const char* message =
            "INVITE sip:601@47.135.162.100:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &tempRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->doesEndpointsLocationImposeMediaRelay() == true );
      }

      void getDiscriminatingTagValueWithDirectionTest()
      {
         const char* message =
            "INVITE sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>;tag=131313213131-13135363563\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.1.11:5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->getDiscriminatingTagValue( testMsg, DIR_CALLER_TO_CALLEE ) == "131313213131-13135363563" );
         CPPUNIT_ASSERT( pSessionContext->getDiscriminatingTagValue( testMsg, DIR_CALLEE_TO_CALLER ) == "94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635" );
      }

      void getDiscriminatingTagValueWithoutDirectionTest_CallerToCallee()
      {
         const char* message =
            "INVITE sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>;tag=131313213131-13135363563\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.1.11:5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext(testMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->getDiscriminatingTagValue( testMsg ) == "131313213131-13135363563" );
      }

      void getDiscriminatingTagValueWithoutDirectionTest_CalleeToCaller()
      {
         const char* message =
            "INVITE sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.1.11:5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage dialogFormingMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext( dialogFormingMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         pSessionContext->handleRequest( dialogFormingMsg, "192.168.0.2", 5060, DIR_CALLER_TO_CALLEE ); // inject dialog-forming INVITE into SessioNContext to set reference

         const char* message2 =
            "INVITE sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060 SIP/2.0\r\n"
            "From: <sip:601@rjolyscs2.ca.nortel.com>;tag=131313213131-13135363563\r\n"
            "To: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.1.11:5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage calleeToCallertestMsg(message2, strlen(message2) );   // To and From reversed.
         CPPUNIT_ASSERT( pSessionContext->getDiscriminatingTagValue( calleeToCallertestMsg ) == "131313213131-13135363563" );
      }

      void downstreamForkHandlingTest()
      {
         const char* message =
            "INVITE sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:602@192.168.1.11:5060>\r\n"
            "Content-Length: 0\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage dialogFormingMsg(message, strlen(message) );

         SessionContext* pSessionContext = new SessionContext( dialogFormingMsg, &mRules, "handle-1", &mMediaRelay, 0, this );
         CPPUNIT_ASSERT( pSessionContext->mpReferenceDialogTracker == 0 );
         pSessionContext->handleRequest( dialogFormingMsg, "192.168.0.2", 5060, DIR_CALLER_TO_CALLEE ); // inject dialog-forming INVITE into SessioNContext to set reference
         CPPUNIT_ASSERT( pSessionContext->mpReferenceDialogTracker != 0 );
         CPPUNIT_ASSERT( pSessionContext->getNumberOfTrackedDialogs() == 0 );

         const char* provisionalResponse1 =
            "SIP/2.0 183 Session Progress\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>;tag=discriminating-tag-1\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "CSeq: 1 INVITE\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.1.11:5060>\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage provisionalResponse1Msg( provisionalResponse1, strlen( provisionalResponse1 ) );

         const char* provisionalResponse2 =
            "SIP/2.0 183 Session Progress\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>;tag=discriminating-tag-2\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "CSeq: 1 INVITE\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.1.11:5060>\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage provisionalResponse2Msg( provisionalResponse2, strlen( provisionalResponse2 ) );

         const char* provisionalResponse3 =
            "SIP/2.0 183 Session Progress\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>;tag=discriminating-tag-3\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "CSeq: 1 INVITE\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.1.11:5060>\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage provisionalResponse3Msg( provisionalResponse3, strlen( provisionalResponse3 ) );

         // inject a response that will create a new dialog
         pSessionContext->handleResponse( provisionalResponse1Msg, "192.168.0.2", 5060 );
         CPPUNIT_ASSERT( pSessionContext->getNumberOfTrackedDialogs() == 1 );
         pSessionContext->handleResponse( provisionalResponse1Msg, "192.168.0.2", 5060 );
         CPPUNIT_ASSERT( pSessionContext->getNumberOfTrackedDialogs() == 1 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForTag( "discriminating-tag-1" ) != 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForTag( "discriminating-tag-2" ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForTag( "discriminating-tag-3" ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse1Msg ) != 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse2Msg ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse3Msg ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse1Msg ) ==
                         pSessionContext->getDialogTrackerForTag( "discriminating-tag-1" ) );

         // inject a response that will create another dialog
         pSessionContext->handleResponse( provisionalResponse2Msg, "192.168.0.2", 5060 );
         CPPUNIT_ASSERT( pSessionContext->getNumberOfTrackedDialogs() == 2 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForTag( "discriminating-tag-1" ) != 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForTag( "discriminating-tag-2" ) != 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForTag( "discriminating-tag-3" ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForTag( "discriminating-tag-1" ) !=
                         pSessionContext->getDialogTrackerForTag( "discriminating-tag-2" ) );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse1Msg ) != 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse2Msg ) != 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse3Msg ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse1Msg ) ==
                         pSessionContext->getDialogTrackerForTag( "discriminating-tag-1" ) );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse2Msg ) ==
                         pSessionContext->getDialogTrackerForTag( "discriminating-tag-2" ) );

         const char* failingResponse1 =
            "SIP/2.0 408 Request Timeout\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>;tag=discriminating-tag-1\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "CSeq: 1 INVITE\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.1.11:5060>\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage failingResponse1Msg( failingResponse1, strlen( failingResponse1 ) );

         // inject a response that will terminate a dialog
         pSessionContext->handleResponse( failingResponse1Msg, "192.168.0.2", 5060 );

         CPPUNIT_ASSERT( pSessionContext->getNumberOfTrackedDialogs() == 1 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForTag( "discriminating-tag-1" ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForTag( "discriminating-tag-2" ) != 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForTag( "discriminating-tag-3" ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse1Msg ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse2Msg ) != 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse3Msg ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse2Msg ) ==
                         pSessionContext->getDialogTrackerForTag( "discriminating-tag-2" ) );

         const char* failingResponse2 =
            "SIP/2.0 408 Request Timeout\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>;tag=discriminating-tag-2\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "CSeq: 1 INVITE\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.1.11:5060>\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage failingResponse2Msg( failingResponse2, strlen( failingResponse2 ) );

         // inject a response that will terminate a dialog
         pSessionContext->handleResponse( failingResponse1Msg, "192.168.0.2", 5060 );

         CPPUNIT_ASSERT( pSessionContext->getNumberOfTrackedDialogs() == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForTag( "discriminating-tag-1" ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForTag( "discriminating-tag-2" ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForTag( "discriminating-tag-3" ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse1Msg ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse2Msg ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse3Msg ) == 0 );

         // inject a response that will create a dialog
         pSessionContext->handleResponse( provisionalResponse3Msg, "192.168.0.2", 5060 );
         CPPUNIT_ASSERT( pSessionContext->getNumberOfTrackedDialogs() == 1 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForTag( "discriminating-tag-1" ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForTag( "discriminating-tag-2" ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForTag( "discriminating-tag-3" ) != 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse1Msg ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse2Msg ) == 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse3Msg ) != 0 );
         CPPUNIT_ASSERT( pSessionContext->getDialogTrackerForMessage( provisionalResponse3Msg ) ==
                         pSessionContext->getDialogTrackerForTag( "discriminating-tag-3" ) );
      }
   };

CPPUNIT_TEST_SUITE_REGISTRATION(SessionContextTest);
