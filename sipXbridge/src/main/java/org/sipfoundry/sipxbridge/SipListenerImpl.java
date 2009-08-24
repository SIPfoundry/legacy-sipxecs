/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.SipStackExt;
import gov.nist.javax.sip.TransactionExt;

import java.util.Collection;
import java.util.Iterator;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.TimeoutEvent;
import javax.sip.Transaction;
import javax.sip.TransactionAlreadyExistsException;
import javax.sip.TransactionState;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.address.SipURI;
import javax.sip.address.Hop;
import javax.sip.header.CSeqHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

/**
 * This is the JAIN-SIP listener that fields all request and response events
 * from the stack.
 *
 * @author M. Ranganathan
 *
 */
public class SipListenerImpl implements SipListener {

    private static Logger logger = Logger.getLogger(SipListenerImpl.class);

    /**
     * Handle a Dialog Terminated event. Cleans up all the resources associated
     * with a Dialog.
     */

    private static void handleAuthenticationChallenge(
            ResponseEvent responseEvent) throws Exception {

        SipProvider provider = (SipProvider) responseEvent.getSource();

        Dialog dialog = responseEvent.getDialog();

        int statusCode = responseEvent.getResponse().getStatusCode();

        ClientTransaction ctx = responseEvent.getClientTransaction();

        /*
         * challenge from LAN side. Cannot handle this.
         */
        if (provider == Gateway.getLanProvider()) {
            /*
             * By default, we do not handle LAN originated challenges unless the inbound domain is the
             * same as the sipx domain -- in which case sipx will challenge us and we will forward that
             * challenge.
             */
            if (Gateway.getBridgeConfiguration().getSipxbridgePassword() == null) {
                ServerTransaction stx = ((TransactionContext) responseEvent
                        .getClientTransaction().getApplicationData())
                        .getServerTransaction();
                if (stx != null && stx.getState() != TransactionState.TERMINATED) {
                    /*
                     * Forward it to the peer. Maybe he knows how to handle the challenge and if not
                     * he will hang up the call.
                     */
                    Response errorResponse = SipUtilities.createResponse(stx, statusCode);
                    SipUtilities.copyHeaders(responseEvent.getResponse(),errorResponse);
                    errorResponse.removeHeader(ContactHeader.NAME);
                    ContactHeader cth = SipUtilities.createContactHeader(null, ((TransactionExt)stx).getSipProvider());
                    errorResponse.setHeader(cth);
                    if ( TransactionContext.get(responseEvent.getClientTransaction()).getItspAccountInfo() == null ||
                            TransactionContext.get(responseEvent.getClientTransaction()).getItspAccountInfo().isGlobalAddressingUsed()) {
                        SipUtilities.setGlobalAddress(errorResponse);
                    }
                    stx.sendResponse(errorResponse);
                }

                return;
            }
        }

        Response response = responseEvent.getResponse();
        CSeqHeader cseqHeader = (CSeqHeader) response
                .getHeader(CSeqHeader.NAME);
        if ( responseEvent.getClientTransaction() == null || responseEvent
                .getClientTransaction().getApplicationData() == null  ) {
            logger.warn("Cannot process event : NullClientTransaction or NullTransactionContext");
            return;
        }
        /*
         * Note that we need to store a pointer in the TransactionContext
         * because REGISTER does not have a dialog.
         */
        ItspAccountInfo accountInfo = ((TransactionContext) responseEvent
                .getClientTransaction().getApplicationData())
                .getItspAccountInfo();

        String method = cseqHeader.getMethod();
        String callId = SipUtilities.getCallId(response);

        /*
         * If we find a non-dummy ITSP account then check to see if we have
         * exceeded the failure count. If we have exceeded that count then
         * we are done with this request.
         */

        if (accountInfo != null
                && !accountInfo.isDummyAccount()
                && (accountInfo.incrementFailureCount(callId) > 1 || accountInfo
                        .getPassword() == null)) {

            /*
             * Got a 4xx response. Increment the failure count for the account
             * and mark it as AUTHENTICATION_FAILED
             */
            accountInfo.setState(AccountState.AUTHENTICATION_FAILED);
            if (logger.isDebugEnabled()) {
                logger
                        .debug("SipListenerImpl: could not authenticate with server. method = "
                                + method);

            }
            accountInfo.removeFailureCounter(callId);
            if (responseEvent.getDialog() != null) {
                BackToBackUserAgent b2bua = DialogContext
                        .getBackToBackUserAgent(responseEvent.getDialog());
                logger
                        .debug("Cannot authenticate request -- tearing down call");
                if (b2bua != null) {
                    b2bua.tearDown(Gateway.SIPXBRIDGE_USER,
                            ReasonCode.AUTHENTICATION_FAILURE,
                            "Could not authenticate request");
                }
            }
            if (!accountInfo.isAlarmSent()) {
                Gateway.getAlarmClient().raiseAlarm(
                        "SIPX_BRIDGE_AUTHENTICATION_FAILED",
                        accountInfo.getSipDomain());
                accountInfo.setAlarmSent(true);
            }
            return;

        } else if ( accountInfo != null && accountInfo.isDummyAccount() ) {
            /*
             * Forward the challenge back to the call originator if this is a dummy account we
             * created for purposes of bridging the call.
             */
            logger.debug("Forwarding challenge from WAN for dummy account");
            ServerTransaction stx = TransactionContext.get(ctx).getServerTransaction();
            if (stx != null &&  stx.getState() != TransactionState.TERMINATED ) {
                Response errorResponse = SipUtilities.createResponse(stx, statusCode);
                SipUtilities.copyHeaders(responseEvent.getResponse(),errorResponse);
                errorResponse.removeHeader(ContactHeader.NAME);
                ContactHeader cth = SipUtilities.createContactHeader(null, ((TransactionExt)stx).getSipProvider());
                errorResponse.setHeader(cth);
                stx.sendResponse(errorResponse);
                return;
            } else {
                logger.debug("Late arriving response for a dummy response -- ignoring. \n" +
                		"Could not find server transaction or server transaction is TERMINATED." +
                		"Discarding the response.");
                return;
            }
        }

        ClientTransaction newClientTransaction = Gateway
                .getAuthenticationHelper().handleChallenge(response,
                        responseEvent.getClientTransaction(), provider,
                        method.equals(Request.REGISTER) ? 0 : -1);

        TransactionContext tad = (TransactionContext) responseEvent
                .getClientTransaction().getApplicationData();
        tad.setClientTransaction(newClientTransaction);

        if (dialog == null) {
            /*
             * Out of dialog challenge ( REGISTER ).
             */
            newClientTransaction.sendRequest();
        } else {
            if (logger.isDebugEnabled()) {
                logger.debug("SipListenerImpl : dialog = " + dialog);
            }

            BackToBackUserAgent b2bua = DialogContext
                    .getBackToBackUserAgent(responseEvent.getDialog());
            if (b2bua != null) {
                b2bua.removeDialog(dialog);
                b2bua.addDialog(newClientTransaction.getDialog());
                DialogContext dialogApplicationData = (DialogContext) dialog
                        .getApplicationData();

                DialogContext newDialogApplicationData = DialogContext
                        .attach(b2bua, newClientTransaction.getDialog(),
                                newClientTransaction, newClientTransaction
                                        .getRequest());
                if ( newDialogApplicationData != dialogApplicationData ) {
                    newDialogApplicationData.setPeerDialog(dialogApplicationData
                            .getPeerDialog());
                    newClientTransaction.getDialog().setApplicationData(
                            newDialogApplicationData);
                    newDialogApplicationData.setItspInfo(dialogApplicationData.getItspInfo());
                    /*
                     * Hook the application data pointer of the previous guy in the
                     * chain at us.
                     */
                    DialogContext peerDialogApplicationData = (DialogContext) dialogApplicationData
                    .getPeerDialog().getApplicationData();
                    peerDialogApplicationData.setPeerDialog(newClientTransaction
                            .getDialog());
                    newDialogApplicationData.setRtpSession(dialogApplicationData
                            .getRtpSession());

                    if (logger.isDebugEnabled()) {
                        logger.debug("SipListenerImpl: New Dialog = "
                                + newClientTransaction.getDialog());
                    }
                }

            }

            if (dialog.getState() == DialogState.CONFIRMED) {
                /*
                 * In-DIALOG challenge. Re-INVITE was challenged.
                 */
                ToHeader toHeader = (ToHeader) newClientTransaction
                        .getRequest().getHeader(ToHeader.NAME);
                if (toHeader.getTag() != null) {
                    /*
                     * This check should not be necessary.
                     */
                    dialog.sendRequest(newClientTransaction);
                }

            } else {
                newClientTransaction.sendRequest();
            }
        }

    }

