/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrest;

import gov.nist.javax.sip.ClientTransactionExt;
import gov.nist.javax.sip.ServerTransactionExt;
import gov.nist.javax.sip.message.MessageExt;

import java.util.HashSet;
import java.util.TimerTask;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionAlreadyExistsException;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.TransactionUnavailableException;
import javax.sip.address.SipURI;
import javax.sip.header.CallIdHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

public abstract class AbstractSipListener implements SipListener {
    private static final Logger logger = Logger.getLogger(AbstractSipListener.class);
    private HashSet<String> callIds = new HashSet<String>();
    private MetaInf metaInf;
    private SipHelper helper;
    private static String COLON = ":";

    class CallIdCollectorTask extends TimerTask {
        private String callId;

        public CallIdCollectorTask(String callId) {
            this.callId = callId;
        }

        @Override
        public void run() {
            callIds.remove(callId);
        }

    }

    public AbstractSipListener() {
        this.helper = new SipHelper(this);
    }
    
    
    
    public SipHelper getHelper() {
        return this.helper;
    }

    final void setMetaInf(MetaInf metaInf) {
        this.metaInf = metaInf;
    }

    public MetaInf getMetaInf() {
        return metaInf;
    }


    protected SipStackBean getSipStackBean() {
        return RestServer.getSipStack();
    }

    public void addCallId(String callId) {
        this.callIds.add(callId);
    }

    public ServerTransactionExt getNewServerTransaction(Request request)
            throws TransactionAlreadyExistsException, TransactionUnavailableException {
        String transport = ((ViaHeader) request.getHeader(ViaHeader.NAME)).getTransport();
        SipProvider sipProvider = getSipStackBean().getSipProvider(transport);
        if (sipProvider == null) {
            logger.debug("Cound not find provider for transport " + transport);
            throw new TransactionUnavailableException("Cound not find provider for transport "
                    + transport);
        }
        ServerTransactionExt serverTransaction = (ServerTransactionExt) sipProvider
                .getNewServerTransaction(request);
        if (serverTransaction.getDialog() != null) {
            String callId = ((CallIdHeader) request.getHeader(CallIdHeader.NAME)).getCallId();
            String toTag = serverTransaction.getDialog().getLocalTag();
            this.addCallId(callId + COLON + toTag);
        } else {
            String callId = ((CallIdHeader) request.getHeader(CallIdHeader.NAME)).getCallId();
            String toTag = SipHelper.getToTag(request);
            this.addCallId(callId + COLON + toTag);

        }
        return serverTransaction;
    }

    public ClientTransactionExt getNewClientTransaction(Request request)
            throws TransactionUnavailableException {
        String transport = ((ViaHeader) request.getHeader(ViaHeader.NAME)).getTransport();
        SipProvider sipProvider = getSipStackBean().getSipProvider(transport);
        if (sipProvider == null) {
            logger.debug("Cound not find provider for transport " + transport);
            throw new TransactionUnavailableException("Cound not find provider for transport "
                    + transport);
        }
        String id;
        ClientTransactionExt clientTransaction = (ClientTransactionExt) sipProvider
                .getNewClientTransaction(request);
        if (clientTransaction.getDialog() != null) {
            String callId = ((CallIdHeader) request.getHeader(CallIdHeader.NAME)).getCallId();
            String fromTag = clientTransaction.getDialog().getLocalTag();
  
            id = callId + COLON + fromTag;
        } else {
            String callId = ((CallIdHeader) request.getHeader(CallIdHeader.NAME)).getCallId();
            String fromTag = SipHelper.getFromTag(request);
           
            id = callId + COLON + fromTag;
        }
        logger.debug("Adding id " + id);
        this.callIds.add(id);
        return clientTransaction;
    }

    final boolean isHandled(RequestEvent requestEvent) {
        Dialog dialog = requestEvent.getDialog();
        if (dialog != null) {
            String callId = dialog.getCallId().getCallId() + COLON + dialog.getLocalTag();
            return this.callIds.contains(callId);
        } else {
            Request request = requestEvent.getRequest();
            SipURI sipUri = ((SipURI) request.getRequestURI());
            String user = sipUri.getUser();
            if (user == null) {
                return false;
            }
            if (this.metaInf.getSipConvergenceName().equals(user)) {
                ((MessageExt) request).setApplicationData(this);
                return true;
            } else
                return false;
        }
    }

