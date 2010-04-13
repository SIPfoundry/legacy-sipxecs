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
#include "DialogTracker.h"
#include "SessionContext.h"
#include "NatTraversalRules.h"

class DialogTrackerTest : public CppUnit::TestCase, public SessionContextInterfaceForDialogTracker
{
      CPPUNIT_TEST_SUITE(DialogTrackerTest);
      CPPUNIT_TEST( timerTickCounterTest );      
      CPPUNIT_TEST( DialogEstablishedFlagManipsTest );
      CPPUNIT_TEST( TransactionDirectionalityFlagManipsTest );
      CPPUNIT_TEST( MediaRelayRequiredFlagManipsTest );
      CPPUNIT_TEST( MediaDescriptorsManipsTest );      
      CPPUNIT_TEST( getOurMediaRelayHandleEncodedInSdpTest );            
      CPPUNIT_TEST( requestHandledMarkerManipTest );            
      CPPUNIT_TEST( RequestRetransmissionDescriptorTest );            
      CPPUNIT_TEST( ResponseRetransmissionDescriptorTest );            
      CPPUNIT_TEST( RemoveUnwantedElementsTest );            
      CPPUNIT_TEST_SUITE_END();

   public:
      DialogTracker* mpDialogTracker;
      NatTraversalRules mRules;
    
      DialogTrackerTest()
      {
      }

      bool doesEndpointsLocationImposeMediaRelay( void ) const {return false;}
      bool allocateMediaRelaySession( const UtlString& handleOfRequestingDialogContext,
                                      tMediaRelayHandle& relayHandle,
                                      int& callerRelayRtpPort,
                                      int& calleeRelayRtpPort ){return false;}
      tMediaRelayHandle cloneMediaRelaySession( const UtlString& handleOfRequestingDialogContext,
                                                tMediaRelayHandle& relayHandleToClone,
                                                bool doSwapCallerAndCallee ){return 0;;}
      bool deallocateMediaRelaySession( const UtlString& handleOfRequestingDialogContext,
                                        const tMediaRelayHandle& relayHandle ){return false;}
      bool setMediaRelayDirectionMode( const UtlString& handleOfRequestingDialogContext,
                                       const tMediaRelayHandle& relayHandle,
                                       MediaDirectionality mediaRelayDirectionMode,
                                       EndpointRole endpointRole ){return false;}
      bool linkFarEndMediaRelayPortToRequester( const UtlString& handleOfRequestingDialogContext,
                                                const tMediaRelayHandle& relayHandle,
                                                const MediaDescriptor* pMediaDescriptor,
                                                EndpointRole endpointRoleOfRequester ){return false;}
      bool getMediaRelayAddressToUseInSdp( UtlString& mediaRelayAddressToUse, EndpointRole endpointRole ) const{return false;}
      void reportDialogTrackerReadyForDeletion( const UtlString& handleOfRequestingDialogContext ){}
      int getRtpRelayPortForMediaRelaySession(const tMediaRelayHandle& handle, EndpointRole role){return 0;}
      bool getPacketProcessingStatsForMediaRelaySession( const tMediaRelayHandle& handle,
                                                                 PacketProcessingStatistics& stats ){return false;}

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

