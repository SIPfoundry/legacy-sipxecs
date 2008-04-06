/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.io.File;
import java.io.FileInputStream;
import java.util.Properties;
import java.util.UUID;

import javax.sip.ClientTransaction;
import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.ListeningPoint;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.SipException;
import javax.sip.SipFactory;
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.SipStack;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.header.CSeqHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import org.sipfoundry.sipxbridge.*;

import junit.framework.TestCase;

public class RegistrationTest extends AbstractSipSignalingTest {
    private SipListener sipListener = new MyListener();

    static {
        Logger logger = Logger.getLogger("org.sipfoundry.gateway");
        PropertyConfigurator.configure("log4j.properties");
    }

    class MyListener implements SipListener {

        public void processDialogTerminated(DialogTerminatedEvent dte) {
            TestCase.fail("Unexpected ecent -- DialogTerminatedEvent");

        }

        public void processIOException(IOExceptionEvent ioex) {
            TestCase.fail("Fail -- unexpected event - IOExceptionEvent");

        }

        public void processRequest(RequestEvent requestEvent) {
            TestCase.fail("Fail - unexpected event - RequestEvent");

        }

        public void processResponse(ResponseEvent responseEvent) {
            try {
                Response response = responseEvent.getResponse();
                if (response.getStatusCode() == Response.PROXY_AUTHENTICATION_REQUIRED
                        || response.getStatusCode() == Response.UNAUTHORIZED) {
                    ItspAccountInfo itspAccount = ((TransactionApplicationData) responseEvent
                            .getClientTransaction().getApplicationData()).itspAccountInfo;
                    ClientTransaction newClientTransaction = Gateway
                            .getAuthenticationHelper().handleChallenge(response,
                                    responseEvent.getClientTransaction(),
                                    (SipProvider) responseEvent.getSource());
                    newClientTransaction.sendRequest();
                } else if (response.getStatusCode() == Response.TRYING) {
                    if (((CSeqHeader) response.getHeader(CSeqHeader.NAME))
                            .getMethod().equals(Request.REGISTER)) {

                    }
                } else if (response.getStatusCode() == Response.OK) {
                    if (((CSeqHeader) response.getHeader(CSeqHeader.NAME))
                            .getMethod().equals(Request.REGISTER)) {

                    }
                } else {
                    TestCase.fail("Unenexpected response code "
                            + response.getStatusCode());
                }
            } catch (Exception ex) {
                ex.printStackTrace();
                TestCase.fail("Unexpected exception processing challenge");
            }
        }

        public void processTimeout(TimeoutEvent arg0) {
            TestCase.fail("Unexpected event -- tx time out");

        }

        public void processTransactionTerminated(TransactionTerminatedEvent arg0) {
            // TODO Auto-generated method stub

        }

    }

    public void setUp() throws Exception {
        super.setUp();
        provider.addSipListener(sipListener);

    }

    public SipListener getSipListener() {
        return this.sipListener;
    }

    public void tearDown() throws Exception {
        super.tearDown();

    }

    public void testRegistration() throws Exception {

        for (ItspAccountInfo itspAccount : accountManager.getItspAccounts()) {
            Gateway.getRegistrationManager().sendRegistrer(itspAccount);

        }
        Thread.sleep(5000);

    }

    public void testDeregistration() throws Exception {
        for (ItspAccountInfo itspAccount : accountManager.getItspAccounts()) {
            Gateway.getRegistrationManager().sendDeregister(itspAccount);

        }
        Thread.sleep(5000);
    }

    public void testRegistrationQuery() throws Exception {
        for (ItspAccountInfo itspAccount : accountManager.getItspAccounts()) {
            Gateway.getRegistrationManager().sendRegisterQuery(itspAccount);

        }
        Thread.sleep(5000);
    }

}
