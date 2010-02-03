/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.callcontroller;

import java.text.ParseException;
import java.util.Iterator;

import gov.nist.javax.sip.ClientTransactionExt;
import gov.nist.javax.sip.ResponseEventExt;
import gov.nist.javax.sip.TransactionExt;
import gov.nist.javax.sip.message.SIPResponse;

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
import javax.sip.TransactionTerminatedEvent;
import javax.sip.header.ContentLengthHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.Header;
import javax.sip.header.SubscriptionStateHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxrest.AbstractSipListener;
import org.sipfoundry.sipxrest.SipHelper;

public class SipListenerImpl extends AbstractSipListener {

    private static final Logger LOG = Logger.getLogger(SipListenerImpl.class);

    private static SipListenerImpl instance;

    public SipListenerImpl() {
        if (instance != null) {
            throw new UnsupportedOperationException("Only singleton allowed");
        }
        instance = this;
    }
    
    public void addCallId(String callId) {
        super.addCallId(callId);
    }

    public static SipListenerImpl getInstance() {
        return instance;
    }

    public void processDialogTerminated(DialogTerminatedEvent dialogTerminatedEvent) {

        Dialog dialog = dialogTerminatedEvent.getDialog();
        DialogContext context = (DialogContext) dialog.getApplicationData();
        if (context != null) {
            context.removeMe(dialog);
        }
    }

    public void processRequest(RequestEvent requestEvent) {
        try {
            ServerTransaction serverTransaction = requestEvent.getServerTransaction();
            SipProvider sipProvider = ((TransactionExt) serverTransaction).getSipProvider();
            Dialog dialog = serverTransaction.getDialog();
            DialogContext dialogContext = DialogContext.get(dialog);
            if (serverTransaction == null) {
                LOG.debug("processRequest : NULL ServerTransaction -- dropping request");
                return;
            }
            LOG.debug("SipListenerImpl: processing incoming request "
                    + serverTransaction.getRequest().getMethod());
            Request request = requestEvent.getRequest();
            
           
         
            if (request.getMethod().equals(Request.BYE)) {               
                dialogContext.forwardRequest(requestEvent);
            } else if (request.getMethod().equals(Request.OPTIONS)) {
                dialogContext.forwardRequest(requestEvent);
            } else if (request.getMethod().equals(Request.INVITE)) {
                dialogContext.forwardRequest(requestEvent);
            } else if (request.getMethod().equals(Request.REFER)) {
                dialogContext.forwardRequest(requestEvent);
            } else if (request.getMethod().equals(Request.NOTIFY)) {
                dialogContext.processNotify(requestEvent);         
            } else if (request.getMethod().equals(Request.ACK)) {
                dialogContext.processAck(requestEvent);
            }
        } catch (Exception ex) {
            LOG.error("Exception  " + requestEvent.getRequest(), ex);
            Dialog dialog = requestEvent.getDialog();
            SipListenerImpl.getInstance().getHelper().tearDownDialog(dialog);
        }
    }

    public void processResponse(ResponseEvent responseEv) {
        ResponseEventExt responseEvent = (ResponseEventExt) responseEv;
        
        try {
            
            ClientTransactionExt clientTransaction = (ClientTransactionExt) responseEvent.getClientTransaction();
            if (clientTransaction == null && responseEvent.getDialog() != null ) {
                if ( responseEvent.isForkedResponse()) {
                    LOG.debug("Forked response seen");
                    clientTransaction = responseEvent.getOriginalTransaction();
                } else {
                    LOG.debug("clientTransaction NULL -- dropping response ");
                    return;
                }
            }
            TransactionContext tad = (TransactionContext) clientTransaction
                    .getApplicationData();
            Dialog dialog = responseEvent.getDialog();
            Response response = responseEvent.getResponse();
            String method = SipHelper.getCSeqMethod(response);
            LOG.debug("processResponse : " + method + " statusCode " + response.getStatusCode());
            DialogContext dialogContext = (DialogContext) dialog.getApplicationData();
            LOG.debug("originalClientTransaction = " + responseEvent.getOriginalTransaction());
            LOG.debug("dialog " + dialog + " originalDialog = " + clientTransaction.getDefaultDialog() + " dialogContext = "  + dialogContext);
            
            if ( dialogContext == null ) {
                // This is a forked response. Get the dialog context from the original dialog.
                dialogContext = DialogContext.get(clientTransaction.getDefaultDialog());
                dialogContext.addDialog(dialog, clientTransaction.getRequest());
            }

            if (tad != null) {

                 if (method.equals(Request.INVITE)) {
                    String callId = SipHelper.getCallId(responseEvent.getResponse());
                    String statusLine = ((SIPResponse) responseEvent.getResponse())
                            .getStatusLine().toString();
                    dialogContext.setStatus(callId, method, statusLine);
                }
                tad.response(responseEvent);
            } else {
                LOG.debug("transaction application data is null!");
            }
        } catch (Exception ex) {
            LOG.error("Error occured processing response", ex);
            Dialog dialog = responseEvent.getDialog();
            if (dialog != null) {
                SipListenerImpl.getInstance().getHelper().tearDownDialog(dialog);
            }
        }
    }

    public void processTimeout(TimeoutEvent timeoutEvent) {
        if (!timeoutEvent.isServerTransaction()) {
            ClientTransaction clientTransaction = timeoutEvent.getClientTransaction();

            TransactionContext tad = (TransactionContext) clientTransaction
                    .getApplicationData();
            if (tad != null) {
                tad.timeout(timeoutEvent);
            } else {
                Dialog dialog = clientTransaction.getDialog();
                if (dialog != null) {
                    dialog.delete();
                }

            }

        }
    }

    public void processTransactionTerminated(TransactionTerminatedEvent transactionTerminatedEvent) {
    }

    

    private void tearDown(RequestEvent requestEvent) {
        Dialog dialog = requestEvent.getDialog();
        DialogContext DialogContext = (DialogContext) dialog.getApplicationData();
        DialogContext.tearDownDialogs();
    }

    @Override
    public void init() {
        // TODO Auto-generated method stub

    }
}
