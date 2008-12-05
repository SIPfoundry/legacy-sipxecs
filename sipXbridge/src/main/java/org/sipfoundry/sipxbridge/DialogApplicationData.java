/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.header.HeaderFactoryExt;
import gov.nist.javax.sip.header.extensions.MinSE;
import gov.nist.javax.sip.header.extensions.SessionExpiresHeader;

import java.util.ListIterator;
import java.util.TimerTask;

import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.ServerTransaction;
import javax.sip.SipProvider;
import javax.sip.Transaction;
import javax.sip.header.AcceptHeader;
import javax.sip.header.AllowHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.SubjectHeader;
import javax.sip.header.SupportedHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

/**
 * Store information that is specific to a Dialog. This is a temporary holding place for dialog
 * specific data.
 * 
 * @author M. Ranganathan
 * 
 */
class DialogApplicationData {

    private static Logger logger = Logger.getLogger(DialogApplicationData.class);

    /*
     * The Peer Dialog of this Dialog.
     */
    Dialog peerDialog;

    /*
     * The last response seen by the dialog.
     */
    Response lastResponse;

    /*
     * The B2BUA associated with the dialog.
     */
    private BackToBackUserAgent backToBackUserAgent;

    /*
     * Rtp session associated with this call leg.
     */

    RtpSession rtpSession;

    /*
     * Flag to indicate that an SDP answer is pending.
     */
    boolean isSdpAnswerPending;

    /*
     * Account information for the outbound dialog.
     */
    private ItspAccountInfo itspInfo;

    /*
     * A flag that indicates whether this Dialog was created by sipxbridge.
     */
    boolean isOriginatedBySipxbridge;

    /*
     * The request that originated the Dialog
     */
    Request request;

    String sessionDescription;

    boolean sendReInviteOnResume;

    Transaction transaction;

    boolean mohCodecNegotiationFailed;

    boolean isSdpOfferPending;

    long lastAckSent; // Session timer - records when last ACK was sent for this dialog.

    int sessionExpires;

    private Dialog dialog;

    private SessionTimerTask sessionTimer;

    boolean isReferAccepted;

    Request referRequest;

    // /////////////////////////////////////////////////////////////////
    // Inner classes.
    // ////////////////////////////////////////////////////////////////

    class SessionTimerTask extends TimerTask {

        String method;

        public SessionTimerTask(String method) {
            this.method = method;

        }

        @Override
        public void run() {
            if (dialog.getState() == DialogState.TERMINATED) {
                this.cancel();
            }

            try {
                Request request;
                long currentTimeMilis = System.currentTimeMillis();
                if (dialog.getState() == DialogState.CONFIRMED
                        && DialogApplicationData.get(dialog).peerDialog != null) {
                    if (method.equalsIgnoreCase(Request.INVITE)) {

                        if (currentTimeMilis < lastAckSent - sessionExpires * 1000) {
                            return;
                        }

                        SipProvider provider = ((DialogExt) dialog).getSipProvider();
                        RtpSession rtpSession = getRtpSession();
                        if (rtpSession == null || rtpSession.getReceiver() == null) {
                            return;
                        }
                        SessionDescription sd = rtpSession.getReceiver().getSessionDescription();

                        request = dialog.createRequest(Request.INVITE);
                        request.removeHeader(AllowHeader.NAME);
                        SipUtilities.addWanAllowHeaders(request);
                        AcceptHeader accept = ProtocolObjects.headerFactory.createAcceptHeader(
                                "application", "sdp");
                        request.setHeader(accept);
                        request.removeHeader(SupportedHeader.NAME);

                        request.setContent(sd.toString(), ProtocolObjects.headerFactory
                                .createContentTypeHeader("application", "sdp"));
                        ContactHeader cth = SipUtilities.createContactHeader(provider,
                                getItspInfo());
                        request.setHeader(cth);
                        SessionExpiresHeader sexp = ((HeaderFactoryExt) ProtocolObjects.headerFactory)
                                .createSessionExpiresHeader(Gateway.getSessionExpires());
                        request.setHeader(sexp);
                        MinSE minSe = new MinSE();
                        minSe.setExpires(Gateway.getSessionExpires());
                        request.setHeader(minSe);
                        SubjectHeader sh = ProtocolObjects.headerFactory
                                .createSubjectHeader("SipxBridge Session Timer");
                        request.setHeader(sh);

                        if (getItspInfo() == null || getItspInfo().isGlobalAddressingUsed()) {
                            SipUtilities.setGlobalAddresses(request);
                        }

                    } else {
                        request = dialog.createRequest(Request.OPTIONS);
                    }

                    DialogExt dialogExt = (DialogExt) dialog;
                    ClientTransaction ctx = dialogExt.getSipProvider().getNewClientTransaction(
                            request);
                    TransactionApplicationData.attach(ctx, Operation.SESSION_TIMER);

                    dialog.sendRequest(ctx);

                    DialogApplicationData.this.sessionTimer = new SessionTimerTask(this.method);
                    
                    int expiryTime = sessionExpires <  Gateway.MIN_EXPIRES ? Gateway.MIN_EXPIRES : sessionExpires ;
                    
                    Gateway.getTimer().schedule(sessionTimer, expiryTime - Gateway.TIMER_ADVANCE*1000);

                }

            } catch (Exception ex) {
                logger.error("Unexpected exception sending Session Timer INVITE", ex);
                this.cancel();

            }
        }
    }

