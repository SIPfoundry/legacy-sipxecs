/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.ClientTransactionExt;

import java.text.ParseException;
import java.util.ListIterator;
import java.util.TimerTask;

import javax.sip.ClientTransaction;
import javax.sip.InvalidArgumentException;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionState;
import javax.sip.address.Hop;
import javax.sip.address.SipURI;
import javax.sip.header.AuthorizationHeader;
import javax.sip.header.ContactHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

/**
 * The registration manager. It refreshes registrations etc.
 *
 * @author root
 *
 */
public class RegistrationManager {

    private static Logger logger = Logger.getLogger(RegistrationManager.class);

 
    public RegistrationManager() {
    }

    public void sendRegistrer(ItspAccountInfo itspAccount, String callId, long cseq) throws SipException {
        Request request = SipUtilities.createRegistrationRequest(itspAccount.getSipProvider(),
                itspAccount,callId,cseq);
        ClientTransaction ct = itspAccount.getSipProvider().getNewClientTransaction(request);
        TransactionContext tad = new TransactionContext(ct,
                Operation.SEND_REGISTER);
        tad.setItspAccountInfo(itspAccount);
        Hop hop = ( (ClientTransactionExt) ct).getNextHop();
        itspAccount.setHopToRegistrar(hop);
        ct.sendRequest();
        itspAccount.setState(AccountState.AUTHENTICATING);


    }

    /**
     * Send a de-register to the specified ITSP account.
     *
     * @param itspAccount -
     *            itsp account with which we want to de-register.
     * @throws SipXbridgeException -
     *             problem with gateway configuration.
     * @throws SipException -
     *             if protocol exception occured.
     */
    public void sendDeregister(ItspAccountInfo itspAccount)
            throws SipXbridgeException, SipException {
        Request request = SipUtilities.createDeregistrationRequest( itspAccount);
        ClientTransaction ct = itspAccount.getSipProvider().getNewClientTransaction(request);
        TransactionContext tad = TransactionContext.attach(ct,
                Operation.SEND_DEREGISTER);
        tad.setItspAccountInfo(itspAccount);

        ct.sendRequest();

    }

    /**
     * Sends a registration query.
     *
     * @param itspAccount --
     *            the ITSP account.
     * @throws SipXbridgeException
     * @throws SipException
     * @throws Exception
     */
    public void sendRegisterQuery(ItspAccountInfo itspAccount)
            throws SipXbridgeException, SipException {
        Request request = SipUtilities.createRegisterQuery(itspAccount);
        ClientTransaction ct = itspAccount.getSipProvider().getNewClientTransaction(request);
        TransactionContext tad = new TransactionContext(ct,
                Operation.SEND_REGISTER_QUERY);
        tad.setItspAccountInfo(itspAccount);

        ct.sendRequest();
    }

