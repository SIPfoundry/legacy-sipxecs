/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.header.ims.PAssertedIdentityHeader;

import java.io.IOException;

import javax.sdp.SdpParseException;
import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.ServerTransaction;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.header.AcceptHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.message.Request;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxbridge.symmitron.SymImpl;
import org.sipfoundry.sipxbridge.symmitron.SymInterface;

class RtpSession {

    private static Logger logger = Logger.getLogger(RtpSession.class);

    private SymImpl symImpl;

    private RtpReceiverEndpoint rtpReceiverEndpoint;

    private RtpTransmitterEndpoint rtpTransmitterEndpoint;
    
    private int referenceCount;

    public RtpSession(SymImpl symImpl) {
        this.symImpl = symImpl;
        this.rtpReceiverEndpoint = new RtpReceiverEndpoint(symImpl.getReceiver());
    }
    
    public void incrementReferenceCount() {
        this.referenceCount ++;
    }
    
    public void decrementReferenceCount() {
        this.referenceCount --;
    }
    
    public int getReferenceCount() {
        return this.referenceCount;
    }

    public SymImpl getSym() {
        return this.symImpl;
    }

    public String getId() {
        return symImpl.getId();
    }

    public RtpReceiverEndpoint getReceiver() {
        return this.rtpReceiverEndpoint;
    }

    public RtpTransmitterEndpoint getTransmitter() {
        return rtpTransmitterEndpoint;
    }

    protected void setTransmitter(RtpTransmitterEndpoint endpoint) {
        this.rtpTransmitterEndpoint = endpoint;
        symImpl.setTransmitter(endpoint.getSymTransmitter());
    }

    protected boolean isHoldRequest(Request request) throws SdpParseException {
        SessionDescription sessionDescription = SipUtilities.getSessionDescription(request);
        int oldPort = this.getTransmitter().getPort();
        String oldIpAddress = this.getTransmitter().getIpAddress();

        int newport = SipUtilities.getSessionDescriptionMediaPort(sessionDescription);
        String newIpAddress = SipUtilities
                .getSessionDescriptionMediaIpAddress(sessionDescription);

        /*
         * Get the a media attribute -- CAUTION - this only takes care of the first media.
         * Question - what to do when only one media stream is put on hold?
         */

        String mediaAttribute = SipUtilities
                .getSessionDescriptionMediaAttributeDuplexity(sessionDescription);

        String sessionAttribute = SipUtilities.getSessionDescriptionAttribute(sessionDescription);

        if (logger.isDebugEnabled()) {
            logger.debug("mediaAttribute = " + mediaAttribute);
            logger.debug("sessionAttribute = " + sessionAttribute);
        }
        /*
         * RFC2543 specified that placing a user on hold was accomplished by setting the
         * connection address to 0.0.0.0. This has been deprecated, since it doesn't allow for
         * RTCP to be used with held streams, and breaks with connection oriented media. However,
         * a UA MUST be capable of receiving SDP with a connection address of 0.0.0.0, in which
         * case it means that neither RTP nor RTCP should be sent to the peer. Whenever the phone
         * puts an external call on hold, it sends a re-INVITE to the gateway with "a=sendonly".
         * Normally, the gateway would respond with "a=recvonly".
         */
        String attribute = sessionAttribute != null ? sessionAttribute : mediaAttribute;
        if (newIpAddress.equals("0.0.0.0") && newport == oldPort) {
            return true;
        } else if (newport == oldPort && oldIpAddress.equals(newIpAddress) && attribute != null
                && (attribute.equals("sendonly") || attribute.equals("inactive"))) {
            return true;
        }
        return false;
    }

    
    /**
     * Re-invite the ITSP.
     * 
     * @param sd
     * @param dialog
     * @throws Exception
     */
    void forwardReInvite(ServerTransaction serverTransaction, Dialog dialog) throws Exception {
        SessionDescription sd = this.getReceiver().getSessionDescription();
        logger.debug("sendMohReInviteToItsp" + sd);
        SipUtilities.setDuplexity(sd,"sendrecv");
     
       
        DialogApplicationData dat = (DialogApplicationData) dialog.getApplicationData();
        BackToBackUserAgent b2bua = dat.getBackToBackUserAgent();
        Dialog peerDialog = DialogApplicationData.getPeerDialog(dialog);
        DialogApplicationData peerDat = DialogApplicationData.get(peerDialog);

        b2bua.getWanRtpSession(peerDialog).getReceiver().setSessionDescription(sd);
        SipUtilities.incrementSessionVersion(sd);
     
        peerDat.sessionDescription = sd.toString();

        Request newInvite = peerDialog.createRequest(Request.INVITE);
        
        SipProvider sipProvider = ((DialogExt) peerDialog).getSipProvider();
        
        /*
         * Sending request to ITSP - make the addressing global if required.
         */
        if ( sipProvider != Gateway.getLanProvider()) {
            if ( peerDat.itspInfo == null || peerDat.itspInfo.isGlobalAddressingUsed()) {
                SipUtilities.setGlobalAddresses(newInvite);
            }
            
        }
 
        newInvite.removeHeader(PAssertedIdentityHeader.NAME);
        AcceptHeader acceptHeader = ProtocolObjects.headerFactory.createAcceptHeader(
                "application", "sdp");
        newInvite.setHeader(acceptHeader);
        ContentTypeHeader cth = ProtocolObjects.headerFactory.createContentTypeHeader(
                "application", "sdp");
        newInvite.setContent(sd.toString(), cth);

        ClientTransaction ctx = ((DialogExt) peerDialog).getSipProvider()
                .getNewClientTransaction(newInvite);
        TransactionApplicationData tad = new TransactionApplicationData(
                Operation.FORWARD_REINVITE);
        
        
        tad.serverTransaction = serverTransaction;
       
        tad.serverTransactionProvider = ((DialogExt) dialog).getSipProvider();
        ctx.setApplicationData(tad);
        ctx.sendRequest();
    }
    
    
    
