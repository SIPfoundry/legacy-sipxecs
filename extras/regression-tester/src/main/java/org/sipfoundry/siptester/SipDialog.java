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

import gov.nist.core.InternalErrorHandler;
import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.message.RequestExt;
import gov.nist.javax.sip.message.ResponseExt;
import gov.nist.javax.sip.message.SIPRequest;
import gov.nist.javax.sip.message.SIPResponse;
import gov.nist.javax.sip.stack.SIPDialog;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketTimeoutException;
import java.text.ParseException;
import java.util.Collection;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentSkipListSet;
import java.util.concurrent.FutureTask;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import javax.sdp.SessionDescription;
import javax.sip.address.Address;
import javax.sip.header.CallIdHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Message;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

public class SipDialog {
    private static Logger logger = Logger.getLogger(SipDialog.class);

    private String localTag;
    private String remoteTag;
    private DialogExt dialog;
    private Response lastResponse;
    private RequestExt lastRequestReceived;
    
    private String dialogIdSetAtStackTrace;
    
    private HashSet<String> dialogIds = new HashSet<String>();

    Collection<SipClientTransaction> clientTransactions = new ConcurrentSkipListSet<SipClientTransaction>();

    Collection<SipServerTransaction> serverTransactions = new ConcurrentSkipListSet<SipServerTransaction>();

    SipDialog peerDialog;

    private String remoteIpAddress;
    private int remotePort;
    static int packetReceivedCounter = 1;
    static int packetSentCounter = 1;

    private static Hashtable<String, MediaListener> mediaListeners = new Hashtable<String, MediaListener>();
    private MediaListener mediaListener;
    private Request lastRequestSent;

    private Address localParty;

    private Address remoteParty;

    private EmulatedEndpoint endpoint;

    private ResponseExt lastResponseSent;

  
    class PacketChecker extends TimerTask {
        MediaListener mediaListener;
        int packets;
        public PacketChecker(MediaListener mediaListener) {
            System.out.println("Expect ECHO request");
            this.mediaListener = mediaListener;
            this.packets = mediaListener.packetsReceived;
        }
        @Override
        public void run() {
           if ( mediaListener.packetsReceived == packets && ! mediaListener.isLocalEndOnHold) {
               System.out.println("Did not see expected packets @ " + mediaListener.ipAddress + ":" + mediaListener.port);
           }
        }
    }
    class MediaListener implements Runnable {

        byte[] buffer = new byte[1024];
        boolean running = false;
        private int packetsReceived;
        private DatagramSocket mediaSocket;
        private String ipAddress;
        private int port;
        private String remoteIpAddress;
        private int remotePort;
        public boolean isLocalEndOnHold;
        
       
        public MediaListener(String ipAddress, int port) throws Exception {
            logger.debug("MediaListener : listening at " + ipAddress + ":" + port);
            InetAddress inetAddress = InetAddress.getByName(ipAddress);
            this.ipAddress = ipAddress;
            this.port = port;
            this.remoteIpAddress = SipDialog.this.remoteIpAddress;
            this.remotePort = SipDialog.this.remotePort;
            mediaSocket = new DatagramSocket(port, inetAddress);
        }
        
        public DatagramSocket getMediaSocket() {
            return mediaSocket;
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
                        System.out.println("Unexpected media packet received from "
                                + datagramPacket.getAddress() + ":" +datagramPacket.getPort());
                        System.out.println("Packet seen at " + ipAddress + ":" + port);
                        System.out.println("Trace Endpoint " + 
                                SipDialog.this.endpoint.getTraceEndpoint().getTraceIpAddresses());
                        System.out.println("Last request received " + lastRequestReceived);
                        System.out.println("Last response sent " + lastResponseSent);
                        System.out.println("Last request sent " + lastRequestSent);
                        
                        SipTester.fail("Unexpected packets seen");
                    } else {
                        this.packetsReceived++;
                        String command = new String(buffer);
                        System.out.println( command + " @ " + ipAddress + ":" + port);
                        if (command.startsWith("ECHO_REQUEST")) {
                            
                            byte[] response = "ECHO_RESPONSE".getBytes();
                            DatagramPacket responsePacket = new DatagramPacket(response,
                                    response.length);
                            InetAddress iaddr = InetAddress.getByName(remoteIpAddress);
                            responsePacket.setAddress(iaddr);
                            responsePacket.setPort(remotePort);
                             logger.debug("sending packet " + packetSentCounter++ + " to "
                                    + remoteIpAddress + ":" + remotePort);
                             mediaSocket.send(responsePacket);
                             

                        }
                    }
                } catch (Exception ex) {
                    logger.error("Exception occured while processing ECHO_REQUEST ");
                    logger.error("Exception sending packet to " + remoteIpAddress + ":" + remotePort,ex);
                    logger.error("LastResponse = " + lastResponse);
                    logger.error("LastRequest " + lastRequestReceived);
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

        public void setRemoteIpAddress(String remoteIpAddress) {
           this.remoteIpAddress = remoteIpAddress;
        }

        public void setRemotePort(int remotePort) {
           this.remotePort = remotePort;
        }

        public void expectPacketIn(int miliseconds) {
            SipTester.timer.schedule(new PacketChecker(this), miliseconds);
        }

      
    }

