/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.util.TimerTask;

import javax.sip.ClientTransaction;
import javax.sip.ResponseEvent;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.TimeoutEvent;
import javax.sip.header.AllowHeader;
import javax.sip.header.ContactHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

/**
 * The registration manager. It refreshes registrations etc.
 * 
 * @author root
 * 
 */
public class RegistrationManager {
    private SipProvider provider;

    public RegistrationManager(SipProvider sipProvider) {
        this.provider = sipProvider;

    }

    public void sendRegistrer(ItspAccountInfo itspAccount) throws SipException,
            GatewayConfigurationException {
        Request request = SipUtilities.createRegistrationRequest(provider,
                itspAccount);
        ClientTransaction ct = provider.getNewClientTransaction(request);
        TransactionApplicationData tad = new TransactionApplicationData(
                Operation.SEND_REGISTER);
        itspAccount.setState(AccountState.AUTHENTICATING);
        tad.itspAccountInfo = itspAccount;

        ct.setApplicationData(tad);
        ct.sendRequest();

    }

    public void sendDeregister(ItspAccountInfo itspAccount)
            throws GatewayConfigurationException, SipException {
        Request request = SipUtilities.createDeregistrationRequest(provider,
                itspAccount);
        ClientTransaction ct = provider.getNewClientTransaction(request);
        TransactionApplicationData tad = new TransactionApplicationData(
                Operation.SEND_DEREGISTER);
        tad.itspAccountInfo = itspAccount;
        ct.setApplicationData(tad);

        ct.sendRequest();

    }

    /**
     * Sends a registration query.
     * 
     * @param itspAccount --
     *            the ITSP account.
     * @throws GatewayConfigurationException
     * @throws SipException
     * @throws Exception
     */
    public void sendRegisterQuery(ItspAccountInfo itspAccount)
            throws GatewayConfigurationException, SipException {
        Request request = SipUtilities.createRegisterQuery(provider,
                itspAccount);
        ClientTransaction ct = provider.getNewClientTransaction(request);
        TransactionApplicationData tad = new TransactionApplicationData(
                Operation.SEND_REGISTER_QUERY);
        tad.itspAccountInfo = itspAccount;

        ct.setApplicationData(tad);
        ct.sendRequest();
    }

    /**
     * Handle the OK response from a Register request. If the original request
     * was a registration attempt and the response is an OK we start a timer to
     * re-register after the current registration expires.
     * 
     * @param responseEvent
     */
    public void processResponse(ResponseEvent responseEvent)
            throws GatewayConfigurationException, SipException {

        Response response = responseEvent.getResponse();
        ClientTransaction ct = responseEvent.getClientTransaction();

        if (response.getStatusCode() == Response.OK) {
            ContactHeader contactHeader = (ContactHeader) response
                    .getHeader(ContactHeader.NAME);
            int time = 0;

            if (contactHeader != null)
                time = contactHeader.getExpires();
            else
                time = ct.getRequest().getExpires().getExpires();
            ItspAccountInfo itspAccount = ((TransactionApplicationData) ct
                    .getApplicationData()).itspAccountInfo;
            itspAccount.setState(AccountState.AUTHENTICATED);

            if (time > 0) {
                TimerTask ttask = new RegistrationTimerTask(itspAccount);
                Gateway.timer.schedule(ttask, time * 1000);
            }
            

        } else {
            ItspAccountInfo itspAccount = ((TransactionApplicationData) ct
                    .getApplicationData()).itspAccountInfo;
            itspAccount.setState(AccountState.AUTHENTICATION_FAILED);
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
        ItspAccountInfo itspAccount = ((TransactionApplicationData) ctx
                .getApplicationData()).itspAccountInfo;
        itspAccount.setState(AccountState.AUTHENTICATION_FAILED);

    }

}