    /*
     * (non-Javadoc)
     *
     * @see javax.sip.SipListener#processDialogTerminated(javax.sip.DialogTerminatedEvent )
     */
    public void processDialogTerminated(DialogTerminatedEvent dte) {

        logger.debug("DialogTerminatedEvent " + dte.getDialog());

        DialogContext dat = DialogContext.get(dte.getDialog());
        if (dat != null) {
            dat.cancelSessionTimer();
        }

        if (dat != null) {
            BackToBackUserAgent b2bua = dat.getBackToBackUserAgent();
            if (b2bua != null) {
                b2bua.removeDialog(dte.getDialog());

            }

        }

    }

    public void processIOException(IOExceptionEvent ioex) {
        logger.error("Got an unexpected IOException " + ioex.getHost() + ":"
                + ioex.getPort() + "/" + ioex.getTransport());

    }

    /*
     * (non-Javadoc)
     *
     * @see javax.sip.SipListener#processRequest(javax.sip.RequestEvent)
     */
    public void processRequest(RequestEvent requestEvent) {

        if (logger.isDebugEnabled()) {
            logger.debug("Gateway: got an incoming request "
                    + requestEvent.getRequest());
        }
        Request request = requestEvent.getRequest();
        String method = request.getMethod();
        SipProvider provider = (SipProvider) requestEvent.getSource();

        ViaHeader viaHeader = (ViaHeader) request.getHeader(ViaHeader.NAME);

        try {

            if (Gateway.getState() == GatewayState.STOPPING) {
                logger.debug("Gateway is stopping -- returning");
                return;
            } else if (Gateway.getState() == GatewayState.INITIALIZING) {
                logger.debug("Rejecting request -- gateway is initializing");

                Response response = ProtocolObjects.messageFactory
                        .createResponse(Response.SERVICE_UNAVAILABLE, request);
                response
                        .setReasonPhrase("Gateway is initializing -- try later");
                ServerTransaction st = requestEvent.getServerTransaction();
                if (st == null) {
                    st = provider.getNewServerTransaction(request);
                }

                st.sendResponse(response);
                return;

            } else if (provider == Gateway.getLanProvider()
                    && method.equals(Request.INVITE)
                    && ((viaHeader.getReceived() != null && !Gateway
                            .isAddressFromProxy(viaHeader.getReceived())) || !Gateway
                            .isAddressFromProxy(viaHeader.getHost()))) {
                /*
                 * Check to see that via header originated from proxy server.
                 */
                ServerTransaction st = requestEvent.getServerTransaction();
                if (st == null) {
                    st = provider.getNewServerTransaction(request);

                }

                Response forbidden = SipUtilities.createResponse(st,
                        Response.FORBIDDEN);
                forbidden
                        .setReasonPhrase("Request not issued from SIPX proxy server");
                st.sendResponse(forbidden);
                return;

            }

            if (method.equals(Request.INVITE) 
                    || method.equals(Request.ACK)
                    || method.equals(Request.CANCEL) 
                    || method.equals(Request.BYE)
                    || method.equals(Request.OPTIONS)
                    || method.equals(Request.REFER) 
                    || method.equals(Request.PRACK)) {
                Gateway.getCallControlManager().processRequest(requestEvent);
            } else if ( method.equals(Request.REGISTER) && provider == Gateway.getLanProvider() ) {
                ItspAccountInfo itspAccount = Gateway.getAccountManager().getAccount(request);
                Gateway.getRegistrationManager().proxyRegisterRequest(requestEvent,itspAccount);
          } else {
                try {
                    Response response = ProtocolObjects.messageFactory
                    .createResponse(Response.METHOD_NOT_ALLOWED, request);
                    ServerTransaction st = requestEvent.getServerTransaction();
                    if (st == null) {
                        st = provider.getNewServerTransaction(request);
                    }
                    st.sendResponse(response);
                } catch (TransactionAlreadyExistsException ex) {
                    logger.error("transaction already exists", ex);
                } catch (Exception ex) {
                    logger.error("unexpected exception", ex);
                    throw new SipXbridgeException("Unexpected exceptione", ex);
                }
            }
        } catch (TransactionAlreadyExistsException ex) {
            logger.error("transaction already exists", ex);
            return;
        } catch (Exception ex) {
            logger.error("Unexpected exception ", ex);
            throw new SipXbridgeException("Unexpected exceptione", ex);
        }

    }

