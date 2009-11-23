/*
 * 
 * 
 * Copyright (C) 2009 Nortel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */

package org.sipfoundry.callcontroller;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.clientauthutils.UserCredentialHash;

import java.text.ParseException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.TimerTask;

import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.RequestEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentLengthHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.Header;
import javax.sip.header.ReasonHeader;
import javax.sip.header.SubscriptionStateHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxrest.RestServer;
import org.sipfoundry.sipxrest.SipHelper;
import org.sipfoundry.sipxrest.SipStackBean;

/**
 * One of these associated with a dialog. Stores the current call status. (sent to us via a NOTIFY ).
 */
public class DialogContext {

    private Response lastResponse;
    private SipStackBean stackBean;
    private static Logger logger = Logger.getLogger(DialogContext.class);
    private String key;
    private StringBuffer status = new StringBuffer();
    private HashSet<Dialog> dialogs = new HashSet<Dialog>();
    private HashMap<Dialog, Request> dialogTable = new HashMap<Dialog, Request>();
    public static String HEADER = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            + "\n<status-lines "
            + "xmlns=\"http://www.sipfoundry.org/sipX/schema/xml/call-status-00-00\">";
    public static String FOOTER = "\n</status-lines>\n";

    public static String DIALOGS = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + "\n<dialogs "
            + "xmlns=\"http://www.sipfoundry.org/sipX/schema/xml/call-status-00-00\">\n";
    public static String DIALOGS_FOOTER = "</dialogs>\n";

    private int cacheTimeout;
    private long creationTime = System.currentTimeMillis();
    private UserCredentialHash userCredentials;
    private String method;
    private PendingOperation pendingOperation;
    private Dialog firstDialog;
    private int timeout;
    private String agent;
    private String callingParty;
    private String calledParty;

    public DialogContext(String key, int timeout, int cachetimeout, String method) {
        this.key = key;
        this.method = method;
        logger.debug("DialogContext : " + timeout + " cachetimeout " + cachetimeout + "method = "
                + method);
        status.append(HEADER);
        this.cacheTimeout = cachetimeout;
        this.method = method;
        this.timeout = timeout;

        RestServer.timer.schedule(new TimerTask() {

            @Override
            public void run() {
                for (Dialog dialog : dialogs) {
                    try {
                        if (dialog.getState() != DialogState.CONFIRMED
                                && dialog.getState() != DialogState.TERMINATED) {
                            SipListenerImpl.getInstance().getHelper().tearDownDialog(dialog);
                        } else {
                            logger.debug("Not firing BYE message " + dialog.getState());
                        }
                    } catch (Exception ex) {
                        logger.error("Exception caught", ex);
                    }
                }
            }

        }, (timeout + 1) * 1000);
    }

    public void remove() {
        try {
            HashSet<Dialog> removeSet = new HashSet<Dialog>();
            for (Dialog dialog : dialogs) {
                if (dialog.getState() != DialogState.TERMINATED) {
                    SipListenerImpl.getInstance().getHelper().tearDownDialog(dialog);
                } else {
                    removeSet.add(dialog);
                }

            }
            for (Dialog dialog : removeSet) {
                this.removeMe(dialog);
            }
        } catch (Exception ex) {
            logger.error("Exception caught", ex);
        }
    }

    public void setStatus(String callId, String method, String status) {
        this.status.append("\n<status>");
        this.status.append("\n\t<timestamp>" + System.currentTimeMillis() + "</timestamp>");
        this.status.append("\n\t<call-id>" + callId + "</call-id>");
        this.status.append("\n\t<method>" + method + "</method>");
        this.status.append("\n\t<status-line>" + status.trim() + "</status-line>");
        this.status.append("\n</status>");
    }

    /**
     * @return the status
     */
    public String getStatus() {
        if (method.equalsIgnoreCase(Request.REFER)) {
            return status.toString() + FOOTER;
        } else {
            StringBuffer sb = new StringBuffer();
            sb.append(DIALOGS);
            for (Dialog dialog : this.dialogs) {
                sb.append("\t<dialog>\n");
                sb.append("\t\t<dialog-id>" + dialog.getDialogId() + "</dialog-id>\n");
                sb.append("\t\t<local-party>" + dialog.getLocalParty().getURI().toString()
                        + "</local-party>\n");
                sb.append("\t\t<remote-party>" + dialog.getRemoteParty().getURI().toString()
                        + "</remote-party>\n");
                sb.append("\t\t<dialog-state>" + dialog.getState() + "</dialog-state>\n");
                sb.append("\t</dialog>\n");
            }
            sb.append(DIALOGS_FOOTER);
            return sb.toString();
        }
    }

