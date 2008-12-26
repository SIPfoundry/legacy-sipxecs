/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.util.ListIterator;
import java.util.TimerTask;

import javax.sip.ClientTransaction;
import javax.sip.ResponseEvent;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.TimeoutEvent;
import javax.sip.address.SipURI;
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

    private SipProvider provider;

    public RegistrationManager(SipProvider sipProvider) {
        this.provider = sipProvider;

    }

    public void sendRegistrer(ItspAccountInfo itspAccount) throws SipException {
        Request request = SipUtilities.createRegistrationRequest(provider, itspAccount);
        ClientTransaction ct = provider.getNewClientTransaction(request);
        TransactionContext tad = new TransactionContext(ct,Operation.SEND_REGISTER);
        itspAccount.setState(AccountState.AUTHENTICATING);
        tad.itspAccountInfo = itspAccount;

      
        ct.sendRequest();

    }

    /**
     * Send a de-register to the specified ITSP account.
     * 
     * @param itspAccount - itsp account with which we want to de-register.
     * @throws SipXbridgeException - problem with gateway configuration.
     * @throws SipException - if protocol exception occured.
     */
    public void sendDeregister(ItspAccountInfo itspAccount) throws SipXbridgeException,
            SipException {
        Request request = SipUtilities.createDeregistrationRequest(provider, itspAccount);
        ClientTransaction ct = provider.getNewClientTransaction(request);
        TransactionContext tad = TransactionContext.attach(ct,Operation.SEND_DEREGISTER);
        tad.itspAccountInfo = itspAccount;
      
        ct.sendRequest();

    }

    /**
     * Sends a registration query.
     * 
     * @param itspAccount -- the ITSP account.
     * @throws SipXbridgeException
     * @throws SipException
     * @throws Exception
     */
    public void sendRegisterQuery(ItspAccountInfo itspAccount)
            throws SipXbridgeException, SipException {
        Request request = SipUtilities.createRegisterQuery(provider, itspAccount);
        ClientTransaction ct = provider.getNewClientTransaction(request);
        TransactionContext tad = new TransactionContext(
                ct, Operation.SEND_REGISTER_QUERY);
        tad.itspAccountInfo = itspAccount;

      
        ct.sendRequest();
    }

    /**
     * Handle the OK response from a Register request. If the original request was a registration
     * attempt and the response is an OK we start a timer to re-register after the current
     * registration expires.
     * 
     * @param responseEvent
     */
    @SuppressWarnings("unchecked")
	public void processResponse(ResponseEvent responseEvent)
            throws SipXbridgeException, SipException {

        Response response = responseEvent.getResponse();
        logger.debug("registrationManager.processResponse() " + response.getStatusCode());
        ClientTransaction ct = responseEvent.getClientTransaction();
        Request request = ct.getRequest();
        ContactHeader requestContactHeader = (ContactHeader) request.getHeader(ContactHeader.NAME);
        SipURI requestContactUri = (SipURI) requestContactHeader.getAddress().getURI();

        if (response.getStatusCode() == Response.OK) {
            ListIterator contactHeaders = (ListIterator) response.getHeaders(ContactHeader.NAME);
            int time = 0;

            if (contactHeaders != null && contactHeaders.hasNext()) {
                while (contactHeaders.hasNext()) {
                    ContactHeader contactHeader = (ContactHeader) contactHeaders.next();
                    SipURI responseContactUri = (SipURI)contactHeader.getAddress().getURI();
                    int port = ((SipURI)contactHeader.getAddress().getURI()).getPort();
                    if ( port == -1 ) {
                        port = 5060;
                    }
                    logger.debug("checking contact " + contactHeader);
                    if ( responseContactUri.getHost().equals(requestContactUri.getHost()) &&
                            requestContactUri.getPort() == port ) {
                        time = contactHeader.getExpires();
                        break;
                    }
                }
            } else {
                time = ct.getRequest().getExpires().getExpires();
            }
            ItspAccountInfo itspAccount = ((TransactionContext) ct.getApplicationData()).itspAccountInfo;
            if (itspAccount.getSipKeepaliveMethod().equals("REGISTER")) {
                time = ct.getRequest().getExpires().getExpires();
            }

            itspAccount.setState(AccountState.AUTHENTICATED);

            if (itspAccount.getSipKeepaliveMethod().equals("CR-LF")) {
                itspAccount.startCrLfTimerTask();

            }
            logger.debug("time = " + time + " Seconds ");
            if (time > 0 && itspAccount.registrationTimerTask == null) {
                TimerTask ttask = new RegistrationTimerTask(itspAccount);
                Gateway.getTimer().schedule(ttask, time * 1000);
            }

        } else {
            /* Authentication failed. Try again after 30 seconds */
            ItspAccountInfo itspAccount = ((TransactionContext) ct.getApplicationData()).itspAccountInfo;
            itspAccount.setState(AccountState.AUTHENTICATION_FAILED);
            if (itspAccount.getSipKeepaliveMethod().equals("CR-LF")) {
                itspAccount.stopCrLfTimerTask();

            }
            if (itspAccount.registrationTimerTask == null) {
                TimerTask ttask = new RegistrationTimerTask(itspAccount);
                Gateway.getTimer().schedule(ttask, 30 * 1000);
            }

        }
    }

    /**
     * Handle a timeout event ( happens when you pont this to a non existant ITSP ).
     * 
     * @param timeoutEvent
     */
    public void processTimeout(TimeoutEvent timeoutEvent) {
        ClientTransaction ctx = timeoutEvent.getClientTransaction();
        ItspAccountInfo itspAccount = ((TransactionContext) ctx.getApplicationData()).itspAccountInfo;
        /*
         * Try again to register after 30 seconds ( maybe somebody pulled the plug).
         */
        if (itspAccount.registrationTimerTask == null) {
            TimerTask ttask = new RegistrationTimerTask(itspAccount);
            Gateway.getTimer().schedule(ttask, 30 * 1000);
        }

    }

}
