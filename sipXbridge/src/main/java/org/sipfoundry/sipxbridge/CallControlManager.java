/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.SipStackExt;
import gov.nist.javax.sip.SipStackImpl;
import gov.nist.javax.sip.TransactionExt;
import gov.nist.javax.sip.header.extensions.MinSE;
import gov.nist.javax.sip.header.extensions.ReferencesHeader;
import gov.nist.javax.sip.header.extensions.ReplacesHeader;
import gov.nist.javax.sip.header.extensions.SessionExpires;
import gov.nist.javax.sip.header.extensions.SessionExpiresHeader;
import gov.nist.javax.sip.message.SIPResponse;
import gov.nist.javax.sip.stack.SIPDialog;
import gov.nist.javax.sip.stack.SIPServerTransaction;

import java.text.ParseException;
import java.util.Collection;
import java.util.ListIterator;
import java.util.Locale;
import java.util.Random;
import java.util.Set;
import java.util.TimerTask;

import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.InvalidArgumentException;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.TransactionAlreadyExistsException;
import javax.sip.TransactionState;
import javax.sip.TransactionUnavailableException;
import javax.sip.address.SipURI;
import javax.sip.header.AcceptHeader;
import javax.sip.header.AcceptLanguageHeader;
import javax.sip.header.AuthorizationHeader;
import javax.sip.header.CSeqHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentLengthHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.EventHeader;
import javax.sip.header.Header;
import javax.sip.header.MaxForwardsHeader;
import javax.sip.header.ReasonHeader;
import javax.sip.header.RequireHeader;
import javax.sip.header.SubscriptionStateHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.WarningHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxrelay.KeepaliveMethod;
import org.sipfoundry.sipxrelay.SymmitronException;
import org.sipfoundry.sipxrelay.SymmitronResetHandler;

/**
 * This class does some pre-processing of requests and then acts as a high level router for
 * routing the request to the appropriate B2BUA. It processes INVITE, REFER, ACK, OPTIONS, BYE.
 *
 * @author M. Ranganathan
 *
 */
class CallControlManager implements SymmitronResetHandler {

    static Logger logger = Logger.getLogger(CallControlManager.class);

    // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Internal classes.
    // ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * This timer task is kicked off when we get a 491 RequestPending. The timer task completes
     * and re-tries the INVITE.
     *
     */
    class RequestPendingTimerTask extends TimerTask {

        private final TransactionContext transactionContext;

        public RequestPendingTimerTask(TransactionContext transactionContext) {
            this.transactionContext = transactionContext;
        }

        @Override
        public void run() {
            try {
                Request request = transactionContext.getClientTransaction().getRequest();
                DialogContext dialogContext = DialogContext.get(transactionContext
                        .getClientTransaction().getDialog());
                Dialog dialog = transactionContext.getClientTransaction().getDialog();
                String method = request.getMethod();
                Request newRequest = dialog.createRequest(method);
                ListIterator listIterator = request.getHeaderNames();
                /*
                 * Copy over any necessary headers from the original request.
                 */
                while (listIterator.hasNext()) {
                    String headerName = (String) listIterator.next();
                    if (newRequest.getHeader(headerName) == null
                            && !headerName.equals(ContentLengthHeader.NAME)) {
                        Header header = request.getHeader(headerName);
                        newRequest.addHeader(header);
                    }
                }
                if (request.getContent() != null) {
                    String content = (String) request.getContent();
                    ContentTypeHeader contentTypeHeader = (ContentTypeHeader) request
                            .getHeader(ContentTypeHeader.NAME);
                    newRequest.setContent(content, contentTypeHeader);
                }
                SipProvider provider = ((DialogExt) dialog).getSipProvider();
                ClientTransaction newCtx = provider.getNewClientTransaction(newRequest);
                transactionContext.setClientTransaction(newCtx);
                newCtx.setApplicationData(transactionContext);
                dialogContext.sendReInvite(newCtx);
            } catch (Exception ex) {
                try {
                    BackToBackUserAgent b2bua = DialogContext
                            .getBackToBackUserAgent(transactionContext.getClientTransaction()
                                    .getDialog());
                    logger.error("Error sending re-INVITE", ex);
                    b2bua.tearDown("sipxbridge", ReasonCode.ERROR_SENDING_REINVITE,
                            "Error sending re-INVITE");
                } catch (Exception e) {
                    logger.error("Error tearing down call", e);
                }
            }

        }
    }

    /**
     * Timer task to tear down the replaced dialog in case the other end went away and left us
     * hanging.
     */
    class TearDownReplacedDialogTimerTask extends TimerTask {
        Dialog replacedDialog;

        public TearDownReplacedDialogTimerTask(Dialog dialog) {
            this.replacedDialog = dialog;
        }

        @Override
        public void run() {
            try {
                if (replacedDialog.getState() != DialogState.TERMINATED) {
                    SipProvider lanProvider = ((DialogExt) replacedDialog).getSipProvider();
                    Request byeRequest = replacedDialog.createRequest(Request.BYE);
                    ClientTransaction byeCtx = lanProvider.getNewClientTransaction(byeRequest);
                    TransactionContext.attach(byeCtx, Operation.SEND_BYE_TO_REPLACED_DIALOG);
                    replacedDialog.sendRequest(byeCtx);
                }
            } catch (Exception ex) {
                logger.error("Error sending bye to replaced dialog", ex);
            }
        }
    }

    class NotifyReferDialogTimerTask extends TimerTask {

        private final Request referRequest;
        private final Dialog referDialog;
        private final Response response;

        public NotifyReferDialogTimerTask(Request referRequest, Dialog referDialog,
                Response response) {
            this.referRequest = referRequest;
            this.referDialog = referDialog;
            this.response = response;
        }

        @Override
        public void run() {
            try {
                if (referDialog.getState() != DialogState.TERMINATED) {
                    notifyReferDialog(referRequest, referDialog, response);
                }
            } catch (Exception ex) {
                logger.error("Error notifying refer dialog ", ex);
            }
        }

    }

    // ///////////////////////////////////////////////////////////////////////////////////////////
    // Request handlers.
    // ///////////////////////////////////////////////////////////////////////////////////////////

    /**
     * Does the request processing for a re-INVITATION. A re-INVITE can be seen on the following
     * conditions: - codec renegotiation. - hold/resume - sdp solicitation.
     *
     * @param requestEvent
     * @throws Exception
     */
    private void handleReInvite(RequestEvent requestEvent) throws Exception {
        if ( logger.isDebugEnabled() ) logger.debug("handleReInvite");

        /*
         * Grab context information from inbound request.
         */
        ServerTransaction serverTransaction = requestEvent.getServerTransaction();
        Dialog dialog = serverTransaction.getDialog();
        Request request = requestEvent.getRequest();
        SipProvider provider = (SipProvider) requestEvent.getSource();
        DialogContext dialogContext = (DialogContext) dialog.getApplicationData();

        ReferencesHeader referencesHeader = SipUtilities.createReferencesHeader(request, ReferencesHeader.CHAIN);

        if ( dialogContext == null ) {
            logger.error("Null Dialog Context detected on dialog " + dialog);
            return;
        }

        Dialog peerDialog = dialogContext.getPeerDialog();

        /*
         * The peer dialog may not be found if our re-INVITE did not succeed.
         */
        if (peerDialog == null) {
            Response response = SipUtilities.createResponse(serverTransaction,
                    Response.CALL_OR_TRANSACTION_DOES_NOT_EXIST);
            ReasonHeader reasonHeader = ProtocolObjects.headerFactory.createReasonHeader(
                    Gateway.SIPXBRIDGE_USER, ReasonCode.CALL_SETUP_ERROR,
                    "Could not process re-INVITE : No peer!");
            response.setReasonPhrase("Peer dialog is null");
            serverTransaction.sendResponse(response);
            if (dialogContext.getBackToBackUserAgent() != null) {
                dialogContext.getBackToBackUserAgent().tearDown(reasonHeader);
            }
            return;
        }

        /*
         * We re-use the RTP session object. The useGlobalAddressing flag determines
         * whether or not the SDP associated with the RTP session gets global (public)
         * addresses assigned to it. In the case of call forwarding, it could be that
         * this RTP session was pointed to the global side. Hence we need to reset it
         * to point to the private side if the request originated from the LAN.
         */
        if ( dialogContext.getRtpSession() != null && provider == Gateway.getLanProvider() ) {
             dialogContext.getRtpSession().getReceiver().setUseGlobalAddressing(false);
        }
        SipProvider peerDialogProvider = ((DialogExt) peerDialog).getSipProvider();

        /*
         * Is the other side trying to solicit an offer? This will be the case if the
         * INVITE has no session description.
         */

        if (SipUtilities.isSdpOfferSolicitation(request)) {
            /*
             * This case occurs if MOH is turned OFF on sipxbridge and is turned ON on the phone.
             * In this case the phone will solicit the ITSP for an offer See Issue 1739
             */
            Request newRequest = peerDialog.createRequest(Request.INVITE);

            newRequest.setHeader(referencesHeader);



            /*
             * Contact header for the re-INVITE we are about to send.
             * Use the contact header from the inbound re-invite and extract the user name
             * from there.
             */
            ContactHeader requestContactHeader = (ContactHeader) request.getHeader(ContactHeader.NAME);
            String contactUser;
            if (requestContactHeader != null) {
               SipURI contactURI = (SipURI) requestContactHeader.getAddress().getURI();
               contactUser = contactURI.getUser();
            } else {
               contactUser = Gateway.SIPXBRIDGE_USER;
            } 
            ContactHeader contactHeader = SipUtilities.createContactHeader(
                    contactUser, peerDialogProvider,
                    SipUtilities.getViaTransport(newRequest));
            newRequest.setHeader(contactHeader);

            if ( request.getHeader(AuthorizationHeader.NAME) != null ) {
               AuthorizationHeader authHeader = (AuthorizationHeader)
                        request.getHeader(AuthorizationHeader.NAME);
                newRequest.setHeader(authHeader);
            }

            /*
             * Create a new client transaction with which to forward the request.
             */

            ClientTransaction ctx = peerDialogProvider.getNewClientTransaction(newRequest);

            /*
             * Set up the transaction context.
             */
            TransactionContext tad = TransactionContext.attach(ctx,
                    Operation.FORWARD_SDP_SOLICITIATION);

            /*
             * Associate the client transaction with the inbound server transaction.
             */
            tad.setServerTransaction(serverTransaction);

            /*
             * Set up the continuation data so we know what to do when the response arrives.
             */
            tad.setContinuationData(new ForwardSdpSolicitationContinuationData(requestEvent));
            serverTransaction.setApplicationData(tad);

            DialogContext peerDialogContext = DialogContext.get(peerDialog);

            /*
             * Incoming request came in on the LAN side. Check if there is a record for the ITSP on
             * the wan side of the association.
             */
            if (provider == Gateway.getLanProvider()
                    && (peerDialogContext.getItspInfo() == null || peerDialogContext.getItspInfo()
                            .isGlobalAddressingUsed())) {
                SipUtilities.setGlobalAddresses(newRequest);
            }

            /*
             * Set the ALLOW header to be the WAN side ALLOW headers.
             */
            if (provider == Gateway.getLanProvider()) {
                SipUtilities.addWanAllowHeaders(newRequest);
            }
            /*
             * Record in the corresponding dialog that that we solicited an offer so we can send
             * the Ack along with the SDP that is offered.
             */
            peerDialogContext.setPendingAction(PendingDialogAction.PENDING_FORWARD_ACK_WITH_SDP_ANSWER);
            if ( (peerDialogContext.getItspInfo() == null ||
                    peerDialogContext.getItspInfo().getPassword() == null ) &&
                   request.getHeader(AuthorizationHeader.NAME) != null ) {
                /*
                 * We have no password information for the peer so just
                 * accept any incoming authorization information from the
                 * caller.
                 */
                AuthorizationHeader authHeader = (AuthorizationHeader)
                        request.getHeader(AuthorizationHeader.NAME);
                newRequest.setHeader(authHeader);
            }
            peerDialogContext.sendReInvite(ctx);

        } else {

            RtpSession rtpSession = dialogContext.getRtpSession();

            /*
             * Associate the inbound session description with the TRANSMITTER side of the
             * rtpSession.
             */
            RtpSessionOperation operation = RtpSessionUtilities
                    .reAssignRtpSessionParameters(serverTransaction);

            if ( logger.isDebugEnabled() ) logger.debug("Rtp Operation " + operation);

            /*
             * The other side sent us a media hold operation.
             */
            if (operation == RtpSessionOperation.PLACE_HOLD) {
                /*
                 * Does the bridge have MOH configured? If so, set up a continuation for when
                 * we get the response from the MOH server and solicit an offer from it. When
                 * the offer comes in, we will continue.
                 */
                if (Gateway.getMusicOnHoldUri() != null) {
                    SendInviteToMohServerContinuationData cdata = new SendInviteToMohServerContinuationData(
                            requestEvent);
                    dialogContext.solicitSdpOfferFromPeerDialog(cdata,requestEvent.getRequest());
                } else {
                    /*
                     * No MOH support on bridge so send OK right away.
                     */
                    Response response = SipUtilities.createResponse(serverTransaction,
                            Response.OK);
                    SessionDescription sessionDescription = rtpSession.getReceiver()
                            .getSessionDescription();
                    SipUtilities.setSessionDescription(response, sessionDescription);
                    /*
                     * Send an OK to the other side with a SD that indicates that the HOLD
                     * operation is successful.
                     */
                    serverTransaction.sendResponse(response);

                }
            } else if (operation == RtpSessionOperation.REMOVE_HOLD
                    || operation == RtpSessionOperation.CODEC_RENEGOTIATION
                    || operation == RtpSessionOperation.PORT_REMAP) {
                /*
                 * Remove hold and codec renegotiation require forwarding of re-INVITE.
                 */
                RtpSessionUtilities.forwardReInvite(rtpSession, serverTransaction, dialog, true);
            } else {
                /*
                 * This is a request that can be handled locally. Grab the previous session
                 * description from the receiver side.
                 */
                if ( logger.isDebugEnabled() ) logger.debug("session Timer INVITE -- sending old response ");
                SessionDescription newDescription = rtpSession.getReceiver()
                        .getSessionDescription();
                Response response = SipUtilities.createResponse(serverTransaction, Response.OK);

                if (newDescription != null) {
                    response.setContent(newDescription, ProtocolObjects.headerFactory
                            .createContentTypeHeader("application", "sdp"));
                }

                ToHeader toHeader = (ToHeader) request.getHeader(ToHeader.NAME);
                String userName = ((SipURI) toHeader.getAddress().getURI()).getUser();
                ContactHeader contactHeader = SipUtilities
                        .createContactHeader(userName, provider, SipUtilities.getViaTransport(response));
                response.setHeader(contactHeader);

                /*
                 * Request was seen from the WAN side.
                 */
                if ( provider != Gateway.getLanProvider()) {
                    if ( dialogContext.getItspInfo() == null || dialogContext.getItspInfo().isGlobalAddressingUsed()) {
                        SipUtilities.setGlobalAddress(response);
                    }
                }

                dialogContext.setSessionTimerResponseSent();

                serverTransaction.sendResponse(response);

            }

        }
    }

