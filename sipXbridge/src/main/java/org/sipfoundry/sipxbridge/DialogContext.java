/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.ServerTransactionExt;
import gov.nist.javax.sip.SipProviderExt;
import gov.nist.javax.sip.header.HeaderFactoryExt;
import gov.nist.javax.sip.header.extensions.MinSE;
import gov.nist.javax.sip.header.extensions.ReferencesHeader;
import gov.nist.javax.sip.header.extensions.SessionExpiresHeader;
import gov.nist.javax.sip.header.ims.PAssertedIdentityHeader;
import gov.nist.javax.sip.header.ims.PPreferredIdentityHeader;
import gov.nist.javax.sip.message.SIPResponse;

import java.text.ParseException;
import java.util.HashMap;
import java.util.ListIterator;
import java.util.Random;
import java.util.TimerTask;

import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.ResponseEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.Transaction;
import javax.sip.address.Address;
import javax.sip.header.AcceptHeader;
import javax.sip.header.AllowHeader;
import javax.sip.header.CSeqHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ProxyAuthorizationHeader;
import javax.sip.header.SupportedHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Message;
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

    private static HashMap<String,DialogContext> dialogContextTable = new HashMap<String,DialogContext>();

    /*
     * Drives what to do for this dialog when a response is seen for an in-dialog response.
     */

    private PendingDialogAction pendingAction = PendingDialogAction.NONE;

    /*
     * Dialog associated with this application data.
     */
    private Dialog dialog;
    /*
     * The Peer Dialog of this Dialog.
     */
    private Dialog peerDialog;

    /*
     * The request that originated the Dialog
     */
    private Request request;

    /*
     * The transaction that created this DialogApplicationData
     */

    Transaction dialogCreatingTransaction;

    /*
     * The last ACK.
     */
    Request lastAck;

    /*
     * The last response seen by the dialog.
     */
    private Response lastResponse;

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

    /*
     * Stack trace to keep track of where this was created.
     */
    private final String creationPointStackTrace;

    /*
     * Stack trace where this was stored in the dialog table.
     */
    private String insertionPointStackTrace;





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
    long timeLastAckSent;

    /*
     * Session timer interval ( seconds ).
     */
    int sessionExpires = Gateway.DEFAULT_SESSION_TIMER_INTERVAL;

    /*
     * Records whether or not an ACCEPTED has been sent for the REFER. This dictates what we need
     * to do when we see a BYE for this dialog.
     */
    private boolean forwardByeToPeer = true;

    /*
     * The generated REFER request.
     */
    private Request referRequest;

    /*
     * If this flag is set to true then the call dialog is torn down immediately after it is
     * CONFIRMed.
     */

    private boolean terminateOnConfirm;

	private ProxyAuthorizationHeader proxyAuthorizationHeader;

	/*
	 * The id for this dialog context.
	 */

    private final String  dialogContextId;

    private long lastReInviteSent;

	private ServerTransaction pendingReInvite;




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


            try {
                if (dialog.getState() == DialogState.TERMINATED) {
                    if ( logger.isDebugEnabled() ) logger.debug("Dialog is terminated -- not firing the  session timer");
                    this.cancel();
                    return;
                }
                Request request;
                if ( logger.isDebugEnabled() ) logger.debug("Firing Session timer " );
                if ( logger.isDebugEnabled() ) logger.debug("DialogState = " + dialog.getState());
                if ( logger.isDebugEnabled() ) logger.debug("peerDialog " + DialogContext.this.getPeerDialog());

                if (dialog.getState() == DialogState.CONFIRMED
                        && DialogContext.get(dialog).getPeerDialog() != null) {

                    /*
                     * This is the case of a hairpinned media where we are not relaying the media.
                     * We implement session timer in this case by soliciting an INVITE and relaying
                     * the SDP unchanged to the other side.
                     */
                    if (CallControlUtilities.isRemoveRelay(DialogContext.this) ) {
                        /*
                         * Recently sent a re-INVITE so no need to send again.
                         */
                        long currentTime = System.currentTimeMillis();
                        if ( (currentTime - DialogContext.this.lastReInviteSent)/1000 <
                             getItspInfo().getSessionTimerInterval()) {
                            if ( logger.isDebugEnabled() ) logger.debug("Session timer was already dispatched. Not sending");
                            return;
                        }
                        /*
                         * We now re-INVITE to shuffle the SDP and get them to point at each other.
                         */

                        if ( logger.isDebugEnabled() ) logger.debug("Sending Session timer re-invite");

                        DialogContext.getPeerDialogContext(getDialog()).setPendingAction(PendingDialogAction.PENDING_RE_INVITE_WITH_SDP_OFFER);
                        DialogContext.getPeerDialogContext(getDialog()).solicitSdpOfferFromPeerDialog(null,getRequest());
                        return;
                    }

                    SipProvider provider = ((DialogExt) dialog).getSipProvider();
                    RtpSession rtpSession = getRtpSession();
                    if (rtpSession == null || rtpSession.getReceiver() == null) {
                        return;
                    }
                    SessionDescription sd = rtpSession.getReceiver().getSessionDescription();

                    request = dialog.createRequest(Request.INVITE);
                    if ( proxyAuthorizationHeader != null ) {
                        request.setHeader(proxyAuthorizationHeader);
                    }
                    request.removeHeader(AllowHeader.NAME);
                    SipUtilities.addWanAllowHeaders(request);
                    AcceptHeader accept = ProtocolObjects.headerFactory.createAcceptHeader(
                            "application", "sdp");
                    request.setHeader(accept);
                    request.removeHeader(SupportedHeader.NAME);

                    request.setContent(sd.toString(), ProtocolObjects.headerFactory
                            .createContentTypeHeader("application", "sdp"));
                    ContactHeader cth = SipUtilities.createContactHeader(provider,
                            getItspInfo(),Gateway.SIPXBRIDGE_USER, null);
                    request.setHeader(cth);
                    SessionExpiresHeader sexp = SipUtilities.createSessionExpires(DialogContext.this.sessionExpires);
                    request.setHeader(sexp);
                    MinSE minSe = new MinSE();
                    minSe.setExpires(Gateway.MIN_EXPIRES);
                    request.setHeader(minSe);
                    if (getItspInfo() != null && getItspInfo().stripPrivateHeaders()) {
                        SipUtilities.stripPrivateHeaders(request);
                    }

                    if (getItspInfo() == null || getItspInfo().isGlobalAddressingUsed()) {
                        SipUtilities.setGlobalAddresses(request);
                    }


                    DialogExt dialogExt = (DialogExt) dialog;
                    ClientTransaction ctx = dialogExt.getSipProvider().getNewClientTransaction(
                            request);
                    TransactionContext.attach(ctx, Operation.SESSION_TIMER);
                    DialogContext.this.sendReInvite(ctx);
                }

            } catch (Exception ex) {
                logger.error("Unexpected exception sending Session Timer INVITE", ex);
                this.cancel();

            }
        }

        public void terminate() {
            if ( logger.isDebugEnabled() ) logger.debug("Terminating session Timer Task for " + dialog);
            this.cancel();
        }
    }

    /**
     * Delays sending the INVITE to the park server. If a RE-INVITE is sent to the ITSP in that
     * interval, the INVITE to the park server is not sent. Instead, we ACK the incoming INVITE so
     * that the INVITE waiting for the ACK can proceed.
     *
     */
    class MohTimer extends TimerTask {

        private ClientTransaction mohCtx;

        public MohTimer(ClientTransaction mohCtx) {
            this.mohCtx = mohCtx;
        }

        @Override
        public void run() {
            try {
                if (!DialogContext.get(mohCtx.getDialog()).terminateOnConfirm) {
                    if ( logger.isDebugEnabled() ) logger.debug("Bridge sending INVITE to MOH server");
                    TransactionContext.get(mohCtx).setDialogPendingSdpAnswer(dialog);
                    DialogContext mohDialogContext = DialogContext.get(mohCtx.getDialog());
                    mohDialogContext.setPendingAction(
                            PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK);
                    mohDialogContext.setPeerDialog(dialog);
                    mohDialogContext.setRtpSession(getPeerRtpSession(dialog));
                    mohCtx.sendRequest();
                } else {
                    if ( logger.isDebugEnabled() ) logger.debug("Phone already sent INVITE - canceling MOH transaction");
                    mohCtx.terminate();
                }
            } catch (Exception ex) {
                logger.error("Error sending moh request", ex);
            } finally {
                mohCtx = null;
            }

        }
    }

    public String getCallLegId() {
       return SipUtilities.getCallLegId(this.getRequest());
    }

    public void startSessionTimer() {
        if ( logger.isDebugEnabled() ) logger.debug("startSessionTimer() for " + this.dialog);
        this.startSessionTimer(this.sessionExpires);
    }

   public void startSessionTimer( int sessionTimeout ) {
       if ( logger.isDebugEnabled() ) logger.debug(String.format("startSessionTimer(%d)",sessionTimeout) + " dialog " + dialog);
       this.sessionTimer = new SessionTimerTask(Gateway.getSessionTimerMethod());
       this.sessionExpires = sessionTimeout;
       int time = (this.sessionExpires - Gateway.TIMER_ADVANCE) * 1000;
       Gateway.getTimer().schedule(this.sessionTimer,time,time);
    }

   public boolean isSessionTimerStarted() {
       return this.sessionTimer != null;
   }

    /*
     * Constructor.
     */
    private DialogContext(Dialog dialog) {
        this.dialogContextId = Integer.toHexString(Math.abs(new Random().nextInt()));
        dialogContextTable.put(this.dialogContextId, this);
        this.sessionExpires = Gateway.DEFAULT_SESSION_TIMER_INTERVAL;
        this.dialog = dialog;
        this.creationPointStackTrace = SipUtilities.getStackTrace();
    }
    /**
     * Get the dialog context for a given ID.
     */
    public static DialogContext getDialogContext(String id) {
        return dialogContextTable.get(id);
    }

    /**
     * Remove the dialog context.
     */
    public static void removeDialogContext(DialogContext dialogContext) {
        dialogContextTable.remove(dialogContext.dialogContextId);
    }

    /**
     * Create a dialog to dialog association.
     *
     * @param dialog1 - first dialog.
     * @param dialog2 - second dialog.
     *
     */
    static void pairDialogs(Dialog dialog1, Dialog dialog2) {
        if ( logger.isDebugEnabled() ) logger.debug("pairDialogs dialogs = " + dialog1 + " " + dialog2);

        DialogContext dad1 = DialogContext.get(dialog1);
        DialogContext dad2 = DialogContext.get(dialog2);
        dad1.setPeerDialog(dialog2);
        dad2.setPeerDialog(dialog1);
    }

    static BackToBackUserAgent getBackToBackUserAgent(Dialog dialog) {
        if (dialog == null) {
            if ( logger.isDebugEnabled() ) logger.debug("null dialog -- returning null ");
            return null;
        } else if (dialog.getApplicationData() == null) {
            if ( logger.isDebugEnabled() ) logger.debug("null dialog application data -- returning null");
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

    static RtpSession getPeerRtpSession(Dialog dialog) {
        return get(getPeerDialog(dialog)).rtpSession;

    }

    static RtpSession getRtpSession(Dialog dialog) {
        if ( logger.isDebugEnabled() ) logger.debug("DialogApplicationData.getRtpSession " + dialog);
        return ((DialogContext) dialog.getApplicationData()).rtpSession;
    }

    static DialogContext attach(BackToBackUserAgent backToBackUserAgent, Dialog dialog,
            Transaction transaction, Request request) {
        if (backToBackUserAgent == null)
            throw new NullPointerException("Null back2back ua");
        if (dialog.getApplicationData() != null) {
            if ( logger.isDebugEnabled() ) logger.debug("DialogContext: Context Already set!!");
            return (DialogContext) dialog.getApplicationData();
        }
        DialogContext dialogContext = new DialogContext(dialog);
        dialogContext.dialogCreatingTransaction = transaction;
        dialogContext.request = request;
        dialogContext.setBackToBackUserAgent(backToBackUserAgent);
        dialog.setApplicationData(dialogContext);
        return dialogContext;
    }

    static DialogContext get(Dialog dialog) {
        return (DialogContext) dialog.getApplicationData();
    }

    /**
     * Get the RTP session of my peer Dialog.
     *
     * @param dialog
     * @return
     */
    static RtpSession getPeerTransmitter(Dialog dialog) {
        return DialogContext.get(DialogContext.getPeerDialog(dialog)).rtpSession;
    }

    /**
     * Convenience method to get the pending action for a dialog.
     *
     */
    static PendingDialogAction getPendingAction(Dialog dialog) {
        return DialogContext.get(dialog).pendingAction;
    }

    /**
     * Convenience method to get the peer dialog context.
     */
    static DialogContext getPeerDialogContext(Dialog dialog) {
        return DialogContext.get(DialogContext.getPeerDialog(dialog));
    }

    /**
     * Get the peer dialog context
     */
    DialogContext getPeerDialogContext() {
        return DialogContext.get(this.peerDialog);
    }

    /**
     * Get the transaction that created this dialog.
     */
    Transaction getDialogCreatingTransaction() {
        return dialogCreatingTransaction;
    }

    /**
     * @param rtpSession the rtpSession to set
     */
    void setRtpSession(RtpSession rtpSession) {
        if (rtpSession == null && this.rtpSession != null ) {
            logger.warn("Setting a Null RTP session!");
        }

        this.rtpSession = rtpSession;
    }

    /**
     * @return the rtpSession
     */
    RtpSession getRtpSession() {
        return rtpSession;
    }

    /**
     * Record the time when ACK was last sent ( for the session timer ).
     */
    void recordLastAckTime() {
        this.timeLastAckSent = System.currentTimeMillis();
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
        if (itspInfo == null ) {
            logger.warn("Setting ITSP info to null!!");
        } else if ( this.itspInfo != null && this.itspInfo != itspInfo ) {
            logger.warn("Overriding ITSP info " + itspInfo + " This is probably a bug");
        }
        this.itspInfo = itspInfo;
        if ( itspInfo != null ) {
            this.sessionExpires = itspInfo.getSessionTimerInterval();
        }
    }

    /**
     * @return the itspInfo
     */
    ItspAccountInfo getItspInfo() {
        return itspInfo;
    }

    /**
     * @return the provider
     */
    SipProvider getSipProvider() {
        return ((DialogExt) dialog).getSipProvider();
    }

    /**
     * Cancel the session timer.
     */
    void cancelSessionTimer() {
        if ( logger.isDebugEnabled() ) logger.debug("cancelSessionTimer " + this.dialog);
        if (this.sessionTimer != null) {
            this.sessionTimer.terminate();
            this.sessionTimer = null;
        }
    }

    /**
     * Set the Expires time for session timer.
     *
     * @param expires
     */
    void setSetExpires(int expires) {
        if ( expires != this.sessionExpires) {
            if ( this.sessionTimer != null ) {
                this.sessionTimer.terminate();
            }
            this.startSessionTimer(expires);
        }


    }

    /**
     * Send ACK to the encapsulated dialog.
     *
     * @throws Exception
     */
    void sendAck(SessionDescription sessionDescription) throws Exception {
        if (this.getLastResponse() == null) {
            Gateway.logInternalError("Method was called with lastResponse null");
            throw new SipXbridgeException("sendAck : null last response");

        }
        if (  this.lastResponse.getStatusCode() != 200 ||
                !SipUtilities.getCSeqMethod(this.lastResponse).equals(Request.INVITE)) {
            Gateway.logInternalError("Method was called with lastResponse null");
            throw new SipXbridgeException("sendAck : last response is not valid " + this.lastResponse);
        }

        if ( this.lastResponse.getHeader(ContactHeader.NAME) == null ) {
            logger.warn("ITSP sent a 200 OK WITHOUT Contact header - silently dropping 200 OK and sending BYE");
            this.sendBye(true,"200 OK without Contact header sent. Dropping call leg.");
            return;
        }
        Request ackRequest = dialog.createAck(SipUtilities.getSeqNumber(this.getLastResponse()));
        if ( this.proxyAuthorizationHeader != null ) {
        	ackRequest.setHeader(proxyAuthorizationHeader);
        }
        this.setLastResponse(null);
        this.recordLastAckTime();
        SipUtilities.setSessionDescription(ackRequest, sessionDescription);
        /*
         * Compensate for the quirks of some ITSPs which will play MOH.
         */
        SipUtilities.setDuplexity(sessionDescription, "sendrecv");
        this.sendAck(ackRequest);

        setPendingAction(PendingDialogAction.NONE);
    }

    /**
     * Check to see if the ITSP allows a REFER request.
     *
     * @return true if REFER is allowed.
     */
    @SuppressWarnings("unchecked")
    boolean isReferAllowed() {
        if (this.dialogCreatingTransaction instanceof ServerTransaction) {
            if (this.getRequest() == null) {
                return false;
            }
            ListIterator li = getRequest().getHeaders(AllowHeader.NAME);

            while (li != null && li.hasNext()) {
                AllowHeader ah = (AllowHeader) li.next();
                if (ah.getMethod().equals(Request.REFER)) {
                    return true;
                }
            }
            return false;

        } else {
            if (this.getLastResponse() == null) {
                return false;
            }

            ListIterator li = getLastResponse().getHeaders(AllowHeader.NAME);

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
     * @param continuationData -- context information so we can process the continuation.
     * @param triggerMessage -- the message that triggered this action.
     *
     * @return true if the offer is sent successfully. false if there is already an offer in
     *         progress and hence we should not send an offer.
     */
    boolean solicitSdpOfferFromPeerDialog(ContinuationData continuationData,
           Message triggerMessage) throws Exception {
        try {

            Dialog peerDialog = DialogContext.getPeerDialog(dialog);

            /*
             * There is already a re-negotiation in progress so return silently
             */

            if (peerDialog != null && peerDialog.getState() != DialogState.TERMINATED) {
                if ( logger.isDebugEnabled() ) logger.debug("solicitSdpOfferFromPeerDialog -- sending query to " + peerDialog
                        + " continuationOperation = " +
                        (continuationData != null ? continuationData.getOperation() : null));

                Request reInvite = peerDialog.createRequest(Request.INVITE);
                DialogContext peerDialogContext = DialogContext.get(peerDialog);
                if ( peerDialogContext.proxyAuthorizationHeader != null ) {
                	reInvite.setHeader(peerDialogContext.proxyAuthorizationHeader);
                }
                reInvite.removeHeader(SupportedHeader.NAME);
                reInvite.removeHeader("remote-party-Id");
                if ( this.itspInfo != null ) {
                    Address address = this.itspInfo.getCallerAlias(dialog.getLocalParty());
                    PAssertedIdentityHeader passertedIdentityHeader = null;
                    if (address != null) {
                        passertedIdentityHeader = ((HeaderFactoryExt) ProtocolObjects.headerFactory)
                        .createPAssertedIdentityHeader(address);
                        reInvite.setHeader(passertedIdentityHeader);
                    }
                    Address paddress = this.itspInfo.getPreferredCallerAlias(dialog.getLocalParty());
                    PPreferredIdentityHeader ppreferredIdentityHeader = null;
                    if (paddress != null) {
                    	ppreferredIdentityHeader = ((HeaderFactoryExt) ProtocolObjects.headerFactory)
                        .createPPreferredIdentityHeader(paddress);
                        reInvite.setHeader(ppreferredIdentityHeader);
                    }
                } else {
                    logger.warn("Could not find ITSP Information. Sending re-INVITE anyway.");
                }
                /*
                 * Add the references header for causality chaining.
                 */
                ReferencesHeader referencesHeader = SipUtilities.createReferencesHeader(triggerMessage,
                        ReferencesHeader.CHAIN);
                reInvite.setHeader(referencesHeader);
                SipUtilities.addWanAllowHeaders(reInvite);
                SipProvider provider = ((DialogExt) peerDialog).getSipProvider();
                String transport = ((SipProviderExt)provider).getListeningPoints()[0].getTransport();
                ViaHeader viaHeader = SipUtilities.createViaHeader(provider, transport);

                reInvite.setHeader(viaHeader);
                ContactHeader contactHeader = SipUtilities.createContactHeader(null,
                        provider, transport);


                reInvite.setHeader(contactHeader);
                AcceptHeader acceptHeader = ProtocolObjects.headerFactory.createAcceptHeader(
                        "application", "sdp");
                reInvite.setHeader(acceptHeader);
                if ( provider == Gateway.getWanProvider(transport) &&
                		(this.itspInfo == null || this.itspInfo.isGlobalAddressingUsed())) {
                	SipUtilities.setGlobalAddresses(reInvite);
                }

                ClientTransaction ctx = provider.getNewClientTransaction(reInvite);
                TransactionContext tad = TransactionContext.attach(ctx,
                        Operation.SOLICIT_SDP_OFFER_FROM_PEER_DIALOG);
                /*
                 * Mark what we should do when we see the 200 OK response. This is what this
                 * dialog expects to see. Mark this as the pending operation for this dialog.
                 */
                DialogContext.get(peerDialog).setPendingAction(PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK);

                /*
                 * The information we need to continue the operation when the Response comes in.
                 */
                tad.setContinuationData(continuationData);

                /*
                 * Send the Re-INVITE and try to avoid the Glare Race condition.
                 */
                DialogContext.get(peerDialog).sendReInvite(ctx);

            }
            return true;
        } catch (Exception ex) {
            logger.error("Exception occured. tearing down call! ", ex);
            this.backToBackUserAgent.tearDown();
            return true;
        }

    }

    /**
     * Sent the pending action ( to be performed when the next in-Dialog request or response
     * arrives ).
     *
     * @param pendingAction
     */
    void setPendingAction(PendingDialogAction pendingAction) {
    	if ( logger.isDebugEnabled() ) logger.debug("setPendingAction " + this + " pendingAction = " + pendingAction);
        /*
         * A dialog can have only a single outstanding action.
         */
        if (this.pendingAction != PendingDialogAction.NONE
                && pendingAction != PendingDialogAction.NONE
                && this.pendingAction != pendingAction) {
            logger.error("Replacing pending action " + this.pendingAction + " with "
                    + pendingAction);
            throw new SipXbridgeException("Pending dialog action is " + this.pendingAction);
        }
        this.pendingAction = pendingAction;
    }

    /**
     * Get the pending action for this dialog.
     */
    PendingDialogAction getPendingAction() {
        return pendingAction;
    }

    /**
     * Set the last seen response for this dialog.
     *
     * @param lastResponse
     */
    void setLastResponse(Response lastResponse) {
        if ( logger.isDebugEnabled() ) logger.debug("DialogContext.setLastResponse ");
        if (lastResponse == null) {
            if ( logger.isDebugEnabled() ) logger.debug("lastResponse = " + null);
        } else {
            if ( logger.isDebugEnabled() ) logger.debug("lastResponse = " + ((SIPResponse) lastResponse).getFirstLine());
        }


        this.lastResponse = lastResponse;



    }

    /**
     * The last response that was seen for this dialog.
     *
     * @return
     */
    Response getLastResponse() {
        return lastResponse;
    }

    /**
     * Send an SDP re-OFFER to the other side of the B2BUA.
     *
     * @param sdpOffer -- the sdp offer session description.
     * @param responseEvent -- the response event that triggered this action.
     *
     * @throws Exception -- if could not send
     */
    void sendSdpReOffer(SessionDescription sdpOffer, ResponseEvent responseEvent) throws Exception {
        if (dialog.getState() == DialogState.TERMINATED) {
            logger.warn("Attempt to send SDP re-offer on a terminated dialog");
            return;
        }
        Response triggeringResponse = responseEvent.getResponse();

        if ( CallControlUtilities.isRemoveRelay(this) ) {
            sdpOffer = SipUtilities.getSessionDescription(triggeringResponse);
        } else {
            if ( logger.isDebugEnabled() ) logger.debug("ITSP has set always relay media -- rewriting SDP ");
           
            this.getRtpSession().getReceiver().setSessionDescription(sdpOffer);
        }

        SipUtilities.incrementSessionVersion(sdpOffer);


        if ( this.pendingReInvite != null ) {
        	Response newResponse = SipUtilities.createResponse(this.pendingReInvite, Response.OK);
        	SipUtilities.setSessionDescription(newResponse, sdpOffer);

        	if (((ServerTransactionExt)this.pendingReInvite).getSipProvider()  != Gateway.getLanProvider() &&
        			(this.itspInfo == null || this.itspInfo.isGlobalAddressingUsed())) {
        		SipUtilities.setGlobalAddress(newResponse);
        	}


        	this.pendingReInvite.sendResponse(newResponse);
        	this.pendingReInvite = null;


        }  else {
        	 if ( this.isLanFacing() || (this.itspInfo != null && !this.itspInfo.isGlobalAddressingUsed())) {
             	this.getRtpSession().getReceiver().setUseGlobalAddressing(false);
             } else {
            	 this.getRtpSession().getReceiver().setUseGlobalAddressing(true);
                     
             }
        	Request sdpOfferInvite = dialog.createRequest(Request.INVITE);
        	ReferencesHeader referencesHeader = SipUtilities.createReferencesHeader(triggeringResponse, ReferencesHeader.CHAIN);
        	sdpOfferInvite.setHeader(referencesHeader);

        	if ( proxyAuthorizationHeader != null ) {
        		sdpOfferInvite.setHeader(proxyAuthorizationHeader);
        	}
        	/*
        	 * Set and fix up the sdp offer to send to the opposite side.
        	 */

        	SipUtilities.fixupOutboundRequest(dialog, sdpOfferInvite);

        	sdpOfferInvite.setContent(sdpOffer.toString(), ProtocolObjects.headerFactory
        			.createContentTypeHeader("application", "sdp"));

        	ClientTransaction ctx = ((DialogExt) dialog).getSipProvider().getNewClientTransaction(
        			sdpOfferInvite);


        	TransactionContext.attach(ctx, Operation.SEND_SDP_RE_OFFER);

        	this.sendReInvite(ctx);
        }


    }

    /**
     * Send an ACK and record it.
     *
     * @param ack
     * @throws SipException
     */
    void sendAck(Request ack) throws SipException {
        this.recordLastAckTime();
        this.lastAck = ack;
        if ( logger.isDebugEnabled() ) logger.debug("SendingAck ON " + dialog);
        if ( this.proxyAuthorizationHeader != null ) {
        	ack.setHeader(proxyAuthorizationHeader);
        }
        dialog.sendAck(ack);


        if (terminateOnConfirm) {
            if ( logger.isDebugEnabled() ) logger.debug("tearing down MOH dialog because of terminateOnConfirm.");
            Request byeRequest = dialog.createRequest(Request.BYE);

            ClientTransaction ctx = ((DialogExt) dialog).getSipProvider()
                    .getNewClientTransaction(byeRequest);
            TransactionContext.attach(ctx, Operation.SEND_BYE_TO_MOH_SERVER);
            dialog.sendRequest(ctx);
        }
    }

    /**
     * Set the "terminate on confirm" flag which will send BYE to the dialog If the flag is set as
     * soon as the MOH server confirms the dialog, the flag is consulted and a BYE sent to the MOH
     * server.
     *
     */
    void setTerminateOnConfirm() {
        this.terminateOnConfirm = true;
        if ( logger.isDebugEnabled() ) logger.debug("setTerminateOnConfirm: " + this);
        /*
         * Fire off a timer to reap this guy if he does not die in 8 seconds.
         */
        Gateway.getTimer().schedule(new TimerTask() {
            @Override
            public void run() {
                try {
                    if (DialogContext.this.dialog.getState() != DialogState.TERMINATED) {
                        if ( logger.isDebugEnabled() ) logger.debug("terminating dialog " + dialog + " because no confirmation received and terminateOnConfirm is set");
                        DialogContext.this.dialog.delete();
                    }
                } catch (Exception ex) {
                    logger.error("Terminate On Confirm processing", ex);
                }
            }
        }, 8000);

    }

    /**
     * return true if "terminate on confirm" flag is set.
     */
    boolean isTerminateOnConfirm() {
        return this.terminateOnConfirm;
    }

    /**
     * Send a re-INVITE. The dialog layer will asynchronously send the re-INVITE
     */
    void sendReInvite(ClientTransaction clientTransaction) {
    	if ( logger.isDebugEnabled() ) logger.debug("sendReInvite " + SipUtilities.getStackTrace());
    	if ( this.proxyAuthorizationHeader != null ) {
    		clientTransaction.getRequest().setHeader(this.proxyAuthorizationHeader);
    	}

    	/*
    	 * Record the last Re-INVITE sent.
    	 */
    	this.lastReInviteSent = System.currentTimeMillis();

        if (dialog.getState() != DialogState.TERMINATED) {
             try {
                dialog.sendRequest(clientTransaction);
            } catch (SipException e) {
                logger.error("Exception sending re-INVITE",e);

            }
        } else {
            logger.warn("sendReInvite was called when the dialog is terminated - ignoring");
        }
    }

    /**
     * Send INVITE to MOH server.
     */
    void sendMohInvite(ClientTransaction mohClientTransaction) {
        Gateway.getTimer().schedule(new MohTimer(mohClientTransaction),
                Gateway.getMusicOnHoldDelayMiliseconds());
    }

    /**
     * Set the peer dialog of this dialog.
     */
    void setPeerDialog(Dialog peerDialog) {
        if ( logger.isDebugEnabled() ) logger.debug("DialogContext.setPeerDialog: " + this.dialog + " peer = " + peerDialog);

        this.peerDialog = peerDialog;

        if (logger.isDebugEnabled() && peerDialog == null ) {
            SipUtilities.printStackTrace();
        }

    }

    /**
     * Get the peer dialog of this dialog.
     */
    Dialog getPeerDialog() {
        return peerDialog;
    }

    /**
     * Flag that controls whether or not BYE is forwarded to the peer dialog.
     */
    void setForwardByeToPeer(boolean forwardByeToPeer) {
        if ( logger.isDebugEnabled() ) logger.debug("setForwardByeToPeer " + forwardByeToPeer);
        this.forwardByeToPeer = forwardByeToPeer;
    }

    /**
     * Retuns true if we should forward the BYE for the peer.
     *
     * @return
     */
    boolean isForwardByeToPeer() {
        return forwardByeToPeer;
    }

    /**
     * The pending REFER request that we are processing.
     *
     * @param referRequest
     */
    void setReferRequest(Request referRequest) {
        this.referRequest = referRequest;
    }

    /**
     * The current REFER request being processed.
     *
     * @return
     */
    Request getReferRequest() {
        return referRequest;
    }

    /**
     * Get the request that created the Dialog.
     *
     * @return the dialog creating request.
     */
    Request getRequest() {
        return request;
    }

    /**
     * Send ACK for the dialog.
     *
     * @param response -- the 200 OK that we are ACKing.
     *
     * @throws Exception
     */
    public void sendAck(Response response) throws Exception {

        this.lastResponse = response;

        if (this.getLastResponse() == null) {
            Gateway.logInternalError("Method was called with lastResponse null");
            throw new SipXbridgeException("sendAck : null last response");

        }
        if (  this.lastResponse.getStatusCode() != 200 ||
                !SipUtilities.getCSeqMethod(this.lastResponse).equals(Request.INVITE)) {
            Gateway.logInternalError("Method was called with lastResponse null");
            throw new SipXbridgeException("sendAck : last response is not valid " + this.lastResponse);
        }

        if ( this.lastResponse.getHeader(ContactHeader.NAME) == null ) {
            logger.warn("ITSP sent a 200 OK WITHOUT Contact header - silently dropping 200 OK and sending BYE");
            this.sendBye(true,"200 OK without Contact header sent. Dropping call leg.");
            return;
        }

        Request ack = dialog.createAck(((CSeqHeader) response.getHeader(CSeqHeader.NAME))
                .getSeqNumber());

        if ( this.proxyAuthorizationHeader != null ) {
        	ack.setHeader(proxyAuthorizationHeader);
        }

        this.sendAck(ack);

    }


    /**
     * Send bye for this dialog.
     *
     * @param forward - whether or not to forward this BYE to the other side of the B2BUA.
     * @param reason - text to place in the Reason header.
     *
     * @throws Exception
     */

    void sendBye(boolean forward, String reason) throws SipException {
        try {
            Request bye = dialog.createRequest(Request.BYE);

            if ( getSipProvider() != Gateway.getLanProvider() ) {
                if ( itspInfo == null || itspInfo.isGlobalAddressingUsed()) {
                    SipUtilities.setGlobalAddresses(bye);
                }
            }

            if ( this.proxyAuthorizationHeader != null ) {
            	bye.setHeader(proxyAuthorizationHeader);
            }
            ViaHeader via  = ((ViaHeader) bye.getHeader(ViaHeader.NAME));
            if ( !forward ) via.setParameter("sipxecs-noforward", "true");

            ClientTransaction clientTransaction = getSipProvider().getNewClientTransaction(bye);

            TransactionContext transactionContext = TransactionContext.attach(
                    clientTransaction, Operation.PROCESS_BYE);

            transactionContext.setItspAccountInfo(this.itspInfo);

            dialog.sendRequest(clientTransaction);
        } catch (ParseException ex) {
            logger.error("Unexpected exception",ex);
            throw new SipXbridgeException("Unexpected exception",ex);
        }
    }

    /**
     * Send BYE for this dialog (no reason provided).
     *
     * @param forward -- whether or not to forward the BYE.
     */

    void sendBye(boolean forward) throws SipException {
        sendBye(forward,null);
    }

    /**
     * Forward BYE.
     *
     * @throws SipException
     */
    void forwardBye(ServerTransaction serverTransaction) throws SipException {
        Request bye = dialog.createRequest(Request.BYE);
        ReferencesHeader referencesHeader = SipUtilities.createReferencesHeader(serverTransaction.getRequest(),
                ReferencesHeader.CHAIN);
        bye.setHeader(referencesHeader);
        if ( this.proxyAuthorizationHeader != null ) {
            bye.setHeader(this.proxyAuthorizationHeader) ;
        }
        if ( getSipProvider() != Gateway.getLanProvider() ) {
            if ( itspInfo == null || itspInfo.isGlobalAddressingUsed()) {
                SipUtilities.setGlobalAddresses(bye);
            }
        }

        ClientTransaction clientTransaction = getSipProvider().getNewClientTransaction(bye);

        TransactionContext transactionContext = TransactionContext.attach(
                clientTransaction, Operation.PROCESS_BYE);

        transactionContext.setItspAccountInfo(this.itspInfo);
        TransactionContext.get(serverTransaction).setClientTransaction(clientTransaction);
        TransactionContext.get(clientTransaction).setServerTransaction(serverTransaction);
        dialog.sendRequest(clientTransaction);
    }

    /**
     * Response sent to session timer so we can kill our own session timer.
     */
    void setSessionTimerResponseSent() {
        if (this.sessionTimer != null) {
            if ( logger.isDebugEnabled() ) logger.debug("setSessionTimerResponseSent()");
            this.cancelSessionTimer();
            this.startSessionTimer();
        }
    }


   /**
    * Set the dialog pointer ( link this structure with a Dialog ).
    *
    * @param dialog
    */


    public void setDialog(Dialog dialog) {
      this.dialog = dialog;
      dialog.setApplicationData(this);
    }

    /**
     * Set the dialog creating transaction.
     *
     * @param newTransaction
     */
    public void setDialogCreatingTransaction(ClientTransaction  dialogCreatingTransaction) {
        this.dialogCreatingTransaction = dialogCreatingTransaction;
        this.request = dialogCreatingTransaction.getRequest();
    }

    /**
     * Detach this structure from the associated dialog.
     */
    public void detach() {
      this.dialog.setApplicationData(null);
      this.dialog = null;
      this.pendingAction = PendingDialogAction.NONE;
    }

    /**
     * Debugging method that returns the stack trace corresponding to where the dialog was
     * created.
     *
     * @return
     */
    public String getCreationPointStackTrace() {
       return this.creationPointStackTrace;
    }

    /**
     * Debugging method that returns where the structure was originally stored into the
     * dialog table.
     */
    public String getInsertionPointStackTrace() {
        return this.insertionPointStackTrace;
    }

    /**
     * Debugging method that records when this is inserted into the dialog table.
     */
    public void recordInsertionPoint() {
        this.insertionPointStackTrace = SipUtilities.getStackTrace();
    }

    /**
     * Get the associated dialog.
     *
     * @return dialog associated with this dialog context.
     */
    public Dialog getDialog() {
        return this.dialog;
    }

    /**
     * Set the proxy authorization header for this dialog context.
     *
     * @param pah - the proxy authorization header.
     */
	public void setProxyAuthorizationHeader(ProxyAuthorizationHeader pah) {
		this.proxyAuthorizationHeader = pah;
	}

    public String getDialogContextId() {
       return dialogContextId;
    }

    /**
     * Get a pending re-INVITE.
     * @return
     */
	public ServerTransaction  getPendingReInvite() {
		return this.pendingReInvite;
	}

	/**
	 * A re-invite is pending for this dialog context.
	 * @param serverTransaction
	 */
	public void setPendingReInvite(ServerTransaction serverTransaction ) {
		this.pendingReInvite = serverTransaction;
	}

	public boolean isLanFacing() {
		return this.getSipProvider() == Gateway.getLanProvider();
	}

	




}
