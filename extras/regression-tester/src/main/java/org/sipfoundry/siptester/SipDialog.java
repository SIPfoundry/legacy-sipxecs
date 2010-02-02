/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */

package org.sipfoundry.siptester;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.message.RequestExt;
import gov.nist.javax.sip.message.ResponseExt;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.Collection;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentSkipListSet;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import javax.sdp.SessionDescription;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Message;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

public class SipDialog {
    private static Logger logger = Logger.getLogger(SipDialog.class);

    private String fromTag;
    private String toTag;
    private DialogExt dialog;
    private Response lastResponse;
    private RequestExt lastRequestReceived;

    Semaphore prackSem = new Semaphore(0);
    Semaphore ackSem = new Semaphore(0);

    Collection<SipClientTransaction> clientTransactions = new ConcurrentSkipListSet<SipClientTransaction>();

    Collection<SipServerTransaction> serverTransactions = new ConcurrentSkipListSet<SipServerTransaction>();

    SipDialog peerDialog;

    private DatagramSocket mediaSocket;
    private String remoteIpAddress;
    private int remotePort;
    static int packetReceivedCounter = 1;
    static int packetSentCounter = 1;

    private Hashtable<String, MediaListener> mediaListeners = new Hashtable<String, MediaListener>();
    private boolean isLocalEndOnHold;
    private MediaListener mediaListener;
    private Request lastRequestSent;

    class MediaListener implements Runnable {

        byte[] buffer = new byte[1024];
        boolean running = false;
        private int packetsReceived;

        public MediaListener(String ipAddress, int port) throws Exception {
            logger.debug("MediaListener : listening at " + ipAddress + ":" + port);
            InetAddress inetAddress = InetAddress.getByName(ipAddress);
            mediaSocket = new DatagramSocket(port, inetAddress);
        }

        public boolean isRunning() {
            return running;
        }

        @Override
        public void run() {
            running = true;
            while (true) {
                try {
                    DatagramPacket datagramPacket = new DatagramPacket(buffer, buffer.length);
                    mediaSocket.receive(datagramPacket);
                    logger.debug("got packet " + packetReceivedCounter++);
                    if (isLocalEndOnHold) {
                        SipTester.fail("Unexpected packet seen");
                    } else {
                        this.packetsReceived++;
                        String command = new String(buffer);
                        System.out.println("command " + command);
                        if (command.startsWith("ECHO_REQUEST")) {
                            byte[] response = "ECHO_RESPONSE".getBytes();
                            DatagramPacket responsePacket = new DatagramPacket(response,
                                    response.length);
                            InetAddress iaddr = InetAddress.getByName(remoteIpAddress);
                            responsePacket.setAddress(iaddr);
                            responsePacket.setPort(remotePort);
                            mediaSocket.send(responsePacket);
                            logger.debug("sending packet " + packetSentCounter++ + " to "
                                    + remoteIpAddress + ":" + remotePort);

                        }
                    }
                } catch (Exception ex) {
                    throw new SipTesterException(ex);
                }
            }

        }

        /**
         * @return the packetsReceived
         */
        public int getPacketsReceived() {
            return packetsReceived;
        }

    }

    public SipDialog() {

    }

    MediaListener createMediaListener(String ipAddress, int port) {
        String key = ipAddress + ":" + port;
        try {
            if (mediaListeners.containsKey(key)) {
                return mediaListeners.get(key);
            } else {
                MediaListener mediaListener = new MediaListener(ipAddress, port);
                mediaListeners.put(key, mediaListener);
                this.mediaListener = mediaListener;
                return mediaListener;
            }

        } catch (Exception ex) {
            SipTester.fail("Unexpected exception", ex);
            return null;
        }
    }

    public void addSipClientTransaction(SipClientTransaction sipClientTransaction) {
        this.clientTransactions.add(sipClientTransaction);
    }

    public void addSipServerTransaction(SipServerTransaction sipServerTransaction) {
        this.serverTransactions.add(sipServerTransaction);
        sipServerTransaction.setDialog(this);

    }

    /**
     * @param dialog the dialog to set
     */
    public void setDialog(DialogExt dialog) {
        if (dialog == null) {
            logger.debug("setDialog: setting dialog to null");
        }
        this.dialog = dialog;
    }

    /**
     * @return the dialog
     */
    public DialogExt getDialog() {
        return dialog;
    }

    public Response getLastResponse() {
        return this.lastResponse;
    }

    public void waitForOk() {
        try {
            boolean acquired = this.ackSem.tryAcquire(10, TimeUnit.SECONDS);
            if (!acquired) {
                SipTester.fail("Could not acuqire ACK semaphore");
            }
        } catch (Exception ex) {
            SipTester.fail("Unexpected exception ", ex);
        }
    }