    /**
     * Processes an incoming invite from the PBX or from the ITSP side. This method fields the
     * inbound request and either routes it to the appropriate b2bua or forwards the request.
     *
     *
     * @param requestEvent
     */
    private void processInvite(RequestEvent requestEvent) {
        if ( logger.isDebugEnabled() ) logger.debug("processInvite");

        Request request = requestEvent.getRequest();


        SipProvider provider = (SipProvider) requestEvent.getSource();
        ServerTransaction serverTransaction = requestEvent.getServerTransaction();

        try {
            if (serverTransaction == null) {
                try {
                    /*
                     * Patch up request for missing Max-Forwards header. See Issue XX-7007
                     */
                    if ( request.getHeader(MaxForwardsHeader.NAME) == null ) {
                        MaxForwardsHeader maxForwardsHeader = ProtocolObjects.headerFactory.createMaxForwardsHeader(20);
                        request.setHeader(maxForwardsHeader);
                    }
                    serverTransaction = provider.getNewServerTransaction(request);
                } catch (TransactionAlreadyExistsException ex) {
                    if (logger.isDebugEnabled()) {
                        logger.debug("Transaction already exists for " + request);
                    }
                    return;
                }
            }
            Dialog dialog = serverTransaction.getDialog();

            if (dialog.getState() == DialogState.TERMINATED) {
                if ( logger.isDebugEnabled() ) logger.debug("Got a stray request on a terminated dialog!");
                Response response = SipUtilities.createResponse(serverTransaction,
                        Response.SERVER_INTERNAL_ERROR);
                serverTransaction.sendResponse(response);
                return;

            } else if (dialog.getState() == DialogState.CONFIRMED) {
                handleReInvite(requestEvent);
                return;

            }

            if (Gateway.getGlobalAddress() == null) {
                if ( logger.isDebugEnabled() ) logger.debug("Global address not available -- cannot process request");
                Response response = SipUtilities.createResponse(serverTransaction,
                        Response.SERVICE_UNAVAILABLE);
                response
                        .setReasonPhrase("SIPXBRIDGE Unable to resolve public address using stun to "
                                + Gateway.getBridgeConfiguration().getStunServerAddress());
                serverTransaction.sendResponse(response);
                return;

            }

            BackToBackUserAgent btobua;

            ItspAccountInfo itspAccount = null;

            if ( provider == Gateway.getLanProvider() ) {
                itspAccount = Gateway.getAccountManager().getAccount(request);
            } else {
                 String viaHost = SipUtilities.getViaHost(request);
                 int viaPort = SipUtilities.getViaPort(request);
                 if ( viaPort == -1 ) {
                     viaPort = 5060;
                 }
                 itspAccount = Gateway.getAccountManager().getItspAccount(viaHost, viaPort);
            }

            /*
             * Look at the Dialog context. The B2BUA structure tracks the call and is pointed to
             * by the dialog application data.
             */
            if ((DialogContext) dialog.getApplicationData() != null) {
                btobua = ((DialogContext) dialog.getApplicationData()).getBackToBackUserAgent();
            } else if (request.getHeader(ReplacesHeader.NAME) != null) {

                /*
                 * Incoming INVITE has a call id that we don't know about but with a replaces
                 * header. This implies call pickup attempt.
                 */

                ReplacesHeader replacesHeader = (ReplacesHeader) request
                        .getHeader(ReplacesHeader.NAME);
                Dialog replacesDialog = ((SipStackExt) ProtocolObjects.getSipStack())
                        .getReplacesDialog(replacesHeader);
                if (replacesDialog == null) {
                    Response response = SipUtilities.createResponse(serverTransaction,
                            Response.NOT_FOUND);
                    response.setReasonPhrase("Dialog Not Found");
                    serverTransaction.sendResponse(response);
                    return;
                }
                BackToBackUserAgent b2bua = DialogContext.getBackToBackUserAgent(replacesDialog);
                DialogContext dat = DialogContext.get(replacesDialog);
                DialogContext newDialogContext = DialogContext.attach(b2bua, dialog, serverTransaction, request);
                newDialogContext.setItspInfo(itspAccount);

                b2bua.addDialog(newDialogContext);

                Dialog peerDialog = dat.getPeerDialog();
                if ( logger.isDebugEnabled() ) logger.debug("replacesDialogState = " + replacesDialog.getState());


                DialogContext.pairDialogs(dialog, peerDialog);

                b2bua.handleInviteWithReplaces(requestEvent, replacesDialog, serverTransaction);
                return;

            } else {

                btobua = Gateway.getBackToBackUserAgentFactory().getBackToBackUserAgent(provider,
                        request, serverTransaction, dialog);
                /*
                 * Make sure we know about the incoming request. Otherwise we return an error
                 * here.
                 */
                if (btobua == null) {
                    if ( SipUtilities.getToTag(request) == null ) {
                        Response response = SipUtilities.createResponse(serverTransaction,
                                Response.NOT_FOUND);
                        response.setReasonPhrase("No record of ITSP. Check configuration.");
                        serverTransaction.sendResponse(response);
                        return;
                    } else {
                        Response response = SipUtilities.createResponse(serverTransaction,
                                Response.CALL_OR_TRANSACTION_DOES_NOT_EXIST);
                        response.setReasonPhrase("No record of ITSP. Check configuration.");
                        serverTransaction.sendResponse(response);
                        return;
                    }
                } else if (btobua.isPendingTermination()) {
                    Response response = SipUtilities.createResponse(serverTransaction,
                            Response.LOOP_DETECTED);
                    response.setReasonPhrase("Retry detected for declined call.");
                    serverTransaction.sendResponse(response);
                    return;
                }

            }

            /*
             * This method was seen from the LAN side. Create a WAN side association and send the
             * INVITE on its way.
             */
            if (provider == Gateway.getLanProvider()) {
                if ( logger.isDebugEnabled() ) logger.debug("Request received from LAN side");
                String toDomain = null;
                // outbound call. better check for valid account

                 if (itspAccount == null) {
                    return;
                }
                if (itspAccount.getState() == AccountState.INVALID) {
                    Response response = SipUtilities.createResponse(serverTransaction,
                            Response.BAD_GATEWAY);
                    response.setReasonPhrase("Account state is INVALID. Check config.");
                    serverTransaction.sendResponse(response);
                    return;
                }
                /*
                 * This case occurs when in and outbound proxy are different.
                 */

                toDomain = itspAccount.getSipDomain();

                /*
                 * Send the call setup invite out.
                 */
                btobua.sendInviteToItsp(requestEvent, serverTransaction, toDomain);
            } else {
                if ( logger.isDebugEnabled() ) logger.debug("request received from Wan side");
                btobua.sendInviteToSipxProxy(requestEvent, serverTransaction);

            }

        } catch (RuntimeException ex) {
            logger.error("Error processing request" + requestEvent.getRequest(), ex);
            CallControlUtilities.sendInternalError(serverTransaction, ex);
        } catch (Exception ex) {
            logger.error("Error processing request " + requestEvent.getRequest(), ex);
            CallControlUtilities.sendBadRequestError(serverTransaction, ex);
        }
    }

    /**
     * Process an OPTIONS request.
     *
     * @param requestEvent -- the requestEvent for the OPTIONS.
     *
     */
    private void processOptions(RequestEvent requestEvent) {
        if ( logger.isDebugEnabled() ) logger.debug("processOptions");
        SipProvider provider = (SipProvider) requestEvent.getSource();
        Request request = requestEvent.getRequest();
        ServerTransaction st = requestEvent.getServerTransaction();

        try {
            if (st == null || requestEvent.getDialog() == null) {
                /*
                 * discard out of dialog requests.
                 */
                Response response = ProtocolObjects.messageFactory.createResponse(
                        Response.NOT_ACCEPTABLE, request);
                if (st == null) {
                    provider.sendResponse(response);
                } else {
                    st.sendResponse(response);
                }
                return;

            }

            Response response = SipUtilities.createResponse(st, Response.OK);
            ContactHeader contactHeader = SipUtilities.createContactHeader(null, provider,
                    SipUtilities.getViaTransport(response));
            AcceptHeader acceptHeader = ProtocolObjects.headerFactory.createAcceptHeader(
                    "application", "sdp");
            response.setHeader(contactHeader);
            response.setHeader(acceptHeader);
            // Should probably have a configurable option for this.
            Locale locale = Locale.ENGLISH;
            AcceptLanguageHeader acceptLanguage = ProtocolObjects.headerFactory
                    .createAcceptLanguageHeader(locale);
            response.setHeader(acceptLanguage);
            Dialog dialog = requestEvent.getDialog();
            SipUtilities.addAllowHeaders(response, provider);
            if (dialog != null) {
                /*
                 * This is an In-dialog request. We add our session description to the response.
                 */
                DialogContext dat = DialogContext.get(dialog);
                if (dat != null) {
                    RtpSession rtpSession = DialogContext.getRtpSession(dialog);
                    if (rtpSession != null) {
                        SessionDescription sd = rtpSession.getReceiver().getSessionDescription();
                        if (sd != null) {
                            response.setContent(sd.toString(), ProtocolObjects.headerFactory
                                    .createContentTypeHeader("application", "sdp"));
                        }
                    }
                }

            }

            /*
             * If In-Dialog, then the stack will create a server transaction for you to respond
             * stateufully.
             */

            st.sendResponse(response);

        } catch (Exception ex) {
            logger.error("Internal error processing request ", ex);
            try {
                CallControlUtilities.sendInternalError(st, ex);
            } catch (Exception e) {
                throw new SipXbridgeException("Check gateway configuration", e);
            }
        }

    }

    /**
     * Handle inbound REFER request.
     *
     * @param requestEvent
     */
    private void processRefer(RequestEvent requestEvent) {
        if ( logger.isDebugEnabled() ) logger.debug("processRefer");
        TransactionContext tad = null;

        BackToBackUserAgent btobua = null;
        try {

            if ( logger.isDebugEnabled() ) logger.debug("Got a REFER - establishing new call leg and tearing down old call leg");
            Dialog dialog = requestEvent.getDialog();
            Request request = requestEvent.getRequest();
            SipProvider provider = (SipProvider) requestEvent.getSource();
            ServerTransaction stx = requestEvent.getServerTransaction();

            /*
             * ONLY in-dialog stateful REFER handling is allowed.
             */
            if (dialog == null || stx == null) {
                logger.error("Got an out of dialog REFER -- dropping");
                Response response = ProtocolObjects.messageFactory.createResponse(
                        Response.NOT_IMPLEMENTED, request);
                response.setReasonPhrase("Can only handle In dialog REFER");
                if (stx != null)
                    stx.sendResponse(response);
                else
                    provider.sendResponse(response);
                return;
            }
            if (provider != Gateway.getLanProvider()) {
                /*
                 * For now do not accept refer from the WAN side later we can relax this
                 * restriction.
                 */
                Response response = SipUtilities.createResponse(stx, Response.NOT_ACCEPTABLE);
                response.setReasonPhrase("Can only handle REFER from Pbx");
                stx.sendResponse(response);
                return;
            }

            DialogContext dat = (DialogContext) dialog.getApplicationData();
            btobua = dat.getBackToBackUserAgent();
            /*
             * Check dialog state -- if terminated, tear down call.This should never happen unless
             * our Dialog is Terminated when we retry.
             */
            if (dialog.getState() == DialogState.TERMINATED) {
                Response response = SipUtilities.createResponse(stx,
                        Response.SERVER_INTERNAL_ERROR);
                response.setReasonPhrase("Received REFER on TERMINATED Dialog");
                stx.sendResponse(response);
                return;
            }

            /*
             * This flag controls whether or not we forward the BYE when the refer agent tears
             * down his end of the dialog.
             */
            DialogContext.get(dialog).setForwardByeToPeer(false);
            DialogContext.get(dialog).setReferRequest(request);

            /*
             * Blind transfer handled by ITSP? If so then forward it if the bridge is configured
             * to do so. With out ITSP support, blind transfer can result in no RINGING and
             * dropped calls (if we handle it locally). Note : REFER is not widely supported by
             * ITSPs Gateway.getBridgeConfiguration.isReferForwarded() is always returning FALSE
             * for now. We will enable this code when ITSPs become better about REFER processing.
             */
            Dialog peerDialog = DialogContext.getPeerDialog(dialog);

            DialogContext peerDat = DialogContext.get(peerDialog);

            if (Gateway.getBridgeConfiguration().isReferForwarded()
                    && !SipUtilities.isReplacesHeaderPresent(requestEvent.getRequest())
                    && peerDat.isReferAllowed()) {
                btobua.forwardReferToItsp(requestEvent);
                return;
            }

            /*
             * Re-INVITE the refer Target.
             *
             * The ITSP supports re-invite. Send him a Re-INVITE to solicit an offer. So we can
             * determine what Codec he supports.
             */
            Request inviteRequest = btobua.createInviteFromReferRequest(requestEvent);
            Response tryingResponse = SipUtilities.createResponse(stx, 100);
            stx.sendResponse(tryingResponse);
            ReferInviteToSipxProxyContinuationData continuation = new ReferInviteToSipxProxyContinuationData(
                    inviteRequest, requestEvent);
            dat.solicitSdpOfferFromPeerDialog(continuation,requestEvent.getRequest());

        } catch (ParseException ex) {
            // This should never happen
            logger.fatal("Internal error constructing message ", ex);
            throw new SipXbridgeException("Internal error", ex);

        } catch (SymmitronException ex) {
            logger.error("An error occured talking to sipxrelay ", ex);
            logger.error("Unexpected exception while processing REFER", ex);
            if (tad != null) {
                ServerTransaction serverTransaction = tad.getServerTransaction();
                CallControlUtilities.sendServiceUnavailableError(serverTransaction, ex);
            }
            if (btobua != null) {
                btobua.tearDown();
            }
        } catch (InvalidArgumentException ex) {
            logger.fatal("Internal error -- invalid argument", ex);
            throw new SipXbridgeException("Internal error", ex);
        } catch (Exception ex) {
            logger.error("Unexpected exception while processing REFER", ex);
            if (tad != null) {
                ServerTransaction serverTransaction = tad.getServerTransaction();
                CallControlUtilities.sendBadRequestError(serverTransaction, ex);
            }

            if (btobua != null) {
                btobua.tearDown();
            }

        }

    }

    /**
     * Process an incoming PRACK.
     *
     * @param requestEvent -- the PRACK request event.
     */
    private void processPrack(RequestEvent requestEvent) {
        try {
            SipProvider provider = (SipProvider) requestEvent.getSource();
            if ( logger.isDebugEnabled() ) logger.debug("processPRACK");

            ServerTransaction serverTransactionId = requestEvent.getServerTransaction();
            /*
             * send 200 OK for PRACK
             */
            Response prackOk = SipUtilities.createResponse(serverTransactionId, Response.OK);

            Dialog dialog = requestEvent.getDialog();
            DialogContext dialogContext = DialogContext.get(dialog);
            ContactHeader requestContactHeader = (ContactHeader) requestEvent.getRequest().getHeader(ContactHeader.NAME);
            String contactUser;   
            if (requestContactHeader != null) {
                SipURI contactURI = (SipURI) requestContactHeader.getAddress().getURI();
                contactUser = contactURI.getUser();
            }  else {
                contactUser = Gateway.SIPXBRIDGE_USER;
            }
            
           
            ItspAccountInfo itspAccount = dialogContext.getItspInfo();
              ContactHeader contactHeader = SipUtilities.createContactHeader(provider, itspAccount,contactUser, serverTransactionId);
            prackOk.setHeader(contactHeader);

            serverTransactionId.sendResponse(prackOk);

        } catch (Exception ex) {
            logger.error("problem sending OK to PRACK", ex);
        }
    }