    public SipDialog(String dialogId, EmulatedEndpoint endpoint) {
        logger.debug("creating sipDialog " + dialogId);
        this.dialogIds.add(dialogId);
        this.endpoint = endpoint;

    }

    MediaListener createMediaListener(String ipAddress, int port) {
        String key = ipAddress + ":" + port;
        logger.debug("createMediaListener : " + this + " ipAddress:port = " + ipAddress + ":" + port);
        try {
            if (mediaListeners.containsKey(key)) {
                this.mediaListener = mediaListeners.get(key);
                return mediaListeners.get(key);
            } else {
                MediaListener mediaListener = new MediaListener(ipAddress, port);
                mediaListeners.put(key, mediaListener);
                this.mediaListener = mediaListener;   
                return mediaListener;
            }

        } catch (Exception ex) {
            logger.error("Problem creating media listener",ex);
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

    

    public void sendBytes() {
        try {
            byte[] buffer = "ECHO_REQUEST".getBytes();
            if ( remoteIpAddress == null || remotePort == 0 ) {
                return;
            }

            logger.debug("sending packet " + packetSentCounter++ + " to " + this.remoteIpAddress
                    + ":" + this.remotePort);
            DatagramPacket datagramPacket = new DatagramPacket(buffer, buffer.length);
            InetAddress iaddr = InetAddress.getByName(this.remoteIpAddress);
            datagramPacket.setAddress(iaddr);
            datagramPacket.setPort(remotePort);

            int currentCount = mediaListener.getPacketsReceived();

            mediaListener.getMediaSocket().send(datagramPacket);

            Thread.sleep(100);
            if (mediaListener.getPacketsReceived() == currentCount ) {
                logger.debug("No Echo RESPONSE seen.");
            }

        } catch (Exception ex) {
            logger.error("Exception occured while sending bytes on " + this);
            SipTester.fail("Unexpected exception sending bytes", ex);
        }

    }

    public void setLastResponse(Response response) {
        this.lastResponse = response;
        this.localTag = ((SIPResponse)response).getFrom().getTag();
        if ( this.remoteTag == null ) {
            this.remoteTag = ((SIPResponse)response).getTo().getTag();
        }
        this.localParty = ((SIPResponse) response).getFrom().getAddress();
        this.remoteParty = ((SIPResponse) response).getTo().getAddress();
        
        /*
         * Actual tag assigned so put a pointer to this from the actual dialog id.
         */
        if ( this.remoteTag != null ) {
            String dialogId = ((SIPResponse) response).getDialogId(false);
            this.addDialogId(dialogId);
        }
        
    
       
        if (response.getContentLength().getContentLength() != 0) {
            SessionDescription sd = SipUtilities.getSessionDescription(response);
            this.remoteIpAddress = SipTester.getMappedAddress(SipUtilities
                    .getSessionDescriptionMediaIpAddress("audio", sd));
            this.remotePort = SipUtilities.getSessionDescriptionMediaPort("audio", sd);
            if ( this.mediaListener != null ) {
                mediaListener.setRemoteIpAddress(remoteIpAddress);
                mediaListener.setRemotePort(remotePort);
            }
        }

    }

    private void addDialogId(String dialogId) {
        this.dialogIds.add(dialogId); 
        if ( this.dialogIds.size() > 2) {
            logger.error("last dialogId set at " + this.dialogIdSetAtStackTrace);
            logger.error("Error occured at : " + SipUtilities.getStackTrace());
            SipTester.fail("Internal inconsistency on dialog " + this);
        }
        this.dialogIdSetAtStackTrace = SipUtilities.getStackTrace();
        
    }

    public void setLastRequestReceived(RequestExt request) {
        this.lastRequestReceived = request;
        boolean tagSet = false;
        if (this.localTag == null)  {
              this.localTag = request.getToHeader().getTag();
              tagSet = true;
        }
        if ( this.remoteTag == null ) {
            this.remoteTag = request.getFromHeader().getTag();
            tagSet = true;
        }
        
        if ( request.getFromHeader().getTag() != null && request.getToHeader().getTag() != null  && tagSet) {      
            String dialogId = ((SIPRequest) request).getDialogId(true);
           
            this.addDialogId(dialogId);
            logger.debug("dialogIds = " + this.dialogIds);
        }
        
        
        if (request.getContentLength().getContentLength() != 0) {
            ContentTypeHeader cth = request.getContentTypeHeader();
            if (cth.getContentType().equals("application")
                    && cth.getContentSubType().equals("sdp")) {
                SessionDescription sd = SipUtilities.getSessionDescription(request);
                if (sd != null) {
                    this.remoteIpAddress = SipTester.getMappedAddress(SipUtilities
                            .getSessionDescriptionMediaIpAddress("audio", sd));
                    this.remotePort = SipUtilities.getSessionDescriptionMediaPort("audio", sd);
                    if ( mediaListener != null ) {
                        mediaListener.setRemoteIpAddress(remoteIpAddress);
                        mediaListener.setRemotePort(remotePort);
                    }
                }
            }
        }

        
        if (request.getMethod().equals(Request.ACK) && this.remoteIpAddress != null  && 
                this.remotePort != 0) {  
               try {
                   Thread.sleep(100);
               } catch (Exception ex) {}
               sendBytes();
        }

    }

    
    
    private void updateRequest(Request request) {
        try {
            FromHeader from = ((RequestExt) request).getFromHeader();
            if (this.localTag != null) {
                from.setTag(localTag);
            } 

            ToHeader to = ((RequestExt) request).getToHeader();
            if (this.remoteTag != null) {
                to.setTag(this.remoteTag);
            }
        } catch (ParseException ex) {
            InternalErrorHandler.handleException(ex);
        }
    }
    
    public void setRequestToSend(Request request) {
        this.updateRequest(request);
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
                this.mediaListener = createMediaListener(ipAddress, port);
                mediaListener.isLocalEndOnHold = SipUtilities.isHoldRequest(sessionDescription);
                if (mediaListener != null && !mediaListener.isRunning()) {
                    new Thread(mediaListener).start();
                }
               
            }
        }
        if (request.getMethod().equals(Request.ACK) && !mediaListener.isLocalEndOnHold ) {
            this.mediaListener.expectPacketIn(300);
        } else if (request.getMethod().equals(Request.ACK)){
           logger.debug("LocalEndOnHold");
        }

        if ( mediaListener != null ) {
        	logger.debug("setRequestToSend : " + this + " method = " + ((RequestExt) request).getMethod() + " isOnHold = " + mediaListener.isLocalEndOnHold);
        }
        this.lastRequestSent = request;
    }

