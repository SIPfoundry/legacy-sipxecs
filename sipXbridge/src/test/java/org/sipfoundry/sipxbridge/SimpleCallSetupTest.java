/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.util.HashSet;
import java.util.Properties;

import javax.sdp.SdpFactory;
import javax.sdp.SessionDescription;
import javax.sip.message.Request;
import javax.sip.message.Response;

import junit.framework.TestCase;

import org.cafesip.sipunit.SipCall;
import org.cafesip.sipunit.SipPhone;
import org.cafesip.sipunit.SipRequest;
import org.cafesip.sipunit.SipStack;
import org.sipfoundry.sipxbridge.MockItsp.SocketReader;

public class SimpleCallSetupTest extends AbstractSipSignalingTest {

    private MockItsp mockItsp;
    private SipStack sipStack;
    private HashSet<SipPhone> caller = new HashSet<SipPhone>();

    private static String sdpBodyFormat = "v=0\r\n"
            + "o=- 3408914179 3408914179 IN IP4 %s\r\n" + "s=iBridge\r\n"
            + "c=IN IP4 %s\r\n" + "t=0 0\r\n" + "a=direction:active\r\n"
            + "m=audio %d RTP/AVP 3 97 98 110 8 0 101\r\n"
            + "a=rtpmap:3 GSM/8000\r\n" + "a=rtpmap:97 iLBC/8000\r\n"
            + "a=rtpmap:98 iLBC/8000\r\n" + "a=fmtp:98 mode=20\r\n"
            + "a=rtpmap:110 speex/8000\r\n" + "a=rtpmap:8 PCMA/8000\r\n"
            + "a=rtpmap:0 PCMU/8000\r\n"
            + "a=rtpmap:101 telephone-event/8000\r\n"
            + "a=fmtp:101 0-11,16\r\n";
    private SessionDescription createSessionDescription() {
        try {
            int mediaPort = this.getMediaPort();

            String sdpBody = String.format(sdpBodyFormat, "127.0.0.1",
                   "127.0.0.1", mediaPort);
            SessionDescription sd = SdpFactory.getInstance()
                    .createSessionDescription(sdpBody);

            return sd;

        } catch (Exception ex) {
            TestCase.fail("Cannot create session description");
            return null;
        }

    }
    class PhoneResponder implements Runnable {
        SipPhone sipPhone;
        SocketReader socketReader;
        SessionDescription remoteSessionDescription;

        public PhoneResponder(SipPhone sipPhone) {
            this.sipPhone = sipPhone;
        }

        public void run() {
            try {
                sipPhone.listenRequestMessage();
                SipCall sipCall = sipPhone.createSipCall();

                sipCall.waitForIncomingCall(3000);

                SipRequest sipRequest = sipCall.getLastReceivedRequest();
                Request request = (Request) sipRequest.getMessage();
                remoteSessionDescription = SipUtilities
                        .getSessionDescription(request);
                SessionDescription sd = createSessionDescription();
                int port = SipUtilities.getSessionDescriptionMediaPort(sd);

                sipCall.sendIncomingCallResponse(Response.OK, "OK", 3000, sd
                        .toString(), "application", "sdp", null, null);
                sipCall.waitForAck(3000);

                sipCall.listenForDisconnect();
                sipCall.waitForDisconnect(3000);
                sipCall.respondToDisconnect();


            } catch (Exception ex) {
                ex.printStackTrace();
                AbstractSipSignalingTest.fail("Unexpected exception ");
            }

        }

    }

    public void testRegistration() throws Exception {
        for (ItspAccountInfo itspAccount : Gateway.getAccountManager()
                .getItspAccounts()) {
            if (itspAccount.getState() != AccountState.AUTHENTICATED) {
                fail("Could not REGISTER " + itspAccount + " state = " + itspAccount.getState());
            }
        }
    }


    public void testOutboundCallCalledPartySendsBye() throws Exception {
   	 try {
            this.mockItsp.createPhones(1,0,0,false);
            String user = "1112223330";
            String to = "sip:" + user + "@" + accountInfo.getProxyDomain();
            int myPort = getMediaPort();
            String sdpBody = String.format(sdpBodyFormat, Gateway
                    .getLocalAddress(), Gateway.getLocalAddress(), myPort);

            System.out.println("sdp = " + sdpBody);

            SipPhone phone = sipStack.createSipPhone(localAddr, "udp",
                    localPort, "sip:mranga@pingtel.com");
            this.caller.add(phone);
            SipCall sipCall = phone.makeCall(to, Response.OK, 2000, null,
                    sdpBody, "application", "sdp", null, null);
            super.assertLastOperationSuccess("Expect an OK" + sipCall.format(),
                    sipCall);
            boolean res = sipCall.sendInviteOkAck();
            assertTrue ("Successful ack sent " + sipCall.getErrorMessage() , res);
            assertNotNull("Null sipCall ", sipCall);

            sipCall.listenForDisconnect();

            sipCall.waitForDisconnect(3000);

            sipCall.respondToDisconnect();
            // Wait for the dialog terminated events to occur.
            Thread.sleep(8000);
            sipCall.dispose();

        } catch (Exception ex) {
            ex.printStackTrace();
            fail("Unexpected exception occured");

        }
   }
  public void testDialBadItsp() {
        try {
            String to = "sip:3015551212@" + accountInfo.getProxyDomain();
            int myPort = getMediaPort();
            String sdpBody = String.format(sdpBodyFormat, Gateway
                    .getLocalAddress(), Gateway.getLocalAddress(), myPort);
            SipPhone phone = sipStack.createSipPhone(localAddr, "udp",
                    localPort, "sip:mranga@pingtel.com");
            this.caller.add(phone);

            SipCall sipCall = phone.makeCall(to, Response.NOT_FOUND, 1000, null,
                    sdpBody, "application", "sdp", null, null);
            super.assertNotNull("Should create a sip call", sipCall);
            //sipCall.disconnect();
            sipCall.dispose();
            Thread.sleep(8000);


        } catch (Exception ex) {
            ex.printStackTrace();
            fail("Unexpected exception occured");

        }

    }