    public void sendBytes() {
        try {
            byte[] buffer = "ECHO_REQUEST".getBytes();

            logger.debug("sending packet " + packetSentCounter++ + " to " + this.remoteIpAddress
                    + ":" + this.remotePort);
            DatagramPacket datagramPacket = new DatagramPacket(buffer, buffer.length);
            InetAddress iaddr = InetAddress.getByName(this.remoteIpAddress);
            datagramPacket.setAddress(iaddr);
            datagramPacket.setPort(remotePort);

            int currentCount = mediaListener.getPacketsReceived();

            this.mediaSocket.send(datagramPacket);

            Thread.sleep(100);
            if (mediaListener.getPacketsReceived() == currentCount) {
                SipTester.fail("Did not see expected packets");
            } else {
                System.out.println("Saw expected packets");
            }

        } catch (Exception ex) {
            SipTester.fail("Unexpected exception sending bytes", ex);
        }

    }

    public void setLastResponse(Response response) {
        this.lastResponse = response;
        if (response.getStatusCode() == Response.OK
                && SipUtilities.getCSeqMethod(response).equals(Request.INVITE)) {
            this.ackSem.release();
        }

        if (response.getContentLength().getContentLength() != 0) {
            SessionDescription sd = SipUtilities.getSessionDescription(response);
            this.remoteIpAddress = SipTester.getMappedAddress(SipUtilities
                    .getSessionDescriptionMediaIpAddress("audio", sd));
            this.remotePort = SipUtilities.getSessionDescriptionMediaPort("audio", sd);
        }

    }

    public void setLastRequestReceived(RequestExt request) {
        this.lastRequestReceived = request;
        if (request.getContentLength().getContentLength() != 0) {
            ContentTypeHeader cth = request.getContentTypeHeader();
            if (cth.getContentType().equals("application")
                    && cth.getContentSubType().equals("sdp")) {
                SessionDescription sd = SipUtilities.getSessionDescription(request);
                if (sd != null) {
                    this.remoteIpAddress = SipTester.getMappedAddress(SipUtilities
                            .getSessionDescriptionMediaIpAddress("audio", sd));
                    this.remotePort = SipUtilities.getSessionDescriptionMediaPort("audio", sd);
                }
            }
        }

        
        if (this.lastRequestReceived.getMethod().equals(Request.PRACK)) {
            this.prackSem.release();
        }
        if (request.getMethod().equals(Request.ACK)) {
            if (peerDialog != null  && !peerDialog.isLocalEndOnHold) {
                System.out.println("Remote end is NOT on hold " + peerDialog);
                sendBytes();
            }
            else if (peerDialog != null && peerDialog.isLocalEndOnHold ) {
                System.out.println("Remote end is on hold " + peerDialog);
            }

        }

    }

    public void waitForPrack() {
        try {
            boolean acquired = this.prackSem.tryAcquire(10, TimeUnit.SECONDS);
            if (!acquired) {
                SipTester.fail("Could not acquire PRACK semaphore " + this);
            }
        } catch (Exception ex) {
            SipTester.fail("Unexpected exception ", ex);
        }

    }

    public void setRequestToSend(Request request) {
        if (request.getContentLength().getContentLength() != 0) {
            ContentTypeHeader cth = ((RequestExt) request).getContentTypeHeader();
            if (cth.getContentType().equals("application")
                    && cth.getContentSubType().equals("sdp")) {
                SessionDescription sessionDescription = SipUtilities
                        .getSessionDescription(request);

                String ipAddress = SipUtilities.getSessionDescriptionMediaIpAddress("audio",
                        sessionDescription);
                int port = SipUtilities.getSessionDescriptionMediaPort("audio",
                        sessionDescription);
                MediaListener mediaListener = createMediaListener(ipAddress, port);
                this.isLocalEndOnHold = SipUtilities.isHoldRequest(sessionDescription);
                if (!mediaListener.isRunning()) {
                    new Thread(mediaListener).start();
                }

            }
        }
        System.out.println("setRequestToSend : " + this + " method = " + ((RequestExt) request).getMethod() + " isOnHold = " + isLocalEndOnHold);
        this.lastRequestSent = request;
    }

    public void setResponseToSend(ResponseExt response) {
        if (response.getCSeqHeader().getMethod().equals(Request.INVITE)) {
            SessionDescription sessionDescription = SipUtilities.getSessionDescription(response);
            String ipAddress = SipUtilities.getSessionDescriptionMediaIpAddress("audio",
                    sessionDescription);
            int port = SipUtilities.getSessionDescriptionMediaPort("audio", sessionDescription);
            MediaListener mediaListener = createMediaListener(ipAddress, port);
            this.isLocalEndOnHold = SipUtilities.isHoldRequest(sessionDescription);
            logger.debug("localEndOnHold " + this.isLocalEndOnHold);

            if (!mediaListener.isRunning()) {
                new Thread(mediaListener).start();
            }

        }
    }

    public void setPeerDialog(SipDialog sipDialog) {
        this.peerDialog = sipDialog;
    }

}
