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
import javax.sip.header.ViaHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

/**
 * Store information that is specific to a Dialog. This is a temporary holding place for dialog
 * specific data that is specific to the lifetime of a SIP Dialog. There is one of these
 * structures per dialog.
 * 
 * @author M. Ranganathan
 * 
 */
class DialogContext {

    private static Logger logger = Logger.getLogger(DialogContext.class);

    private PendingDialogAction pendingAction = PendingDialogAction.NONE;

    /*
     * Dialog associated with this application data.
     */
    private Dialog dialog;
    /*
     * The Peer Dialog of this Dialog.
     */
    Dialog peerDialog;

    /*
     * The request that originated the Dialog
     */
    Request request;

    /*
     * The transaction that created this DialogApplicationData
     */

    Transaction transaction;

    /*
     * The last response seen by the dialog.
     */
    Response lastResponse;

    /*
     * The B2BUA associated with the dialog. The BackToBackUserAgent structure tracks call state.
     */
    private BackToBackUserAgent backToBackUserAgent;

    /*
     * Account information for the associated dialog. There can be several ITSPs involved in a
     * single call ( each call leg can have its own ITSP).
     */
    private ItspAccountInfo itspInfo;

    /*
     * A flag that indicates whether this Dialog was created by sipxbridge and is managed by
     * sipXbridge.
     */
    boolean isOriginatedBySipxbridge;

    // /////////////////////////////////////////////////////////////
    // Auxilliary data structures associated with dialog state machine.
    // ////////////////////////////////////////////////////////////
    /*
     * Rtp session associated with this call leg.
     */

    RtpSession rtpSession;

    /*
     * Session timer associated with this call leg.
     */
    private SessionTimerTask sessionTimer;

    // //////////////////////////////////////////////////////////////
    // The following are state variables associated with the Dialog State
    // machine.
    // ///////////////////////////////////////////////////////////////

    
    /*
     * Used by the session timer - to compute whether or not to send a session timer re-INVITE.
     */
    long lastAckSent;

    /*
     * Session timer interval ( seconds ).
     */
    int sessionExpires;

    /*
     * Records whether or not an ACCEPTED has been sent for the REFER. This dictates what we need
     * to do when we see a BYE for this dialog.
     */
    boolean forwardByeToPeer = true;

    /*
     * The generated REFER request.
     */
    Request referRequest;

    // /////////////////////////////////////////////////////////////////
    // Inner classes.
    // ////////////////////////////////////////////////////////////////