         mpDialogTracker = new DialogTracker( "handle-1", "192.168.0.2", this );
      }
      
      void tearDown()
      {
         delete mpDialogTracker;
      }
      
      void timerTickCounterTest()
      {
         CPPUNIT_ASSERT( mpDialogTracker->getTimerTickCounter() == 0 );
         CPPUNIT_ASSERT( mpDialogTracker->incrementTimerTickCounter() == 1 );
         CPPUNIT_ASSERT( mpDialogTracker->getTimerTickCounter() == 1 );
         CPPUNIT_ASSERT( mpDialogTracker->incrementTimerTickCounter() == 2 );
         CPPUNIT_ASSERT( mpDialogTracker->getTimerTickCounter() == 2 );
         mpDialogTracker->resetTimerTickCounter();
         CPPUNIT_ASSERT( mpDialogTracker->getTimerTickCounter() == 0 );
         CPPUNIT_ASSERT( mpDialogTracker->incrementTimerTickCounter() == 1 );
         CPPUNIT_ASSERT( mpDialogTracker->getTimerTickCounter() == 1 );
         CPPUNIT_ASSERT( mpDialogTracker->incrementTimerTickCounter() == 2 );
         CPPUNIT_ASSERT( mpDialogTracker->getTimerTickCounter() == 2 );
      }   
      
      void DialogEstablishedFlagManipsTest()
      {
         CPPUNIT_ASSERT( mpDialogTracker->getDialogEstablishedFlag() == false );
         mpDialogTracker->setDialogEstablishedFlag();
         CPPUNIT_ASSERT( mpDialogTracker->getDialogEstablishedFlag() == true );
         mpDialogTracker->setDialogEstablishedFlag();
         CPPUNIT_ASSERT( mpDialogTracker->getDialogEstablishedFlag() == true );
         mpDialogTracker->clearDialogEstablishedFlag();
         CPPUNIT_ASSERT( mpDialogTracker->getDialogEstablishedFlag() == false );
         mpDialogTracker->clearDialogEstablishedFlag();
         CPPUNIT_ASSERT( mpDialogTracker->getDialogEstablishedFlag() == false );
      }
      
      void TransactionDirectionalityFlagManipsTest()
      {
         mpDialogTracker->setTransactionDirectionality( DIR_CALLER_TO_CALLEE );
         CPPUNIT_ASSERT( mpDialogTracker->getTransactionDirectionality() == DIR_CALLER_TO_CALLEE );
         mpDialogTracker->setTransactionDirectionality( DIR_CALLEE_TO_CALLER );
         CPPUNIT_ASSERT( mpDialogTracker->getTransactionDirectionality() == DIR_CALLEE_TO_CALLER );
         mpDialogTracker->setTransactionDirectionality( DIR_CALLER_TO_CALLEE );
         CPPUNIT_ASSERT( mpDialogTracker->getTransactionDirectionality() == DIR_CALLER_TO_CALLEE );
      }
      
      void MediaRelayRequiredFlagManipsTest()
      {
         CPPUNIT_ASSERT( mpDialogTracker->getMediaRelayRequiredFlag() == false );
         mpDialogTracker->setMediaRelayRequiredFlag();
         CPPUNIT_ASSERT( mpDialogTracker->getMediaRelayRequiredFlag() == true );
         mpDialogTracker->setMediaRelayRequiredFlag();
         CPPUNIT_ASSERT( mpDialogTracker->getMediaRelayRequiredFlag() == true );
         mpDialogTracker->clearMediaRelayRequiredFlag();
         CPPUNIT_ASSERT( mpDialogTracker->getMediaRelayRequiredFlag() == false );
         mpDialogTracker->clearMediaRelayRequiredFlag();
         CPPUNIT_ASSERT( mpDialogTracker->getMediaRelayRequiredFlag() == false );
      }
      
      void MediaDescriptorsManipsTest()
      {
         const char *sdp = 
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12/127\r\n"
            "t=2873397496 2873404696\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"    // media description 0
            "c=IN IP4 224.2.17.12/127\r\n"
            "a=recvonly\r\n"
            "m=video 51372 RTP/AVP 31\r\n"   // media description 1
            "a=inactive\r\n"
            "m=application 32416 udp wb\r\n" // media description 2
            "a=sendonly\r\n"
            "m=audio 55554 RTP/AVP 0\r\n"    // media description 3
            "c=IN IP4 224.2.17.12/127\r\n"
            "a=sendrecv\r\n"
            "m=audio 55560 RTP/AVP 0\r\n"    // media description 4
            ;
         SdpBody body(sdp);

         MediaDescriptor* md0 = new  MediaDescriptor( body, 0, CALLER );
         MediaDescriptor* md1 = new  MediaDescriptor( body, 1, CALLER );
         MediaDescriptor* md2 = new  MediaDescriptor( body, 2, CALLER );
         MediaDescriptor* md3 = new  MediaDescriptor( body, 3, CALLER );
         MediaDescriptor* md4 = new  MediaDescriptor( body, 4, CALLER );

         CPPUNIT_ASSERT( mpDialogTracker->getModifiableMediaDescriptor( 0 ) == 0 );
         CPPUNIT_ASSERT( mpDialogTracker->getReadOnlyMediaDescriptor( 0 ) == 0 );

         CPPUNIT_ASSERT( mpDialogTracker->getNumberOfMediaDescriptors() == 0 );
         mpDialogTracker->appendMediaDescriptor( md0 );  
         CPPUNIT_ASSERT( mpDialogTracker->getNumberOfMediaDescriptors() == 1 );
         mpDialogTracker->appendMediaDescriptor( md1 );  
         CPPUNIT_ASSERT( mpDialogTracker->getNumberOfMediaDescriptors() == 2 );
         mpDialogTracker->appendMediaDescriptor( md2 );  
         CPPUNIT_ASSERT( mpDialogTracker->getNumberOfMediaDescriptors() == 3 );
         mpDialogTracker->appendMediaDescriptor( md3 );  
         CPPUNIT_ASSERT( mpDialogTracker->getNumberOfMediaDescriptors() == 4 );
         mpDialogTracker->appendMediaDescriptor( md4 );  
         CPPUNIT_ASSERT( mpDialogTracker->getNumberOfMediaDescriptors() == 5 );
         
         CPPUNIT_ASSERT( mpDialogTracker->getModifiableMediaDescriptor( 0 ) == md0 );
         CPPUNIT_ASSERT( mpDialogTracker->getReadOnlyMediaDescriptor( 0 ) == md0 );
         CPPUNIT_ASSERT( mpDialogTracker->getModifiableMediaDescriptor( 1 ) == md1 );
         CPPUNIT_ASSERT( mpDialogTracker->getReadOnlyMediaDescriptor( 1 ) == md1 );
         CPPUNIT_ASSERT( mpDialogTracker->getModifiableMediaDescriptor( 2 ) == md2 );
         CPPUNIT_ASSERT( mpDialogTracker->getReadOnlyMediaDescriptor( 2 ) == md2 );
         CPPUNIT_ASSERT( mpDialogTracker->getModifiableMediaDescriptor( 3 ) == md3 );
         CPPUNIT_ASSERT( mpDialogTracker->getReadOnlyMediaDescriptor( 3 ) == md3 );
         CPPUNIT_ASSERT( mpDialogTracker->getModifiableMediaDescriptor( 4 ) == md4 );
         CPPUNIT_ASSERT( mpDialogTracker->getReadOnlyMediaDescriptor( 4 ) == md4 );
         CPPUNIT_ASSERT( mpDialogTracker->getModifiableMediaDescriptor( 5 ) == 0 );
         CPPUNIT_ASSERT( mpDialogTracker->getReadOnlyMediaDescriptor( 5 ) == 0 );
      }

      void getOurMediaRelayHandleEncodedInSdpTest()
      {
         const char *sdp = 
            "v=0\r\n"
            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\r\n"
            "s=SDP Seminar\r\n"
            "i=A Seminar on the session description protocol\r\n"
            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\r\n"
            "e=mjh@isi.edu (Mark Handley)\r\n"
            "c=IN IP4 224.2.17.12/127\r\n"
            "t=2873397496 2873404696\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"       // media description 0
            "c=IN IP4 224.2.17.12/127\r\n"
            "a=recvonly\r\n"
            "a=x-sipx-ntap:192.168.0.2;1\r\n"   // <---- encoded handle
            "m=video 51372 RTP/AVP 31\r\n"      // media description 1
            "a=inactive\r\n"
            "a=x-sipx-ntap:192.168.0.2;2\r\n"   // <---- encoded handle
            "m=application 32416 udp wb\r\n"    // media description 2
            "a=sendonly\r\n"
            "a=x-sipx-ntap:192.168.222.2;2\r\n" // <---- encoded handle
            "m=audio 55554 RTP/AVP 0\r\n"       // media description 3
            "c=IN IP4 224.2.17.12/127\r\n"
            "a=sendrecv\r\n"
            "m=audio 55560 RTP/AVP 0\r\n"       // media description 4
            "a=x-sipx-ntap:192.168.0.2;3\r\n";  // <---- encoded handle
            SdpBody body(sdp);
            
            CPPUNIT_ASSERT( mpDialogTracker->getOurMediaRelayHandleEncodedInSdp( &body, 0 ) == 1 );
            CPPUNIT_ASSERT( mpDialogTracker->hasSdpAlreadyBeenPatchedByUs( &body, 0 ) == true );
            CPPUNIT_ASSERT( mpDialogTracker->getOurMediaRelayHandleEncodedInSdp( &body, 1 ) == 2 );
            CPPUNIT_ASSERT( mpDialogTracker->hasSdpAlreadyBeenPatchedByUs( &body, 1 ) == true );
            CPPUNIT_ASSERT( mpDialogTracker->getOurMediaRelayHandleEncodedInSdp( &body, 2 ) == INVALID_MEDIA_RELAY_HANDLE );
            CPPUNIT_ASSERT( mpDialogTracker->hasSdpAlreadyBeenPatchedByUs( &body, 2 ) == false );
            CPPUNIT_ASSERT( mpDialogTracker->getOurMediaRelayHandleEncodedInSdp( &body, 3 ) == INVALID_MEDIA_RELAY_HANDLE );
            CPPUNIT_ASSERT( mpDialogTracker->hasSdpAlreadyBeenPatchedByUs( &body, 3 ) == false );
            CPPUNIT_ASSERT( mpDialogTracker->getOurMediaRelayHandleEncodedInSdp( &body, 4 ) == 3 );
            CPPUNIT_ASSERT( mpDialogTracker->hasSdpAlreadyBeenPatchedByUs( &body, 4 ) == true );
      }

      void requestHandledMarkerManipTest()
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
            
         CPPUNIT_ASSERT( mpDialogTracker->isRequestAlreadyHandledByUs( testMsg ) == false );
         CPPUNIT_ASSERT( mpDialogTracker->isRequestAlreadyHandledByOther( testMsg ) == false );
         CPPUNIT_ASSERT( mpDialogTracker->isRequestAlreadyHandledByAnyone( testMsg ) == false );
         mpDialogTracker->markRequestAsHandledByUs( testMsg );
         CPPUNIT_ASSERT( mpDialogTracker->isRequestAlreadyHandledByUs( testMsg ) == true );
         CPPUNIT_ASSERT( mpDialogTracker->isRequestAlreadyHandledByOther( testMsg ) == false );
         CPPUNIT_ASSERT( mpDialogTracker->isRequestAlreadyHandledByAnyone( testMsg ) == true );

         const char* message2 =
            "INVITE sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.0.11:5060>\r\n"
            "Content-Length: 0\r\n"
            "X-Sipx-Handled: 192.168.0.2:222.222.222.222"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "\r\n";
         SipMessage testMsgHandledByOther(message2, strlen(message2) );

         CPPUNIT_ASSERT( mpDialogTracker->isRequestAlreadyHandledByUs( testMsgHandledByOther ) == false );
         CPPUNIT_ASSERT( mpDialogTracker->isRequestAlreadyHandledByOther( testMsgHandledByOther ) == true );
         CPPUNIT_ASSERT( mpDialogTracker->isRequestAlreadyHandledByAnyone( testMsgHandledByOther ) == true );
      }

      void RequestRetransmissionDescriptorTest()
      {
         DialogTracker::RequestRetransmissionDescriptor requestRetransmissionDescriptor;
         CPPUNIT_ASSERT( requestRetransmissionDescriptor.getCopyOfSdpBody() == 0 );
         
         const char* message =
            "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
            "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Supported: replaces\r\n"
            "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
            "Contact: <sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060>\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length: 297\r\n"
            "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
            "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
            "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=47.135.162.145;rport=14956\r\n"
            "\r\n"
            "\r\n"
            "v=0\r\n"
            "o=LGEIPP 874 874 IN IP4 192.168.1.101\r\n"
            "s=SIP Call\r\n"
            "c=IN IP4 192.168.1.101\r\n"
            "t=0 0\r\n"
            "m=audio 23000 RTP/AVP 18 0 8 4 101\r\n"
            "a=rtpmap:18 G729/8000\r\n"
            "a=rtpmap:0 PCMU/8000\r\n"
            "a=rtpmap:8 PCMA/8000\r\n"
            "a=rtpmap:4 G723/8000\r\n"
            "a=rtpmap:101 telephone-event/8000\r\n"
            "a=fmtp:18 annexb=no\r\n"
            "a=fmtp:101 0-11\r\n"
            "a=sendrecv\r\n";
         SipMessage* pReferenceMessage = new SipMessage(message, strlen(message) );
         const SdpBody* pSdpBody = pReferenceMessage->getSdpBody();
         
         requestRetransmissionDescriptor.setMessageToTrackRetransmissionsOf( message, *pSdpBody );
         CPPUNIT_ASSERT( requestRetransmissionDescriptor == *pReferenceMessage );

         // create message that does not match because of sequence number
         const char* nonMatchingMessage1 =
            "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 2 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Contact: <sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060>\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage nonMatchingSipMessage1(nonMatchingMessage1, strlen(nonMatchingMessage1) );
         CPPUNIT_ASSERT( requestRetransmissionDescriptor != nonMatchingSipMessage1 );         
               
         // create message that does not match because of method
         const char* nonMatchingMessage2 =
            "REFER sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 REFER\r\n"
            "Max-Forwards: 19\r\n"
            "Contact: <sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060>\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage nonMatchingSipMessage2(nonMatchingMessage2, strlen(nonMatchingMessage2) );
         CPPUNIT_ASSERT( requestRetransmissionDescriptor != nonMatchingSipMessage2 );         

         // create message that matches sequence number and method
         const char* matchingMessage =
            "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Contact: <sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060>\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage matchingSipMessage(matchingMessage, strlen(matchingMessage) );
         CPPUNIT_ASSERT( requestRetransmissionDescriptor == matchingSipMessage );         
         
         // Check that class has made a copy for the right SDP body.
         UtlString referenceSdpBodyText;
         ssize_t size;
         pSdpBody->getBytes( &referenceSdpBodyText, &size );
         delete pReferenceMessage;
         
         SdpBody *pSavedBody;
         pSavedBody = requestRetransmissionDescriptor.getCopyOfSdpBody();
         CPPUNIT_ASSERT( pSavedBody != 0 );

         UtlString savedSdpBodyText;
         pSavedBody->getBytes( &savedSdpBodyText, &size );
         ASSERT_STR_EQUAL( savedSdpBodyText.data(), referenceSdpBodyText.data() );
      }
      
      void ResponseRetransmissionDescriptorTest()
      {
         DialogTracker::ResponseRetransmissionDescriptor responseRetransmissionDescriptor;
         CPPUNIT_ASSERT( responseRetransmissionDescriptor.getCopyOfSdpBody() == 0 );
         
         const char* message =
         "SIP/2.0 200 OK\r\n"
         "From: \"R1_2 - 602\"<sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>;tag=94bf84d0-c0a8010b-13c4-3e8d1-496994c2-3e8d1\r\n"
         "Call-ID: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "CSeq: 1 INVITE\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;received=47.135.162.145;rport=14956;branch=z9hG4bK-3e635-f3b41fc-310ddca7\r\n"
         "Supported: replaces\r\n"
         "Contact: <sip:601@192.168.1.11:5060>\r\n"
         "Record-Route: <sip:47.135.162.140:5060;lr;sipXecs-rs=%2Afrom%7EOTRiYzI1YjgtYzBhODAxNjUtMTNjNC0zZTYzNS0zN2FhMTk4OS0zZTYzNQ%60%60.400_authrules%2Aauth%7E.900_ntap%2ACeT%7ENDcuMTM1LjE2Mi4xNDU6Mjk1NDQ7dHJhbnNwb3J0PXVkcA%60%60.900_ntap%2ACrT%7ENDcuMTM1LjE2Mi4xNDU6MTQ5NTY7dHJhbnNwb3J0PXVkcA%60%60.900_ntap%2Aid%7EMC0w%215d4c53f8d9c9ecd8d089e57ba7a59513>\r\n"
         "Content-Type: application/sdp\r\n"
         "Content-Length: 202\r\n"
         "\r\n"
         "\r\n"
         "v=0\r\n"
         "o=LGEIPP 5466 5466 IN IP4 192.168.1.11\r\n"
         "s=SIP Call\r\n"
         "c=IN IP4 192.168.1.11\r\n"
         "t=0 0\r\n"
         "m=audio 23020 RTP/AVP 0 101\r\n"
         "a=rtpmap:0 PCMU/8000\r\n"
         "a=rtpmap:101 telephone-event/8000\r\n"
         "a=fmtp:101 0-11\r\n"
         "a=sendrecv\r\n"
         "a=x-sip-ntap:192.168.0.2;1\r\n";
         SipMessage* pReferenceMessage = new SipMessage(message, strlen(message) );
         const SdpBody* pSdpBody = pReferenceMessage->getSdpBody();
         
         responseRetransmissionDescriptor.setMessageToTrackRetransmissionsOf( message, *pSdpBody );
         CPPUNIT_ASSERT( responseRetransmissionDescriptor == *pReferenceMessage );

         // create message that does not match because of sequence number
         const char* nonMatchingMessage1 =
            "SIP/2.0 200 OK\r\n"
            "From: \"R1_2 - 602\"<sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>;tag=94bf84d0-c0a8010b-13c4-3e8d1-496994c2-3e8d1\r\n"
            "Call-ID: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "CSeq: 2 INVITE\r\n"
            "Via: SIP/2.0/UDP 192.168.1.101:5060;received=47.135.162.145;rport=14956;branch=z9hG4bK-3e635-f3b41fc-310ddca7\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.1.11:5060>\r\n"
            "Record-Route: <sip:47.135.162.140:5060;lr;sipXecs-rs=%2Afrom%7EOTRiYzI1YjgtYzBhODAxNjUtMTNjNC0zZTYzNS0zN2FhMTk4OS0zZTYzNQ%60%60.400_authrules%2Aauth%7E.900_ntap%2ACeT%7ENDcuMTM1LjE2Mi4xNDU6Mjk1NDQ7dHJhbnNwb3J0PXVkcA%60%60.900_ntap%2ACrT%7ENDcuMTM1LjE2Mi4xNDU6MTQ5NTY7dHJhbnNwb3J0PXVkcA%60%60.900_ntap%2Aid%7EMC0w%215d4c53f8d9c9ecd8d089e57ba7a59513>\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage nonMatchingSipMessage1(nonMatchingMessage1, strlen(nonMatchingMessage1) );
         CPPUNIT_ASSERT( responseRetransmissionDescriptor != nonMatchingSipMessage1 );         

         // create message that does not match because of method
         const char* nonMatchingMessage2 =
            "SIP/2.0 200 OK\r\n"
            "From: \"R1_2 - 602\"<sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>;tag=94bf84d0-c0a8010b-13c4-3e8d1-496994c2-3e8d1\r\n"
            "Call-ID: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "CSeq: 1 REFER\r\n"
            "Via: SIP/2.0/UDP 192.168.1.101:5060;received=47.135.162.145;rport=14956;branch=z9hG4bK-3e635-f3b41fc-310ddca7\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.1.11:5060>\r\n"
            "Record-Route: <sip:47.135.162.140:5060;lr;sipXecs-rs=%2Afrom%7EOTRiYzI1YjgtYzBhODAxNjUtMTNjNC0zZTYzNS0zN2FhMTk4OS0zZTYzNQ%60%60.400_authrules%2Aauth%7E.900_ntap%2ACeT%7ENDcuMTM1LjE2Mi4xNDU6Mjk1NDQ7dHJhbnNwb3J0PXVkcA%60%60.900_ntap%2ACrT%7ENDcuMTM1LjE2Mi4xNDU6MTQ5NTY7dHJhbnNwb3J0PXVkcA%60%60.900_ntap%2Aid%7EMC0w%215d4c53f8d9c9ecd8d089e57ba7a59513>\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage nonMatchingSipMessage2(nonMatchingMessage2, strlen(nonMatchingMessage2) );
         CPPUNIT_ASSERT( responseRetransmissionDescriptor != nonMatchingSipMessage2 );         

         // create message that does not match because of response code
         const char* nonMatchingMessage3 =
            "SIP/2.0 202 Accepted\r\n"
            "From: \"R1_2 - 602\"<sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>;tag=94bf84d0-c0a8010b-13c4-3e8d1-496994c2-3e8d1\r\n"
            "Call-ID: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "CSeq: 1 INVITE\r\n"
            "Via: SIP/2.0/UDP 192.168.1.101:5060;received=47.135.162.145;rport=14956;branch=z9hG4bK-3e635-f3b41fc-310ddca7\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.1.11:5060>\r\n"
            "Record-Route: <sip:47.135.162.140:5060;lr;sipXecs-rs=%2Afrom%7EOTRiYzI1YjgtYzBhODAxNjUtMTNjNC0zZTYzNS0zN2FhMTk4OS0zZTYzNQ%60%60.400_authrules%2Aauth%7E.900_ntap%2ACeT%7ENDcuMTM1LjE2Mi4xNDU6Mjk1NDQ7dHJhbnNwb3J0PXVkcA%60%60.900_ntap%2ACrT%7ENDcuMTM1LjE2Mi4xNDU6MTQ5NTY7dHJhbnNwb3J0PXVkcA%60%60.900_ntap%2Aid%7EMC0w%215d4c53f8d9c9ecd8d089e57ba7a59513>\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage nonMatchingSipMessage3(nonMatchingMessage3, strlen(nonMatchingMessage3) );
         CPPUNIT_ASSERT( responseRetransmissionDescriptor != nonMatchingSipMessage3 );         
         
         // create message that matches sequence number and method
         const char* matchingMessage =
            "SIP/2.0 200 OK\r\n"
            "From: \"R1_2 - 602\"<sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>;tag=94bf84d0-c0a8010b-13c4-3e8d1-496994c2-3e8d1\r\n"
            "Call-ID: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "CSeq: 1 INVITE\r\n"
            "Via: SIP/2.0/UDP 192.168.1.101:5060;received=47.135.162.145;rport=14956;branch=z9hG4bK-3e635-f3b41fc-310ddca7\r\n"
            "Supported: replaces\r\n"
            "Contact: <sip:601@192.168.1.11:5060>\r\n"
            "Record-Route: <sip:47.135.162.140:5060;lr;sipXecs-rs=%2Afrom%7EOTRiYzI1YjgtYzBhODAxNjUtMTNjNC0zZTYzNS0zN2FhMTk4OS0zZTYzNQ%60%60.400_authrules%2Aauth%7E.900_ntap%2ACeT%7ENDcuMTM1LjE2Mi4xNDU6Mjk1NDQ7dHJhbnNwb3J0PXVkcA%60%60.900_ntap%2ACrT%7ENDcuMTM1LjE2Mi4xNDU6MTQ5NTY7dHJhbnNwb3J0PXVkcA%60%60.900_ntap%2Aid%7EMC0w%215d4c53f8d9c9ecd8d089e57ba7a59513>\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage matchingSipMessage(matchingMessage, strlen(matchingMessage) );
         CPPUNIT_ASSERT( responseRetransmissionDescriptor == matchingSipMessage );         
         
         // Check that class has made a copy for the right SDP body.
         UtlString referenceSdpBodyText;
         ssize_t size;
         pSdpBody->getBytes( &referenceSdpBodyText, &size );
         delete pReferenceMessage;
         
         SdpBody *pSavedBody;
         pSavedBody = responseRetransmissionDescriptor.getCopyOfSdpBody();
         CPPUNIT_ASSERT( pSavedBody != 0 );

         UtlString savedSdpBodyText;
         pSavedBody->getBytes( &savedSdpBodyText, &size );
         ASSERT_STR_EQUAL( savedSdpBodyText.data(), referenceSdpBodyText.data() );
      }
      
      void RemoveUnwantedElementsTest()
      {

         UtlString originalContactString, modifiedContactString;

         const char* message1 =
            "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Contact: <sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060>;dummy=no\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage sipMessage1(message1, strlen(message1) );
         sipMessage1.getContactEntry(0, &originalContactString);
         DialogTracker::removeUnwantedElements( sipMessage1 );
         sipMessage1.getContactEntry(0, &modifiedContactString);
         ASSERT_STR_EQUAL( originalContactString.data(), modifiedContactString.data() );

         const char* message2 =
            "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Contact: <sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060>;+sip.rendering=\"wantedvalue\";test\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage sipMessage2(message2, strlen(message2) );
         sipMessage2.getContactEntry(0, &originalContactString);
         DialogTracker::removeUnwantedElements( sipMessage2 );
         sipMessage2.getContactEntry(0, &modifiedContactString);
         ASSERT_STR_EQUAL( originalContactString.data(), modifiedContactString.data() );

         const char* message3 =
            "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Contact: <sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060>;+sip.rendering=\"no\";test\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage sipMessage3(message3, strlen(message3) );
         sipMessage3.getContactEntry(0, &originalContactString);
         DialogTracker::removeUnwantedElements( sipMessage3 );
         sipMessage3.getContactEntry(0, &modifiedContactString);
         ASSERT_STR_EQUAL( "<sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060>;test", modifiedContactString.data() );

         const char* message4 =
            "INVITE sip:601@47.135.162.145:29544;x-sipX-privcontact=192.168.1.11%3A5060 SIP/2.0\r\n"
            "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
            "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
            "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 19\r\n"
            "Contact: <sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060>;dummy;+sip.rendering=\"unknown\";test\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage sipMessage4(message4, strlen(message4) );
         sipMessage4.getContactEntry(0, &originalContactString);
         DialogTracker::removeUnwantedElements( sipMessage4 );
         sipMessage4.getContactEntry(0, &modifiedContactString);
         ASSERT_STR_EQUAL( "<sip:602@47.135.162.145:14956;x-sipX-privcontact=192.168.1.101%3A5060>;dummy;test", modifiedContactString.data() );
      }
   };

CPPUNIT_TEST_SUITE_REGISTRATION(DialogTrackerTest);

