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
import javax.sip.RequestEvent;
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

    private ConcurrentSkipListSet<SipMessage> happensBefore = new ConcurrentSkipListSet<SipMessage>();

    private EmulatedEndpoint endpoint;

    boolean processed;

    /*
     * The remote server transactions that this creates.
     */
    private Collection<SipServerTransaction> serverTransactions = new ConcurrentSkipListSet<SipServerTransaction>();

    private Semaphore preconditionsSem;

    private SipMessage triggeringMessage;

    private boolean waiting;

    private Collection<SipResponse> postCondition = new HashSet<SipResponse>();

    boolean fired;

    public SipClientTransaction(SipRequest sipRequest) {
        this.sipRequest = sipRequest;
        if (sipRequest == null)
            throw new NullPointerException("Null sipRequest");
        if (sipRequest.getTime() < minDelay) {
            minDelay = sipRequest.getTime();
        }
        this.delay = sipRequest.getTime() - minDelay;
    }

    public Collection<SipResponse> getPostCondition() {
        return postCondition;
    }

    public void addPostCondition(SipResponse resp) {
        if (!this.postCondition.contains(resp)) {
            this.postCondition.add(resp);
            logger.debug("addPostCondition " + this.getTime() + " respTime = " + resp.getTime());
            resp.addPermit();
        }
    }

    public int compareTo(SipClientTransaction clientTx) {
        if (this.sipRequest.getTime() == clientTx.sipRequest.getTime())
            return 0;
        else if (this.sipRequest.getTime() < clientTx.getSipRequest().getTime())
            return -1;
        else
            return 1;

    }

    public void addRequest(SipRequest sipRequest) {
        if (sipRequest == null) {
            throw new NullPointerException("null siprequest");
        }
        this.sipRequest = sipRequest;
    }

    public void addResponse(SipResponse sipResponse) {
        logger.debug("SipClientTransaction: addSipResponse : "
                + sipResponse.getSipResponse().getFirstLine());
        this.sipResponses.add(sipResponse);
    }

    public void addMatchingServerTransactions(Collection<SipServerTransaction> serverTransactions) {
        this.serverTransactions = serverTransactions;
        for (SipServerTransaction sst : this.serverTransactions) {
            sst.setMatchingClientTransaction(this);
        }
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
     * Get the Dialog Id corresponding to a response that has both From and To tag assigned to it.
     * We check if the tags match the response tag and return the dialog Id corresponding to that
     * match.
     * 
     * @return dialogId -- the dialog ID corresponding to the response.
     */
    public String getDialogId(ResponseExt response) {
        if (response.getFromHeader().getTag() == null || response.getToHeader().getTag() == null) {
            throw new IllegalArgumentException("To or From tag is null");
        }
        for (SipResponse sipResponse : this.sipResponses) {
            if (sipResponse.getSipResponse().getFromHeader().getTag() != null
                    && sipResponse.getSipResponse().getToHeader().getTag() != null
                    && response.getFromHeader().getTag().equals(
                            sipResponse.getSipResponse().getFromHeader().getTag())
                    && response.getToHeader().getTag().equals(
                            sipResponse.getSipResponse().getToHeader().getTag())) {
                return ((SIPResponse) sipResponse.getSipResponse()).getDialogId(false);
            }
        }
        for (SipResponse sipResponse : this.sipResponses) {
            if (sipResponse.getSipResponse().getFromHeader().getTag() != null
                    && sipResponse.getSipResponse().getToHeader().getTag() != null
                    && response.getFromHeader().getTag().equals(
                            sipResponse.getSipResponse().getFromHeader().getTag())) {
                return ((SIPResponse) sipResponse.getSipResponse()).getDialogId(false);

            }
        }

        return null;
    }

    /**
     * Get the actual client transaction associated with the emulated ctx.
     */
    public void createAndSend() {
        try {
            this.processed = true;
            RequestExt sipRequest = (RequestExt) this.sipRequest.getSipRequest();
            boolean spiral = this.endpoint.isSpiral(sipRequest);
            String transport = sipRequest.getTopmostViaHeader().getTransport();
            SipProviderExt provider = this.endpoint.getProvider(transport);
            logger.debug("createAndSend : " + sipRequest.getMethod() + " transactionId = "
                    + ((SIPRequest) this.getSipRequest().getSipRequest()).getTransactionId());
            System.out.println("sendRequest at frameId = " +
                    + this.sipRequest.getFrameId() + " method = "
                    + sipRequest.getMethod());
            if (this.triggeringMessage != null
                    && triggeringMessage instanceof SipResponse
                    && triggeringMessage.getSipMessage().getCallIdHeader().getCallId().equals(
                            this.getCallId())
                    && (((SipResponse) triggeringMessage).getStatusCode() == Response.PROXY_AUTHENTICATION_REQUIRED || ((SipResponse) triggeringMessage)
                            .getStatusCode() == Response.UNAUTHORIZED)) {
                this.handleChallenge();

            } else if (sipRequest.getFromHeader().getTag() != null
                    && sipRequest.getToHeader().getTag() != null) {
                String method = sipRequest.getMethod();
                if (method.equals(Request.ACK)) {
                    logger.debug("createAndSend ACK");
                    String dialogId = ((SIPRequest) sipRequest).getDialogId(false);

                    logger.debug("dialogId " + dialogId);
                    SipDialog sipDialog = SipTester.getDialog(dialogId,this.endpoint);
                    SipResponse sipResponse = (SipResponse) this.triggeringMessage;
                    for (SipServerTransaction sst : this.serverTransactions) {
                        SipDialog hisDialog = SipTester.getDialog(sst.getDialogId(),this.endpoint);
                        hisDialog.setPeerDialog(sipDialog);
                    }
                    
                    DialogExt dialog = sipDialog.getDialog();
                    
                   
                    /*
                     * If tester is in Dialog Stateful Mode.
                     */
                    if ( dialog != null ) {
                        Request ack = dialog.createAck(SipUtilities.getSequenceNumber(sipResponse
                            .getResponseEvent().getResponse()));
                        SipUtilities.copyHeaders(this.sipRequest,
                            this.triggeringMessage, ack, this.endpoint.getTraceEndpoint());
                        sipDialog.setRequestToSend(ack,spiral);
                        dialog.sendAck(ack);
                       
                    } else {
                        Request ack = SipUtilities.createAck(this.sipRequest, triggeringMessage, endpoint,
                                sipDialog.getLastResponse());
                        SipUtilities.copyHeaders(this.sipRequest,
                                this.triggeringMessage, ack, this.endpoint.getTraceEndpoint());
                        
                        logger.debug(this.getSipRequest().getFrameId() + " Sending stateless ACK " + ack);
                        sipDialog.setRequestToSend(ack,spiral);
                        provider.sendRequest(ack);
                    }
                } else if (method.equals(Request.PRACK)) {
                    if (triggeringMessage instanceof SipRequest) {
                        logger.debug("trigger = " + triggeringMessage.getSipMessage());
                    }
                    String dialogId = ((SIPRequest) sipRequest).getDialogId(false);

                    logger.debug("dialogId " + dialogId);
                    SipDialog sipDialog = SipTester.getDialog(dialogId,this.endpoint);

                    SipResponse sipResponse = (SipResponse) this.triggeringMessage;
                    Dialog dialog = sipResponse.getResponseEvent().getDialog();
                    if (dialog != null) {
                        Response response = sipResponse.getResponseEvent().getResponse();
                        Request prack = dialog.createPrack(response);
                        SipUtilities.copyHeaders(this.sipRequest,
                                this.triggeringMessage, prack, this.endpoint.getTraceEndpoint());
                        ClientTransaction clientTransaction = provider
                                .getNewClientTransaction(prack);
                        clientTransaction.setApplicationData(this);
                        /*
                         * Establish the end to end dialog association.
                         */
                        for (SipServerTransaction sipServerTransaction : this
                                .getMatchingServerTransactions()) {
                            sipServerTransaction.setBranch(SipUtilities
                                    .getCorrelator((RequestExt) prack));
                        }

                        dialog.sendRequest(clientTransaction);
                    } else {
                        Request prack = SipUtilities.createRequest(this.sipRequest, triggeringMessage, endpoint);
                        ClientTransaction clientTransaction = provider
                                .getNewClientTransaction(prack);
                        clientTransaction.setApplicationData(this);
                        /*
                         * Establish the end to end dialog association.
                         */
                        for (SipServerTransaction sipServerTransaction : this
                                .getMatchingServerTransactions()) {
                            sipServerTransaction.setBranch(SipUtilities
                                    .getCorrelator((RequestExt) prack));
                        }

                        clientTransaction.sendRequest();
                    }

                } else {
                    String dialogId = ((SIPRequest) sipRequest).getDialogId(false);

                    logger.debug("dialogId " + dialogId);
                    SipDialog sipDialog = SipTester.getDialog(dialogId,this.endpoint);
                    logger.debug("sipDialog = " + sipDialog + "frameId = "
                            + this.sipRequest.getFrameId() + " method = "
                            + sipRequest.getMethod());
                    Request newRequest;
                    if (sipDialog.getDialog() == null) {
                        newRequest = SipUtilities.createRequest(this.sipRequest, triggeringMessage,
                                endpoint);
                    } else {
                        newRequest = sipDialog.getDialog().createRequest(sipRequest.getMethod());
                    }
                    SipUtilities.copyHeaders(this.sipRequest,
                            this.triggeringMessage, newRequest, this.endpoint.getTraceEndpoint());
                    ClientTransaction clientTransaction = provider
                            .getNewClientTransaction(newRequest);
                    clientTransaction.setApplicationData(this);
                    for (SipServerTransaction sipServerTransaction : this
                            .getMatchingServerTransactions()) {
                        sipServerTransaction.setBranch(SipUtilities
                                .getCorrelator((RequestExt) newRequest));
                        SipDialog serverDialog = sipServerTransaction.getSipDialog();
                        serverDialog.setPeerDialog(sipDialog);
                    }
                    sipDialog.setRequestToSend(newRequest,spiral);
                    if (sipDialog.getDialog() != null)
                        sipDialog.getDialog().sendRequest(clientTransaction);
                    else
                        clientTransaction.sendRequest();
                }
            } else {

                RequestExt newRequest = SipUtilities.createRequest(this.sipRequest, 
                        this.triggeringMessage, endpoint);

                ClientTransaction clientTransaction = provider
                        .getNewClientTransaction(newRequest);
                clientTransaction.setApplicationData(this);

                /*
                 * Mark the corrlated Server tansactions that will be expected to process the
                 * incoming request.
                 */
                for (SipServerTransaction sipServerTransaction : this
                        .getMatchingServerTransactions()) {
                    sipServerTransaction.setBranch(SipUtilities
                            .getCorrelator((RequestExt) newRequest));

                }

                for (String dialogId : this.getDialogIds()) {
                    SipDialog sipDialog = SipTester.getDialog(dialogId,this.endpoint);

                    sipDialog.setRequestToSend(newRequest,spiral);

                }
                clientTransaction.sendRequest();
            }
            
        } catch (Exception ex) {
            SipTester.fail("unexpectedException", ex);
        } finally {
            for (SipResponse sipResponse : this.getPostCondition()) {
                sipResponse.triggerPreCondition();
            }
        }
    }

    /**
     * @param endpoint the endpoint to set
     */
    public void setEndpoint(EmulatedEndpoint endpoint) {
        this.endpoint = endpoint;
    }

    /**
     * @return the endpoint
     */
    public EmulatedEndpoint getEndpoint() {
        return endpoint;
    }

    public void handleChallenge() {
        try {

            Response response = ((SipResponse) triggeringMessage).getResponseEvent()
                    .getResponse();
            AuthenticationHelper authenticationHelper = this.endpoint.getStackBean()
                    .getAuthenticationHelper();
            ResponseEvent responseEvent = ((SipResponse) triggeringMessage).getResponseEvent();
            SipProvider sipProvider = (SipProvider) responseEvent.getSource();
            ClientTransaction challengedTx = responseEvent.getClientTransaction();
            ClientTransaction newClientTransaction = authenticationHelper.handleChallenge(
                    response, challengedTx, sipProvider, 5);
            ResponseExt responseExt = (ResponseExt) response;
            this.processed = true;
            newClientTransaction.setApplicationData(this);
            logger.debug("handleChallenge " + this.getTransactionId());

            for (SipServerTransaction st : this.serverTransactions) {
                logger.debug("setBranch " + st.getTransactionId() + " bid "
                        + newClientTransaction.getBranchId());
                st.setBranch(newClientTransaction.getBranchId());
            }
            if (responseExt.getFromHeader().getTag() != null
                    && responseExt.getToHeader().getTag() != null
                    && newClientTransaction.getDialog() != null) {
                newClientTransaction.getDialog().sendRequest(newClientTransaction);
            } else {
                newClientTransaction.sendRequest();
            }
        } catch (Exception ex) {
            SipTester.fail("handleChallenge : unexpected exception", ex);
        }

    }

    public void processResponse(ResponseEvent responseEvent) {
        try {
            logger.debug("processResponse " + responseEvent.getResponse().getStatusCode()
                    + " transactionId " + this.getTransactionId());
            ResponseExt response = (ResponseExt) responseEvent.getResponse();

            for (SipResponse sipResponse : this.sipResponses) {
                if (response.getStatusCode() == sipResponse.getStatusCode()) {
                    sipResponse.setResponseEvent(responseEvent);
                }
            }
            /*
             * If this is a final response check if we have this final response in our set.
             */
            Dialog dialog = responseEvent.getDialog();
            if (response.getFromHeader().getTag() != null
                    && response.getToHeader().getTag() != null) {
                SipDialog sipDialog = SipTester.getDialog(this.getDialogId(response),endpoint);
                if (sipDialog != null) {
                    sipDialog.setLastResponse(response);
                    sipDialog.setDialog((DialogExt) dialog);
                    for (SipServerTransaction sst : serverTransactions) {
                        sst.getSipDialog().setPeerDialog(sipDialog);
                    }
                }
            }

            if (response.getStatusCode() / 100 >= 2) {
                for (SipResponse sipResponse : this.sipResponses) {
                    int statusCode = sipResponse.getSipResponse().getStatusCode();
                    if (statusCode / 100 >= 2 && statusCode != response.getStatusCode()) {
                     
                         SipTester.fail("Unexpected response seen! " + statusCode + " / "
                                + response.getStatusCode() + " transactionId = "
                                + this.getTransactionId());
                    }
                }
            }
            for (SipResponse sipResponse : this.sipResponses) {
                /*
                 * Find a matching sipResponse where the status code matches the response status
                 * code.
                 */
                if (sipResponse.getSipResponse().getStatusCode() == response.getStatusCode()) {
                    if ( sipResponse.getSipResponse().getToHeader().getTag() != null && 
                            response.getToHeader().getTag() != null ) {
                        SipTester.mapToTag(sipResponse.getSipResponse(), response);
                    }
                    if (response.getStatusCode() / 100 >= 2
                            && !SipTester.checkProvisionalResponses()) {
                        for (SipResponse sipResponse1 : this.sipResponses) {
                            for (SipClientTransaction sipClientTransaction : sipResponse1
                                    .getPostConditions()) {
                                if (sipResponse1.getStatusCode() / 100 < 2) {
                                    sipClientTransaction.removePrecondition(sipResponse1);
                                }

                            }
                        }
                    }
                    for (SipClientTransaction sipClientTransaction : sipResponse
                            .getPostConditions()) {
                        sipClientTransaction.removePrecondition(sipResponse);

                    }

                }

            }

        } catch (Exception ex) {
            ex.printStackTrace();
            System.exit(-1);
        }

    }

    public String getFromTag() {
        return this.sipRequest.getSipRequest().getFromHeader().getTag();
    }

    public String getCallId() {
        return this.sipRequest.getSipRequest().getCallIdHeader().getCallId();
    }

    public String getMethod() {
        return this.sipRequest.getSipRequest().getMethod();
    }

    public synchronized void removePrecondition(SipMessage sipMessage) {
        boolean contains = this.happensBefore.remove(sipMessage);
        if (happensBefore.isEmpty() && contains) {
            this.preconditionsSem.release();
        }
    }

    public boolean checkPreconditions() {
        try {
            this.waiting = true;
            boolean retval = this.preconditionsSem.tryAcquire(10, TimeUnit.SECONDS);

            if (!retval) {
                if (!SipTester.failed.getAndSet(true)) {
                    for (EmulatedEndpoint endpoint : SipTester.getEmulatedEndpoints()) {
                        for (SipClientTransaction ctx : endpoint.getClientTransactions()) {
                            if (ctx.waiting) {
                                ctx.debugPrintHappensBefore();
                            }
                        }
                    }

                    SipTester.fail("Could not satisfy precondition");
                }
            }
            this.waiting = false;
            return retval;

        } catch (Exception ex) {
            SipTester.fail("Unexpected exception ", ex);
            return false;
        }
    }

    public void addHappensBefore(SipMessage sipMessage) {

        this.happensBefore.add(sipMessage);
        this.triggeringMessage = this.happensBefore.descendingIterator().next();
    }

    public void printHappensBefore() {

        SipTester.getPrintWriter().println("<happens-before>");
        if (this.triggeringMessage != null) {
            SipTester.getPrintWriter().println(
                    "<trigger>" + this.triggeringMessage.getFrameId() + "</trigger>");
            SipTester.getPrintWriter().println(this.triggeringMessage.getMessageAsXmlComment());
        }
        for (SipMessage sipMessage : this.happensBefore) {
            SipTester.getPrintWriter().println(
                    "<sip-message>" + sipMessage.getFrameId() + "</sip-message>");
            SipTester.getPrintWriter().println(sipMessage.getMessageAsXmlComment());
        }

        SipTester.getPrintWriter().println("</happens-before>");
    }

    public void printResponses() {
        SipTester.getPrintWriter().println("<responses>");
        for (SipResponse sipResponse : this.sipResponses) {
            SipTester.getPrintWriter().println("<response>");
            SipTester.getPrintWriter().println(
                    "<sip-response>" + sipResponse.getFrameId() + "</sip-response>");
            SipTester.getPrintWriter().println(sipResponse.getMessageAsXmlComment());
            SipTester.getPrintWriter().println("<post-condition>");
            for (SipClientTransaction postCondition : sipResponse.getPostConditions()) {
                SipTester.getPrintWriter().println(
                        "<triggers-transaction>" + postCondition.getTransactionId()
                                + "</triggers-transaction>");
            }
            SipTester.getPrintWriter().println("</post-condition>");
            SipTester.getPrintWriter().println("</response>");
        }
        SipTester.getPrintWriter().println("</responses>");
    }

    public void printTransaction() {
        SipTester.getPrintWriter().println("<sip-client-transaction>");
        SipTester.getPrintWriter().println(
                "<transaction-id>" + this.getTransactionId() + "</transaction-id>");
        SipTester.getPrintWriter().println("<time>" + this.getTime() + "</time>");
        SipTester.getPrintWriter().println(
                "<sip-request>" + this.getSipRequest().getFrameId() + "</sip-request>");
        SipTester.getPrintWriter().println(sipRequest.getMessageAsXmlComment());

        printHappensBefore();
        printResponses();
        printMatchingServerTansactions();
        SipTester.getPrintWriter().println("</sip-client-transaction>\n");

    }

    private void printMatchingServerTansactions() {
        SipTester.getPrintWriter().println("<matching-server-transactions>");
        for (SipServerTransaction sst : this.serverTransactions) {
            SipTester.getPrintWriter().println(
                    "<server-transaction-id>" + sst.getTransactionId()
                            + "</server-transaction-id>");
        }
        SipTester.getPrintWriter().println("</matching-server-transactions>");
    }

    public void setPreconditionSem() {
        this.preconditionsSem = this.happensBefore.isEmpty() ? new Semaphore(1)
                : new Semaphore(0);
    }

    public void debugPrintHappensBefore() {
        System.err.println("************************************************");
        System.err.println("Waiting to start client transaction frameId = " + this.sipRequest.getFrameId() + " "
                + this.sipRequest.getSipMessage().getFirstLine().trim());
        System.err.println("Waiting for preconditions to be satisfied.");
        for (SipMessage sipMessage : this.happensBefore) {
            System.err
                    .println("Waiting for: " + sipMessage.getSipMessage().getFirstLine().trim());
        }
        System.err.println("************************************************");

    }

}