    /**
     * The session timer task -- sends Re-INVITE to the associated dialog at specific intervals.
     */
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
                        && DialogContext.get(dialog).peerDialog != null) {
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
                        if ( getItspInfo() != null && ! getItspInfo().stripPrivateHeaders() ) {
                            SubjectHeader sh = ProtocolObjects.headerFactory
                                .createSubjectHeader("SipxBridge Session Timer");
                            request.setHeader(sh);
                        } else {
                            SipUtilities.stripPrivateHeaders(request);
                            
                        }

                        if (getItspInfo() == null || getItspInfo().isGlobalAddressingUsed()) {
                            SipUtilities.setGlobalAddresses(request);
                        }

                    } else {
                        /*
                         * This is never used but keep it here for now.
                         */
                        request = dialog.createRequest(Request.OPTIONS);
                    }

                    DialogExt dialogExt = (DialogExt) dialog;
                    ClientTransaction ctx = dialogExt.getSipProvider().getNewClientTransaction(
                            request);
                    TransactionContext.attach(ctx, Operation.SESSION_TIMER);

                    dialog.sendRequest(ctx);

                    DialogContext.this.sessionTimer = new SessionTimerTask(this.method);

                    int expiryTime = sessionExpires < Gateway.MIN_EXPIRES ? Gateway.MIN_EXPIRES
                            : sessionExpires;

                    Gateway.getTimer().schedule(sessionTimer,
                            expiryTime - Gateway.TIMER_ADVANCE * 1000);

                }

            } catch (Exception ex) {
                logger.error("Unexpected exception sending Session Timer INVITE", ex);
                this.cancel();

            }
        }
    }

    /*
     * Constructor.
     */
    private DialogContext(Dialog dialog) {
        this.sessionExpires = Gateway.getSessionExpires();
        this.dialog = dialog;
        // Kick off a task to test for session liveness.
        SipProvider provider = ((DialogExt) dialog).getSipProvider();
        if (Gateway.getSessionTimerMethod() != null && provider != Gateway.getLanProvider()) {
            this.sessionTimer = new SessionTimerTask(Gateway.getSessionTimerMethod());
            Gateway.getTimer().schedule(this.sessionTimer,
                    Gateway.SESSION_EXPIRES * 1000 - Gateway.TIMER_ADVANCE * 1000);
        }

    }
    
    /**
	 * Create a dialog to dialog association.
	 * 
	 * @param dialog1
	 *            - first dialog.
	 * @param dialog2
	 *            - second dialog.
	 * 
	 */
	static void pairDialogs(Dialog dialog1, Dialog dialog2) {
		logger.debug("pairDialogs dialogs = " + dialog1 + " " + dialog2);

		DialogContext dad1 = DialogContext.get(dialog1);
		DialogContext dad2 = DialogContext.get(dialog2);
		dad1.peerDialog = dialog2;
		dad2.peerDialog = dialog1;
	}

    static BackToBackUserAgent getBackToBackUserAgent(Dialog dialog) {
        if (dialog == null) {
            logger.debug("null dialog -- returning null ");
            return null;
        } else if (dialog.getApplicationData() == null) {
            logger.debug("null dialog application data -- returning null");
            return null;
        } else {
            return ((DialogContext) dialog.getApplicationData()).getBackToBackUserAgent();
        }
    }

    /**
     * Conveniance methods
     */
    static Dialog getPeerDialog(Dialog dialog) {
        return ((DialogContext) dialog.getApplicationData()).peerDialog;
    }

    public static RtpSession getPeerRtpSession(Dialog dialog) {
        return get(getPeerDialog(dialog)).rtpSession;

    }

    static RtpSession getRtpSession(Dialog dialog) {
        logger.debug("DialogApplicationData.getRtpSession " + dialog);

        return ((DialogContext) dialog.getApplicationData()).rtpSession;
    }

    static DialogContext attach(BackToBackUserAgent backToBackUserAgent, Dialog dialog,
            Transaction transaction, Request request) {
        if (backToBackUserAgent == null)
            throw new NullPointerException("Null back2back ua");
        if (dialog.getApplicationData() != null) {
            throw new SipXbridgeException("DialogContext: Context Already set!!");
        }
        DialogContext dat = new DialogContext(dialog);
        dat.transaction = transaction;
        dat.request = request;
        dat.setBackToBackUserAgent(backToBackUserAgent);
        dialog.setApplicationData(dat);

        return dat;
    }

    static DialogContext get(Dialog dialog) {
        return (DialogContext) dialog.getApplicationData();
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
        // set the back pointer to our set of active dialogs.

        this.backToBackUserAgent.addDialog(this.dialog);
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

    /**
     * Cancel the session timer.
     */
    void cancelSessionTimer() {
        if (this.sessionTimer != null) {
            this.sessionTimer.cancel();
        }
    }

    void setSetExpires(int expires) {
        this.sessionExpires = expires;

    }
    
    /**
     * Send ACK to the encapsulated dialog.
     * 
     * @throws Exception
     */
    void sendAck(SessionDescription sessionDescription ) throws Exception {
        if ( this.lastResponse == null ) {
            Gateway.logInternalError("Method was called with lastResponse null");
            
        }
        Request ackRequest = dialog.createAck(SipUtilities.getSeqNumber(this.lastResponse));
        this.lastResponse = null;
        this.recordLastAckTime();
        SipUtilities.setSessionDescription(ackRequest, sessionDescription);
        /*
         * Compensate for the quirks of some ITSPs which will play MOH.
         */
        SipUtilities.setDuplexity(sessionDescription, "sendrecv");
        dialog.sendAck(ackRequest);
    }

    /**
     * Check to see if the ITSP allows a REFER request.
     * 
     * @return true if REFER is allowed.
     * 
     * 
     */
    @SuppressWarnings("unchecked")
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
    
    
    /**
     * Send an INVITE with no SDP to the peer dialog. This solicits an SDP offer from the peer of
     * the given dialog.
     * 
     * @param requestEvent -- the request event for which we have to solicit the offer.
     * @param continuationData -- context information so we can process the continuation.
     * 
     * @return true if the offer is sent successfully. false if there is already an offer in
     *         progress and hence we should not send an offer.
     */
    boolean solicitSdpOfferFromPeerDialog(
            ContinuationData continuationData) throws Exception {
        try {
          
            Dialog peerDialog = DialogContext.getPeerDialog(dialog);
            /*
             * There is already a re-negotiation in progress so return silently
             */

            if (peerDialog != null && peerDialog.getState() != DialogState.TERMINATED ) {
                logger.debug("queryDialogFromPeer -- sending query to " + peerDialog);
                DialogContext peerDialogContext = DialogContext.get(peerDialog);
                /* Are we pending a query already ? If so need to retry later. ITSPs do take well
                 * to nested media negotiation attempts.
                 */
                if ( peerDialogContext.getPendingAction() == PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK) {
                    return false;
                }
                Request reInvite = peerDialog.createRequest(Request.INVITE);
                reInvite.removeHeader(SupportedHeader.NAME);
                SipUtilities.addWanAllowHeaders(reInvite);
                SipProvider provider = ((DialogExt) peerDialog).getSipProvider();
                ItspAccountInfo peerAccountInfo = DialogContext.getPeerDialogContext(dialog).getItspInfo();
                ViaHeader viaHeader = SipUtilities.createViaHeader(provider, 
                        peerAccountInfo);
                reInvite.setHeader(viaHeader);
                ContactHeader contactHeader = SipUtilities.createContactHeader(provider,
                        peerAccountInfo);
                /*
                 * Do not place a content type header in the content solicitation.
                 * 
                 * ContentTypeHeader cth = ProtocolObjects.headerFactory
                 * .createContentTypeHeader("application", "sdp"); reInvite.setHeader(cth);
                 */

                reInvite.setHeader(contactHeader);
                AcceptHeader acceptHeader = ProtocolObjects.headerFactory.createAcceptHeader(
                        "application", "sdp");
                reInvite.setHeader(acceptHeader);
                ClientTransaction ctx = provider.getNewClientTransaction(reInvite);
                TransactionContext tad = TransactionContext.attach(ctx,
                        Operation.SOLICIT_SDP_OFFER_FROM_PEER_DIALOG);
                /*
                 * Mark what we should do when we see the 200 OK response. This is what this
                 * dialog expects to see. Mark this as the pending operation for this dialog.
                 */
                DialogContext.get(peerDialog).setPendingAction(
                        PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK);

                /*
                 * The information we need to continue the operation when the Response comes in.
                 */
                tad.setContinuationData(continuationData);

                peerDialog.sendRequest(ctx);
               

            }
            return true;
        } catch (Exception ex) {
            logger.error("Exception occured. tearing down call! ", ex);
            this.backToBackUserAgent.tearDown();
            return true;
        }

    }

    public static RtpSession getPeerTransmitter(Dialog dialog) {
        return DialogContext.get(DialogContext.getPeerDialog(dialog)).rtpSession;
    }

    void setPendingAction(PendingDialogAction pendingAction) {
    	/*
    	 * A dialog can have only a single outstanding action.
    	 */
        if ( this.pendingAction != PendingDialogAction.NONE 
        	  && pendingAction != PendingDialogAction.NONE 
        	  && this.pendingAction != pendingAction) {
            	logger.error("Replacing pending action " + this.pendingAction + " with "
                    + pendingAction);
            	throw new SipXbridgeException("Pending dialog action is " + this.pendingAction);
        }
        this.pendingAction = pendingAction;
    }

    PendingDialogAction getPendingAction() {
        return pendingAction;
    }

    public static PendingDialogAction getPendingAction(Dialog dialog) {
        return DialogContext.get(dialog).pendingAction;
    }

    public static DialogContext getPeerDialogContext(Dialog dialog) {
        return DialogContext.get(DialogContext.getPeerDialog(dialog));
    }

}