    /**
     * Handle the OK response from a Register request. If the original request
     * was a registration attempt and the response is an OK we start a timer to
     * re-register after the current registration expires.
     *
     * @param responseEvent
     * @throws InvalidArgumentException
     *
     */
    @SuppressWarnings("unchecked")
    public void processResponse(ResponseEvent responseEvent)
            throws SipXbridgeException, SipException, InvalidArgumentException {

        Response response = responseEvent.getResponse();
        logger.debug("registrationManager.processResponse() "
                + response.getStatusCode());
        ClientTransaction ct = responseEvent.getClientTransaction();
        if ( ct == null ) {
            logger.warn("Null transaction. Probably delayed response. Dropping response");
            return;
        }
        Request request = ct.getRequest();
        ContactHeader requestContactHeader = (ContactHeader) request
                .getHeader(ContactHeader.NAME);
        SipURI requestContactUri = (SipURI) requestContactHeader.getAddress()
                .getURI();
        TransactionContext transactionContext = TransactionContext.get(ct);
        ItspAccountInfo itspAccount = transactionContext.getItspAccountInfo();

        if (!itspAccount.isRegisterOnInitialization() ) {
            /*
             * Somebody sent a request to the ITSP from within the PBX and we are not handling the
             * REGISTER for this account. In that case we simply proxy  the response.
             */
            ServerTransaction serverTransaction = transactionContext.getServerTransaction();
            if ( serverTransaction.getState() != TransactionState.TERMINATED) {
                Response newResponse = SipUtilities.createResponse(serverTransaction, response.getStatusCode());
                SipUtilities.copyHeaders(response, newResponse);
                serverTransaction.sendResponse(newResponse);
            }
        } else if (response.getStatusCode() == Response.OK) {

            ListIterator contactHeaders = (ListIterator) response.getHeaders(ContactHeader.NAME);
            int time = 0;

            if (contactHeaders != null && contactHeaders.hasNext()) {
                while (contactHeaders.hasNext()) {
                    ContactHeader contactHeader = (ContactHeader) contactHeaders
                    .next();
                    SipURI responseContactUri = (SipURI) contactHeader
                    .getAddress().getURI();
                    int port = ((SipURI) contactHeader.getAddress().getURI())
                    .getPort();
                    if (port == -1) {
                        port = 5060;
                    }
                    logger.debug("checking contact " + contactHeader);
                    if (responseContactUri.getHost().equals(
                            requestContactUri.getHost())
                            && requestContactUri.getPort() == port) {
                        time = contactHeader.getExpires();
                        // No contact parameter present but there may be an Expires header.
                        if ( time == -1 ) {
                            if ( response.getExpires() != null ) {
                                time = response.getExpires().getExpires();
                            } else {
                                // This is a protocol error but we keep retrying anyway.
                                logger.warn("ITSP did not return an Expires interval for the contact - " +
                                "using Expires from the Request");
                                time = ct.getRequest().getExpires().getExpires();
                            }
                        }
                        break;
                    }
                }
            } else {
                time = ct.getRequest().getExpires().getExpires();
            }

            if (itspAccount.getSipKeepaliveMethod().equals("REGISTER")) {
                time = ct.getRequest().getExpires().getExpires();
            }

            if (time == 0 ) {
                logger.warn("ITSP did not return a contact address matching the REGISTER contact address - report this to your ITSP\n");
                time = itspAccount.getRegistrationInterval();
            }

            if (time > 2 * Gateway.REGISTER_DELTA) {
                time = time - Gateway.REGISTER_DELTA;
            }


            if (itspAccount.isAlarmSent() && time > 0) {
                try {
                    Gateway.getAlarmClient().raiseAlarm(
                            Gateway.SIPX_BRIDGE_ACCOUNT_OK,
                            itspAccount.getProxyDomain());
                    itspAccount.setAlarmSent(false);
                } catch (Exception ex) {
                    logger.error("Could not send alarm ", ex);
                }
            }

            itspAccount.setState(AccountState.AUTHENTICATED);

            if (itspAccount.getSipKeepaliveMethod().equals("CR-LF")) {
                itspAccount.startCrLfTimerTask();

            }
            logger.debug("time = " + time + " Seconds ");
            if (time > 0) {
                if (itspAccount.registrationTimerTask != null)
                    itspAccount.registrationTimerTask.cancel();
                String callId = SipUtilities.getCallId(response);
                long cseq = SipUtilities.getSeqNumber(response);
                TimerTask ttask = new RegistrationTimerTask(itspAccount,callId,cseq);
                Gateway.getTimer().schedule(ttask, time * 1000);
            }
      } else {
            if (response.getStatusCode() == Response.FORBIDDEN) {
                if (!itspAccount.isAlarmSent()) {
                    try {
                        Gateway.getAlarmClient().raiseAlarm(
                                Gateway.SIPX_BRIDGE_AUTHENTICATION_FAILED,
                                itspAccount.getSipDomain());
                        itspAccount.setAlarmSent(true);
                    } catch (Exception ex) {
                        logger.debug("Could not send alarm", ex);
                    }
                }
                /*
                 * Retry the server again after 60 seconds.
                 */
                if (itspAccount.registrationTimerTask == null) {
                    TimerTask ttask = new RegistrationTimerTask(itspAccount,null,1L);
                    Gateway.getTimer().schedule(ttask, 60 * 1000);
                }
            } else if (response.getStatusCode() == Response.REQUEST_TIMEOUT) {
                if (itspAccount.getSipKeepaliveMethod().equals("CR-LF")) {
                    itspAccount.stopCrLfTimerTask();
                }
                try {
                    if (!itspAccount.isAlarmSent()) {
                        Gateway.getAlarmClient().raiseAlarm(
                                Gateway.SIPX_BRIDGE_OPERATION_TIMED_OUT,
                                itspAccount.getSipDomain());
                        itspAccount.setAlarmSent(true);
                    }
                } catch (Exception ex) {
                    logger.error("Could not send alarm.", ex);
                }
                /*
                 * Retry the server again after 60 seconds.
                 */
                if (itspAccount.registrationTimerTask == null) {
                    TimerTask ttask = new RegistrationTimerTask(itspAccount,null,1L);
                    Gateway.getTimer().schedule(ttask, 60 * 1000);
                }
            } else if (response.getStatusCode() / 100 == 5
                    || response.getStatusCode() / 100 == 6
                    || response.getStatusCode() / 100 == 4) {
                if (itspAccount.getSipKeepaliveMethod().equals("CR-LF")) {
                    itspAccount.stopCrLfTimerTask();
                }
                if (!itspAccount.isAlarmSent()) {
                    try {
                        Gateway.getAlarmClient().raiseAlarm(
                                Gateway.SIPX_BRIDGE_ITSP_SERVER_FAILURE,
                                itspAccount.getSipDomain());
                        itspAccount.setAlarmSent(true);
                    } catch (Exception ex) {
                        logger.debug("Could not send alarm", ex);
                    }
                }
                /*
                 * Retry the server again after 60 seconds.
                 */
                if (itspAccount.registrationTimerTask == null) {
                    TimerTask ttask = new RegistrationTimerTask(itspAccount,null,1L);
                    Gateway.getTimer().schedule(ttask, 60 * 1000);
                }
            } else {
                if (response.getStatusCode() != 100) {
                    logger
                            .warn("RegistrationManager: Unexpected Status Code seen "
                                    + response.getStatusCode());
                }
            }
        }
    }