    public void addDialog(Dialog dialog, Request request) {
        logger.debug("addDialog " + this.dialogs);
        this.dialogs.add(dialog);
        dialog.setApplicationData(this);
        this.dialogTable.put(dialog, request);
        if (firstDialog == null) {
            this.firstDialog = dialog;
        }
    }

    public Dialog getPeer(Dialog dialog) {
        Object[] dialogs = this.dialogs.toArray();
        if (dialogs.length != 2)
            return null;
        assert (dialogs[0] == dialog || dialogs[1] == dialog);
        return dialogs[0] == dialog ? (Dialog) dialogs[1] : (Dialog) dialogs[0];
    }

    public void setLastResponse(Response lastResponse) {
        this.lastResponse = lastResponse;
    }

    public Response getLastResponse() {
        return lastResponse;
    }

    public void tearDownDialogs(String reason) {
        logger.debug("Tearinging Down Dialogs");
        for (Dialog dialog : dialogs) {
            try {
                if (dialog != null && dialog.getState() == DialogState.CONFIRMED) {
                    Request request = dialog.createRequest(Request.BYE);
                    ReasonHeader reasonHeader = SipListenerImpl.getInstance().getHelper()
                            .createReasonHeader(reason);
                    request.addHeader(reasonHeader);
                    SipProvider provider = ((DialogExt) dialog).getSipProvider();
                    ClientTransaction ctx = provider.getNewClientTransaction(request);
                    dialog.sendRequest(ctx);
                } else if (dialog != null && dialog.getState() != DialogState.TERMINATED) {
                    dialog.delete();
                }
            } catch (Exception e) {
                logger.error("Unexpected exception sending BYE", e);
            }
        }
    }

    public void tearDownDialogs() {
        this.tearDownDialogs(null);
    }

    public void removeMe(Dialog dialog) {
        logger.debug("removeMe " + dialog);
        logger.debug("removeMe: dialogs = " + this.dialogs);
        this.dialogs.remove(dialog);
        this.dialogTable.remove(dialog);
        if (dialogs.isEmpty()) {
            if (System.currentTimeMillis() - this.creationTime >= cacheTimeout * 1000) {
                SipUtils.removeDialogContext(key, this);
            } else {
                long delta = cacheTimeout * 1000
                        - (System.currentTimeMillis() - this.creationTime);
                if (delta > 0) {
                    RestServer.timer.schedule(new TimerTask() {
                        @Override
                        public void run() {
                            SipUtils.removeDialogContext(key, DialogContext.this);
                        }

                    }, delta);
                }
            }
        }
    }

    public static DialogContext get(Dialog dialog) {
        return (DialogContext) dialog.getApplicationData();
    }

    /**
     * Timer task that sends a periodic in-dialog Options to test the liveness of the phone on the
     * other side.
     */
    class OptionsTimer extends TimerTask {

        @Override
        public void run() {
            try {
                for (Dialog dialog : dialogs) {
                    if (dialog != null && dialog.getState() == DialogState.CONFIRMED) {
                        Request request = dialog.createRequest(Request.OPTIONS);
                        SipProvider provider = ((DialogExt) dialog).getSipProvider();
                        ClientTransaction ctx = provider.getNewClientTransaction(request);
                        dialog.sendRequest(ctx);
                    }
                }
            } catch (Exception ex) {
                this.cancel();
                for (Dialog dialog : dialogs) {
                    try {
                        if (dialog != null && dialog.getState() == DialogState.CONFIRMED) {
                            Request request = dialog.createRequest(Request.BYE);
                            SipProvider provider = ((DialogExt) dialog).getSipProvider();
                            ClientTransaction ctx = provider.getNewClientTransaction(request);
                            dialog.sendRequest(ctx);
                        }
                    } catch (SipException e) {
                        logger.error("Unexpected exception sending BYE", e);
                    }
                }
            }

        }

    }

    public void setUserCredentials(UserCredentialHash credentials) {
        this.userCredentials = credentials;
    }

    public UserCredentialHash getCredentials() {
        return this.userCredentials;
    }

    public Request getRequest(Dialog dialog) {
        return this.dialogTable.get(dialog);
    }

    /**
     * @param pendingOperation the pendingOperation to set
     */
    public void setPendingOperation(PendingOperation pendingOperation) {
        this.pendingOperation = pendingOperation;
    }

