/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.DialogTimeoutEvent;
import gov.nist.javax.sip.SipListenerExt;
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
import javax.sip.header.ProxyAuthorizationHeader;
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
public class SipListenerImpl implements SipListenerExt {

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
        
        if ( ctx == null ) {
            logger.debug("Dropping response");
            return;
        }

        TransactionContext transactionContext = TransactionContext.get(ctx);

        /*
         * challenge from LAN side. Forward it to the WAN. 
         */
        if (provider == Gateway.getLanProvider()) {
            /*
             * By default, we do not handle LAN originated challenges unless the inbound domain is the
             * same as the sipx domain -- in which case sipx will challenge us and we will forward that
             * challenge.
             * xx-6663: Forward authentication challenges.
             */

            ServerTransaction stx = ((TransactionContext) responseEvent
                    .getClientTransaction().getApplicationData())
                    .getServerTransaction();
            if (stx != null && stx.getState() != TransactionState.TERMINATED) {
                /*
                * Tear down the Back to back user agent immediately.
                */
                BackToBackUserAgent backToBackUserAgent = DialogContext.getBackToBackUserAgent(dialog);
                if ( backToBackUserAgent != null && transactionContext.getOperation() == Operation.SEND_INVITE_TO_SIPX_PROXY ) {
                    backToBackUserAgent.tearDownNow();
                }
                /*
                 * Forward it to the peer. Maybe he knows how to handle the challenge and if not
                 * he will hang up the call.
                 */
                Response errorResponse = SipUtilities.createResponse(stx, statusCode);
                SipUtilities.copyHeaders(responseEvent.getResponse(),errorResponse);
                errorResponse.removeHeader(ContactHeader.NAME);
                ContactHeader cth = SipUtilities.createContactHeader(null,
                        ((TransactionExt)stx).getSipProvider(),
                        SipUtilities.getViaTransport(errorResponse));
                errorResponse.setHeader(cth);
                if ( TransactionContext.get(responseEvent.getClientTransaction()).getItspAccountInfo() == null ||
                        TransactionContext.get(responseEvent.getClientTransaction()).getItspAccountInfo().isGlobalAddressingUsed()) {
                    SipUtilities.setGlobalAddress(errorResponse);
                }
                stx.sendResponse(errorResponse);
            }
            return;
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
        ServerTransaction stx = TransactionContext.get(ctx).getServerTransaction();

        if (accountInfo != null && stx == null &&
                 ( accountInfo.incrementFailureCount(callId) > 1 ||
                         accountInfo.getPassword() == null ) ) {

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

        } else if ( accountInfo != null && accountInfo.getPassword() == null && stx != null ) {
            /*
             * Forward the challenge back to the call originator if this is a dummy account we
             * created for purposes of bridging the call.
             */
            logger.debug("Forwarding challenge from WAN for dummy account");
             if (stx.getState() != TransactionState.TERMINATED ) {
                Response errorResponse = SipUtilities.createResponse(stx, statusCode);
                SipUtilities.copyHeaders(responseEvent.getResponse(),errorResponse);
                errorResponse.removeHeader(ContactHeader.NAME);
                ContactHeader cth = SipUtilities.createContactHeader(null,
                        ((TransactionExt)stx).getSipProvider(),
                        SipUtilities.getViaTransport(errorResponse));
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
                 DialogContext dialogApplicationData = (DialogContext) dialog.getApplicationData();
                 DialogContext newDialogApplicationData = DialogContext.attach(b2bua, newClientTransaction.getDialog(),
                        newClientTransaction, newClientTransaction
                        .getRequest());
                b2bua.addDialog(newDialogApplicationData);
             
                
                if ( newDialogApplicationData != dialogApplicationData ) {
                    b2bua.removeDialog(dialog);
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
                    
                    ProxyAuthorizationHeader pah = (ProxyAuthorizationHeader) newClientTransaction.getRequest().getHeader(ProxyAuthorizationHeader.NAME);
                    
                    newDialogApplicationData.setProxyAuthorizationHeader(pah);

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
                DialogContext dialogContext  = DialogContext.get(newClientTransaction.getDialog());
                b2bua.addDialog(dialogContext);  
                newClientTransaction.sendRequest();
            }
           
            DialogContext dialogContext  = DialogContext.get(newClientTransaction.getDialog());
            if ( !dialogContext.isSessionTimerStarted()) {
                dialogContext.startSessionTimer();
            }
            
        }

    }

    /*
     * (non-Javadoc)
     *
     * @see javax.sip.SipListener#processDialogTerminated(javax.sip.DialogTerminatedEvent )
     */
    public void processDialogTerminated(DialogTerminatedEvent dte) {
        DialogContext dialogContext = DialogContext.get(dte.getDialog());
        if ( dialogContext != null ) {
            logger.debug("DialogTerminatedEvent:  dialog created at " + dialogContext.getCreationPointStackTrace());
            logger.debug("DialogTerminatedEvent: dialog inserted at " + dialogContext.getInsertionPointStackTrace());
            logger.debug("DialogCreated by request: " + dialogContext.getRequest());
            dialogContext.cancelSessionTimer();
            BackToBackUserAgent b2bua = dialogContext.getBackToBackUserAgent();
            if (b2bua != null) {
                b2bua.removeDialog(dte.getDialog());
            }
            DialogContext.removeDialogContext(dialogContext);
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
            } else if ( SipUtilities.requestContainsUnsupportedExtension(request, provider == Gateway.getLanProvider()) ) {
            	  logger.debug("Rejecting request -- request contains unsupported extension");

                  Response response = ProtocolObjects.messageFactory
                          .createResponse(Response.BAD_EXTENSION, request);
                  
                  ServerTransaction st = requestEvent.getServerTransaction();
                  if (st == null) {
                      st = provider.getNewServerTransaction(request);
                  }

                  st.sendResponse(response);
                  return;

            } else if (provider == Gateway.getLanProvider()
                    && method.equals(Request.INVITE)
                    && ((viaHeader.getReceived() != null && !Gateway
                            .isAddressFromProxy(viaHeader.getReceived())) || 
                            (viaHeader.getReceived() == null && !Gateway
                            .isAddressFromProxy(viaHeader.getHost())))) {
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
            
            /*
             * Find the ITSP account and check if enabled. If so, then proceed, otherwise
             * send an error and bail out here.
             */
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
            
            if ( !request.getMethod().equals(Request.ACK) && itspAccount != null && ! itspAccount.isEnabled() ) {
                ServerTransaction st = requestEvent.getServerTransaction();
                if ( st == null ) {
                    st = provider.getNewServerTransaction(requestEvent.getRequest());
                }
                Response response = SipUtilities.createResponse(st, Response.SERVICE_UNAVAILABLE);
                response.setReasonPhrase("ITSP account is disabled");
                st.sendResponse(response);
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
            if ( method.equals(Request.INVITE)      &&
                 response.getStatusCode() == 200    && 
                 response.getHeader(ContactHeader.NAME) == null ) {
                logger.debug("Dropping bad response");
                if (dialog != null && DialogContext.get(dialog) != null) {
                    DialogContext.get(dialog).getBackToBackUserAgent().tearDown("sipXbridge", ReasonCode.PROTOCOL_ERROR, "Protocol Error - 200 OK with no contact");
                } else if ( dialog != null ) {
                    dialog.delete();
                }
                return;
                
            }
            if (dialog != null && dialog.getApplicationData() == null
                    && method.equals(Request.INVITE)) {
                
               
                /*
                 * if the tx does not exist but the dialog does exist then this
                 * is a forked response
                 */

                SipProvider provider = (SipProvider) responseEvent.getSource();
                logger.debug("Forked dialog response detected.");
                String callId = SipUtilities.getCallId(response);
                BackToBackUserAgent b2bua = Gateway.getBackToBackUserAgentFactory().getBackToBackUserAgent(callId);
                
                
                /*
                 * Kill off the dialog if we cannot find a dialog context.
                 */
                if (b2bua == null && response.getStatusCode() == Response.OK) {
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
                /*
                 * This is a forked response. We need to find the original call
                 * leg and retrieve the original RTP session from that call leg.
                 * TODO : Each call leg must get its own RTP bridge so this code
                 * needs to be rewritten. For now, they all share the same bridge.
                 */
                String callLegId = SipUtilities.getCallLegId(response);
                for ( Dialog sipDialog : b2bua.dialogTable) {
                    if ( DialogContext.get(sipDialog).getCallLegId().equals(callLegId) && DialogContext.get(sipDialog).rtpSession != null ) {
                        DialogContext context = DialogContext.get(sipDialog);
                        Request request = context.getRequest();
                        DialogContext newContext = DialogContext.attach(b2bua, dialog,context.getDialogCreatingTransaction() , request);
                        String host = SipUtilities.getViaHost(response);
                        int port = SipUtilities.getViaPort(response);
                        ItspAccountInfo itspInfo = Gateway.getAccountManager().getItspAccount(host, port);
                        newContext.setItspInfo(itspInfo);
                        newContext.setRtpSession(context.getRtpSession());
                        /*
                         * At this point we only do one half of the association
                         * with the peer dialog. When the ACK is sent, the other
                         * half of the association is established.
                         */
                        newContext.setPeerDialog(context.getPeerDialog());
                        dialog.setApplicationData(newContext);  
                        break;
                    }
                }
                
                
                /*
                 * Could not find the original dialog context.
                 * This means the fork response came in too late. Send BYE
                 * to that leg.
                 */
                if ( dialog.getApplicationData() == null  ) {
                    logger.debug("callLegId = " + callLegId);
                    logger.debug("dialogTable = " + b2bua.dialogTable);
                    b2bua.tearDown(Gateway.SIPXBRIDGE_USER, ReasonCode.FORK_TIMED_OUT, "Fork timed out"); 
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
                    dialog.delete();
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
                                /*
                                 * We have another hop to try. OK send it to the 
                                 * other side.
                                 */
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

        logger.debug("Transaction terminated event");
    }

    /**
     * Dialog timed out (resending INVITE or sending BYE).
     */
    public void processDialogTimeout(DialogTimeoutEvent dialogTimeoutEvent) {
        
        DialogContext dialogContext = DialogContext.get(dialogTimeoutEvent.getDialog());
        try {
            String reason = dialogTimeoutEvent.getReason().toString();
            dialogContext.sendBye(true,reason);
        } catch (Exception ex) {
            logger.error("Exception sending BYE on timed out Dialog",ex);
        }
    
    }

}