    /**
     * Processes an INCOMING ack.
     *
     * @param requestEvent -- the ACK request event.
     */
    private void processAck(RequestEvent requestEvent) {
        if ( logger.isDebugEnabled() ) logger.debug("processAck");
        try {
            BackToBackUserAgent btobua = DialogContext.getBackToBackUserAgent(requestEvent
                    .getDialog());

            if (btobua == null) {
                if ( logger.isDebugEnabled() ) logger.debug("Could not find B2BUA -- not forwarding ACK ");
                return;
            }
            DialogContext dialogContext = (DialogContext) requestEvent.getDialog()
                    .getApplicationData();

            Dialog peerDialog = dialogContext.getPeerDialog();

            if ( logger.isDebugEnabled() ) logger.debug(String.format("processAck: Dialog/peerDialog  = %s/%s",requestEvent.getDialog(),peerDialog));
            /*
             * Forward the ACK if we have not already done so.
             */

            Request inboundAck = requestEvent.getRequest();

            SipProvider provider = (SipProvider) requestEvent.getSource();


            if (peerDialog == null) {
                if ( logger.isDebugEnabled() ) logger.debug("Could not find peer dialog -- not forwarding ACK!");
                if ( SipUtilities.isOriginatorSipXbridge(inboundAck) && provider == Gateway.getLanProvider()) {
                    if ( logger.isDebugEnabled() ) logger.debug("The ack was originated by sipxbridge - tearing down call leg");
                    btobua.addDialogToCleanup(requestEvent.getDialog());
                }
                return;
            }


            DialogContext peerDialogContext = (DialogContext) peerDialog.getApplicationData();

            if (logger.isDebugEnabled()) {
                logger.debug("peerDialog = " + peerDialog + " peerDialogState = " + peerDialog.getState());
                logger.debug("dialogContext : " + dialogContext + " peerDialogContext " + peerDialogContext);
                if (peerDialogContext != null ) {
                    logger.debug("peerDialogContext.lastResponse " + peerDialogContext.getLastResponse());
                    logger.debug("peerDialogContext.pendingAction = " + peerDialogContext.getPendingAction());
                } else {
                    logger.debug("peerDialogContext is null " );
                }
                if ( dialogContext != null ) {
                    logger.debug("dialogContext.pendingAction " + dialogContext.getPendingAction());
                }
            }

            if (peerDialogContext != null
                    && peerDialog.getState() == DialogState.CONFIRMED
                    && peerDialogContext.getLastResponse() != null
                    && peerDialogContext.getLastResponse().getStatusCode() == Response.OK
                    && ((CSeqHeader) peerDialogContext.getLastResponse().getHeader(
                            CSeqHeader.NAME)).getMethod().equals(Request.INVITE)) {
                if ( logger.isDebugEnabled() ) logger.debug("createAck: " + peerDialog);

                /*
                 * This case happens in loopback calls. We can query sdp from a peer that is in
                 * the pbx.
                 */

                Request ack;
                ContentTypeHeader contentTypeHeader = (ContentTypeHeader) inboundAck.getHeader(ContentTypeHeader.NAME);

                String contentType =  contentTypeHeader != null ? contentTypeHeader.getContentType() : null ;
                String contentSubType =  contentTypeHeader != null ? contentTypeHeader.getContentSubType() : null;

                if (inboundAck.getContentLength().getContentLength() != 0 &&
                       contentType.equalsIgnoreCase("application") &&
                       contentSubType.equalsIgnoreCase("sdp")) {
                    if (peerDialogContext.getPendingAction() == PendingDialogAction.PENDING_FORWARD_ACK_WITH_SDP_ANSWER) {
                        /*
                         * ACK had a content length so we extract the sdp answer, we re-write it
                         * and forward it.
                         */
                        ack = peerDialog.createAck(SipUtilities.getSeqNumber(peerDialogContext
                                .getLastResponse()));


                        ContentTypeHeader cth = ProtocolObjects.headerFactory
                                .createContentTypeHeader("application", "sdp");
                        SessionDescription sd = SipUtilities.getSessionDescription(inboundAck);
                        /*
                         * XX-5709 set the destination so we know where to expect RTP from.
                         */
                        String ipAddress = SipUtilities.getSessionDescriptionMediaIpAddress(sd);
                        int port = SipUtilities.getSessionDescriptionMediaPort(sd);
                        dialogContext.getRtpSession().getTransmitter().setIpAddressAndPort(ipAddress, port);

                        if ( dialogContext.getRtpSession().isHoldRequest(sd) ) {
                            dialogContext.getRtpSession().getTransmitter().setOnHold(true);
                        } else {
                            dialogContext.getRtpSession().getTransmitter().setOnHold(false);
                        }

                        SipUtilities.incrementSessionVersion(sd);
                        peerDialogContext.getRtpSession().getReceiver().setSessionDescription(sd);
                        /*
                         * We want to play MOH when the local party is on hold so always
                         * set the attribute to sendrecv when sending ACK to itsp.
                         */
                        SipUtilities.setDuplexity(sd, "sendrecv");

                        ack.setContent(sd.toString(), cth);
                     } else {
                        logger
                                .error("Got an ACK with SDP but other side does not expect it -- not forwarding ACK");
                        dialogContext.getBackToBackUserAgent().tearDown(Gateway.SIPXBRIDGE_USER,ReasonCode.PROTOCOL_ERROR,"Unexpected ACK with SDP ");
                        return;
                    }
                } else {
                    /*
                     * Inbound ack had no sdp answer. so we just replay the old sdp back. This ACK
                     * came back as a result of codec negotiation failure. This is another HACK to
                     * try to support ITSPs that do not respond correctly to SDP offer
                     * solicitations.
                     */

                    if (peerDialogContext.getPendingAction() == PendingDialogAction.PENDING_FORWARD_ACK_WITH_SDP_ANSWER) {
                        /*
                         * The content length is 0. There is no answer but the other side expects
                         * one. Just silently return.
                         */
                        if ( logger.isDebugEnabled() ) logger.debug("no SDP body in ACK but the other side expects one");
                        return;

                    } else if (peerDialogContext.getPendingAction() == PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK) {
                        if ( logger.isDebugEnabled() ) logger.debug("Pending SDP Answer in ACK  -- not forwarding inbound ACK. Will forward later when OK comes in");
                        return;

                    } else {
                        /*
                         * There is no answer and no last response and the other side does NOT
                         * expect one. This is just the default forwarding action.
                         */
                        ack = peerDialog.createAck(SipUtilities.getSeqNumber(peerDialogContext
                                .getLastResponse()));
                    }
                }

                if ( ack.getContentLength().getContentLength() == 0 &&
                     inboundAck.getContentLength().getContentLength() != 0) {
                    byte[] content = inboundAck.getRawContent();
                    ack.setContent(content, contentTypeHeader);
                }

                DialogContext.get(peerDialog).sendAck(ack);



                /*
                 * Setting this to null here handles the case of Re-invitations.
                 */
                peerDialogContext.setLastResponse(null);


                /*
                 * Set the pending flag to false.
                 */
                peerDialogContext.setPendingAction(PendingDialogAction.NONE);

            } else if (peerDialogContext != null &&  peerDialog.getState() == DialogState.CONFIRMED) {


                 if (peerDialogContext.getPendingAction() == PendingDialogAction.PENDING_SOLICIT_SDP_OFFER_ON_ACK) {
                    dialogContext.setPendingAction(PendingDialogAction.PENDING_RE_INVITE_WITH_SDP_OFFER);
                     peerDialogContext.setPendingAction(PendingDialogAction.NONE);
                    dialogContext.solicitSdpOfferFromPeerDialog(null,requestEvent.getRequest());
                 } else  if ( dialogContext.getItspInfo() != null &&
                         peerDialogContext.getItspInfo() != null ) {
                     String contextId = SipUtilities.getDialogContextId(requestEvent.getRequest());
                     if (contextId != null && DialogContext.getDialogContext(contextId) != null ) {
                    	 Dialog inboundDialog = DialogContext.getDialogContext(contextId).getPeerDialog();
                    	 BackToBackUserAgent b2bua = dialogContext.getBackToBackUserAgent();
                    	 b2bua.addDialogToCleanup(dialogContext.getDialog());
                    	 b2bua.addDialogToCleanup(DialogContext.getPeerDialog(inboundDialog));
                    	 DialogContext.pairDialogs(inboundDialog, peerDialogContext.getDialog());
                    	 if (! dialogContext.getItspInfo().isAlwaysRelayMedia() &&
                    			 ! peerDialogContext.getItspInfo().isAlwaysRelayMedia() &&
                    			 dialogContext.getItspInfo() == peerDialogContext.getItspInfo()
                    			 && SipUtilities.getDialogContextId(requestEvent.getRequest()) != null ) {
                    		 /*
                    		  * We now re-INVITE to shuffle the SDP and get them to point at each other.
                    		  */
                    		 Request reInvite = peerDialogContext.getDialog().createRequest(Request.INVITE);
                    		 ReferencesHeader referencesHeader =
                    			 SipUtilities.createReferencesHeader(requestEvent.getRequest(),ReferencesHeader.CHAIN);
                    		 reInvite.setHeader(referencesHeader);
                    		 peerDialogContext.setPendingAction(PendingDialogAction.PENDING_RE_INVITE_WITH_SDP_OFFER);
                    		 peerDialogContext.solicitSdpOfferFromPeerDialog(null,requestEvent.getRequest());
                    	 }
                     }
                 }
            }
        } catch (Exception ex) {
            logger.error("Problem sending ack ", ex);
        }

    }

    /**
     * Processes an INCOMING CANCEL.
     *
     * @param requestEvent -- the inbound CANCEL.
     *
     */
    private void processCancel(RequestEvent requestEvent) {
        if ( logger.isDebugEnabled() ) logger.debug("processCancel");

        Dialog dialog = requestEvent.getDialog();

        try {
            Response cancelOk = SipUtilities.createResponse(requestEvent.getServerTransaction(),
                    Response.OK);
            requestEvent.getServerTransaction().sendResponse(cancelOk);

            if (requestEvent.getServerTransaction() == null) {
                if ( logger.isDebugEnabled() ) logger.debug("Null ServerTx: Late arriving cancel");
                return;
            }
            ServerTransaction inviteServerTransaction = ((SIPServerTransaction) requestEvent
                    .getServerTransaction()).getCanceledInviteTransaction();

            if (inviteServerTransaction.getState() != TransactionState.PROCEEDING) {

                // Too late to cancel.
                if ( logger.isDebugEnabled() ) logger.debug(String.format("Transaction State is %s too late to cancel",
                        inviteServerTransaction.getState()));
                return;
            }
            TransactionContext tad = (TransactionContext) inviteServerTransaction
                    .getApplicationData();
            if (tad == null) {
                logger.error("No transaction application context state found -- returning");
                return;
            }
            ClientTransaction ct = tad.getClientTransaction();
            ItspAccountInfo itspAccount = DialogContext.get(dialog).getItspInfo();

            String transport = itspAccount != null ? itspAccount.getOutboundTransport()
                    : Gateway.DEFAULT_ITSP_TRANSPORT;
            /*
             * Look at the state of the peer client transaction. If in calling or proceeding
             * state we can process the CANCEL. If not we have to decline it.
             */
            if (ct.getState() == TransactionState.CALLING
                    || ct.getState() == TransactionState.PROCEEDING) {
                Request cancelRequest = ct.createCancel();

                SipProvider provider = SipUtilities.getPeerProvider((SipProvider) requestEvent
                        .getSource(), transport);
                ClientTransaction clientTransaction  = null;
                try {
                    clientTransaction = provider
                        .getNewClientTransaction(cancelRequest);
                } catch (TransactionUnavailableException ex) {
                    if ( logger.isDebugEnabled() ) logger.debug("Cancel Already in progress -- returning silrently");
                    return;
                }
                clientTransaction.sendRequest();
                /*
                 * In case the dialog ever goes into a Confirmed state, it will be killed right
                 * away by sending a BYE after an ACK is sent.
                 */
                Dialog peerDialog = DialogContext.getPeerDialog(dialog);
                if (peerDialog != null && peerDialog.getState() != DialogState.CONFIRMED) {
                    DialogContext.get(peerDialog).setTerminateOnConfirm();
                }
                /*
                 * Send the Request TERMINATED response right away. We are done with the server
                 * transaction at this point. This REQUEST_TERMINATED response must be sent back
                 * to the ITSP here ( we cannot wait for the other end to send us the response --
                 * see XX-517 workaround. )
                 */
                Response requestTerminatedResponse = SipUtilities.createResponse(
                        inviteServerTransaction, Response.REQUEST_TERMINATED);

                inviteServerTransaction.sendResponse(requestTerminatedResponse);
            } else {
                if ( logger.isDebugEnabled() ) logger.debug("CallControlManager:processCancel -- too late to CANCEL " + ct.getState());
            }
        } catch (Exception ex) {
            logger.error("Unexpected exception processing cancel", ex);
        }

    }

    /**
     * Processes an INCOMING BYE
     */
    private void processBye(RequestEvent requestEvent) {
        if ( logger.isDebugEnabled() ) logger.debug("processBye");
        try {
            BackToBackUserAgent b2bua = DialogContext.getBackToBackUserAgent(requestEvent
                    .getDialog());

            if (requestEvent.getServerTransaction() != null) {
                if ( logger.isDebugEnabled() ) logger.debug("serverTransaction Not found -- stray request -- discarding ");
            }

            if (b2bua != null) {

                b2bua.processBye(requestEvent);

            } else {
                // Respond with an error
                Response response = SipUtilities.createResponse(requestEvent
                        .getServerTransaction(), Response.CALL_OR_TRANSACTION_DOES_NOT_EXIST);
                requestEvent.getServerTransaction().sendResponse(response);
            }

        } catch (Exception ex) {
            logger.error("Problem sending bye", ex);
        }

    }

