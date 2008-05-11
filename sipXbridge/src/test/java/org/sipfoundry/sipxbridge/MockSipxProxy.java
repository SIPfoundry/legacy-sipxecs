/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.SipStackImpl;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.HashSet;
import java.util.Properties;

import javax.sdp.MediaDescription;
import javax.sdp.Origin;
import javax.sdp.SdpFactory;
import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.ListeningPoint;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipFactory;
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.SipStack;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.address.AddressFactory;
import javax.sip.header.CSeqHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.HeaderFactory;
import javax.sip.message.MessageFactory;
import javax.sip.message.Request;
import javax.sip.message.Response;

import junit.framework.TestCase;

import org.apache.log4j.Logger;
import org.cafesip.sipunit.SipPhone;
import org.sipfoundry.sipxbridge.ProtocolObjects;
import org.sipfoundry.sipxbridge.ProxyAddressResolver;
import org.sipfoundry.sipxbridge.SipUtilities;

/**
 * This class mocks the SipxProxy server. It is strictly for unit testing.
 * 
 * @author mranga
 * 
 */
public class MockSipxProxy implements SipListener {

    public static final SipStack sipStack;
    public static String proxyAddress;
    public static String proxyDomain;
    public static int proxyPort;
    public static String gatewayAddress;
    public static int gatewayPort;
    public static int mediaPort = 7738; // some random num
    private static SipProvider provider;
    private boolean inviteOkSeen;
    private boolean sendDataFlag;
    private int readCount;
    private static String ibridgeDomain = "sipxpbx.example.com";

    private int bytesRead;
    private int writeCount;
    private org.cafesip.sipunit.SipStack phoneStack;
    private HashSet<SipPhone> sipPhones = new HashSet<SipPhone>();

    private static Logger logger = Logger.getLogger(MockSipxProxy.class);

    private static String sdpBody = "v=0\r\n"
            + "o=- 3408914179 3408914179 IN IP4 192.168.1.105\r\n"
            + "s=iBridge\r\n" + "c=IN IP4 192.168.1.105\r\n" + "t=0 0\r\n"
            + "a=direction:active\r\n"
            + "m=audio 49156 RTP/AVP 3 97 98 110 8 0 101\r\n"
            + "a=rtpmap:3 GSM/8000\r\n" + "a=rtpmap:97 iLBC/8000\r\n"
            + "a=rtpmap:98 iLBC/8000\r\n" + "a=fmtp:98 mode=20\r\n"
            + "a=rtpmap:110 speex/8000\r\n" + "a=rtpmap:8 PCMA/8000\r\n"
            + "a=rtpmap:0 PCMU/8000\r\n"
            + "a=rtpmap:101 telephone-event/8000\r\n"
            + "a=fmtp:101 0-11,16\r\n";

    private static Properties unittestProperties;

    static {
        try {
            Properties stackProperties = new Properties();
            stackProperties
                    .setProperty("javax.sip.STACK_NAME", "MockSipxProxy");
            stackProperties.setProperty("gov.nist.javax.sip.TRACE_LEVEL",
                    "DEBUG");
            stackProperties.setProperty("gov.nist.javax.sip.DEBUG_LOG",
                    "logs/debuglog.txt");
            stackProperties.setProperty(
                    "gov.nist.javax.sip.LOG_MESSAGE_CONTENT", "true");

            sipStack = ProtocolObjects.sipFactory
                    .createSipStack(stackProperties);
            ((SipStackImpl) sipStack)
                    .setAddressResolver(new ProxyAddressResolver());
            Properties properties = new Properties();
            String startupProperties = System.getProperties().getProperty(
                    "startupProperties") == null ? "gateway.properties"
                    : System.getProperties().getProperty("startupProperties");
            logger.debug("strtupProperties " + startupProperties);

            properties.load(new FileInputStream(new File(startupProperties)));
            proxyAddress = properties
                    .getProperty("org.sipfoundry.gateway.sipxProxyAddress");
            proxyPort = Integer.parseInt(properties
                    .getProperty("org.sipfoundry.gateway.sipxProxyPort"));
            String proxyTransport = properties
                    .getProperty("org.sipfoundry.gateway.sipxProxyTransport");
            gatewayAddress = properties
                    .getProperty("org.sipfoundry.gateway.ipAddress");
            gatewayPort = Integer.parseInt(properties
                    .getProperty("org.sipfoundry.gateway.localPort"));

            ListeningPoint lp = sipStack.createListeningPoint(proxyAddress,
                    proxyPort, proxyTransport);
            provider = sipStack.createSipProvider(lp);
            unittestProperties = new Properties();
            unittestProperties.load(new FileInputStream(new File(
                    "unittest.properties")));

        } catch (Exception ex) {
            throw new RuntimeException("Error loading factories", ex);
        }

    }

    class SocketReader implements Runnable {

        private DatagramSocket socketToRead;

        public SocketReader(DatagramSocket socketToRead) {
            this.socketToRead = socketToRead;
        }

        public void run() {
            byte[] buffer = new byte[8192];
            DatagramPacket datagramPacket = new DatagramPacket(buffer,
                    buffer.length);
            while (true) {
                try {
                    socketToRead.receive(datagramPacket);
                    bytesRead = bytesRead + datagramPacket.getLength();
                    readCount = readCount + 1;
                } catch (IOException e) {
                    TestCase.fail("Cannot read bytes");
                }
            }

        }

    }