    final boolean isHandled(ResponseEvent responseEvent) {

        ClientTransaction ct = responseEvent.getClientTransaction();
        Response response = responseEvent.getResponse();
        String id = null;
        if (ct != null) {
            if (ct.getDialog() != null) {
                id = ct.getDialog().getCallId().getCallId() + COLON + ct.getDialog().getLocalTag();
                
            } else {
                String callId = SipHelper.getCallId(ct.getRequest());
                String fromTag = SipHelper.getFromTag(ct.getRequest());
                id = callId + COLON + fromTag;
               
            }
        } else {
            String callId = SipHelper.getCallId(response);
            String fromTag = SipHelper.getFromTag(response);
            id = callId + COLON + fromTag;
        }
        logger.debug("checking for id " + id);
        return callIds.contains(id);
    }

    public boolean isHandled(DialogTerminatedEvent dialogTerminatedEvent) {
        Dialog dialog = dialogTerminatedEvent.getDialog();
        String callId = dialog.getCallId().getCallId();
        String id = callId + COLON + dialog.getLocalTag();
        boolean retval = this.callIds.contains(id);

        if (retval) {

            RestServer.timer.schedule(new CallIdCollectorTask(id), 180000);
        }
        return retval;
    }

    public boolean isHandled(TransactionTerminatedEvent transactionTerminatedEvent) {
        if (transactionTerminatedEvent.getServerTransaction() != null) {
            ServerTransaction st = transactionTerminatedEvent.getServerTransaction();

            if (st.getDialog() != null) {
                String id = st.getDialog().getCallId().getCallId() + COLON + st.getDialog().getLocalTag();
                return this.callIds.contains(id);
            } else {
                String callId = SipHelper.getCallId(st.getRequest());
                String toTag = SipHelper.getToTag(st.getRequest());
                String id = callId + COLON + toTag;
                return callIds.contains(id);
            }

        } else {
            ClientTransaction ct = transactionTerminatedEvent.getClientTransaction();

            if (ct.getDialog() != null) {
                String id = ct.getDialog().getDialogId() + COLON + ct.getDialog().getLocalTag();
                return this.callIds.contains(id);
            } else {
                String callId = SipHelper.getCallId(ct.getRequest());
                String fromTag = SipHelper.getFromTag(ct.getRequest());
                String id = callId + COLON + fromTag;
                return callIds.contains(id);
            }

        }
    }

    public boolean isHandled(TimeoutEvent timeoutEvent) {
        if (timeoutEvent.getServerTransaction() != null) {
            ServerTransaction st = timeoutEvent.getServerTransaction();

            if (timeoutEvent.getServerTransaction().getDialog() != null) {
                String id = st.getDialog().getDialogId() + COLON + st.getDialog().getLocalTag();
                return this.callIds.contains(id);
            } else {
                String callId = SipHelper.getCallId(st.getRequest());
                String toTag = SipHelper.getToTag(st.getRequest());
                String id = callId + COLON + toTag;
                return callIds.contains(id);
            }

        } else {
            ClientTransaction ct = timeoutEvent.getClientTransaction();
            Request request = ct.getRequest();
            if (ct.getDialog() != null) {
                String id = ct.getDialog().getDialogId() + COLON + ct.getDialog().getLocalTag();
                return this.callIds.contains(id);
            } else {
                String callId = SipHelper.getCallId(ct.getRequest());
                String toTag = SipHelper.getFromTag(ct.getRequest());
                String id = callId + COLON + toTag;
                return callIds.contains(id);
            }

        }
    }

    @Override
    public final void processIOException(IOExceptionEvent exceptionEvent) {

        logger.error("An IO Exception occured : " + exceptionEvent.getHost() + " "
                + exceptionEvent.getPort());

    }

    public abstract void init();

    @Override
    public abstract void processDialogTerminated(DialogTerminatedEvent dialogTerminatedEvent);

    @Override
    public abstract void processRequest(RequestEvent requestEvent);

    @Override
    public abstract void processResponse(ResponseEvent responseEvent);

    @Override
    public abstract void processTimeout(TimeoutEvent timeoutEvent);

    @Override
    public abstract void processTransactionTerminated(
            TransactionTerminatedEvent transactionTerminatedEvent);


}