    // ///////////////////////////////////////////////////////////////////////////////////////////////
    // Response Handlers
    // ///////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *
     * Handle an ERROR response.
     *
     * @param responseEvent -- the incoming error response event.
     *
     */
    private void inviteErrorResponse(ResponseEvent responseEvent) throws Exception {
        /*
         * Grab the response from the IB response event.
         */

        Response response = responseEvent.getResponse();

        /*
         * Dialog for the response.
         */
        Dialog dialog = responseEvent.getDialog();

        /*
         * The dialog context associated with this dialog.
         */
        DialogContext dialogContext = DialogContext.get(dialog);

        /*
         * Set the last response.
         */
        dialogContext.setLastResponse(response);

        /*
         * The call context from the Dialog context.
         */

        BackToBackUserAgent b2bua = dialogContext.getBackToBackUserAgent();

        if (responseEvent.getClientTransaction() == null) {
            logger.warn("null client transaction");
            return;
        }

        b2bua.sendByeToMohServer();

        if ( logger.isDebugEnabled() ) logger.debug("Processing ERROR Response " + response.getStatusCode());

        // Processing an error resonse.
        ClientTransaction ct = responseEvent.getClientTransaction();
        TransactionContext transactionContext = (TransactionContext) ct.getApplicationData();
        if (transactionContext != null) {
            ServerTransaction serverTransaction = transactionContext.getServerTransaction();
            if ( transactionContext.getOperation() == Operation.SEND_INVITE_TO_ITSP ||
                    transactionContext.getOperation() == Operation.SEND_INVITE_TO_SIPX_PROXY) {
                /*
                 * This is a redirect response we want to get rid of this record
                 * right now so we can retry with a new relay. Otherwise we
                 * find the same record cached and wind up with two relays and
                 * extraneous media which scares phones. TODO - clean up this code
                 * by moving it into the block below.
                 */
                if ( response.getStatusCode() /100 == 3 ) {
                    b2bua.tearDownNow();
                }
            }
            /*
            * The continuation operation is what determines what we have to do next after we
            * get that response (i.e. the next state in the FSM ).
            * If we have a continuation operation, we send the error down that path.
            */
            Operation continuationOperation = transactionContext.getContinuationOperation();

            if (continuationOperation != Operation.NONE) {
                ContinuationData cdata = transactionContext.getContinuationData();
                Response errorResponse = SipUtilities.createErrorResponse(cdata.getRequestEvent()
                        .getServerTransaction(), dialogContext.getItspInfo(), response);
                if (cdata.getRequestEvent().getServerTransaction().getState() != TransactionState.TERMINATED) {
                    cdata.getRequestEvent().getServerTransaction().sendResponse(errorResponse);
                }
            }


            if (transactionContext.getOperation() == Operation.CANCEL_REPLACED_INVITE
                    || transactionContext.getOperation() == Operation.CANCEL_MOH_INVITE
                    || transactionContext.getOperation() == Operation.SEND_INVITE_TO_MOH_SERVER) {
                if ( logger.isDebugEnabled() ) logger.debug("ingoring 4xx response " + transactionContext.getOperation());
            } else if (transactionContext.getOperation() != Operation.REFER_INVITE_TO_SIPX_PROXY) {
                // Is the dialog marked as one that needs an SDP answer in ACK? If so replay the old SDP answer
                if (serverTransaction != null) {
                    if (serverTransaction.getState() != TransactionState.TERMINATED) {
                        if ((transactionContext.getOperation().equals(Operation.SEND_INVITE_TO_SIPX_PROXY)
                                || transactionContext.getOperation().equals(Operation.SEND_INVITE_TO_ITSP))
                                && (response.getStatusCode() / 100 == 6 || response.getStatusCode() / 100 == 5)) {
                            b2bua.setPendingTermination(true);

                        }
                        Response newResponse = SipUtilities.createErrorResponse(serverTransaction,
                                dialogContext.getItspInfo(),response);
                        serverTransaction.sendResponse(newResponse);
                    } else {
                        logger
                                .error("Received an error response after final response sent -- ignoring the response");
                    }
                } else {
                    b2bua.tearDown(ProtocolObjects.headerFactory.createReasonHeader(
                            Gateway.SIPXBRIDGE_USER, ReasonCode.CALL_SETUP_ERROR,
                            "CallControlManager Call transfer error"));
                }
            } else {
                Dialog referDialog = transactionContext.getReferingDialog();
                Request referRequest = transactionContext.getReferRequest();
                if (referDialog != null && referDialog.getState() == DialogState.CONFIRMED
                        && SipUtilities.isOriginatorSipXbridge(response)) {
                    this.notifyReferDialog(referRequest, referDialog, response);

                }
                /*
                 * Handle the case of 404 not found. If the other end is expecting an SDP answer
                 * we replay the previous SDP offer in the answer.
                 */
                DialogContext peerDialogContext = DialogContext.getPeerDialogContext(ct.getDialog());
                if ( peerDialogContext.getPendingAction() == PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK) {
                    CallControlUtilities.sendSdpAnswerInAck(DialogContext.getPeerDialog(ct.getDialog()),null);
                }
                /*
                 * Tear down the call if the response status code requires for Should this also
                 * include 5xx and 6xx responses from the transfer target?
                 */
                if ((referDialog == null || referDialog.getState() != DialogState.CONFIRMED)
                        && SipUtilities.isOriginatorSipXbridge(response)) {
                    b2bua.tearDown(ProtocolObjects.headerFactory.createReasonHeader(
                            Gateway.SIPXBRIDGE_USER, ReasonCode.CALL_SETUP_ERROR,
                            "CallControlManager received " + response.getStatusCode()
                                    + " from transfer target."));
                }
            }
        }
    }


    /**
     * Response handling for response received from ITSP or sipx proxy.
     *
     * @param responseEvent
     * @throws Exception
     */

    private void inviteToItspOrProxyResponse(ResponseEvent responseEvent) throws Exception {
        if ( logger.isDebugEnabled() ) logger.debug("inviteToItspOrProxyResponse");

        Dialog dialog = responseEvent.getDialog();
        DialogContext dialogContext = DialogContext.get(dialog);
        ClientTransaction ctx = null;
        if ( responseEvent.getClientTransaction() == null ) {
            ctx = (ClientTransaction) DialogContext.get(dialog).getDialogCreatingTransaction();
        } else {
            ctx = responseEvent.getClientTransaction();
        }
        Response response = responseEvent.getResponse();
        TransactionContext transactionContext = TransactionContext.get(ctx);
        BackToBackUserAgent b2bua = dialogContext.getBackToBackUserAgent();

        if (logger.isDebugEnabled()) {
            logger.debug("dialogContext = " + dialogContext);
            logger.debug("dialogPeer = " + DialogContext.getPeerDialog(dialog));
            logger.debug("dialog  = " + dialog);
            logger.debug("b2bua = " + b2bua);
        }
        /*
         * Store away our incoming response - get ready for ACKL
         */
        dialogContext.setLastResponse(response);

        /*
         * Now send the response to the server side of the transaction.
         */
        ServerTransaction serverTransaction = transactionContext.getServerTransaction();

        Response newResponse = ProtocolObjects.messageFactory.createResponse(response
                .getStatusCode(), serverTransaction.getRequest());

        ToHeader toHeader = (ToHeader) transactionContext.getServerTransaction().getRequest()
                .getHeader(ToHeader.NAME);

        String user = ((SipURI) toHeader.getAddress().getURI()).getUser();
        ContactHeader contactHeader = null;

        /*
         * Set the contact address for the OK. Note that ITSP may want global addressing.
         */
        if (transactionContext.getOperation() == Operation.SEND_INVITE_TO_ITSP) {
            contactHeader = SipUtilities.createContactHeader(user,
                    transactionContext.getServerTransactionProvider(),
                    SipUtilities.getViaTransport(newResponse));
        } else {
            contactHeader = SipUtilities.createContactHeader(transactionContext
                    .getServerTransactionProvider(), transactionContext.getItspAccountInfo(), null, serverTransaction);
        }

        newResponse.setHeader(contactHeader);
        ToHeader newToHeader = (ToHeader) newResponse.getHeader(ToHeader.NAME);
        String toTag = transactionContext.createToTag();

        newToHeader.setTag(toTag);

        /*
         * Fix up the media session using the port in the incoming sdp answer.
         */
        ContentTypeHeader cth = (ContentTypeHeader) response.getHeader(ContentTypeHeader.NAME);

        SessionDescription newSd = null;
        if (response.getRawContent() != null
                && cth.getContentType().equalsIgnoreCase("application")
                && cth.getContentSubType().equalsIgnoreCase("sdp")) {
            /*
             * The incoming media session.
             */
            SessionDescription sessionDescription = SipUtilities.getSessionDescription(response);
            if (logger.isDebugEnabled()) {
                logger.debug("SessionDescription = " + new String(response.getRawContent()));
            }

            /*
             * Get the outbound RTP session.
             */
            RtpSession rtpSession = dialogContext.getRtpSession();
            RtpTransmitterEndpoint hisEndpoint = null;
            if (rtpSession != null) {
                hisEndpoint = rtpSession.getTransmitter();
            } else {
                if ( logger.isDebugEnabled() ) {
                    logger.debug("CallControlManager: inviteToItspOrProxyResponse: null rtpSession");
                    logger.debug("DialogContext " + dialogContext);
                }
            }

            if (hisEndpoint == null) {
                hisEndpoint = new RtpTransmitterEndpoint(rtpSession, b2bua.getSymmitronClient());
                rtpSession.setTransmitter(hisEndpoint);
            } else {
                if ( logger.isDebugEnabled() ) logger.debug("CallControlManager: inviteToItspOrProxyResponse: hisEndpoint != null" );
            }

            KeepaliveMethod keepaliveMethod;

            if (transactionContext.getOperation() == Operation.SEND_INVITE_TO_ITSP) {
                keepaliveMethod = transactionContext.getItspAccountInfo().getRtpKeepaliveMethod();
            } else {
                // Operation is SEND_INVITE_TO_SIPX_PROXY
                // The other end request us to run a session timer.
                final String REFRESHER = "refresher";
                SessionExpires seh = (SessionExpires) serverTransaction.getRequest().getHeader(
                        SessionExpiresHeader.NAME);
                if (seh != null && seh.getParameter(REFRESHER) != null &&
                        seh.getParameter(REFRESHER).equals("uas")) {
                    int timerInterval = seh.getExpires();
                    DialogContext peerDialogContext = DialogContext.getPeerDialogContext(dialog);
                    peerDialogContext.startSessionTimer(timerInterval);
                }
                keepaliveMethod = KeepaliveMethod.NONE;
            }
            hisEndpoint.setKeepAliveMethod(keepaliveMethod);

            hisEndpoint.setSessionDescription(sessionDescription, false);

            Dialog peerDialog = DialogContext.getPeerDialog(dialog);
            RtpReceiverEndpoint incomingEndpoint = DialogContext.get(peerDialog).getRtpSession()
                    .getReceiver();
            newSd = SipUtilities.getSessionDescription(response);

            /*
             * Set and update the session description of the inbound session. This updates the
             * session description.
             */
            incomingEndpoint.setSessionDescription(newSd);

            newResponse.setContent(newSd.toString(), cth);

            transactionContext.getBackToBackUa().getRtpBridge().start();

        } else if (response.getRawContent() != null) {
            // Cannot recognize header.
            logger.warn("content type is not application/sdp");
            String body = new String(response.getRawContent());
            WarningHeader warningHeader = ProtocolObjects.headerFactory.createWarningHeader(
                    Gateway.SIPXBRIDGE_USER, WarningCode.UNRECOGNIZED_CONTENT_TYPE,
                    "Could not recognize content type");
            newResponse.setHeader(warningHeader);
            newResponse.setContent(body, cth);
        }

        /*
         * If the inbound dialog is not our peer, by sending an ACK right away. If he sends a BYE
         * do not forward it. Set a Flag indicating BYE should not be forwarded.
         */
        if (DialogContext.getPeerDialog(serverTransaction.getDialog()) != dialog) {
            if (response.getStatusCode() == 200) {
                Request ackRequest = dialog.createAck(SipUtilities.getSeqNumber(response));
                // DialogContext.get(dialog).setForwardByeToPeer(false);
                DialogContext.get(dialog).setLastResponse(null);
                DialogContext.get(dialog).sendAck(ackRequest);
                DialogContext.pairDialogs(serverTransaction.getDialog(), dialog);
            }
        }
        serverTransaction.sendResponse(newResponse);
    }

    /**
     * Response processing for sendSdpReOffer
     *
     * @param responseEvent
     * @throws Exception
     */
    private void sendSdpReofferResponse(ResponseEvent responseEvent ) throws Exception {
        if ( logger.isDebugEnabled() ) logger.debug("sendSdpReofferResponse");
    	 Dialog dialog = responseEvent.getDialog();
         DialogContext dialogContext = DialogContext.get(dialog);
         Response response = responseEvent.getResponse();
         long seqno = SipUtilities.getSeqNumber(response);
      	 PendingDialogAction pendingOperation = dialogContext.getPendingAction();
      	 if ( logger.isDebugEnabled() ) logger.debug("sendSdpReOfferResponse pendingDialogAction " + pendingOperation);
         if (response.getStatusCode() == Response.OK) {
            RtpSession rtpSession = DialogContext.getRtpSession(dialog);
            SessionDescription inboundSessionDescription = SipUtilities
                    .getSessionDescription(response);
            rtpSession.getTransmitter().setSessionDescription(
                    inboundSessionDescription, false);
            rtpSession.getTransmitter().setOnHold(false);
            Request ack = dialog.createAck(seqno);
            DialogContext.get(dialog).sendAck(ack);
            Dialog peerDialog  = DialogContext.getPeerDialog(dialog);
            /*
             * If we need to send an SDP answer to the peer, then send it.
             */
            if ( DialogContext.getPendingAction(peerDialog) == PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK ) {
               CallControlUtilities.sendSdpAnswerInAck(response, peerDialog);
            }

        }

        return;
    }

    /**
     * Handle the response for the sdp solicitation. This method handles the response to the sdp
     * offer solicitation re-invite.
     *
     * @param responseEvent
     */
    private void forwardSdpSolicitationResponse(ResponseEvent responseEvent) throws Exception {
        if ( logger.isDebugEnabled() ) logger.debug("forwardSdpSolicitationResponse");
        Dialog dialog = responseEvent.getDialog();
        DialogContext dialogContext = DialogContext.get(dialog);

        ClientTransaction ctx = responseEvent.getClientTransaction();
        TransactionContext transactionContext = TransactionContext.get(ctx);
        Response response = responseEvent.getResponse();
        dialogContext.setLastResponse(response);
        ServerTransaction st = transactionContext.getServerTransaction();

        Response newResponse = SipUtilities.createResponse(st, Response.OK);

        if (response.getContentLength().getContentLength() != 0
                && st.getState() != TransactionState.TERMINATED) {
            SessionDescription responseSessionDescription = SipUtilities
                    .getSessionDescription(response);
            /*
             * This is an inbound SDP offer. We record the last offer from the ITSP in the
             * transmitter. This potentially rebinds the ports to the new ports in the offer.
             */
            SessionDescription sdCloned = SipUtilities
                    .cloneSessionDescription(responseSessionDescription);
            /*
             * Fix up the session descriptions.
             */

            DialogContext.getRtpSession(dialog).getTransmitter().setSessionDescription(sdCloned,
                    true);
            DialogContext.getPeerRtpSession(dialog).getReceiver().setSessionDescription(
                    responseSessionDescription);

            SipProvider wanProvider = ((TransactionExt) st).getSipProvider();

            ContactHeader responseContactHeader = (ContactHeader) response.getHeader(ContactHeader.NAME);
            String contactUser;
            if (responseContactHeader != null) {
                SipURI contactURI = (SipURI) responseContactHeader.getAddress().getURI();
                contactUser = contactURI.getUser();
            }
            else {
                contactUser = Gateway.SIPXBRIDGE_USER;
            }
            ContactHeader contactHeader = SipUtilities.createContactHeader(wanProvider,
                    dialogContext.getItspInfo(), contactUser, st);
            ContentTypeHeader cth = ProtocolObjects.headerFactory.createContentTypeHeader(
                    "application", "sdp");

            newResponse.setContent(responseSessionDescription.toString(), cth);
            newResponse.setHeader(contactHeader);
            /*
             * Mark that we should forward the sdp answer in ACK. We expect an ACK with sdp answer
             * on the peer dialog. This is what the other side expects. He wants an ACK with SDP
             * answer in it.
             */
            dialogContext
                    .setPendingAction(PendingDialogAction.PENDING_FORWARD_ACK_WITH_SDP_ANSWER);
            st.sendResponse(newResponse);
        } else if ( response.getContentLength().getContentLength() == 0 ) {
            /*
             * The ITSP returned an error. Send a dummy SDP answer back to the
             * ITSP. This is an ITSP error.
             */
            CallControlUtilities.sendSdpAnswerInAck(response, dialog);
        }
    }