    public void setResponseToSend(ResponseExt response) {
        this.lastResponseSent = response;
        if ( response.getFromHeader().getTag() != null && response.getToHeader().getTag() != null ) {
            this.localTag = response.getToHeader().getTag();
            String dialogId = ((SIPResponse) response).getDialogId(true);
            this.addDialogId(dialogId);
        }
        if (response.getCSeqHeader().getMethod().equals(Request.INVITE) && 
                response.getContentLength().getContentLength() != 0) {
            SessionDescription sessionDescription = SipUtilities.getSessionDescription(response);
            String ipAddress = SipUtilities.getSessionDescriptionMediaIpAddress("audio",
                    sessionDescription);
            int port = SipUtilities.getSessionDescriptionMediaPort("audio", sessionDescription);
            this.mediaListener = createMediaListener(ipAddress, port);
            mediaListener.isLocalEndOnHold = SipUtilities.isHoldRequest(sessionDescription);
            logger.debug("localEndOnHold " + mediaListener.isLocalEndOnHold);

            if (mediaListener != null && !mediaListener.isRunning()) {
                new Thread(mediaListener).start();
            }
        }
    }

    public void setPeerDialog(SipDialog sipDialog) {
        this.peerDialog = sipDialog;
    }

    public Collection<String> getDialogIds() {
        return this.dialogIds;
    }

}
