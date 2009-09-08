package org.sipfoundry.sipxrest;

import java.util.HashSet;
import java.util.Random;
import java.util.TimerTask;

import gov.nist.javax.sip.ClientTransactionExt;
import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.ServerTransactionExt;

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
import javax.sip.header.FromHeader;
import javax.sip.header.ToHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

public abstract class AbstractSipListener implements SipListener {
    private static final Logger logger = Logger.getLogger(AbstractSipListener.class);
    private SipProvider sipProvider;
    private HashSet<String> callIds = new HashSet<String>();
    private MetaInf metaInf;
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
        
    }
    public void setMetaInf(MetaInf metaInf) {
        this.metaInf = metaInf;
    }

    public MetaInf getMetaInf() {
        return metaInf;
    }

    void setSipProvider(SipProvider sipProvider) {
        this.sipProvider = sipProvider;
    }

    protected SipProvider getSipProvider() {
        return this.sipProvider;
    }

    protected ServerTransactionExt getNewServerTransaction(Request request)
            throws TransactionAlreadyExistsException, TransactionUnavailableException {
        ServerTransactionExt serverTransaction = (ServerTransactionExt) this.sipProvider
                .getNewServerTransaction(request);
        if (serverTransaction.getDialog() != null) {
            String callId = ((CallIdHeader) request.getHeader(CallIdHeader.NAME)).getCallId();
            String toTag = serverTransaction.getDialog().getLocalTag();
            this.callIds.add(callId + COLON + toTag);
        } else {
            String callId = ((CallIdHeader) request.getHeader(CallIdHeader.NAME)).getCallId();
            String toTag = Util.getToTag(request);
            this.callIds.add(callId + COLON + toTag);

        }
        return serverTransaction;
    }

    protected ClientTransactionExt getNewClientTransaction(Request request)
            throws TransactionUnavailableException {
        ClientTransactionExt clientTransaction = (ClientTransactionExt) this.sipProvider
                .getNewClientTransaction(request);
        if (clientTransaction.getDialog() != null) {
            String callId = ((CallIdHeader) request.getHeader(CallIdHeader.NAME)).getCallId();
            String fromTag = clientTransaction.getDialog().getLocalTag();
            this.callIds.add(callId + COLON + fromTag);
        } else {
            String callId = ((CallIdHeader) request.getHeader(CallIdHeader.NAME)).getCallId();
            String fromTag = Util.getFromTag(request);
            this.callIds.add(callId + COLON + fromTag);
        }
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
            if ( user == null ) {
                return false;
            }
            if (this.metaInf.getSipUserName().equals(user)) {
                return true;
            } else
                return false;
        }
    }

    final boolean isHandled(ResponseEvent responseEvent) {

        ClientTransaction ct = responseEvent.getClientTransaction();
        Response response = responseEvent.getResponse();
        if (ct != null) {
            if (ct.getDialog() != null) {
                String id = ct.getDialog().getDialogId() + COLON + ct.getDialog().getLocalTag();
                return this.callIds.contains(id);
            } else {
                String callId = Util.getCallId(ct.getRequest());
                String fromTag = Util.getFromTag(ct.getRequest());
                String id = callId + COLON + fromTag;
                return callIds.contains(id);
            }
        } else {
            String callId = Util.getCallId(response);
            String fromTag = Util.getFromTag(response);
            String id = callId + COLON + fromTag;
            return callIds.contains(id);
        }
    }

    public boolean isHandled(DialogTerminatedEvent dialogTerminatedEvent) {
        Dialog dialog = dialogTerminatedEvent.getDialog();
        String callId = dialog.getCallId().getCallId();
        String id = callId + COLON + dialog.getLocalTag();
        boolean retval = this.callIds.contains(id);

        if (retval) {

            RestServer.timer.schedule(new CallIdCollectorTask(id), 8000);
        }
        return retval;
    }

    public boolean isHandled(TransactionTerminatedEvent transactionTerminatedEvent) {
        if (transactionTerminatedEvent.getServerTransaction() != null) {
            ServerTransaction st = transactionTerminatedEvent.getServerTransaction();

            if (st.getDialog() != null) {
                String id = st.getDialog().getDialogId() + COLON + st.getDialog().getLocalTag();
                return this.callIds.contains(id);
            } else {
                String callId = Util.getCallId(st.getRequest());
                String toTag = Util.getToTag(st.getRequest());
                String id = callId + COLON + toTag;
                if (this.callIds.contains(id)) {
                    RestServer.timer.schedule(new CallIdCollectorTask(id), 8000);
                }
                return callIds.contains(id);
            }

        } else {
            ClientTransaction ct = transactionTerminatedEvent.getClientTransaction();

            if (ct.getDialog() != null) {
                String id = ct.getDialog().getDialogId() + COLON + ct.getDialog().getLocalTag();
                return this.callIds.contains(id);
            } else {
                String callId = Util.getCallId(ct.getRequest());
                String fromTag = Util.getFromTag(ct.getRequest());
                String id = callId + COLON + fromTag;
                if (this.callIds.contains(id)) {
                    RestServer.timer.schedule(new CallIdCollectorTask(id), 8000);
                }
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
                String callId = Util.getCallId(st.getRequest());
                String toTag = Util.getToTag(st.getRequest());
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
                String callId = Util.getCallId(ct.getRequest());
                String toTag = Util.getFromTag(ct.getRequest());
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