    /**
     * INVITE with Replaces response handling.
     *
     * @param responseEvent
     * @throws Exception
     */
    private void handleInviteWithReplacesResponse(ResponseEvent responseEvent) throws Exception {
        if ( logger.isDebugEnabled() ) logger.debug("handleInviteWithReplacesResponse");

        Dialog dialog = responseEvent.getDialog();
        DialogContext dialogContext = DialogContext.get(dialog);
        ClientTransaction ctx = responseEvent.getClientTransaction();
        TransactionContext tad = TransactionContext.get(ctx);
        Response response = responseEvent.getResponse();

        dialogContext.setLastResponse(response);
        ServerTransaction serverTransaction = tad.getServerTransaction();
        Dialog replacedDialog = tad.getReplacedDialog();

        SipProvider peerProvider = ((TransactionExt) serverTransaction).getSipProvider();
        Response serverResponse = SipUtilities.createResponse(serverTransaction, response
                .getStatusCode());
        
        //Extract contact header user name from response.
        ContactHeader responseContactHeader = (ContactHeader) response.getHeader(ContactHeader.NAME);
        String contactUser;
        if (responseContactHeader != null) {
            SipURI contactURI = (SipURI) responseContactHeader.getAddress().getURI();
            contactUser = contactURI.getUser();
        } else {
            contactUser = Gateway.SIPXBRIDGE_USER;
        }
        ContactHeader contactHeader = SipUtilities.createContactHeader(contactUser,
                peerProvider, SipUtilities.getViaTransport(serverResponse));
        serverResponse.setHeader(contactHeader);

        if (response.getContentLength().getContentLength() != 0) {
            SessionDescription sdes = SipUtilities.getSessionDescription(response);
            SessionDescription sdesCloned = SipUtilities.getSessionDescription(response);

            RtpSession inboundRtpSession = DialogContext.getRtpSession(dialog);
            inboundRtpSession.getTransmitter().setSessionDescription(sdesCloned, false);

            RtpSession outboundRtpSession = DialogContext.get(replacedDialog).getRtpSession();
            outboundRtpSession.getTransmitter().setOnHold(false);

            outboundRtpSession.getReceiver().setSessionDescription(sdes);
            ContentTypeHeader cth = ProtocolObjects.headerFactory.createContentTypeHeader(
                    "application", "sdp");
            serverResponse.setContent(sdes.toString(), cth);
        }

        /*
         * Bid adieu to the replaced dialog if we have not already done so. Since this might
         * already have been done, we schedule a timer to do so in due course.
         */
        if (response.getStatusCode() == Response.OK) {
            Gateway.getTimer().schedule(new TearDownReplacedDialogTimerTask(replacedDialog),
                    30 * 1000);
        }

        /*
         * accept the dialog that replaces this dialog.
         */
        DialogContext serverDat = DialogContext.get(serverTransaction.getDialog());
        serverDat.setPeerDialog(dialog);
        serverTransaction.sendResponse(serverResponse);
        serverDat.setRtpSession(DialogContext.get(replacedDialog).getRtpSession());

        if (replacedDialog.getState() != DialogState.TERMINATED) {
            DialogContext replacedDat = DialogContext.get(replacedDialog);
            replacedDat.setRtpSession(null);
            replacedDat.setPeerDialog(null);
        }
    }

    /**
     * Response from a re-invite forwarding
     *
     * @param responseEvent - inbound response event.
     *
     */
    private void forwardReInviteResponse(ResponseEvent responseEvent) throws Exception {
        if ( logger.isDebugEnabled() ) logger.debug("forwardReInviteResponse");

        Dialog dialog = responseEvent.getDialog();
        DialogContext dialogContext = DialogContext.get(dialog);
        ClientTransaction ctx = responseEvent.getClientTransaction();
        TransactionContext transasctionContext = TransactionContext.get(ctx);
        Response response = responseEvent.getResponse();


        /*
         * Store away our incoming response - get ready for sending ACK.
         */
        dialogContext.setLastResponse(response);

        /*
         * Now send the respose to the server side of the transaction.
         */
        ServerTransaction serverTransaction = transasctionContext.getServerTransaction();

        /*
         * If we have a server transaction associated with the response, we ack when the other
         * side acks.
         */
        if (serverTransaction != null
                && serverTransaction.getState() != TransactionState.TERMINATED) {
            Response newResponse = SipUtilities.createResponse(serverTransaction, response
                    .getStatusCode());
            //Get the contact from the inbound response and use that for the outbound response
            ContactHeader responseContactHeader = (ContactHeader) response.getHeader(ContactHeader.NAME);
            String contactUser;
            if (responseContactHeader != null) {
                SipURI contactURI = (SipURI) responseContactHeader.getAddress().getURI();
                contactUser = contactURI.getUser();
            } else {
                contactUser = Gateway.SIPXBRIDGE_USER;
            }
            ContactHeader contactHeader = SipUtilities.createContactHeader(
                    contactUser, transasctionContext.getServerTransactionProvider(),
                    SipUtilities.getViaTransport(newResponse));
            newResponse.setHeader(contactHeader);
            Dialog peerDialog = serverTransaction.getDialog();
            SipProvider peerProvider = ((TransactionExt) serverTransaction).getSipProvider();
            if (response.getContentLength().getContentLength() != 0) {

                RtpSession originalRtpSession = DialogContext.getRtpSession(dialog);
                if (originalRtpSession.getTransmitter() != null) {
                    SessionDescription transmitterSd = SipUtilities
                            .getSessionDescription(response);
                    originalRtpSession.getTransmitter().setSessionDescription(transmitterSd,
                            false);
                }

                SessionDescription receiverSd = SipUtilities.getSessionDescription(response);
                ContentTypeHeader cth = ProtocolObjects.headerFactory.createContentTypeHeader(
                        "application", "sdp");
                RtpSession rtpSession = DialogContext.getRtpSession(peerDialog);

                if (peerProvider == Gateway.getLanProvider()) {
                    rtpSession.getReceiver().setUseGlobalAddressing(false);
                } else if (DialogContext.get(peerDialog).getItspInfo() == null
                        || DialogContext.get(peerDialog).getItspInfo().isGlobalAddressingUsed()) {
                    SipUtilities.setGlobalAddress(newResponse);
                } else {
                    rtpSession.getReceiver().setUseGlobalAddressing(false);
                }

                rtpSession.getReceiver().setSessionDescription(receiverSd);
                newResponse.setContent(receiverSd.toString(), cth);
                serverTransaction.sendResponse(newResponse);
            } else {
                /* No SDP returned in the response */
                if (DialogContext.get(peerDialog).getItspInfo() == null
                        || DialogContext.get(peerDialog).getItspInfo().isGlobalAddressingUsed()) {
                    SipUtilities.setGlobalAddress(newResponse);
                }
                serverTransaction.sendResponse(newResponse);
            }
        } else {
            Request ack = dialog.createAck(SipUtilities.getSeqNumber(response));
            DialogContext.get(dialog).sendAck(ack);
        }

    }

    /**
     * Handles responses for ReferInviteToSipxProxy or Blind transfer to ITSP.
     *
     * @param responseEvent
     * @throws Exception
     */
    private void referInviteToSipxProxyResponse(ResponseEvent responseEvent) throws Exception {
        if ( logger.isDebugEnabled() ) logger.debug("referInviteToSipxProxyResponse");

        Dialog dialog = responseEvent.getDialog();
        DialogContext dialogContext = DialogContext.get(dialog);
        ClientTransaction ctx = null;
        if ( responseEvent.getClientTransaction() == null ) {
            ctx = (ClientTransaction) DialogContext.get(dialog).getDialogCreatingTransaction();
        } else {
            ctx = responseEvent.getClientTransaction();
        }
        TransactionContext tad = TransactionContext.get(ctx);
        ClientTransaction mohCtx = TransactionContext.get(ctx).getMohClientTransaction();
        Response response = responseEvent.getResponse();
        BackToBackUserAgent b2bua = DialogContext.getBackToBackUserAgent(dialog);

        /*
         * This is the case of Refer redirection. In this case, we have already established a call
         * leg with transfer agent. We already have a RTP session established with the transfer
         * agent. We need to redirect the outbound RTP stream to the transfer target. To do this,
         * we fix up the media session using the port in the incoming sdp answer.
         * We do not expect that phones will send us multipart Mime.
         */
        ContentTypeHeader cth = (ContentTypeHeader) response.getHeader(ContentTypeHeader.NAME);
        Dialog referDialog = tad.getReferingDialog();
        Request referRequest = tad.getReferRequest();
        Dialog peerDialog = DialogContext.getPeerDialog(dialog);
        DialogContext peerDat = DialogContext.get(peerDialog);

        if (response.getRawContent() != null
                && cth.getContentType().equalsIgnoreCase("application")
                && cth.getContentSubType().equalsIgnoreCase("sdp")) {
            /*
             * The incoming media session.
             */

            SessionDescription sessionDescription = SipUtilities.getSessionDescription(response);

            RtpSession rtpSession = ((DialogContext) referDialog.getApplicationData())
                    .getRtpSession();

            if (rtpSession != null) {

                /*
                 * Note that we are just pointing the transmitter to another location. The
                 * receiver stays as is.
                 */
                rtpSession.getTransmitter().setSessionDescription(sessionDescription, false);
                if (logger.isDebugEnabled()) {
                    logger.debug("Receiver State : " + rtpSession.getReceiverState());
                }

                /*
                 * Grab the RTP session previously pointed at by the REFER dialog.
                 */
                b2bua.getRtpBridge().addSym(rtpSession);

                ((DialogContext) dialog.getApplicationData()).setRtpSession(rtpSession);

                /*
                 * Check if we need to forward that response and do so if needed. see issue 1718
                 */
                if (peerDat.dialogCreatingTransaction != null
                        && peerDat.dialogCreatingTransaction instanceof ServerTransaction
                        && peerDat.dialogCreatingTransaction.getState() != TransactionState.TERMINATED) {

                    Request request = ((ServerTransaction) peerDat.dialogCreatingTransaction)
                            .getRequest();
                    Response forwardedResponse = ProtocolObjects.messageFactory.createResponse(
                            response.getStatusCode(), request);
                    SipUtilities.setSessionDescription(forwardedResponse, sessionDescription);
                    ContactHeader responseContactHeader = (ContactHeader) response.getHeader(ContactHeader.NAME);
                    String contactUser;
                    if (responseContactHeader != null) {
                        SipURI contactURI = (SipURI) responseContactHeader.getAddress().getURI();
                        contactUser = contactURI.getUser();
                    }
                    else {
                        contactUser = Gateway.SIPXBRIDGE_USER;
                    }
                    ContactHeader contact = SipUtilities
                            .createContactHeader(
                                    ((TransactionExt) peerDat.dialogCreatingTransaction)
                                            .getSipProvider(), peerDat.getItspInfo(), contactUser,
                                            (peerDat.dialogCreatingTransaction));
                    forwardedResponse.setHeader(contact);
                    ((ServerTransaction) peerDat.dialogCreatingTransaction)
                            .sendResponse(forwardedResponse);
                } else {
                    if ( logger.isDebugEnabled() ) logger.debug("not forwarding response peerDat.transaction  = "
                            + peerDat.dialogCreatingTransaction);
                }

            } else {
                logger
                        .debug("Processing ReferRedirection: Could not find RtpSession for referred dialog");
            }

        } else if (response.getRawContent() != null) {
            /*
             * Got content but it was not SDP.
             */
            logger
                    .error("Encountered unexpected content type from response - not forwarding response");
            ReasonHeader reasonHeader = ProtocolObjects.headerFactory.createReasonHeader(
                    Gateway.SIPXBRIDGE_USER, ReasonCode.UNEXPECTED_CONTENT_TYPE,
                    "unknown content type encountered");
            dialogContext.getBackToBackUserAgent().tearDown(reasonHeader);
            return;
        }

        /*
         * Got an OK for the INVITE ( that means that somebody picked up ) so we can hang up the
         * call. We have already redirected the RTP media to the redirected party at this point.
         */

        if ( logger.isDebugEnabled()) {
        	logger.debug("referDialog.getState() " + referDialog.getState()
        		+ " isOrignatorSipXbridge " + SipUtilities.isOriginatorSipXbridge(response));
        }
        if (referDialog.getState() == DialogState.CONFIRMED
                && SipUtilities.isOriginatorSipXbridge(response)) {
            /*
             * We terminate the dialog. There is no peer. The REFER agent will send a BYE on
             * notification. We dont want to forward it.
             */
            if (response.getStatusCode() == Response.OK) {
                DialogContext.get(referDialog).setForwardByeToPeer(false);
            }
            /*
             * Send A NOTIFY to the phone. He will tear down the call.
             */
            this.notifyReferDialog(referRequest, referDialog, response);
        }

        /*
         * SDP was returned from the transfer target. In this case we have to either re-offer or
         * send an sdp answer to the other side in an ACK.
         */
        if (response.getContentLength().getContentLength() != 0) {
            if (tad.getDialogPendingSdpAnswer() != null
                    && DialogContext.get(tad.getDialogPendingSdpAnswer()).getPendingAction() == PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK) {
                Dialog dialogToAck = tad.getDialogPendingSdpAnswer();
                tad.setDialogPendingSdpAnswer(null);
                CallControlUtilities.sendSdpAnswerInAck(response, dialogToAck);
            } else if (dialogContext.getPendingAction() == PendingDialogAction.PENDING_RE_INVITE_WITH_SDP_OFFER) {
            	 dialogContext.setPendingAction(PendingDialogAction.NONE);


            	CallControlUtilities.sendSdpReOffer(responseEvent, dialog, peerDialog);

            } else {
                if (logger.isDebugEnabled()) {
                    if ( logger.isDebugEnabled() ) logger.debug("tad.dialogPendingSdpAnswer = "
                            + tad.getDialogPendingSdpAnswer());
                    if (tad.getDialogPendingSdpAnswer() != null) {
                        if ( logger.isDebugEnabled() ) logger.debug("pendingAction = "
                                + DialogContext.get(tad.getDialogPendingSdpAnswer())
                                        .getPendingAction());
                    }
                }
            }

            if (mohCtx != null) {
                /*
                 * Already sent a response or a re-offer back. Alert the delayed MOH timer not to
                 * do anything. When the MOH dialog completes it will immediately Terminate.
                 */

                DialogContext mohDialogContext = DialogContext.get(mohCtx.getDialog());
                mohDialogContext.setTerminateOnConfirm();

            }

        }

        /*
         * We directly send ACK unless the sending of ACK is deferred till OK is received.
         */
        if (response.getStatusCode() == Response.OK &&
        		DialogContext.get(dialog).getPendingAction() != PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK) {
            b2bua.addDialog(DialogContext.get(dialog));
            Request ackRequest = dialog.createAck(((CSeqHeader) response
                    .getHeader(CSeqHeader.NAME)).getSeqNumber());
            DialogContext.get(dialog).sendAck(ackRequest);
        }
        /*
         * If there is a Music on hold dialog -- tear it down
         */

        if (response.getStatusCode() == Response.OK) {
            b2bua.sendByeToMohServer();
        }

    }