    class SocketWriter implements Runnable {
        private DatagramSocket socketToWrite;

        public SocketWriter(DatagramSocket socketToWrite) {
            this.socketToWrite = socketToWrite;

        }

        public void run() {

            byte[] buffer = new byte[8192];
            DatagramPacket datagramPacket = new DatagramPacket(buffer,
                    buffer.length);

            for (int i = 0; i < writeCount; i++) {
                try {
                    socketToWrite.send(datagramPacket);
                } catch (IOException e) {
                    e.printStackTrace();
                    TestCase.fail("Unexpected exception sending");
                }
            }

        }
    }

    /**
     * The initializer.
     */
    public void init(int sendDataCount) throws Exception {
        provider.addSipListener(this);
        this.sendDataFlag = sendDataFlag;
        DatagramSocket datagramSocket = new DatagramSocket(this.mediaPort,
                InetAddress.getByName(proxyAddress));
        this.writeCount = sendDataCount;
        new Thread(new SocketReader(datagramSocket)).start();
    }

    private SessionDescription createSessionDescription() {
        try {
            SessionDescription sd = SdpFactory.getInstance()
                    .createSessionDescription(sdpBody);
            sd.getConnection().setAddress(
                    provider.getListeningPoint("udp").getIPAddress());
            MediaDescription md = (MediaDescription) sd.getMediaDescriptions(
                    true).get(0);
            md.getMedia().setMediaPort(mediaPort);
            Origin origin = sd.getOrigin();
            origin.setAddress(provider.getListeningPoint("udp").getIPAddress());
            return sd;

        } catch (Exception ex) {
            TestCase.fail("Cannot create session description");
            return null;
        }

    }

    public void processDialogTerminated(DialogTerminatedEvent dte) {

    }

    public void processIOException(IOExceptionEvent arg0) {
        TestCase.fail("IOException occured");

    }

    public void processRequest(RequestEvent requestEvent) {
        try {
            SipProvider provider = (SipProvider) requestEvent.getSource();
            Request request = requestEvent.getRequest();
            if (request.getMethod().equals(Request.INVITE)) {
                ServerTransaction st = requestEvent.getServerTransaction();
                if (st == null) {
                    st = provider.getNewServerTransaction(request);
                }
                Response response = SipFactories.messageFactory.createResponse(
                        Response.OK, request);
                SessionDescription sd = createSessionDescription();
                ContentTypeHeader cth = SipFactories.headerFactory
                        .createContentTypeHeader("application", "sdp");
                response.setContent(sd.toString(), cth);
                ContactHeader contactHeader = TestUtilities
                        .createContactHeader(provider);
                response.addHeader(contactHeader);
                st.sendResponse(response);

            }
        } catch (Exception ex) {
            ex.printStackTrace();
        }

    }

    public void sendInviteToIbridge() {
        try {
            String toUser = unittestProperties.getProperty("test.calledUser");
            String toDomain = ibridgeDomain;
            Request request = TestUtilities.createOutboundInviteRequest(
                    provider, toUser, toDomain, false);
            SessionDescription sd = createSessionDescription();
            ContentTypeHeader cth = SipFactories.headerFactory
                    .createContentTypeHeader("application", "sdp");

            request.setContent(sd.toString(), cth);

            ClientTransaction ct = provider.getNewClientTransaction(request);
            ct.sendRequest();
        } catch (Exception ex) {
            ex.printStackTrace();
            TestCase.fail("Could not send out INVITE");
        }

    }

    public void processResponse(ResponseEvent responseEvent) {
        try {
            Dialog dialog = responseEvent.getDialog();
            Response response = responseEvent.getResponse();
            CSeqHeader cseqHeader = (CSeqHeader) response
                    .getHeader(CSeqHeader.NAME);
            if (cseqHeader.getMethod().equals(Request.INVITE)) {
                if (response.getStatusCode() == Response.OK) {
                    logger.debug("MockSipxProxy: Saw INVITE OK");
                    this.inviteOkSeen = true;
                    long seqno = cseqHeader.getSeqNumber();
                    Request ack = dialog.createAck(seqno);
                    dialog.sendAck(ack);
                    SessionDescription sd = SipUtilities
                            .getSessionDescription(response);
                    String ipAddress = sd.getConnection().getAddress();
                    int port = ((MediaDescription) sd
                            .getMediaDescriptions(true).get(0)).getMedia()
                            .getMediaPort();
                    System.out.println("Sending to port " + port);
                    DatagramSocket socket = new DatagramSocket();
                    socket.connect(InetAddress.getByName(ipAddress), port);
                    if (this.writeCount > 0)
                        new Thread(new SocketWriter(socket)).start();

                }
            }

        } catch (Exception ex) {
            TestCase.fail("Exception processing response");
        }

    }

    public void processTimeout(TimeoutEvent arg0) {
        TestCase.fail("Unexpected timeout event occured");

    }

    public void processTransactionTerminated(TransactionTerminatedEvent arg0) {

    }

    public boolean isInviteOkSeen() {
        return this.inviteOkSeen;
    }

    /**
     * @return the readCount
     */
    public int getReadCount() {
        return readCount;
    }

    public void stop() {
        this.sipStack.stop();
    }

}