    /*
     * (non-Javadoc)
     *
     * @see javax.sip.SipListener#processResponse(javax.sip.ResponseEvent)
     */

    public void processResponse(ResponseEvent responseEvent) {

        if (Gateway.getState() == GatewayState.STOPPING) {
            logger.debug("Gateway is stopping -- returning");
            return;
        }

        Response response = responseEvent.getResponse();
        CSeqHeader cseqHeader = (CSeqHeader) response
                .getHeader(CSeqHeader.NAME);

        String method = cseqHeader.getMethod();
        Dialog dialog = responseEvent.getDialog();

        try {

            if (dialog != null && dialog.getApplicationData() == null
                    && method.equals(Request.INVITE)) {
                /*
                 * if the tx does not exist but the dialog does exist then this
                 * is a forked response
                 */

                SipProvider provider = (SipProvider) responseEvent.getSource();
                logger.debug("Forked dialog response detected.");
                /*
                 * This is a forked dialog response. Look through and pick up a
                 * dialog context for this dialog.
                 */
                SipStackExt sipStack = (SipStackExt) ProtocolObjects
                        .getSipStack();
                boolean attached = false;
                for (Dialog dialogExt : sipStack.getDialogs()) {
                    DialogContext dialogContext = (DialogContext) dialogExt
                            .getApplicationData();
                    /*
                     * Grab the dialog context of the other fork.
                     */
                    if (dialogContext != null
                            && SipUtilities.getCallLegId(response).equals(
                                    dialogContext.getCallLegId())) {
                        dialogContext.detach();
                        dialogContext.setDialog(dialog);
                        dialogContext.setLastResponse(responseEvent
                                .getResponse());
                        dialog.setApplicationData(dialogContext);
                        attached = true;
                    }
                }
                /*
                 * Kill off the dialog if we cannot file a dialog context.
                 */
                if (!attached && response.getStatusCode() == Response.OK) {
                        Request ackRequest = dialog.createAck(cseqHeader
                                .getSeqNumber());
                        /* Cannot access the dialogContext here */
                        dialog.sendAck(ackRequest);
                        Request byeRequest = dialog.createRequest(Request.BYE);
                        ClientTransaction byeClientTransaction = provider
                        .getNewClientTransaction(byeRequest);
                        dialog.sendRequest(byeClientTransaction);
                        return;
                }
            }

            /*
             * Handle proxy challenge.
             */
            if (response.getStatusCode() == Response.PROXY_AUTHENTICATION_REQUIRED
                    || response.getStatusCode() == Response.UNAUTHORIZED) {
                handleAuthenticationChallenge(responseEvent);
                return;
            }

            ItspAccountInfo accountInfo = null;

            if (responseEvent.getClientTransaction() != null
                    && ((TransactionContext) responseEvent
                            .getClientTransaction().getApplicationData()) != null) {
                accountInfo = ((TransactionContext) responseEvent.getClientTransaction()
                        .getApplicationData()).getItspAccountInfo();
            }

            String callId = SipUtilities.getCallId(response);

            /*
             * Garbage collect the failure counter if it exists.
             */
            if (accountInfo != null && response.getStatusCode() / 200 == 1) {
                accountInfo.removeFailureCounter(callId);
            }

            if (method.equals(Request.REGISTER)) {
                Gateway.getRegistrationManager().processResponse(responseEvent);
            } else if (method.equals(Request.INVITE)
                    || method.equals(Request.CANCEL)
                    || method.equals(Request.BYE)
                    || method.equals(Request.REFER)
                    || method.equals(Request.OPTIONS)) {
                Gateway.getCallControlManager().processResponse(responseEvent);
            } else {
                logger.warn("dropping response " + method);
            }

        } catch (Exception ex) {
            logger.error("Unexpected error processing response >>>> "
                    + response, ex);
            logger.error("cause = " + ex.getCause());
            if (dialog != null && DialogContext.get(dialog) != null) {
                DialogContext.get(dialog).getBackToBackUserAgent().tearDown();
            }

        }

    }