    /**
     * Remove the music on hold.
     * 
     * @param dialog
     */
    void removeHold(ServerTransaction serverTransaction, Dialog dialog) {
        try {
            logger.debug("Remove media on hold!");
            SipUtilities.setDuplexity(this.getReceiver().getSessionDescription(), "sendrecv");
            SipUtilities.incrementSessionVersion(this.getReceiver().getSessionDescription());
            if ( this.getTransmitter() != null ) {
                this.getTransmitter().setOnHold(false);
            }

            DialogApplicationData dat = (DialogApplicationData) dialog.getApplicationData();
            BackToBackUserAgent b2bua = dat.getBackToBackUserAgent();
            
            if (dat.musicOnHoldDialog != null
                    && dat.musicOnHoldDialog.getState() != DialogState.TERMINATED) {
                   b2bua.sendByeToMohServer(dat.musicOnHoldDialog);
            }
            
            SessionDescription sd = this.getReceiver().getSessionDescription();
            
            //SipUtilities.setDuplexity(sd,"sendrecv");
            //SipUtilities.incrementSessionVersion(sd);
               
            SipProvider provider = ((DialogExt) dialog).getSipProvider();
            
            if ( Gateway.isReInviteSupported() && dat.musicOnHoldDialog != null  ) {
                // Send Re-INVITE to the ITSP.
                this.forwardReInvite(serverTransaction, dialog);
            } else if ( provider != Gateway.getLanProvider() ) {
                // RE-INVITE was received from WAN side. Just forward it.
                this.forwardReInvite(serverTransaction, dialog);
            }
        } catch (Exception ex) {
            throw new RuntimeException(ex);
        }
    }

    
    /**
     * Put the session on hold.
     * 
     * @param dialog
     * @param peerDialog
     * @throws SipException
     */
    void putOnHold(Dialog dialog, Dialog peerDialog) throws SipException {

        if (logger.isDebugEnabled()) {
            logger.debug("setting media on hold " + this.toString());
        }
        this.getTransmitter().setOnHold(true);
        if (Gateway.getMusicOnHoldAddress() != null) {
             /*
             * For the standard MOH, the URI is defined to be <sip:~~mh~@[domain]>. There
             * is thought that other URIs in the ~~mh~ series can be allocated
             */
            ClientTransaction mohCtx;
            try {
                mohCtx = DialogApplicationData.getBackToBackUserAgent(dialog)
                        .createClientTxToMohServer(
                                (SessionDescription) this.getReceiver()
                                        .getSessionDescription().clone());
            } catch (CloneNotSupportedException e) {
                throw new RuntimeException("Unexpected exception ", e);
            }
            DialogApplicationData dat = (DialogApplicationData) dialog
                    .getApplicationData();
            Dialog mohDialog = mohCtx.getDialog();
            dat.musicOnHoldDialog = mohDialog;
            DialogApplicationData mohDat = DialogApplicationData.get(mohDialog);
            mohDat.peerDialog = peerDialog;
            mohCtx.sendRequest();

        }
        SipUtilities.setDuplexity(this.getReceiver().getSessionDescription(), "recvonly");
        SipUtilities.incrementSessionVersion(this.getReceiver().getSessionDescription());
    }
    
    
    
    /**
     * Reassign the session parameters ( possibly putting the media on hold and playing music ).
     * 
     * @param sessionDescription
     * @param dat -- the dialog application data
     * @return -- the recomputed session description.
     */
    SessionDescription reAssignSessionParameters(Request request, ServerTransaction serverTransaction, Dialog dialog, Dialog peerDialog)
            throws SdpParseException, SipException {
        try {
            if (peerDialog != null) {
                logger.debug("reAssignSessionParameters: peerDialog = " + peerDialog
                        + " peerDialogApplicationData = " + peerDialog.getApplicationData()
                        + "\n lastResponse = "
                        + DialogApplicationData.get(peerDialog).lastResponse);
            }
            SessionDescription sessionDescription = SipUtilities.getSessionDescription(request);

            int newport = SipUtilities.getSessionDescriptionMediaPort(sessionDescription);
            String newIpAddress = SipUtilities
                    .getSessionDescriptionMediaIpAddress(sessionDescription);

            /*
             * Get the a media attribute -- CAUTION - this only takes care of the first media.
             * Question - what to do when only one media stream is put on hold?
             */

            String mediaAttribute = SipUtilities
                    .getSessionDescriptionMediaAttributeDuplexity(sessionDescription);

            String sessionAttribute = SipUtilities
                    .getSessionDescriptionAttribute(sessionDescription);

            if (logger.isDebugEnabled()) {
                logger.debug("mediaAttribute = " + mediaAttribute);
                logger.debug("sessionAttribute = " + sessionAttribute);
            }

            String attribute = sessionAttribute != null ? sessionAttribute : mediaAttribute;

            if (this.isHoldRequest(request)) {
                this.putOnHold(dialog,peerDialog);
                return this.getReceiver().getSessionDescription();
            } else if (attribute == null || attribute.equals("sendrecv")) {
                this.removeHold(serverTransaction,dialog);
                return this.getReceiver().getSessionDescription();
            } else {
                SessionDescription retval = this.getReceiver().getSessionDescription();
                this.getTransmitter().setIpAddressAndPort(newIpAddress, newport);
                return retval;
            }

        } catch (IOException ex) {
            throw new SipException("Exception occured while connecting", ex);
        }

    }
}
