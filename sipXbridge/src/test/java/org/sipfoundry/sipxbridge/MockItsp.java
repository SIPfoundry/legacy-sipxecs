/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.SipStackImpl;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.util.Properties;

import javax.sdp.MediaDescription;
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
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.SipStack;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionAlreadyExistsException;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.header.CSeqHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import junit.framework.TestCase;

import org.apache.log4j.Logger;

/**
 * Test ITSP. This is endpoint that mocks ITSP behavior.
 * 
 * @author M. Ranganathan
 * 
 */
public class MockItsp implements SipListener {

    private static SipStack sipStack;
    private SipProvider provider;
    private static AccountManagerImpl accountManager;
    private static final int mediaPort = 3664;
    private String myIpAddress;
    private int myPort;

    private Dialog dialog;
    boolean inviteSeen = false;
    private boolean inviteOkSeen;
    private boolean ackSeen;
    int bytesRead;
    int readCount;
    private int writeCount;
    private SessionDescription remoteSessionDescripion;

    private static Logger logger = Logger.getLogger(MockItsp.class);

    private static String sdpBody = "v=0\r\n"
            + "o=alice 2890844526 2890844526 IN IP4 host.atlanta.example.com\r\n"
            + "s=\r\n" + "c=IN IP4 host.atlanta.example.com\r\n" + "t=0 0\r\n"
            + "m=audio 49170 RTP/AVP 0 8 97\r\n" + "a=rtpmap:0 PCMU/8000\r\n";

    static  {
        try {
            Properties stackProperties = new Properties();
            stackProperties.setProperty("javax.sip.STACK_NAME", "MockITSP");
            stackProperties.setProperty("gov.nist.javax.sip.TRACE_LEVEL",
                    "DEBUG");
            stackProperties.setProperty("gov.nist.javax.sip.DEBUG_LOG",
                    "logs/itsp-debuglog.txt");
            stackProperties.setProperty(
                    "gov.nist.javax.sip.LOG_MESSAGE_CONTENT", "true");

            sipStack = ProtocolObjects.sipFactory
                    .createSipStack(stackProperties);
            ((SipStackImpl) sipStack)
                    .setAddressResolver(new ProxyAddressResolver());

          
            accountManager = Gateway.getAccountManager();

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
                    readCount++;
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

    private SessionDescription createSessionDescription() {
        try {
            SessionDescription sd = SdpFactory.getInstance()
                    .createSessionDescription(sdpBody);
            sd.getConnection().setAddress(
                    provider.getListeningPoint("udp").getIPAddress());
            MediaDescription md = (MediaDescription) sd.getMediaDescriptions(
                    true).get(0);
            md.getMedia().setMediaPort(mediaPort);
            sd.getOrigin().setAddress(
                    provider.getListeningPoint("udp").getIPAddress());
            return sd;

        } catch (Exception ex) {
            TestCase.fail("Cannot create session description");
            return null;
        }

    }

    public void processDialogTerminated(DialogTerminatedEvent dte) {
        // TODO Auto-generated method stub

    }

    public void processIOException(IOExceptionEvent ioex) {
        // TODO Auto-generated method stub

    }

    /*
     * (non-Javadoc)
     * 
     * @see javax.sip.SipListener#processRequest(javax.sip.RequestEvent)
     */

    public void processRequest(RequestEvent requestEvent) {
        try {
            Request request = requestEvent.getRequest();
            if (request.getMethod().equals(Request.INVITE)) {
                this.inviteSeen = true;
                ServerTransaction st = requestEvent.getServerTransaction();
                if (st == null) {
                    try {
                        st = provider.getNewServerTransaction(request);
                    } catch (TransactionAlreadyExistsException ex) {
                        // Ignore.
                    }
                }
                this.remoteSessionDescripion = SipUtilities
                        .getSessionDescription(request);

                // For now we are only responding with OK.
                // Other responses such BUSY are to be emulated.
                Response response = SipFactories.messageFactory.createResponse(
                        Response.OK, request);

                SessionDescription sd = createSessionDescription();
                ContentTypeHeader cth = SipFactories.headerFactory
                        .createContentTypeHeader("application", "sdp");
                dialog = st.getDialog();
                response.setContent(sd.toString(), cth);
                ContactHeader contactHeader = TestUtilities
                        .createContactHeader(provider);
                response.setHeader(contactHeader);
                st.sendResponse(response);
            } else if (request.getMethod().equals(Request.ACK)) {
                this.ackSeen = true;

                if (this.writeCount > 0) {
                    String ipAddress = this.remoteSessionDescripion
                            .getConnection().getAddress();
                    int port = ((MediaDescription) this.remoteSessionDescripion
                            .getMediaDescriptions(true).get(0)).getMedia()
                            .getMediaPort();
                    DatagramSocket datagramSocket = new DatagramSocket();
                    datagramSocket.connect(new InetSocketAddress(InetAddress
                            .getByName(ipAddress), port));
                    new Thread(new SocketWriter(datagramSocket)).start();
                }
            } else if (request.getMethod().equals(Request.REGISTER)) {
                ServerTransaction st = provider
                        .getNewServerTransaction(request);
                // TODO -- deal with challenge and response here.
                Response response = SipFactories.messageFactory.createResponse(
                        Response.OK, request);
                st.sendResponse(response);

            } 
        } catch (Exception ex) {
            logger.error("Unexpected exception", ex);
            ex.printStackTrace();
            TestCase.fail("Unexpected exception");
        }

    }

    public void processResponse(ResponseEvent responseEvent) {
        try {

            Response response = responseEvent.getResponse();
            CSeqHeader cseqHeader = (CSeqHeader) response
                    .getHeader(CSeqHeader.NAME);
            if (cseqHeader.getMethod().equals(Request.INVITE)) {
                if (response.getStatusCode() == Response.OK) {
                    this.inviteOkSeen = true;
                    long seqno = cseqHeader.getSeqNumber();
                    Request ack = dialog.createAck(seqno);
                    dialog.sendAck(ack);
                }
            }
        } catch (Exception ex) {
            TestCase.fail("Exception processing response");
        }

    }

    public void processTimeout(TimeoutEvent arg0) {
        // TODO Auto-generated method stub

    }

    public void processTransactionTerminated(TransactionTerminatedEvent arg0) {
        // TODO Auto-generated method stub

    }

    public void init(int bytesToSend) throws Exception {
        
        ItspAccountInfo accountInfo = accountManager.getDefaultAccount();
        myIpAddress = accountInfo.getOutboundProxy();
        myPort = accountInfo.getProxyPort();
        
        System.out.println("myIpAddress = " + myIpAddress + " myPort " + myPort);
        ListeningPoint listeningPoint = sipStack.createListeningPoint(
                myIpAddress, myPort, "udp");
        provider = sipStack.createSipProvider(listeningPoint);

        provider.addSipListener(this);
        this.writeCount = bytesToSend;
        DatagramSocket socket = new DatagramSocket(this.mediaPort, InetAddress
                .getByName(this.myIpAddress));
        new Thread(new SocketReader(socket)).start();

    }

    public int getReadCount() {
        return this.readCount;
    }

    public boolean isInviteSeen() {
        return inviteSeen;
    }

    public boolean isInviteOkSeen() {
        return this.inviteOkSeen;
    }

    public boolean isAckSeen() {
        return this.ackSeen;
    }

    public void sendBye() throws Exception {
        Request request = this.dialog.createRequest(Request.BYE);
        ClientTransaction ct = this.provider.getNewClientTransaction(request);
        dialog.sendRequest(ct);

    }

    public void stop() {
        this.sipStack.stop();
    }

}