    /**
     * Handles responses for ReferInviteToSipxProxy or Blind transfer to ITSP.
     *
     * @param responseEvent
     * @throws Exception
     */
    private void blindTransferToItspResponse(ResponseEvent responseEvent) throws Exception {
    	if ( logger.isDebugEnabled() ) {
    		logger.debug("blindTransferToItspResponse");
    	}

        Dialog dialog = responseEvent.getDialog();
        DialogContext dialogContext = DialogContext.get(dialog);
        ClientTransaction ctx = responseEvent.getClientTransaction();
        TransactionContext transactionContext = TransactionContext.get(ctx);
        Response response = responseEvent.getResponse();

        /*
         * Do not record the last response here in the dialogContext as we are going to ack the
         * dialog right away.
         */

        BackToBackUserAgent b2bua = DialogContext.getBackToBackUserAgent(dialog);

        /*
         * This is the case of Refer redirection. In this case, we have already established a call
         * leg with transfer agent. We already have a RTP session established with the transfer
         * agent. We need to redirect the outbound RTP stream to the transfer target. To do this,
         * we fix up the media session using the port in the incoming sdp answer.
         */
        ContentTypeHeader cth = (ContentTypeHeader) response.getHeader(ContentTypeHeader.NAME);
        Dialog referDialog = transactionContext.getReferingDialog();
        Request referRequest = transactionContext.getReferRequest();
        Dialog peerDialog = DialogContext.getPeerDialog(dialog);
        DialogContext peerDialogContext = DialogContext.get(peerDialog);


        if (response.getRawContent() != null
                && cth.getContentType().equalsIgnoreCase("application")
                && cth.getContentSubType().equalsIgnoreCase("sdp")) {

            /*
             * The incoming media session.
             */

            SessionDescription sessionDescription = SipUtilities.getSessionDescription(response);

            RtpSession rtpSession = ((DialogContext) referDialog.getApplicationData())
                    .getRtpSession();

            if (rtpSession != null) {

                rtpSession.getTransmitter().setSessionDescription(sessionDescription, false);


                if (logger.isDebugEnabled()) {
                    logger.debug("Receiver State : " + rtpSession.getReceiverState());
                }

                /*
                 * Grab the RTP session previously pointed at by the REFER dialog.
                 */
                b2bua.getRtpBridge().addSym(rtpSession);

                ((DialogContext) dialog.getApplicationData()).setRtpSession(rtpSession);

                /*
                 * Check if we need to forward that response and do so if needed. see issue 1718
                 */

                if (transactionContext.getServerTransaction() != null
                        && transactionContext.getServerTransaction().getState() != TransactionState.TERMINATED) {
                    Response forwardedResponse = SipUtilities.createResponse(transactionContext
                            .getServerTransaction(), response.getStatusCode());
                    
                    
                    SipUtilities.setSessionDescription(forwardedResponse, sessionDescription);
                    ContactHeader responseContactHeader = (ContactHeader) response.getHeader(ContactHeader.NAME);
                    String contactUser;
                    if (responseContactHeader != null) {
                        SipURI contactURI = (SipURI) responseContactHeader.getAddress().getURI();
                        contactUser = contactURI.getUser();
                    }
                    else {
                        contactUser = Gateway.SIPXBRIDGE_USER;
                    }
                    ContactHeader contact = SipUtilities.createContactHeader(
                            ((TransactionExt) transactionContext.getServerTransaction()).getSipProvider(),
                            peerDialogContext.getItspInfo(), contactUser,
                            transactionContext.getServerTransaction());
                    forwardedResponse.setHeader(contact);

                    transactionContext.getServerTransaction().sendResponse(forwardedResponse);


                } else {
                    if ( logger.isDebugEnabled() ) logger.debug("not forwarding response peerDat.transaction  = "
                            + transactionContext.getServerTransaction());
                }

            } else {
                logger
                        .debug("Processing ReferRedirection: Could not find RtpSession for referred dialog");
            }

        } else if (response.getRawContent() != null) {
            /*
             * Got content but it was not SDP.
             */
            logger
                    .error("Encountered unexpected content type from response - not forwarding response");
            ReasonHeader reasonHeader = ProtocolObjects.headerFactory.createReasonHeader(
                    Gateway.SIPXBRIDGE_USER, ReasonCode.UNEXPECTED_CONTENT_TYPE,
                    "unknown content type encountered");
            dialogContext.getBackToBackUserAgent().tearDown(reasonHeader);
            return;
        }

        /*
         * Got an OK for the INVITE ( that means that somebody picked up ) so we can hang up the
         * call. We have already redirected the RTP media to the redirected party at this point.
         */

        if ( logger.isDebugEnabled() ) logger.debug("referDialog.getState() " + referDialog.getState() + " isOriginatorSipxbridge = "  + SipUtilities.isOriginatorSipXbridge(response));
        if (referDialog.getState() == DialogState.CONFIRMED
                && SipUtilities.isOriginatorSipXbridge(response)) {

            this.notifyReferDialog(referRequest, referDialog, response);

        }

        /*
         * SDP was returned from the transfer target.
         */
        if (response.getContentLength().getContentLength() != 0) {
            if (transactionContext.getDialogPendingSdpAnswer() != null
                    && DialogContext.get(transactionContext.getDialogPendingSdpAnswer()).getPendingAction() == PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK) {
                Dialog dialogToAck = transactionContext.getDialogPendingSdpAnswer();
                transactionContext.setDialogPendingSdpAnswer(null);
                CallControlUtilities.sendSdpAnswerInAck(response, dialogToAck);
            } else if (dialogContext.getPendingAction() == PendingDialogAction.PENDING_RE_INVITE_WITH_SDP_OFFER) {
                dialogContext.setPendingAction(PendingDialogAction.NONE);
                CallControlUtilities.sendSdpReOffer(responseEvent, dialog, peerDialog);
            }
        }

        /*
         * We directly send ACK.
         */
        if (response.getStatusCode() == Response.OK) {
            b2bua.addDialog(DialogContext.get(dialog));
            Request ackRequest = dialog.createAck(((CSeqHeader) response
                    .getHeader(CSeqHeader.NAME)).getSeqNumber());
            if ( logger.isDebugEnabled() ) logger.debug("peerDialog : " + DialogContext.getPeerDialog(dialog));
            DialogContext.pairDialogs(DialogContext.getPeerDialog(dialog), dialog);

            DialogContext.get(dialog).sendAck(ackRequest);
            b2bua.sendByeToMohServer();
        } else {
            ServerTransaction stx = transactionContext.getServerTransaction();
            if ( stx.getState() != TransactionState.TERMINATED ) {
               Response newResponse = SipUtilities.createResponse(stx, response.getStatusCode() );
               SipUtilities.copyHeaders(response, newResponse);
               stx.sendResponse(newResponse);
            }
        }

    }

    /**
     * Handle the OK from the response to the MOH server.
     *
     * @param responseEvent
     * @throws Exception
     */
    private void sendInviteToMohServerResponse(ResponseEvent responseEvent) throws Exception {
        if ( logger.isDebugEnabled() ) logger.debug("sendInviteToMohServerResponse");
        Response response = responseEvent.getResponse();
        Dialog dialog = responseEvent.getDialog();
        TransactionContext tad = TransactionContext.get(responseEvent.getClientTransaction());
        /*
         * Send him an ACK.
         */
        try {
            if (response.getStatusCode() == Response.OK) {
                /* To avoid rejecting stray packets set up the transmitter side (although this is a
                 * send-only stream ). We do not want to do this if this is already scheduled for termination
                 * on confirm because otherwise we will make a mess of the port mappings (race condition).
                 */
                SessionDescription sd = SipUtilities.getSessionDescription(response);
                String ipAddress = SipUtilities.getSessionDescriptionMediaIpAddress(sd);
                int port = SipUtilities.getSessionDescriptionMediaPort(sd);
                if (!DialogContext.get(dialog).isTerminateOnConfirm() &&  DialogContext.get(dialog).getRtpSession() != null  &&
                    DialogContext.get(dialog).getRtpSession().getTransmitter() != null ) {
                    DialogContext.get(dialog).getRtpSession().getTransmitter().setIpAddressAndPort(ipAddress, port);
                }
                DialogContext.get(dialog).sendAck(response);
                /*
                 * Check the pending action for the peer dialog (pointing to the ITSP ).
                 */
                if (tad.getDialogPendingSdpAnswer() != null
                        && DialogContext.getPendingAction(tad.getDialogPendingSdpAnswer()) ==
                            PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK) {
                    /*
                     * Send the Answer to the peer dialog.
                     */
                    if (DialogContext.getRtpSession(tad.getDialogPendingSdpAnswer())!= null) {
                        CallControlUtilities
                        .sendSdpAnswerInAck(response, tad.getDialogPendingSdpAnswer());
                    } else {
                        // The MOH dialog can be safely killed off.
                        DialogContext.get(dialog).sendBye(true);
                    }
                } else {
                    if (logger.isDebugEnabled()) {
                        logger.debug("dialogPendingSdpAnswer = " + tad.getDialogPendingSdpAnswer());
                        if (tad.getDialogPendingSdpAnswer() != null) {
                            logger
                            .debug("PendingAction = "
                                    + DialogContext.getPendingAction(tad
                                            .getDialogPendingSdpAnswer()));
                        }
                    }
                }
            } else {
                if ( logger.isDebugEnabled() ) logger.debug("MOH negotiation failed. Response code is  " + response.getStatusCode());
                if (tad.getDialogPendingSdpAnswer() != null
                        && DialogContext.getPendingAction(tad.getDialogPendingSdpAnswer()) ==
                            PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK) {
                    /*
                     * Send the previous Answer to the peer dialog.
                     */
                    if (DialogContext.getRtpSession(tad.getDialogPendingSdpAnswer())!= null) {
                        CallControlUtilities
                        .sendSdpAnswerInAck( tad.getDialogPendingSdpAnswer(),null);
                    }

               }
            }
        } catch (Exception ex) {
            logger.error("Exception occured sending SDP in ACK to MOH Server",ex);
            // The MOH dialog can be safely killed off.
            DialogContext.get(dialog).sendBye(true);
        }


    }

    /**
     * Handle an SDP offer received in a response which is as a result of sending an SDP
     * solicitation with 0 length SDP content to the peer of a dialog.
     *
     *
     * @param responseEvent
     * @throws Exception
     */