    /**
     * @return the pendingOperation
     */
    public PendingOperation getPendingOperation() {
        return pendingOperation;
    }

    public void processAck(RequestEvent requestEvent) {
        try {
            Dialog dialog = requestEvent.getDialog();
            Dialog peerDialog = this.getPeer(dialog);
            Request request = requestEvent.getRequest();
            if (request.getMethod().equals(Request.ACK)) {
                if (this.pendingOperation == PendingOperation.PENDING_ACK) {
                    this.pendingOperation = PendingOperation.NONE;
                    long cseq = SipHelper.getSequenceNumber(this.lastResponse);
                    Request ack = peerDialog.createAck(cseq);
                    peerDialog.sendAck(ack);
                }
            }
        } catch (Exception ex) {
            this.tearDownDialogs("Execption processing dialog request");
        }

    }

    public void processNotify(RequestEvent requestEvent) {
        try {
            if (this.method.equalsIgnoreCase("refer")) {
                ServerTransaction serverTransaction = requestEvent.getServerTransaction();
                Request request = requestEvent.getRequest();
                Dialog dialog = requestEvent.getDialog();
                SipHelper sipHelper = SipListenerImpl.getInstance().getHelper();
                Response response = sipHelper.createResponse(request, Response.OK);
                serverTransaction.sendResponse(response);

                logger.debug("got a NOTIFY");
                SubscriptionStateHeader subscriptionState = (SubscriptionStateHeader) request
                        .getHeader(SubscriptionStateHeader.NAME);
                if (request.getContentLength().getContentLength() != 0) {
                    String statusLine = new String(request.getRawContent());
                    logger.debug("dialog = " + dialog);
                    logger.debug("status line = " + statusLine);

                    if (!statusLine.equals("")) {
                        this.setStatus(SipHelper.getCallId(request), request.getMethod(),
                                statusLine);
                    }
                }

                if (subscriptionState.getState().equalsIgnoreCase(
                        SubscriptionStateHeader.TERMINATED)) {
                    SipListenerImpl.getInstance().getHelper().tearDownDialog(dialog);
                }
            } else {
                this.forwardRequest(requestEvent);
            }
        } catch (Exception ex) {
            this.tearDownDialogs("Problem processing NOTIFY");
        }
    }

    public void forwardRequest(RequestEvent requestEvent) {
        try {
            Dialog dialog = requestEvent.getDialog();
            DialogContext dat = (DialogContext) dialog.getApplicationData();

            if (dat == null) {
                logger.debug("No DialogContext found -- not forwarding");
                return;
            }
            Dialog peerDialog = this.getPeer(dialog);
            if (peerDialog == null) {
                logger.debug("No peer Dialog found -- not forwardng");
                return;
            }
            Request request = requestEvent.getRequest();
            logger.debug("dialogContext : forward Request " + request.getMethod());
            Request newRequest = peerDialog.createRequest(request.getMethod());
            byte[] content = request.getRawContent();
            if (content != null) {
                ContentTypeHeader cth = (ContentTypeHeader) request
                        .getHeader(ContentTypeHeader.NAME);
                newRequest.setContent(content, cth);
            }
            for (Iterator it = request.getHeaderNames(); it.hasNext();) {
                String headerName = (String) it.next();
                if (!headerName.equals(ContentLengthHeader.NAME)) {
                    if (newRequest.getHeader(headerName) == null) {
                        for (Iterator headerIterator = request.getHeaders(headerName); headerIterator
                                .hasNext();) {
                            Header header = (Header) headerIterator.next();
                            newRequest.addHeader((Header) header.clone());
                        }
                    }
                }
            }

            SipProvider provider = (SipProvider) requestEvent.getSource();
            ClientTransaction ctx = provider.getNewClientTransaction(newRequest);
            TransactionContext tad = new TransactionContext(Operator.FORWARD_REQUEST, ctx);
            ctx.setApplicationData(tad);
            ServerTransaction st = requestEvent.getServerTransaction();
            tad.setServerTransaction(st);
            peerDialog.sendRequest(ctx);
        } catch (Exception ex) {
            this.tearDownDialogs("Problem in forwarding request");
        }
    }

    public boolean isCallInProgress() {
        if (method.equalsIgnoreCase(Request.REFER)) {
            throw new UnsupportedOperationException("Cannot query call state");
        } else {
            for (Dialog dialog : this.dialogs) {
                if (dialog.getState() == DialogState.TERMINATED) {
                    return false;
                }
            }
            return true;
        }
    }