  public void testSendInviteFromSipxProxy() {
      try {
          this.mockItsp.createPhones(1,0,0,true);
          String user = "1112223330";
          String to = "sip:" + user + "@" + accountInfo.getProxyDomain();
          int myPort = getMediaPort();
          String sdpBody = String.format(sdpBodyFormat, Gateway
                  .getLocalAddress(), Gateway.getLocalAddress(), myPort);

          System.out.println("sdp = " + sdpBody);

          SipPhone phone = sipStack.createSipPhone(localAddr, "udp",
                  localPort, "sip:mranga@pingtel.com");
          this.caller.add(phone);
          SipCall sipCall = phone.makeCall(to, Response.OK, 2000, null,
                  sdpBody, "application", "sdp", null, null);
          super.assertLastOperationSuccess("Expect an OK" + sipCall.format(),
                  sipCall);
          boolean res = sipCall.sendInviteOkAck();
          assertTrue ("Successful ack sent " + sipCall.getErrorMessage() , res);
          assertNotNull("Null sipCall ", sipCall);
          sipCall.disconnect();
          assertTrue ("Successful bye sent " + sipCall.getErrorMessage() , res);
          // Wait for the dialog terminated events to occur.
          Thread.sleep(8000);
          sipCall.dispose();



      } catch (Exception ex) {
          ex.printStackTrace();
          fail("Unexpected exception occured");

      }

  }
    public void testMultiInviteFromSipxProxy() {
        try {
            this.mockItsp.createPhones(3,0,0,true);
            for (int i = 0; i < 3; i++) {
                String user = "111222333" + i;
                String to = "sip:" + user + "@" + accountInfo.getProxyDomain();
                String sdpBody = String.format(sdpBodyFormat, Gateway
                        .getLocalAddress(), Gateway.getLocalAddress(),
                        getMediaPort());

                SipPhone phone = sipStack.createSipPhone(localAddr, "udp",
                        localPort, "sip:mranga@pingtel.com");
                this.caller.add(phone);
                SipCall sipCall = phone.makeCall(to, Response.OK, 2000, null,
                        sdpBody, "application", "sdp", null, null);

                super.assertLastOperationSuccess("Expect an OK" + sipCall.format(),
                        sipCall);
                boolean res = sipCall.sendInviteOkAck();
                assertTrue ("Successful ack sent " + sipCall.getErrorMessage() , res);
                assertNotNull("Null sipCall ", sipCall);

                assertNotNull("Null sipCall ", sipCall);
                super.assertLastOperationSuccess("Expect an OK"
                        + sipCall.format(), sipCall);
                sipCall.disconnect();
                assertTrue ("Successful bye sent " + sipCall.getErrorMessage() , res);


            }
            // Wait for the dialog terminated events to occur.
            Thread.sleep(8000);
        } catch (Exception ex) {
            ex.printStackTrace();
            fail("Unexpected exception occured " + ex.getMessage());
        }
    }

    public void testInboundCallFromItsp()  throws Exception {
        SipPhone sipPhone = sipStack.createSipPhone("127.0.0.1", "udp", this.accountManager.getBridgeConfiguration().getLocalPort(),
                "sip:"+
                this.accountManager.getBridgeConfiguration().getAutoAttendantName()+
                "@"+this.accountManager.getBridgeConfiguration().getSipxProxyDomain());
        PhoneResponder responder = new PhoneResponder (sipPhone);
        new Thread(responder).start();
        mockItsp.makePhoneInboundCall();
        Thread.sleep(4000);

    }




    @Override
    public void tearDown() throws Exception {
    	System.out.println("TearDown!");
        mockItsp.stop();
        sipStack.dispose();
        SipXbridgeXmlRpcServerImpl.stopXmlRpcServer();
        Gateway.stop();
        for ( SipPhone phone : this.caller) {
            phone.dispose();
        }


    }

    @Override
    public void setUp() throws Exception {
        super.setUp();

        Properties properties = new Properties();
        properties.setProperty("javax.sip.STACK_NAME", "sipx-phone-emulator");
        properties.setProperty("javax.sip.IP_ADDRESS", "127.0.0.1");
        sipStack = new SipStack("udp", sipxProxyPort,properties);
        SipStack.setTraceEnabled(true);

        System.out.println("localAddr = " + localAddr);

        this.mockItsp = new MockItsp(this);

        this.mockItsp.init(1000);
        /* Must start the mock itsp first */
        Gateway.start();
		Thread.sleep(1000);
		System.out.println("sipxbridge started");

    }

}