    private DialogApplicationData(Dialog dialog) {
        this.sessionExpires = Gateway.getSessionExpires();
        this.dialog = dialog;
        // Kick off a task to test for session liveness.
        SipProvider provider = ((DialogExt) dialog).getSipProvider();
        if (Gateway.getSessionTimerMethod() != null && provider != Gateway.getLanProvider()) {
            this.sessionTimer = new SessionTimerTask(Gateway.getSessionTimerMethod());
            Gateway.getTimer().schedule(this.sessionTimer, Gateway.SESSION_EXPIRES * 1000 - Gateway.TIMER_ADVANCE*1000);
        }

    }

    static BackToBackUserAgent getBackToBackUserAgent(Dialog dialog) {
        if (dialog == null) {
            logger.debug("null dialog -- returning null ");
            return null;
        } else if (dialog.getApplicationData() == null) {
            logger.debug("null dialog application data -- returning null");
            return null;
        } else {
            return ((DialogApplicationData) dialog.getApplicationData()).getBackToBackUserAgent();
        }
    }

    /**
     * Conveniance methods
     */
    static Dialog getPeerDialog(Dialog dialog) {
        return ((DialogApplicationData) dialog.getApplicationData()).peerDialog;
    }

    static RtpSession getRtpSession(Dialog dialog) {
        logger.debug("DialogApplicationData.getRtpSession " + dialog);

        return ((DialogApplicationData) dialog.getApplicationData()).rtpSession;
    }

    static DialogApplicationData attach(BackToBackUserAgent backToBackUserAgent, Dialog dialog,
            Transaction transaction, Request request) {
        if (backToBackUserAgent == null)
            throw new NullPointerException("Null back2back ua");
        if (dialog.getApplicationData() != null)
            throw new RuntimeException("Already set!!");
       
        DialogApplicationData dat = new DialogApplicationData(dialog);
        dat.transaction = transaction;
        dat.request = request;
        dat.setBackToBackUserAgent(backToBackUserAgent);
        dialog.setApplicationData(dat);

        return dat;
    }

    static DialogApplicationData get(Dialog dialog) {
        return (DialogApplicationData) dialog.getApplicationData();
    }

    /**
     * @param rtpSession the rtpSession to set
     */
    void setRtpSession(RtpSession rtpSession) {  
        this.rtpSession = rtpSession;
    }

    /**
     * @return the rtpSession
     */
    RtpSession getRtpSession() {
        return rtpSession;
    }

    void recordLastAckTime() {
        this.lastAckSent = System.currentTimeMillis();
    }

    /**
     * @param backToBackUserAgent the backToBackUserAgent to set
     */
    void setBackToBackUserAgent(BackToBackUserAgent backToBackUserAgent) {
        this.backToBackUserAgent = backToBackUserAgent;
    }

    /**
     * @return the backToBackUserAgent
     */
    BackToBackUserAgent getBackToBackUserAgent() {
        return backToBackUserAgent;
    }

    /**
     * @param itspInfo the itspInfo to set
     */
    void setItspInfo(ItspAccountInfo itspInfo) {
        if (this.itspInfo != null) {
            logger.warn("Re-Setting ITSP info to null!!");
        }
        this.itspInfo = itspInfo;
    }

    /**
     * @return the itspInfo
     */
    ItspAccountInfo getItspInfo() {
        return itspInfo;
    }

    void cancelSessionTimer() {
        if (this.sessionTimer != null) {
            this.sessionTimer.cancel();
        }
    }

    void setSetExpires(int expires) {
        this.sessionExpires = expires;

    }

    /**
     * Check to see if the ITSP allows a REFER request.
     * 
     * @return true if REFER is allowed.
     * 
     * 
     */
    boolean isReferAllowed() {
        if (this.transaction instanceof ServerTransaction) {
            if (this.request == null) {
                return false;
            }
            ListIterator li = request.getHeaders(AllowHeader.NAME);

            while (li != null && li.hasNext()) {
                AllowHeader ah = (AllowHeader) li.next();
                if (ah.getMethod().equals(Request.REFER)) {
                    return true;
                }
            }
            return false;

        } else {
            if (this.lastResponse == null) {
                return false;
            }

            ListIterator li = lastResponse.getHeaders(AllowHeader.NAME);

            while (li != null && li.hasNext()) {
                AllowHeader ah = (AllowHeader) li.next();
                if (ah.getMethod().equals(Request.REFER)) {
                    return true;
                }
            }
            return false;
        }
    }

}
