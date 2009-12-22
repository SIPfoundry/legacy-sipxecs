package org.sipfoundry.siptester;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.SipProviderExt;
import gov.nist.javax.sip.clientauthutils.AuthenticationHelper;
import gov.nist.javax.sip.message.RequestExt;
import gov.nist.javax.sip.message.ResponseExt;
import gov.nist.javax.sip.message.SIPRequest;
import gov.nist.javax.sip.message.SIPResponse;

import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.concurrent.ConcurrentSkipListSet;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.ResponseEvent;
import javax.sip.SipProvider;
import javax.sip.address.SipURI;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

public class SipClientTransaction extends SipTransaction implements
        Comparable<SipClientTransaction> {

    private static Logger logger = Logger.getLogger(SipClientTransaction.class);

    ConcurrentSkipListSet<SipResponse> sipResponses = new ConcurrentSkipListSet<SipResponse>();

    ConcurrentSkipListSet<SipRequest> retransmittedRequests = new ConcurrentSkipListSet<SipRequest>();

    private HashSet<SipMessage> happensBefore = new HashSet<SipMessage>();

    private Endpoint endpoint;

    /*
     * The remote server transactions that this creates.
     */
    private Collection<SipServerTransaction> serverTransactions = new ConcurrentSkipListSet<SipServerTransaction>();


    private Semaphore preconditionsSem = new Semaphore(0);

    private ResponseEvent triggeringResponseEvent;

    public SipClientTransaction(SipRequest sipRequest) {
        this.sipRequest = sipRequest;
        if (sipRequest.getTime() < minDelay) {
            minDelay = sipRequest.getTime();
        }
        this.delay = sipRequest.getTime() - minDelay;
    }

    public int compareTo(SipClientTransaction clientTx) {
        if (this.getDelay() == clientTx.getDelay())
            return 0;
        else if (this.getDelay() < clientTx.getDelay())
            return -1;
        else
            return 1;

    }

    public void addRequest(SipRequest sipRequest) {
        if (this.sipRequest.getLogFile() == sipRequest.getLogFile()) {
            this.retransmittedRequests.add(sipRequest);
        } else {
            logger.trace("Request file differs from request file - discarding");
        }
    }

    public void addResponse(SipResponse sipResponse) {
        if (this.sipRequest.getLogFile() == sipResponse.getLogFile()) {
            logger.debug("SipClientTransaction: addSipResponse : "
                    + sipResponse.getSipResponse().getFirstLine());
            this.sipResponses.add(sipResponse);
        } else {
            logger.trace("Response file differs from request file - discarding");
        }
    }

    public void addMatchingServerTransactions(Collection<SipServerTransaction> serverTransactions) {
        this.serverTransactions = serverTransactions;
    }

    public Collection<SipServerTransaction> getMatchingServerTransactions() {
        return this.serverTransactions;
    }

    /**
     * Get the dialog Ids associated with the clientTx.
     * 
     * @return
     */
    public HashSet<String> getDialogIds() {
        HashSet<String> dialogIds = new HashSet<String>();
        for (SipResponse sipResponse : this.sipResponses) {
            ResponseExt response = sipResponse.getSipResponse();
            if (response.getFromHeader().getTag() != null
                    && response.getToHeader().getTag() != null) {
                dialogIds.add(((SIPResponse) response).getDialogId(false));
            }
        }
        return dialogIds;
    }

    /**
     * Get the actual client transaction associated with the emulated ctx.
     */
    public void createAndSend() {
        try {
            RequestExt sipRequest = (RequestExt) this.sipRequest.getSipRequest();
            String transport = sipRequest.getTopmostViaHeader().getTransport();
            SipProviderExt provider = this.endpoint.getProvider(transport);
            System.out.println("createAndSend : " + sipRequest.getMethod());
           
            if (this.triggeringResponseEvent != null
                    && (triggeringResponseEvent.getResponse().getStatusCode() == Response.PROXY_AUTHENTICATION_REQUIRED || triggeringResponseEvent
                            .getResponse().getStatusCode() == Response.UNAUTHORIZED)) {
                this.handleChallenge();

            } else if (sipRequest.getFromHeader().getTag() != null
                    && sipRequest.getToHeader().getTag() != null) {
                String method = sipRequest.getMethod();
                if (method.equals(Request.ACK)) {
                    logger.debug("createAndSend ACK -- not sending");
                } else if (method.equals(Request.PRACK)) {
                    String dialogId = ((SIPRequest)sipRequest).getDialogId(false);
                    SipDialog sipDialog = endpoint.getDialog(dialogId);
                    Request prack = sipDialog.getDialog()
                            .createPrack(sipDialog.getLastResponse());
                    ClientTransaction clientTransaction = provider.getNewClientTransaction(prack);
                    clientTransaction.setApplicationData(this);
                    for (SipServerTransaction sipServerTransaction : this
                            .getMatchingServerTransactions()) {
                        sipServerTransaction.setBranch(((RequestExt) prack).getTopmostViaHeader()
                                .getBranch());
                    }
                    sipDialog.getDialog().sendRequest(clientTransaction);
                } else {
                    String dialogId = ((SIPRequest)sipRequest).getDialogId(false);
                    SipDialog sipDialog = endpoint.getDialog(dialogId);
                  
                    Request newRequest = sipDialog.getDialog().createRequest(
                            sipRequest.getMethod());
                    ClientTransaction clientTransaction = provider
                            .getNewClientTransaction(newRequest);
                    clientTransaction.setApplicationData(this);
                    for (SipServerTransaction sipServerTransaction : this
                            .getMatchingServerTransactions()) {
                        sipServerTransaction.setBranch(((RequestExt) newRequest)
                                .getTopmostViaHeader().getBranch());
                    }
                    sipDialog.getDialog().sendRequest(clientTransaction);
                }
            } else {

                RequestExt newRequest = SipUtilities.createInviteRequest(this.sipRequest
                        .getSipRequest(), endpoint);
                ClientTransaction clientTransaction = provider
                        .getNewClientTransaction(newRequest);
                clientTransaction.setApplicationData(this);
                
                for (SipServerTransaction sipServerTransaction : this
                        .getMatchingServerTransactions()) {
                    sipServerTransaction.setBranch(((RequestExt) newRequest)
                            .getTopmostViaHeader().getBranch());
                }
                clientTransaction.sendRequest();

            }
        } catch (Exception ex) {
            SipTester.fail("unexpectedException", ex);
        }
    }

    /**
     * @param endpoint the endpoint to set
     */
    public void setEndpoint(Endpoint endpoint) {
        this.endpoint = endpoint;
    }

    /**
     * @return the endpoint
     */
    public Endpoint getEndpoint() {
        return endpoint;
    }


    public void handleChallenge() {
        try {
            ResponseEvent responseEvent = this.triggeringResponseEvent;
            Response response = responseEvent.getResponse();
            AuthenticationHelper authenticationHelper = this.endpoint.getStackBean()
                    .getAuthenticationHelper();
            SipProvider sipProvider = (SipProvider) responseEvent.getSource();
            ClientTransaction challengedTx = responseEvent.getClientTransaction();
            ClientTransaction newClientTransaction = authenticationHelper.handleChallenge(
                    response, challengedTx, sipProvider, 5);
            newClientTransaction.setApplicationData(this);
            newClientTransaction.sendRequest();
        } catch (Exception ex) {
            SipTester.fail("handleChallenge : unexpected exception", ex);
        }

    }

    public void processResponse(ResponseEvent responseEvent) {
        try {
            ResponseExt response = (ResponseExt) responseEvent.getResponse();

            /*
             * If this is a final response check if we have this final response in our set.
             */
            Dialog dialog = responseEvent.getDialog();
            if (response.getFromHeader().getTag() != null && response.getToHeader().getTag() != null  ) {
                SipDialog sipDialog = endpoint.getDialog(((SIPResponse) response).getDialogId(false));
                if (sipDialog != null) {
                    sipDialog.setLastResponse(response);
                    sipDialog.setDialog((DialogExt) dialog );
                }
            }

            if (response.getStatusCode() / 100 >= 2) {
                for (SipResponse sipResponse : this.sipResponses) {
                    int statusCode = sipResponse.getSipResponse().getStatusCode();
                    if (statusCode / 100 >= 2 && statusCode != response.getStatusCode()) {
                        SipTester.fail("Unexpected response seen! " + statusCode + " / "
                                + response.getStatusCode());
                    }
                }
                if (SipUtilities.getCSeqMethod(response).equals(Request.INVITE)
                        && response.getStatusCode() == Response.OK) {

                    long cseq = SipUtilities.getSequenceNumber(response);
                    Request ack = dialog.createAck(cseq);
                    dialog.sendAck(ack);
                }
            }
            for (SipResponse sipResponse : this.sipResponses) {
                /*
                 * Find a matching sipResponse where the status code matches the response status
                 * code.
                 */
                if (sipResponse.getSipResponse().getStatusCode() == response.getStatusCode()) {
                    for (SipClientTransaction sipClientTransaction : sipResponse
                            .getPostConditions()) {
                        sipClientTransaction.setTriggeringResponseEvent(responseEvent);
                        sipClientTransaction.removePrecondition(sipResponse);

                    }
                }
            }

        } catch (Exception ex) {
            ex.printStackTrace();
            System.exit(-1);
        }

    }

    private void setTriggeringResponseEvent(ResponseEvent responseEvent) {
        this.triggeringResponseEvent = responseEvent;
    }

    private void removePrecondition(SipResponse sipResponse) {
        this.happensBefore.remove(sipResponse);
        if (happensBefore.isEmpty()) {
            this.preconditionsSem.release();
        }
    }

    public boolean checkPreconditions() {
        try {
            if (this.happensBefore.isEmpty()) {
                return true;
            } else {
                return this.preconditionsSem.tryAcquire(100, TimeUnit.SECONDS);
            }
        } catch (Exception ex) {
            SipTester.fail("Unexpected exception ", ex);
            return false;
        }
    }

    public void addHappensBefore(SipMessage sipMessage) {
        this.happensBefore.add(sipMessage);
    }

    public void printHappensBefore() {
     
        logger.debug("happensBefore = {");
        for (SipMessage sipMessage : this.happensBefore) {
            logger.debug("sipMessage " + sipMessage.getSipMessage().getFirstLine().trim()
                    + " " + sipMessage.getSipMessage().getCSeqHeader().toString().trim());
        }
        logger.debug("}");
    }

    public void printPostCondition() {
        for (SipResponse sipResponse : this.sipResponses ) {
            logger.debug("sip response " + sipResponse.getSipResponse().getFirstLine() );
            for ( SipClientTransaction sipClientTransaction : sipResponse.getPostConditions() ) {
               logger.debug("frees dependency for ctx " + sipClientTransaction.getSipRequest().getSipRequest().getFirstLine().trim());
            }
        }
    }
    public void printTransaction() {
        RequestExt request = this.getSipRequest().getSipRequest();
        logger.debug("sipClientTransaction {");
        logger.debug(request.getFirstLine().trim());
        printHappensBefore();
        printPostCondition();
        logger.debug("}");
    }

}
