package org.sipfoundry.sipxbridge;

import javax.sip.Dialog;
import javax.sip.RequestEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipProvider;
import javax.sip.message.Request;

/**
 * Continuation data for re-invite processing.
 */
class ReInviteProcessingContinuationData implements ContinuationData {
    
    Dialog dialog;
    
    Request request;

    SipProvider provider;

    ServerTransaction serverTransaction;

    private RequestEvent requestEvent;

    private Operation operation;
    
    public ReInviteProcessingContinuationData(RequestEvent requestEvent, Dialog dialog, SipProvider provider, ServerTransaction serverTransaction, Request request) {
      
        this.requestEvent = requestEvent;
        this.dialog = dialog;
        this.request = request;
        this.provider = provider;
        this.serverTransaction = serverTransaction;
        this.operation = Operation.HANDLE_RE_INVITE;
    }

  
    public RequestEvent getRequestEvent() {
      return this.requestEvent;
    }
    
    public Operation getOperation() {
        return operation;
    }

}