    private void solicitSdpOfferFromPeerDialogResponse(ResponseEvent responseEvent)
            throws Exception {

        if ( logger.isDebugEnabled() ) logger.debug("solicitSdpOfferFromPeerDialogResponse");
        /*
         * Grab the response from the IB response event.
         */

        Response response = responseEvent.getResponse();

        if (response.getContentLength().getContentLength() == 0) {
            logger
                    .warn("PROTOCOL ERROR -- Expecting a content length != 0. Re-use previous SDP answer ");

        }

        /*
         * Grab the client tx from the inbound response.
         */
        ClientTransaction clientTransaction = responseEvent.getClientTransaction();

        /*
         * Grab the transaction context from the inbound transaction.
         */
        TransactionContext transactionContext = TransactionContext.get(clientTransaction);
        /*
         * Dialog for the response.
         */
        Dialog dialog = responseEvent.getDialog();

        /*
         * The dialog context associated with this dialog.
         */
        DialogContext dialogContext = DialogContext.get(dialog);

        /*
         * The call context from the Dialog context.
         */

        BackToBackUserAgent b2bua = dialogContext.getBackToBackUserAgent();

        /*
         * The continuation context.
         */
        Operation continuationOperation = transactionContext.getContinuationOperation();


        SessionDescription responseSessionDescription;
        dialogContext.setLastResponse(response);

        if ( logger.isDebugEnabled() ) logger.debug("continuationOperation = " + continuationOperation);
        DialogContext peerDialogContext = DialogContext.getPeerDialogContext(dialog);
        if ( logger.isDebugEnabled() ) logger.debug("peerDialog.pendingAction = " + peerDialogContext.getPendingAction());
        
        if (response.getContentLength().getContentLength() != 0) {
            /*
             * Reasonable response.
             */
            responseSessionDescription = SipUtilities.getSessionDescription(response);
        } else {
            /*
             * HACK ALERT : This tries to compensate for the protocol error. Some ITSPs do not
             * properly handle sdp offers solicitation. They return back a 0 length sdp answer. In
             * this case, we re-use the previously sent sdp answer.
             */
            RtpSession peerRtpSession = DialogContext.getPeerRtpSession(transactionContext
                    .getContinuationData().getDialog());
           

            responseSessionDescription = peerRtpSession.getReceiver().getSessionDescription();
           

        }
  


        if (continuationOperation == Operation.REFER_INVITE_TO_SIPX_PROXY) {

            /*
             * ACK the query operation with the previous response. This prevents the Dialog from
             * timing out while the transfer target waits for phone pickup.
             */
            SessionDescription clonedSessionDescription = SipUtilities
                    .cloneSessionDescription(responseSessionDescription);

            DialogContext.getRtpSession(dialog).getTransmitter().setSessionDescription(
                    clonedSessionDescription, true);

            ClientTransaction mohCtx = null;

            if (dialog.getState() != DialogState.TERMINATED) {
                /*
                 * If we do not support MOH or if the park server codecs are not supported in the
                 * answer, ACK right away. Otherwise Send an INVITE with the answer to the Park
                 * Server and concurrently send another invite to the phone. The Park server will
                 * play music when the phone is ringing ( simulates RINGING ).
                 */
                ReferInviteToSipxProxyContinuationData continuation = (ReferInviteToSipxProxyContinuationData) transactionContext
                        .getContinuationData();

                if (!Gateway.getBridgeConfiguration().isMusicOnHoldSupportEnabled()
                        || b2bua.isMohDisabled()
                        || (b2bua.getMusicOnHoldDialog() != null && b2bua.getMusicOnHoldDialog()
                                .getState() != DialogState.TERMINATED)) {
                    DialogContext.get(dialog).setLastResponse(response);
                    DialogContext.get(continuation.getDialog()).setPendingAction(
                            PendingDialogAction.PENDING_RE_INVITE_WITH_SDP_OFFER);
                    CallControlUtilities.sendSdpAnswerInAck(dialog,null);

                } else {
                    DialogContext.get(dialog).setLastResponse(response);
                    DialogContext.get(continuation.getDialog()).setPendingAction(
                            PendingDialogAction.PENDING_RE_INVITE_WITH_SDP_OFFER);
                  

                    DialogContext.getRtpSession(continuation.getDialog()).getReceiver()
                            .setSessionDescription(responseSessionDescription);
                    /*
                     * We do owe him an SDP answer. Mark it as such so when we get an answer from
                     * Park server, we can pass it on.
                     */
                    mohCtx = DialogContext.get(dialog).getBackToBackUserAgent()
                            .createClientTxToMohServer(responseSessionDescription,responseEvent.getResponse());

                    DialogContext.get(dialog).sendMohInvite(mohCtx);
                    
                    CallControlUtilities.sendSdpAnswerInAck(dialog,null);

                }

                /*
                 * Need to set the attribute to sendrecv because the MOH client tx recvonly.
                 */
                SessionDescription referSessionDescription = SipUtilities
                        .cloneSessionDescription(responseSessionDescription);
                SipUtilities.setDuplexity(referSessionDescription, "sendrecv");
                DialogContext.getRtpSession(continuation.getDialog()).getReceiver()
                        .setSessionDescription(responseSessionDescription);

                /*
                 * process the REFER by generating an INVITE and sending it along.
                 */
                b2bua.referInviteToSipxProxy(continuation.getRequest(), mohCtx, dialog,
                        continuation.getRequestEvent(), responseSessionDescription);
            }

        } else if (continuationOperation == Operation.SEND_INVITE_TO_MOH_SERVER) {
            /*
             * This is a query for MOH server. Lets see if he returned a codec that park server
             * handle in the query. If the ITSP does not return a codec that the park server
             * supports, we just ACK with the previously negotiated codec. MOH will not play in
             * this case. Note that MOH answers right away so we can forward the request.
             */
            if (response.getContentLength().getContentLength() == 0) {
                logger
                        .warn("PROTOCOL ERROR -- Expecting a content length != 0. Re-use previous SDP answer ");

                long cseq = SipUtilities.getSeqNumber(response);
                Request ack = dialog.createAck(cseq);

                DialogContext.get(dialog).sendAck(ack);
                return;

            }

            /*
             * Does our park server support the codecs in the SDP offer? If not, or if we already
             * have an existing MOH dialog with the park server, we just reply back with a
             * suitable ACK right away. Note that this ACK should be an SDP answer.
             */
            if ( b2bua.isMohDisabled()
                    || (b2bua.getMusicOnHoldDialog() != null && b2bua.getMusicOnHoldDialog()
                            .getState() != DialogState.TERMINATED)) {
                /*
                 * If codec is not supported by park server then we simply do not forward the
                 * answer to the park server in an INIVTE. We just replay the old respose
                 * (Example: this case happens for AT&T HIPCS ).
                 */
                DialogContext.get(dialog).setLastResponse(response);

                SessionDescription ackSd = dialogContext.getRtpSession().getReceiver()
                        .getSessionDescription();
                /*
                 * Limit the Answer to the codec set found in the offer. Note that the OFFER is in
                 * the INBOUND OK.
                 */
                Set<Integer> codecs = SipUtilities.getMediaFormats(responseSessionDescription);
                SipUtilities.cleanSessionDescription(ackSd, codecs);

                /*
                 * Now reply back to the original Transaction and put the WAN side on hold. Note
                 * tha this case MOH will not play.
                 */
                SendInviteToMohServerContinuationData continuation = (SendInviteToMohServerContinuationData) transactionContext
                        .getContinuationData();

                /*
                 * Send an ACK back to the WAN side and replay the same Session description as
                 * before.
                 */
                DialogContext.get(dialog).sendAck(ackSd);
                Request request = continuation.getServerTransaction().getRequest();
                Response newResponse = ProtocolObjects.messageFactory.createResponse(Response.OK,
                        request);

                RtpSession lanRtpSession = DialogContext.getPeerRtpSession(dialog);
                RtpSession wanRtpSession = DialogContext.getRtpSession(dialog);

                SipUtilities.setDuplexity(lanRtpSession.getReceiver().getSessionDescription(),
                        "recvonly");
                SipUtilities.incrementSessionVersion(lanRtpSession.getReceiver()
                        .getSessionDescription());

                /*
                 * Put the rtp session on hold.
                 */
                wanRtpSession.getTransmitter().setOnHold(true);

                SessionDescription newDescription = lanRtpSession.getReceiver()
                        .getSessionDescription();
                if (newDescription != null) {
                    newResponse.setContent(newDescription, ProtocolObjects.headerFactory
                            .createContentTypeHeader("application", "sdp"));
                }

                ToHeader toHeader = (ToHeader) request.getHeader(ToHeader.NAME);
                String userName = ((SipURI) toHeader.getAddress().getURI()).getUser();
                ContactHeader contactHeader = SipUtilities.createContactHeader(userName,
                        ((DialogExt) continuation.getDialog()).getSipProvider(),
                        SipUtilities.getViaTransport(newResponse));
                newResponse.setHeader(contactHeader);
                newResponse.setReasonPhrase("OK MOH Codec not supported by ITSP");

                continuation.getServerTransaction().sendResponse(newResponse);

                return;
            } else {
                SendInviteToMohServerContinuationData continuation = (SendInviteToMohServerContinuationData) transactionContext
                        .getContinuationData();

                SessionDescription clonedSd = SipUtilities
                        .cloneSessionDescription(responseSessionDescription);
                /*
                 * Set the session description on the wan side.
                 */
                DialogContext.getRtpSession(dialog).getTransmitter().setSessionDescription(
                        responseSessionDescription, false);

                /*
                 * Update the ports of the SD to forward to to the lan side.
                 */
                DialogContext.getRtpSession(continuation.getDialog()).getReceiver()
                        .setSessionDescription(clonedSd);

                /*
                 * Make sure we have not been beaten to the punch. If not, we can set up a dialog
                 * with the MOH server.
                 */
                if (b2bua.getMusicOnHoldDialog() == null
                        || b2bua.getMusicOnHoldDialog().getState() == DialogState.TERMINATED) {
                    ClientTransaction ctx = b2bua.createClientTxToMohServer(clonedSd,responseEvent.getResponse());
                    RtpSession mohRtpSession = DialogContext.getPeerRtpSession(dialog);
                    DialogContext.get(ctx.getDialog()).setRtpSession(mohRtpSession);
                    if ( logger.isDebugEnabled() ) logger.debug("mohRtpSession = " + mohRtpSession );

                    /*
                     * Note that we owe the dialog an sdp answer when we get it.
                     */
                    TransactionContext.get(ctx).setDialogPendingSdpAnswer(dialog);
                    ctx.sendRequest();
                }

                /*
                 * We have slipped in an INVITE to the MOH server. Now send the response to the
                 * other side. He thinks we are on hold but we are now playing MOH to him.
                 */

                Response newResponse = SipUtilities.createResponse(continuation
                        .getServerTransaction(), response.getStatusCode());
                ContactHeader contactHeader = SipUtilities.createContactHeader(
                        Gateway.SIPXBRIDGE_USER,
                        ((DialogExt) continuation.getDialog()).getSipProvider(),
                        SipUtilities.getViaTransport(newResponse));
                newResponse.setHeader(contactHeader);
                SipUtilities.setSessionDescription(newResponse, clonedSd);
                continuation.getServerTransaction().sendResponse(newResponse);

            }

        } else {
             if ( peerDialogContext.getPendingAction() == PendingDialogAction.PENDING_RE_INVITE_WITH_SDP_OFFER) {
                DialogContext.get(dialog).setPendingAction(PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK);
                DialogContext.get(dialog).setLastResponse(response);
                peerDialogContext.setPendingAction(PendingDialogAction.NONE);
               
                peerDialogContext.sendSdpReOffer(responseSessionDescription,responseEvent);
            } else {
            	SessionDescription sd = dialogContext.getRtpSession().getReceiver().getSessionDescription();
            	dialogContext.sendAck(sd);
            }
        }
    }

    /**
     * Handle the response event for SIPIRAL_INVITE_WITH_REPLACES
     *
     * @param responseEvent -- the response
     */
    private void handleSpiralInviteWithReplacesResponse(ResponseEvent responseEvent) throws Exception {

        Dialog dialog = responseEvent.getDialog();
        Response response = responseEvent.getResponse();
        TransactionContext tad = TransactionContext.get(responseEvent.getClientTransaction());

        if (response.getStatusCode() == Response.OK) {
            Request ack = dialog.createAck(((CSeqHeader) response
                    .getHeader(CSeqHeader.NAME)).getSeqNumber());

            DialogContext.get(dialog).sendAck(ack);

            if (tad.getDialogPendingSdpAnswer() != null
                    && DialogContext
                            .getPendingAction(tad.getDialogPendingSdpAnswer()) == PendingDialogAction.PENDING_SDP_ANSWER_IN_ACK) {
                CallControlUtilities.sendSdpAnswerInAck(response, tad
                        .getDialogPendingSdpAnswer());
            }
        }
    }

    /**
     * Handle the session timer response.
     *
     * @param responseEvent
     * @throws Exception
     */
    private void handleSessionTimerResponse(ResponseEvent responseEvent) throws Exception {
        Response response = responseEvent.getResponse();
        Dialog dialog = responseEvent.getDialog();
        DialogContext dialogContext = DialogContext.get(dialog);
        if (response.getStatusCode() == 200) {
            Request ack = dialog.createAck(((CSeqHeader) response
                    .getHeader(CSeqHeader.NAME)).getSeqNumber());
            dialogContext.recordLastAckTime();
            dialogContext.sendAck(ack);
            SessionDescription sessionDescription = SipUtilities.getSessionDescription(response);
            if ( sessionDescription != null ) {
                dialogContext.getRtpSession().getTransmitter().setSessionDescription(sessionDescription,
                        false);
            }
        } else if (response.getStatusCode() != 100 ) {
            dialogContext.sendBye(false);
        }
    }

    /**
     * Process an INVITE response.
     *
     * @param responseEvent -- the response event.
     */
    private void processInviteResponse(ResponseEvent responseEvent) {
        if ( logger.isDebugEnabled() ) logger.debug("processInviteResponse");

        Response response = responseEvent.getResponse();

        Dialog dialog = responseEvent.getDialog();

        ClientTransaction clientTransaction = null;

        /*
         * No client transaction in response. Discard it unless it is a forked
         * response.
         */
        if ( responseEvent.getClientTransaction() == null ) {
            /*
             * An In-Dialog response. If no client transaction
             * then return.
             */
            if ( SipUtilities.getFromTag(response) != null
                    && SipUtilities.getToTag(response) != null ) {
                if ( logger.isDebugEnabled() ) logger.debug("Dropping IN Dialog response -- no client transaction");
                return;
            } else if ( dialog == null ) {
                if ( logger.isDebugEnabled() ) logger.debug("Dialog not found -- dropping response!");
                return;
            } else if (DialogContext.get(dialog).getDialogCreatingTransaction() instanceof ClientTransaction ) {
                clientTransaction = (ClientTransaction) DialogContext.get(dialog).getDialogCreatingTransaction();
            } else {
                if ( logger.isDebugEnabled() ) logger.debug("Dropping response -- no client transaction");
                return;
            }
        } else {
            clientTransaction = responseEvent.getClientTransaction();
        }


        if (logger.isDebugEnabled()) {
            logger.debug("processInviteResponse : " + ((SIPResponse) response).getFirstLine()
                    + " dialog = " + dialog);
        }

        /*
         * The dialog context associated with this dialog.
         */
        DialogContext dialogContext = DialogContext.get(dialog);
        BackToBackUserAgent b2bua;

        try {
            if (dialogContext == null) {
                logger.error("Could not find DialogApplicationData -- dropping the response");
                try {
                    if (response.getStatusCode() == 200) {
                        Request ack = dialog.createAck(((CSeqHeader) response
                                .getHeader(CSeqHeader.NAME)).getSeqNumber());
                        dialog.sendAck(ack);

                    }
                    return;
                } catch (Exception ex) {
                    logger.error("Unexpected error sending ACK for 200 OK");
                    return;
                }
            }
            /*
             * The call context.
             */
            b2bua = dialogContext.getBackToBackUserAgent();
        } catch (Exception ex) {
            logger.error("Unexpected error sending ACK for 200 OK", ex);
            return;
        }


        TransactionContext transactionContext = TransactionContext.get(clientTransaction);

        if (b2bua == null) {
            logger.fatal("Could not find a BackToBackUA -- dropping the response");
            throw new SipXbridgeException("Could not find a B2BUA for this response : "
                    + response);
        }

        try {

            if (response.getStatusCode() == Response.TRYING) {
                /*
                 * We store away our outgoing sdp offer in the application data of the client tx.
                 */
                TransactionContext tad = (TransactionContext) responseEvent
                        .getClientTransaction().getApplicationData();
                if (tad.getOperation() == Operation.REFER_INVITE_TO_SIPX_PROXY
                        || tad.getOperation() == Operation.SPIRAL_BLIND_TRANSFER_INVITE_TO_ITSP) {
                    Dialog referDialog = tad.getReferingDialog();
                    Request referRequest = tad.getReferRequest();
                    if (referDialog.getState() == DialogState.CONFIRMED
                            && SipUtilities.isOriginatorSipXbridge(response)) {

                        this.notifyReferDialog(referRequest, referDialog, response);

                    }
                }

            } else if (response.getStatusCode() > 100 && response.getStatusCode() <= 200) {
                /*
                 * We store away our outgoing sdp offer in the application data of the client tx.
                 */
                TransactionContext tad = (TransactionContext) responseEvent
                        .getClientTransaction().getApplicationData();

                if ( logger.isDebugEnabled() ) logger.debug("Operation = " + tad.getOperation());
                /*
                 * Set our final dialog. Note that the 1xx Dialog may be different.
                 */
                if (tad.getOperation() != Operation.SEND_INVITE_TO_MOH_SERVER) {
                    b2bua.addDialog(DialogContext.get(dialog));
                } else {
                    /*
                     * We don't care about keeping references to the MOH dialog so we just put it into
                     * the cleanup list.
                     */
                    b2bua.addDialogToCleanup(dialog);
                }

                /*
                 * The TransactionApplicationData operator will indicate what the response is for.
                 * This is an OK for the session timer.
                 */
                RequireHeader requireHeader = (RequireHeader) response
                        .getHeader(RequireHeader.NAME);
                /*
                 * He wants a PRACK so let him have a PRACK.
                 */
                if (response.getStatusCode() / 100 == 1 && requireHeader != null
                        && requireHeader.getOptionTag().equalsIgnoreCase("100rel")) {
                    Request prackRequest = dialog.createPrack(response);
                    SipUtilities.addWanAllowHeaders(prackRequest);
                    ReferencesHeader referencesHeader = SipUtilities.createReferencesHeader(response,
                            ReferencesHeader.CHAIN);
                    prackRequest.addHeader(referencesHeader);

                    SipProvider provider = (SipProvider) responseEvent.getSource();
                    ClientTransaction ct = provider.getNewClientTransaction(prackRequest);
                    ContactHeader responseContactHeader = (ContactHeader) response.getHeader(ContactHeader.NAME);
                    String contactUser;
                    if (responseContactHeader != null) {
                        SipURI contactURI = (SipURI) responseContactHeader.getAddress().getURI();
                        contactUser = contactURI.getUser();
                    }
                    else {
                        contactUser = Gateway.SIPXBRIDGE_USER;
                    }
                    ContactHeader cth = SipUtilities.createContactHeader(provider, dialogContext
                             .getItspInfo(),contactUser, ct);
                    prackRequest.setHeader(cth);

                    /*
                     * No server transaction -- this is one sided.
                     */
                    TransactionContext.attach(ct, Operation.SEND_PRACK);
                    dialog.sendRequest(ct);
                }

                if (tad.getOperation() == Operation.SESSION_TIMER) {
                    this.handleSessionTimerResponse(responseEvent);
                } else if (tad.getOperation() == Operation.SEND_SDP_RE_OFFER) {
                    this.sendSdpReofferResponse(responseEvent);
                } else if (tad.getOperation() == Operation.FORWARD_SDP_SOLICITIATION) {
                    this.forwardSdpSolicitationResponse(responseEvent);
                } else if (tad.getOperation() == Operation.SOLICIT_SDP_OFFER_FROM_PEER_DIALOG
                        && response.getStatusCode() == 200) {
                    this.solicitSdpOfferFromPeerDialogResponse(responseEvent);
                } else if (tad.getOperation() == Operation.SEND_INVITE_TO_ITSP
                        || tad.getOperation() == Operation.SEND_INVITE_TO_SIPX_PROXY) {
                    this.inviteToItspOrProxyResponse(responseEvent);
                } else if (tad.getOperation() == Operation.REFER_INVITE_TO_SIPX_PROXY) {
                    this.referInviteToSipxProxyResponse(responseEvent);
                } else if (tad.getOperation() == Operation.SPIRAL_BLIND_TRANSFER_INVITE_TO_ITSP) {
                    this.blindTransferToItspResponse(responseEvent);
                } else if (tad.getOperation().equals(Operation.SEND_INVITE_TO_MOH_SERVER)) {
                    this.sendInviteToMohServerResponse(responseEvent);
                } else if (tad.getOperation().equals(Operation.FORWARD_REINVITE)) {
                    this.forwardReInviteResponse(responseEvent);
                } else if ( tad.getOperation().equals(Operation.RE_INVITE_REMOVE_RELAY)) {
                    this.processReInviteRemoveRelayResponse(responseEvent);
                } else if (tad.getOperation().equals(Operation.RE_INVITE_REMOVE_RELAY_CONTINUATION)) {
                    this.processReInviteRemoveRelayContinuationResponse(responseEvent);
                } else if (tad.getOperation() == Operation.HANDLE_SPIRAL_INVITE_WITH_REPLACES) {
                    this.handleSpiralInviteWithReplacesResponse(responseEvent);
                } else if (tad.getOperation() == Operation.HANDLE_INVITE_WITH_REPLACES) {
                    handleInviteWithReplacesResponse(responseEvent);
                } else {
                    logger.fatal("CallControlManager: Unknown Case in if statement ");
                }
            } else if ((response.getStatusCode() == Response.REQUEST_PENDING || response
                    .getStatusCode() == Response.BAD_REQUEST)
                    && dialog.getState() == DialogState.CONFIRMED
                    && transactionContext.counter < 2) {
                /*
                 * Note that the BAD_REQUEST check should not be necessary here Some ITSPs send a
                 * bad error code and hence this check for BAD_REQUEST.
                 */
                transactionContext.counter++;
                Gateway.getTimer()
                        .schedule(new RequestPendingTimerTask(transactionContext), 1000);
            } else if (response.getStatusCode() == Response.INTERVAL_TOO_BRIEF) {
                MinSE minSe = (MinSE) response.getHeader(MinSE.NAME);
                if (minSe != null) {
                    dialogContext.setSetExpires(minSe.getExpires());
                }

            } else if (response.getStatusCode() > 200) {
                this.inviteErrorResponse(responseEvent);
            }

        } catch (ParseException ex) {
            logger.error("Unexpected parse exception", ex);
            throw new SipXbridgeException("Unexpected exception", ex);
        } catch (InvalidArgumentException ex) {
            logger.error("Unpexpected exception", ex);
            throw new SipXbridgeException("Unexpected exception", ex);
        } catch (Exception ex) {
            /*
             * Some other exception occured during processing of the request.
             */
            logger.error("Exception while processing inbound response ", ex);

            if (b2bua != null) {
                try {
                    b2bua.tearDown(ProtocolObjects.headerFactory.createReasonHeader("sipxbridge",
                            ReasonCode.UNCAUGHT_EXCEPTION,
                            "Unexpected exception processing response"));
                } catch (Exception e) {
                    logger.error("unexpected exception", e);
                }
            }
        }

    }