    /**
     * Initiate a transfer.
     * 
     * @param target -- the transfer target.
     * @throws SipException -- if the transfer request could not be sent.
     */
    public void transferCaller(String target) throws SipException {
        try {
            Dialog peerDialog = this.getPeer(this.firstDialog);
            SipHelper sipHelper = SipListenerImpl.getInstance().getHelper();
            Request inviteRequest = peerDialog.createRequest(Request.INVITE);
            ContactHeader contact = SipListenerImpl.getInstance().getHelper()
                    .createContactHeader();
            inviteRequest.setHeader(contact);
            ContentTypeHeader cth = sipHelper.createContentTypeHeader();
            inviteRequest.setHeader(cth);
            ClientTransaction clientTx = sipHelper.getNewClientTransaction(inviteRequest);
            TransactionContext tad = new TransactionContext(Operator.SOLICIT_SDP_OFFER, clientTx);
            tad.setTarget(target);
            peerDialog.sendRequest(clientTx);
        } catch (ParseException ex) {
            this.tearDownDialogs("Unexpected exception");
        }

    }

    public void sendSdpAnswerInAck(Dialog dialog, Response response) {
        try {
            this.pendingOperation = PendingOperation.PENDING_RE_INVITE;
            Request request = this.dialogTable.get(dialog);
            SessionDescription sd = SipHelper.getSessionDescription(request);
            long cseq = SipHelper.getSequenceNumber(response);
            Request ack = dialog.createAck(cseq);
            SipHelper sipHelper = SipListenerImpl.getInstance().getHelper();
            sipHelper.setSdpContent(ack, sd.toString());
            dialog.sendAck(ack);
        } catch (Exception ex) {
            logger.error("Unexpected exception", ex);
            this.tearDownDialogs("Unexpected exception");
        }
    }

    public void sendSdpOfferToPeerDialog(Dialog dialog,  Response response) {
        try {
            Dialog peerDialog = this.getPeer(dialog);
            Request request = peerDialog.createRequest(Request.INVITE);
            SessionDescription sd = SipHelper.getSessionDescription(response);
            SipHelper sipHelper = SipListenerImpl.getInstance().getHelper();
            sipHelper.addSdpContent(request, sd.toString());
            ClientTransaction ctx = sipHelper.getNewClientTransaction(request);
            TransactionContext newContext = TransactionContext.attach(ctx,
                    Operator.SEND_SDP_OFFER);
            newContext.setUserCredentials(this.userCredentials);
            peerDialog.sendRequest(ctx);
        } catch (Exception ex) {
            logger.error("Unexpected exception", ex);
            this.tearDownDialogs("Unexpected exception");
        }
    }

    public void sendSdpOffer(String target,  Response response) {

        try {
            SipHelper sipHelper = SipListenerImpl.getInstance().getHelper();
            String userName = userCredentials.getUserName();
            String fromAddrSec = SipHelper.getFromAddrSpec(response);
            String toAddrSpec = target;

            Request request = sipHelper.createRequest(Request.INVITE, userName, null,
                    fromAddrSec, toAddrSpec, false);
            ClientTransaction ctx = sipHelper.getNewClientTransaction(request);        
            TransactionContext context =  TransactionContext.attach(ctx, Operator.SEND_SDP_OFFER);
            context.setUserCredentials(this.userCredentials);
            Dialog peerDialog = this.getPeer(this.firstDialog);      
            String key = agent + ":" + target + ":" + calledParty;
            DialogContext dialogContext = SipUtils.createDialogContext(key, timeout, cacheTimeout, this.userCredentials, method,
                    agent,target,calledParty);           
            dialogContext.addDialog(ctx.getDialog(), request);
            Request peerDialogRequest = this.getRequest(peerDialog);
            dialogContext.addDialog(peerDialog, peerDialogRequest);
            dialogContext.setPendingOperation(PendingOperation.PENDING_RE_INVITE);
            ctx.sendRequest();
        } catch (Exception ex) {
            logger.error("Unexpected exception", ex);
            this.tearDownDialogs("Unexpected exception");
        }
    }

    public void setAgent(String agent) {
       this.agent = agent;
    }
    
    public void setCallingParty(String callingParty) {
        this.callingParty = callingParty;
    }
    
    public void setCalledParty(String calledParty) {
        this.calledParty = calledParty;
    }

}