    /**
     * Handle a timeout event ( happens when you pont this to a non existant
     * ITSP ).
     *
     * @param timeoutEvent
     */
    public void processTimeout(TimeoutEvent timeoutEvent) {
        ClientTransaction ctx = timeoutEvent.getClientTransaction();
        ItspAccountInfo itspAccount = ((TransactionContext) ctx
                .getApplicationData()).getItspAccountInfo();
        /*
         * Try again to register after 30 seconds ( maybe somebody pulled the
         * plug).
         */
        if (itspAccount.registrationTimerTask == null) {
            TimerTask ttask = new RegistrationTimerTask(itspAccount,null,1L);
            Gateway.getTimer().schedule(ttask, 60 * 1000);
        }
        try {
            if (!itspAccount.isAlarmSent()) {
                Gateway.getAlarmClient().raiseAlarm(
                        Gateway.SIPX_BRIDGE_OPERATION_TIMED_OUT,
                        itspAccount.getSipDomain());
                itspAccount.setAlarmSent(true);
            }
        } catch (Exception ex) {
            logger.error("Could not send alarm.", ex);
        }

    }

    public void proxyRegisterRequest(RequestEvent requestEvent, ItspAccountInfo itspAccount) throws
        SipException, ParseException, InvalidArgumentException  {

        ServerTransaction st = requestEvent.getServerTransaction();
       

        if (st == null) {
            st = itspAccount.getSipProvider().getNewServerTransaction(requestEvent.getRequest());
        }

        Request request = requestEvent.getRequest();

        if (itspAccount.isRegisterOnInitialization()) {
            Response response = ProtocolObjects.messageFactory
            .createResponse(Response.METHOD_NOT_ALLOWED, request);
            response.setReasonPhrase("sipXbridge handles registration for this account");
            st.sendResponse(response);
            return;

        }
        TransactionContext tad = TransactionContext.attach(st,Operation.PROXY_REGISTER_REQUEST);
        String callId = SipUtilities.getCallId(request);
        Request newRequest = SipUtilities.createRegistrationRequest(Gateway.getWanProvider(itspAccount.getOutboundTransport()), itspAccount,
                callId, SipUtilities.getSeqNumber(request));
        SipUtilities.setGlobalAddresses(newRequest);
        if ( request.getHeader(AuthorizationHeader.NAME) != null ) {
            // Need to fix up the authorization header to point to the ITSP
            AuthorizationHeader authorizationHeader = (AuthorizationHeader) request.getHeader(AuthorizationHeader.NAME);
            newRequest.setHeader(authorizationHeader);
        }
        ClientTransaction newClientTransaction = Gateway.getWanProvider(itspAccount.getOutboundTransport()).getNewClientTransaction(newRequest);
        SipUtilities.addWanAllowHeaders(newRequest);
        tad.setClientTransaction(newClientTransaction);
        tad.setItspAccountInfo(itspAccount);
        Hop hop = ((ClientTransactionExt)newClientTransaction).getNextHop();
        itspAccount.setHopToRegistrar(hop);
        newClientTransaction.sendRequest();
    }

}