    /**
     * Remove state. Drop B2Bua structrue from our table so we will drop all
     * requests corresponding to this call in future.
     */

    public void processTimeout(TimeoutEvent timeoutEvent) {
        ClientTransaction ctx = timeoutEvent.getClientTransaction();
        try {
            if (ctx != null) {
                Request request = ctx.getRequest();

                if (request.getMethod().equals(Request.OPTIONS)) {
                    ClientTransaction clientTransaction = timeoutEvent
                            .getClientTransaction();
                    Dialog dialog = clientTransaction.getDialog();
                    BackToBackUserAgent b2bua = DialogContext.get(dialog)
                            .getBackToBackUserAgent();
                    b2bua.tearDown(Gateway.SIPXBRIDGE_USER,
                            ReasonCode.SESSION_TIMER_ERROR,
                            "OPTIONS Session timer timed out.");
                } else if (request.getMethod().equals(Request.REGISTER)) {
                    Gateway.getRegistrationManager().processTimeout(
                            timeoutEvent);
                } else if (request.getMethod().equals(Request.BYE)) {
                    ClientTransaction clientTransaction = timeoutEvent
                            .getClientTransaction();
                    Dialog dialog = clientTransaction.getDialog();
                    BackToBackUserAgent b2bua = DialogContext.get(dialog)
                            .getBackToBackUserAgent();
                    dialog.delete();
                    b2bua.removeDialog(dialog);
                } else if (request.getMethod().equals(Request.INVITE)) {
                    /*
                     * If this is a refer request -- grab the MOH Dialog and
                     * kill it. Otherwise we are stuck with the MOH dialog.
                     */
                    BackToBackUserAgent b2bua = DialogContext.get(
                            ctx.getDialog()).getBackToBackUserAgent();

                    TransactionContext transactionContext = TransactionContext
                            .get(ctx);
                    if (transactionContext.getOperation() == Operation.SEND_INVITE_TO_SIPX_PROXY) {
                            b2bua.tearDown(Gateway.SIPXBRIDGE_USER,
                                    ReasonCode.CALL_SETUP_ERROR,
                                    "SipxProxy is down");
                    } else {
                        if (transactionContext.getOperation() == Operation.SEND_INVITE_TO_ITSP
                                || transactionContext.getOperation() == Operation.SPIRAL_BLIND_TRANSFER_INVITE_TO_ITSP) {
                            logger.debug("Timed sending request to ITSP -- trying alternate proxy");
                            if ( ctx.getState() != TransactionState.TERMINATED ) {
                                ctx.terminate();
                            }
                            /* Try another hop */
                            Collection<Hop> hops = transactionContext
                                    .getProxyAddresses();
                            if (hops == null || hops.size() == 0 ) {
                                b2bua.sendByeToMohServer();
                                TransactionContext txContext  = TransactionContext.get(ctx);
                                if ( txContext.getServerTransaction() != null
                                        && txContext.getServerTransaction().getState()
                                        != TransactionState.TERMINATED ) {
                                    Response errorResponse = SipUtilities.createResponse(txContext.getServerTransaction(),
                                            Response.REQUEST_TIMEOUT);
                                    errorResponse.setReasonPhrase("ITSP Timed Out");
                                    SipUtilities.addSipFrag(errorResponse, "ITSP Domain : "
                                            + txContext.getItspAccountInfo().getProxyDomain());
                                    txContext.getServerTransaction().sendResponse(errorResponse);
                                }
                            } else {
                                b2bua.resendInviteToItsp(timeoutEvent
                                        .getClientTransaction());
                            }

                        } else {
                            logger.debug("Timed out processing "
                                    + transactionContext.getOperation());

                            b2bua.sendByeToMohServer();
                        }
                    }

                }
            }
        } catch (Exception ex) {
            logger.error("Error processing timeout event", ex);
        }

    }

    public void processTransactionTerminated(TransactionTerminatedEvent tte) {

        Transaction transaction = tte.getClientTransaction() != null ? tte
                .getClientTransaction() : tte.getServerTransaction();
        Dialog dialog = transaction.getDialog();
        Request request = transaction.getRequest();
        /*
         * When the INVITE tx terminates and the associated dialog state is
         * CONFIRMED, we increment the call count.
         */
        if (request.getMethod().equals(Request.INVITE)
                && dialog.getState() == DialogState.CONFIRMED
                && ((ToHeader) request.getHeader(ToHeader.NAME))
                        .getParameter("tag") == null) {
            TransactionContext txContext = TransactionContext.get(transaction);
            if (txContext != null
                    && (txContext.getOperation() == Operation.SEND_INVITE_TO_ITSP || txContext
                            .getOperation() == Operation.SEND_INVITE_TO_ITSP)) {
                Gateway.incrementCallCount();
            }
        }

    }

}
