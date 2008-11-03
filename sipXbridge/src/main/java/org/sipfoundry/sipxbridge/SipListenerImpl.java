/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.DialogExt;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipException;
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.TimeoutEvent;
import javax.sip.Transaction;
import javax.sip.TransactionAlreadyExistsException;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.header.CSeqHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

/**
 * This is the listener that fields all request and response events from the stack. INVITES, ACKS,
 * BYES get handled by the BackToBackUserAgentManager.
 * 
 * @author M. Ranganathan
 * 
 */
public class SipListenerImpl implements SipListener {

    private static Logger logger = Logger.getLogger(SipListenerImpl.class);

    /**
     * Handle a Dialog Terminated event. Cleans up all the resources associated with a Dialog.
     */
    public void processDialogTerminated(DialogTerminatedEvent dte) {

        logger.debug("DialogTerminatedEvent " + dte.getDialog());
        

        DialogApplicationData dat = DialogApplicationData.get(dte.getDialog());
        if (dat != null && dat.musicOnHoldDialog != null
                && dat.musicOnHoldDialog.getState() != DialogState.TERMINATED) {
            try {
                SipProvider provider = ((DialogExt) dat.musicOnHoldDialog).getSipProvider();
                Request byeRequest = dat.musicOnHoldDialog.createRequest(Request.BYE);
                ClientTransaction ctx = provider.getNewClientTransaction(byeRequest);
                TransactionApplicationData tad = new TransactionApplicationData(
                        Operation.SEND_BYE_TO_MOH_SERVER);
                ctx.setApplicationData(tad);
                dat.musicOnHoldDialog.sendRequest(ctx);

            } catch (Exception ex) {
                logger.error("Exception in dialog termination processing", ex);
            }

        }
        if (dat != null) {
            BackToBackUserAgent b2bua = dat.getBackToBackUserAgent();
            if (b2bua != null) {
                b2bua.removeDialog(dte.getDialog());

            }

            if (dat.getBackToBackUserAgent() != null
                    && dat.getBackToBackUserAgent().getCreatingDialog() == dte.getDialog()) {

                ItspAccountInfo itspAccountInfo = dat.getBackToBackUserAgent()
                        .getItspAccountInfo();

                Gateway.decrementCallCount();

                if (itspAccountInfo != null) {
                    itspAccountInfo.decrementCallCount();
                }
            }
        }

    }

    public void processIOException(IOExceptionEvent ioex) {
        logger.error("Got an unexpected IOException " + ioex.getHost() + ":" + ioex.getPort()
                + "/" + ioex.getTransport());

    }

    /*
     * (non-Javadoc)
     * 
     * @see javax.sip.SipListener#processRequest(javax.sip.RequestEvent)
     */
    public void processRequest(RequestEvent requestEvent) {

        if (logger.isDebugEnabled()) {
            logger.debug("Gateway: got an invoming request " + requestEvent.getRequest());
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

                Response response = ProtocolObjects.messageFactory.createResponse(
                        Response.SERVICE_UNAVAILABLE, request);
                response.setReasonPhrase("Gateway is initializing -- try later");
                ServerTransaction st = requestEvent.getServerTransaction();
                if (st == null) {
                    st = provider.getNewServerTransaction(request);
                }

                st.sendResponse(response);
                return;

            } else if (provider == Gateway.getLanProvider()
                    && method.equals(Request.INVITE)
                    && (viaHeader.getReceived() != null || !Gateway.isAddressFromProxy(viaHeader
                            .getHost()))) {
                ServerTransaction st = requestEvent.getServerTransaction();
                if (st == null) {
                    st = provider.getNewServerTransaction(request);

                }

                Response forbidden = SipUtilities.createResponse(st, Response.FORBIDDEN);
                forbidden.setReasonPhrase("Request not issued from SIPX proxy server");
                st.sendResponse(forbidden);
                return;

            }

        } catch (TransactionAlreadyExistsException ex) {
            logger.error("transaction already exists", ex);
            return;
        } catch (SipException ex) {
            throw new RuntimeException("Unexpected exceptione", ex);
        } catch (Exception ex) {
            logger.error("Unexpected exception ", ex);
            throw new RuntimeException("unexpected exception", ex);
        }

        if (method.equals(Request.INVITE) || method.equals(Request.ACK)
                || method.equals(Request.CANCEL) || method.equals(Request.BYE)
                || method.equals(Request.OPTIONS) || method.equals(Request.REFER))
            Gateway.getCallControlManager().processRequest(requestEvent);