    /**
     * Process the response received from remapping the second leg of the media.
     *
     * @param responseEvent
     * @throws Exception
     */
    private void processReInviteRemoveRelayContinuationResponse(ResponseEvent responseEvent) throws Exception {

        Response response = responseEvent.getResponse();
        Dialog dialog = responseEvent.getDialog();
        DialogContext dialogContext = DialogContext.get(dialog);

        if ( response.getStatusCode() == Response.OK ) {
            dialogContext.sendAck(response);
        } else if ( response.getStatusCode()/100 > 2 ) {
            dialogContext.getBackToBackUserAgent().tearDown(Gateway.SIPXBRIDGE_USER,ReasonCode.UNEXPECTED_RESPONSE_CODE,
                    "Unexpected error while remapping media");
        }

    }

    /**
     * Process the response for the re-INVITE that attempts to remove the relay from the media path.
     *
     * @param responseEvent
     * @throws Exception
     */
    private void processReInviteRemoveRelayResponse(ResponseEvent responseEvent) throws Exception {

        Dialog dialog = responseEvent.getDialog();
        DialogContext peerDialogContext = DialogContext.getPeerDialogContext(dialog);
        Response response = responseEvent.getResponse();
        SessionDescription sessionDescription = SipUtilities.getSessionDescription(response);
        if ( response.getStatusCode() == Response.OK ) {
            DialogContext.get(dialog).sendAck(response);
            if ( ! peerDialogContext.getDialog().getState().equals(DialogState.TERMINATED)) {
                Request reInviteRequest = peerDialogContext.getDialog().createRequest(Request.INVITE);
                ReferencesHeader referencesHeader = SipUtilities.createReferencesHeader(response, ReferencesHeader.CHAIN);
                reInviteRequest.setHeader(referencesHeader);
                SipUtilities.setSessionDescription(reInviteRequest, sessionDescription);
                SipProvider provider = peerDialogContext.getSipProvider();
                ClientTransaction clientTransaction = provider.getNewClientTransaction(reInviteRequest);
                TransactionContext.attach(clientTransaction, Operation.RE_INVITE_REMOVE_RELAY_CONTINUATION);

                peerDialogContext.sendReInvite(clientTransaction);
            } else {
                DialogContext.get(dialog).getBackToBackUserAgent().tearDown(Gateway.SIPXBRIDGE_USER,ReasonCode.UNEXPECTED_DIALOG_STATE,
                        "Peer dialog is terminated -- killing call");
            }
        } else if (response.getStatusCode()/ 100 > 2 ) {
            /*
             * Error response in trying to do call setup.
             */
            DialogContext.get(dialog).getBackToBackUserAgent().tearDown(Gateway.SIPXBRIDGE_USER,ReasonCode.UNEXPECTED_RESPONSE_CODE,
            "Error remapping media endpoint");
        }
    }



    /**
     * Process response to an OPTIONS request.
     *
     * @param responseEvent
     */
    private void processOptionsResponse(ResponseEvent responseEvent) {
        if ( logger.isDebugEnabled() ) logger.debug("processOptionsResponse ");
    }

    /**
     * Sends a NOTIFY to the transfer agent containing a SipFrag with the response received from
     * the Tansafer Target.
     *
     * @param referRequest -- the REFER request
     * @param referDialog -- The REFER dialog in which we send NOTIFY.
     * @param response -- the response from the transfer target
     * @throws SipException
     */
    private void notifyReferDialog(Request referRequest, Dialog referDialog, Response response)
            throws SipException {
        if ( logger.isDebugEnabled() ) logger.debug("notifyReferDialog");
        try {
            Request notifyRequest = referDialog.createRequest(Request.NOTIFY);
            EventHeader eventHeader = ProtocolObjects.headerFactory.createEventHeader("refer");
            CSeqHeader cseq = (CSeqHeader) referRequest.getHeader(CSeqHeader.NAME);
            long cseqValue = cseq.getSeqNumber();
            eventHeader.setEventId(Integer.toString((int) cseqValue));
            notifyRequest.addHeader(eventHeader);

            String subscriptionState = "active";
            if (response.getStatusCode() >= 200) {
                /*
                 * Final response so TERMINATE the subscription.
                 */
                subscriptionState = "terminated";
            }
            SubscriptionStateHeader subscriptionStateHeader = ProtocolObjects.headerFactory
                    .createSubscriptionStateHeader(subscriptionState);
            if (subscriptionState.equals("terminated")) {
                subscriptionStateHeader.setReasonCode("deactivated");
            }
            notifyRequest.addHeader(subscriptionStateHeader);
            ReferencesHeader referencesHeader = SipUtilities.createReferencesHeader(response, "sipxecs-notify-response");
            notifyRequest.setHeader(referencesHeader);
            ContentTypeHeader contentTypeHeader = ProtocolObjects.headerFactory
                    .createContentTypeHeader("message", "sipfrag");
            String content = ((SIPResponse) response).getStatusLine().toString();
            notifyRequest.setContent(content, contentTypeHeader);
            SipUtilities.addLanAllowHeaders(notifyRequest);
            SipProvider referProvider = ((SIPDialog) referDialog).getSipProvider();
            ClientTransaction ctx = referProvider.getNewClientTransaction(notifyRequest);

            referDialog.sendRequest(ctx);
        } catch (ParseException ex) {
            logger.error("Unexpected parse exception ", ex);
            throw new SipXbridgeException("Unexpected parse exception ", ex);
        }

    }

    /**
     * Process a cancel response.
     *
     * @param responseEvent
     */
    private void processCancelResponse(ResponseEvent responseEvent) {
        if ( logger.isDebugEnabled() ) logger.debug("CallControlManager: processCancelResponse");

    }

    /**
     * Process a NOTIFY response.
     *
     * @param responseEvent
     */
    private void processNotifyResponse(ResponseEvent responseEvent) {
        if ( logger.isDebugEnabled() ) logger.debug("CallControlManager: processNotifyResponse");
    }

    /**
     * Process a REFER response.
     *
     * @param responseEvent
     */
    private void processReferResponse(ResponseEvent responseEvent) {
        try {
            if ( logger.isDebugEnabled() ) logger.debug("CallControlManager: processReferResponse");
            ClientTransaction ctx = responseEvent.getClientTransaction();
            TransactionContext tad = (TransactionContext) ctx.getApplicationData();
            ServerTransaction referServerTx = tad.getServerTransaction();
            Response referResponse = SipUtilities.createResponse(referServerTx, responseEvent
                    .getResponse().getStatusCode());
            if (referServerTx.getState() != TransactionState.TERMINATED) {
                referServerTx.sendResponse(referResponse);
            }
        } catch (Exception ex) {
            try {
                logger.error("Unexpected exception sending response", ex);
                if (responseEvent.getDialog() != null) {
                    DialogContext dat = DialogContext.get(responseEvent.getDialog());
                    BackToBackUserAgent b2bua = dat.getBackToBackUserAgent();
                    b2bua.tearDown(ProtocolObjects.headerFactory.createReasonHeader("sipxbridge",
                            ReasonCode.UNCAUGHT_EXCEPTION, "Error processing REFER response"));

                }
            } catch (Exception ex1) {
                logger.fatal("Unexpected exception tearing down call", ex1);

            }
        }

    }

    /**
     * Process a bye RESPONSE.
     *
     * @param responseEvent
     */

    private void processByeResponse(ResponseEvent responseEvent) {
        try {
            if ( logger.isDebugEnabled() ) logger.debug("CallControlManager: processByeResponse");
            ClientTransaction ct = responseEvent.getClientTransaction();
            TransactionContext tad = (TransactionContext) ct.getApplicationData();
            if (tad != null) {
                ServerTransaction st = tad.getServerTransaction();
                if (st != null) {
                    Response response = responseEvent.getResponse();
                    Response newResponse = SipUtilities.createResponse(st, response
                            .getStatusCode());
                    st.sendResponse(newResponse);
                    BackToBackUserAgent b2bua = DialogContext
                            .getBackToBackUserAgent(responseEvent.getDialog());
                }
            }

        } catch (Exception ex) {
            logger.error("Exception forwarding bye response", ex);
        }
    }

    // /////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * Process an incoming request.
     */
    void processRequest(RequestEvent requestEvent) {
        Request request = requestEvent.getRequest();
        String method = request.getMethod();
        if (method.equals(Request.INVITE)) {
            processInvite(requestEvent);
        } else if (method.equals(Request.ACK)) {
            processAck(requestEvent);
        } else if (method.equals(Request.BYE)) {
            processBye(requestEvent);
        } else if (method.equals(Request.CANCEL)) {
            processCancel(requestEvent);
        } else if (method.equals(Request.OPTIONS)) {
            processOptions(requestEvent);
        } else if (method.equals(Request.REFER)) {
            processRefer(requestEvent);
        } else if (method.equals(Request.PRACK)) {
            processPrack(requestEvent);
        }

    }

    /**
     * Process an incoming response
     */
    void processResponse(ResponseEvent responseEvent) {
        Response response = responseEvent.getResponse();
        String method = ((CSeqHeader) response.getHeader(CSeqHeader.NAME)).getMethod();
        if (method.equals(Request.INVITE)) {
            processInviteResponse(responseEvent);
        } else if (method.equals(Request.CANCEL)) {
            processCancelResponse(responseEvent);
        } else if (method.equals(Request.BYE)) {
            processByeResponse(responseEvent);
        } else if (method.equals(Request.NOTIFY)) {
            processNotifyResponse(responseEvent);
        } else if (method.equals(Request.REFER)) {
            processReferResponse(responseEvent);
        } else if (method.equals(Request.OPTIONS)) {
            processOptionsResponse(responseEvent);
        }
    }

    /**
     * The Reset handler for the symmitron. Just tear down all ongoing calls. This is detected by
     * the Ping operation to the symmitron.
     *
     * Note that each b2bua may be using a different symitron.
     *
     */
    public void reset(String serverHandle) {
        Collection<Dialog> dialogs = ((SipStackImpl) ProtocolObjects.getSipStack()).getDialogs();
        for (Dialog dialog : dialogs) {
            if (dialog.getApplicationData() instanceof DialogContext) {
                BackToBackUserAgent btobua = DialogContext.getBackToBackUserAgent(dialog);
                if (serverHandle.equals(btobua.getSymmitronServerHandle())) {
                    try {
                        btobua.tearDown();
                    } catch (Exception ex) {
                        logger.error("Error tearing down call ", ex);
                    }
                }
            }
        }

    }

}
