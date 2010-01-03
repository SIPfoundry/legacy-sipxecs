package org.sipfoundry.siptester;

import java.util.Iterator;
import java.util.TimerTask;

import gov.nist.javax.sip.DialogTimeoutEvent;
import gov.nist.javax.sip.SipListenerExt;
import gov.nist.javax.sip.clientauthutils.SecureAccountManager;

import javax.sip.ClientTransaction;
import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.SipProvider;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.address.SipURI;
import javax.sip.header.ContactHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

public class RegistrationManager implements SipListenerExt {

    private static Logger logger = Logger.getLogger(RegistrationManager.class);

    private EmulatedEndpoint endpoint;
    private String userName;

    private RegistrationTimerTask registrationTimerTask;

    class RegistrationTimerTask extends TimerTask {
        private String callId;
        private long cseq;

        public RegistrationTimerTask(String callId, long cseq) {
            this.callId = callId;
            this.cseq = cseq;
        }

        @Override
        public void run() {
            // TODO Auto-generated method stub
            try {
                cseq++;
                SipProvider sipProvider = endpoint.getProvider(endpoint.getSutUA()
                        .getDefaultTransport());
                Request registrationRequest = SipUtilities.createRegistrationRequest(sipProvider,
                        userName, endpoint, callId, cseq);
                ClientTransaction clientTransaction = sipProvider
                        .getNewClientTransaction(registrationRequest);
                clientTransaction.sendRequest();
            } catch (Exception ex) {
                ex.printStackTrace();
                logger.error("timer task failed re-register", ex);
                System.exit(-1);
            }
        }

    }

    public RegistrationManager(String userName, EmulatedEndpoint endpoint) {

        this.endpoint = endpoint;
        this.userName = userName;

    }

    public void sendRegistrationRequest() throws Exception {
        SipProvider sipProvider = endpoint.getProvider(endpoint.getSutUA().getDefaultTransport());
        Request registrationRequest = SipUtilities.createRegistrationRequest(sipProvider,
                userName, endpoint, null, 1L);
        ClientTransaction clientTransaction = sipProvider
                .getNewClientTransaction(registrationRequest);
        clientTransaction.setApplicationData(this);
        clientTransaction.sendRequest();
    }
    
    public void sendDeRegistrationRequest() throws Exception {
        SipProvider sipProvider = endpoint.getProvider(endpoint.getSutUA().getDefaultTransport());
        
        Request registrationRequest = SipUtilities.createRegistrationRequest(sipProvider,
                userName, endpoint, null, 1L);
        registrationRequest.getExpires().setExpires(0);
        ClientTransaction clientTransaction = sipProvider
                .getNewClientTransaction(registrationRequest);
        clientTransaction.setApplicationData(this);
        clientTransaction.sendRequest();
    }

    public void processResponse(ResponseEvent responseEvent) {
        Response response = responseEvent.getResponse();
        long cseq = SipUtilities.getSequenceNumber(response);
        String callId = SipUtilities.getCallId(response);
        Request request = responseEvent.getClientTransaction().getRequest();
        ContactHeader requestContact = (ContactHeader) request.getHeader(ContactHeader.NAME);
        SipURI requestContactUri = (SipURI) requestContact.getAddress().getURI();

        if (response.getStatusCode() == Response.PROXY_AUTHENTICATION_REQUIRED
                || response.getStatusCode() == Response.UNAUTHORIZED) {
            try {
                ClientTransaction clientTransaction = endpoint.getStackBean()
                        .getAuthenticationHelper().handleChallenge(response,
                                responseEvent.getClientTransaction(),
                                (SipProvider) responseEvent.getSource(), 5);
                clientTransaction.setApplicationData(this);
                clientTransaction.sendRequest();
            } catch (Exception ex) {
                logger.error("Unexpected exception ", ex);
                ex.printStackTrace();
                System.exit(-1);
            }
        } else if (response.getStatusCode() == Response.OK) {
            int time = 0;
            Iterator contactHeaders = response.getHeaders(ContactHeader.NAME);
            if (contactHeaders != null && contactHeaders.hasNext()) {
                while (contactHeaders.hasNext()) {
                    ContactHeader contactHeader = (ContactHeader) contactHeaders.next();
                    SipURI responseContactUri = (SipURI) contactHeader.getAddress().getURI();
                    int port = ((SipURI) contactHeader.getAddress().getURI()).getPort();
                    if (port == -1) {
                        port = 5060;
                    }
                    logger.debug("checking contact " + contactHeader);
                    if (responseContactUri.getHost().equals(requestContactUri.getHost())
                            && requestContactUri.getPort() == port) {
                        time = contactHeader.getExpires();
                        // No contact parameter present but there may be an Expires header.
                        if (time == -1) {
                            if (response.getExpires() != null) {
                                time = response.getExpires().getExpires();
                            } else {
                                // This is a protocol error but we keep retrying anyway.
                                logger
                                        .warn("sipx proxy did not return an Expires interval for the contact - "
                                                + "using Expires from the Request");
                                time = responseEvent.getClientTransaction().getRequest()
                                        .getExpires().getExpires();
                            }
                        }
                        break;
                    }
                }
            } else {
                time = responseEvent.getClientTransaction().getRequest().getExpires()
                        .getExpires();
            }
            if (this.registrationTimerTask == null && time > 0) {
                this.registrationTimerTask = new RegistrationTimerTask(callId, cseq);
                SipTester.timer.schedule(registrationTimerTask, time*1000, time * 1000);
            }
        } else {
            throw new SipTesterException("Unexpected response");
        }
    }

    @Override
    public void processDialogTimeout(DialogTimeoutEvent dialogTimeoutEvent) {
        throw new SipTesterException("Unexpected method call");
    }

    @Override
    public void processDialogTerminated(DialogTerminatedEvent arg0) {
        throw new SipTesterException("Unexpected method call");
        
    }

    @Override
    public void processIOException(IOExceptionEvent arg0) {
         throw new SipTesterException("Unexpected method call");
        
    }

    @Override
    public void processRequest(RequestEvent requestEvent) {
        logger.debug("processRequest " + requestEvent.getRequest().getMethod());
    }

    @Override
    public void processTimeout(TimeoutEvent timeoutEvent) {
        System.err.println("ProcessTimeout "  + timeoutEvent.getClientTransaction().getRequest());
        System.exit(-1);
    }

    @Override
    public void processTransactionTerminated(TransactionTerminatedEvent transactionTerminatedEvent) {
        // TODO Auto-generated method stub

    }

   

}