        else {
            try {
                Response response = ProtocolObjects.messageFactory.createResponse(
                        Response.METHOD_NOT_ALLOWED, request);
                ServerTransaction st = requestEvent.getServerTransaction();
                if (st == null) {
                    st = provider.getNewServerTransaction(request);

                }
                st.sendResponse(response);
            } catch (TransactionAlreadyExistsException ex) {
                logger.error("transaction already exists", ex);
            } catch (SipException ex) {
                throw new RuntimeException("Unexpected exceptione", ex);
            } catch (Exception ex) {
                logger.error("Unexpected exception ", ex);
                throw new RuntimeException("unexpected exception", ex);
            }
        }

    }

    public void processResponse(ResponseEvent responseEvent) {

        if (Gateway.getState() == GatewayState.STOPPING) {
            logger.debug("Gateway is stopping -- returning");
            return;
        }

        Response response = responseEvent.getResponse();
        CSeqHeader cseqHeader = (CSeqHeader) response.getHeader(CSeqHeader.NAME);

        String method = cseqHeader.getMethod();
        String callId = SipUtilities.getCallId(response);

        try {

            if (responseEvent.getClientTransaction() == null) {
                /*
                 * Cannot find transaction state -- this must be a timed out transaction. Just
                 * ignore the response and wait for the other end to time out.
                 */

                logger.debug("Discarding response - no client tx state found");
                /*
                 * TODO -- what if the DIALOG exists but the transaction does not.
                 */
                return;
            }
            // Processing an OK for a NOTIFY -- do nothing.
            if (responseEvent.getClientTransaction().getApplicationData() == null) {
                logger.debug("Discarding response -- no transaction context information");
                return;
            }

            ItspAccountInfo accountInfo = ((TransactionApplicationData) responseEvent
                    .getClientTransaction().getApplicationData()).itspAccountInfo;

            if (method.equals(Request.OPTIONS)) {

                if (response.getStatusCode() == Response.CALL_OR_TRANSACTION_DOES_NOT_EXIST) {
                    Dialog dialog = responseEvent.getClientTransaction().getDialog();
                    BackToBackUserAgent b2bua = DialogApplicationData.get(dialog)
                            .getBackToBackUserAgent();
                    b2bua.tearDown();
                }

            } else if ((response.getStatusCode() == Response.PROXY_AUTHENTICATION_REQUIRED
                    || response.getStatusCode() == Response.UNAUTHORIZED || (response
                    .getStatusCode() > 200 && method.equals(Request.REGISTER)))
                    && accountInfo != null && accountInfo.incrementFailureCount(callId) > 1) {

                /*
                 * Got a 4xx response. Increment the failure count for the account and mark it as
                 * AUTHENTICATION_FAILED
                 */
                accountInfo.setState(AccountState.AUTHENTICATION_FAILED);
                if (logger.isDebugEnabled()) {
                    logger.debug("SipListenerImpl: could not authenticate with server. "
                            + "Here is the response " + response);

                }

                if (method.equals(Request.REGISTER))
                    return;
            }

            if (response.getStatusCode() == Response.PROXY_AUTHENTICATION_REQUIRED
                    || response.getStatusCode() == Response.UNAUTHORIZED) {

                SipProvider provider = (SipProvider) responseEvent.getSource();

                Dialog dialog = responseEvent.getDialog();
                if (logger.isDebugEnabled()) {
                    logger.debug("SipListenerImpl : dialog = " + dialog);
                }
                BackToBackUserAgent b2bua = DialogApplicationData
                        .getBackToBackUserAgent(responseEvent.getDialog());
                if (b2bua != null) {
                    b2bua.removeDialog(dialog);
                }

                if (provider == Gateway.getLanProvider()) {
                    // Unexpected challenge from LAN side.
                    // We are not configured to handle challenge for inbound calling
                    // If we get such a challenge, we just decline the call.
                    ServerTransaction stx = ((TransactionApplicationData) responseEvent
                            .getClientTransaction().getApplicationData()).serverTransaction;
                    Response errorResponse = SipUtilities.createResponse(stx, Response.DECLINE);
                    stx.sendResponse(errorResponse);
                    stx.getDialog().delete();
                    return;
                }

                ClientTransaction newClientTransaction = Gateway.getAuthenticationHelper()
                        .handleChallenge(response, responseEvent.getClientTransaction(),
                                provider, method.equals(Request.REGISTER) ? 0 : -1);

                // Handle authentication responses locally.

                TransactionApplicationData tad = (TransactionApplicationData) responseEvent
                        .getClientTransaction().getApplicationData();
                tad.clientTransaction = newClientTransaction;
                newClientTransaction.setApplicationData(tad);
                if (b2bua != null) {
                    b2bua.addDialog(newClientTransaction.getDialog());
                    DialogApplicationData dialogApplicationData = (DialogApplicationData) dialog
                            .getApplicationData();

                    DialogApplicationData newDialogApplicationData = DialogApplicationData
                            .attach(b2bua, newClientTransaction.getDialog(),
                                    newClientTransaction, newClientTransaction.getRequest());
                    newDialogApplicationData.peerDialog = dialogApplicationData.peerDialog;
                    newClientTransaction.getDialog().setApplicationData(newDialogApplicationData);
                    /*
                     * Hook the application data pointer of the previous guy in the chain at us.
                     */
                    DialogApplicationData peerDialogApplicationData = (DialogApplicationData) dialogApplicationData.peerDialog
                            .getApplicationData();
                    peerDialogApplicationData.peerDialog = newClientTransaction.getDialog();
                    newDialogApplicationData.setRtpSession(dialogApplicationData.getRtpSession());

                    if (logger.isDebugEnabled()) {
                        logger.debug("SipListenerImpl: New Dialog = "
                                + newClientTransaction.getDialog());
                    }

                }

                if (dialog != null && dialog.getState() == DialogState.CONFIRMED) {
                    ToHeader toHeader = (ToHeader) newClientTransaction.getRequest().getHeader(
                            ToHeader.NAME);
                    if (toHeader.getTag() != null) {
                        dialog.sendRequest(newClientTransaction);
                    }

                } else {
                    newClientTransaction.sendRequest();
                }

                return;
            }

            if (method.equals(Request.REGISTER)) {
                Gateway.getRegistrationManager().processResponse(responseEvent);
            } else if (method.equals(Request.INVITE) || method.equals(Request.CANCEL)
                    || method.equals(Request.BYE)) {
                Gateway.getCallControlManager().processResponse(responseEvent);
            } else {
                logger.debug("dropping response " + method);
            }

        } catch (Throwable ex) {
            Dialog dialog = responseEvent.getDialog();
            if (dialog != null)
                dialog.delete();

            logger.error("Unexpected error processing response >>>> " + response, ex);
            logger.error("cause = " + ex.getCause());
        }

    }

    /**
     * Remove state. Drop B2Bua structrue from our table so we will drop all requests
     * corresponding to this call in future.
     */

    public void processTimeout(TimeoutEvent timeoutEvent) {
        ClientTransaction ctx = timeoutEvent.getClientTransaction();
        try {
            if (ctx != null) {
                Request request = ctx.getRequest();
                if (request.getMethod().equals(Request.OPTIONS)) {
                    ClientTransaction clientTransaction = timeoutEvent.getClientTransaction();
                    Dialog dialog = clientTransaction.getDialog();
                    BackToBackUserAgent b2bua = DialogApplicationData.get(dialog)
                            .getBackToBackUserAgent();
                    b2bua.tearDown();
                } else if (request.getMethod().equals(Request.REGISTER)) {
                    Gateway.getRegistrationManager().processTimeout(timeoutEvent);
                } else if (request.getMethod().equals(Request.BYE)) {
                    ClientTransaction clientTransaction = timeoutEvent.getClientTransaction();
                    Dialog dialog = clientTransaction.getDialog();
                    BackToBackUserAgent b2bua = DialogApplicationData.get(dialog)
                            .getBackToBackUserAgent();
                    dialog.delete();
                    b2bua.removeDialog(dialog);
                } else if (request.getMethod().equals(Request.INVITE)) {
                    /*
                     * If this is a refer request -- grab the MOH Dialog and kill it. Otherwise we
                     * are stuck with the MOH dialog.
                     */
                    DialogApplicationData dialogApplicationData = DialogApplicationData.get(ctx
                            .getDialog());
                    if (dialogApplicationData.musicOnHoldDialog != null) {
                        Dialog mohDialog = dialogApplicationData.musicOnHoldDialog;
                        if (mohDialog != null && mohDialog.getState() != DialogState.TERMINATED) {
                            Request byeRequest = mohDialog.createRequest(Request.BYE);
                            SipProvider sipProvider = ((DialogExt) mohDialog).getSipProvider();
                            ClientTransaction byetx = sipProvider
                                    .getNewClientTransaction(byeRequest);
                            mohDialog.sendRequest(byetx);
                        }
                    }

                }
            }
        } catch (Exception ex) {
            logger.error("Error processing timeout event", ex);
        }

    }

    public void processTransactionTerminated(TransactionTerminatedEvent tte) {

        Transaction transaction = tte.getClientTransaction() != null ? tte.getClientTransaction()
                : tte.getServerTransaction();
        Dialog dialog = transaction.getDialog();
        Request request = transaction.getRequest();
        /*
         * When the INVITE tx terminates and the associated dialog state is CONFIRMED, we
         * increment the call count.
         * 
         */
        if (request.getMethod().equals(Request.INVITE)
                && dialog.getState() == DialogState.CONFIRMED
                && ((ToHeader) request.getHeader(ToHeader.NAME)).getParameter("tag") == null) {
            DialogApplicationData dat = (DialogApplicationData) dialog.getApplicationData();
            if (dat == null)
                return; // Nothing to do must be MOH Dialog.
            BackToBackUserAgent b2bua = dat.getBackToBackUserAgent();
            if (b2bua != null && dialog == b2bua.getCreatingDialog()
                    && b2bua.getItspAccountInfo() != null) {
                b2bua.getItspAccountInfo().incrementCallCount();
            }
            Gateway.incrementCallCount();
        }

    }

}
