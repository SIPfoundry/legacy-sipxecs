package org.sipfoundry.sipxbridge;

import javax.sip.Dialog;
import javax.sip.ServerTransaction;
import javax.sip.SipProvider;
import javax.sip.message.Request;

/**
 * Continuation data for re-invite processing.
 */
class ReInviteProcessingContinuationData {
    
    Dialog dialog;
    
    Request request;

    SipProvider provider;

    ServerTransaction serverTransaction;
    
    public ReInviteProcessingContinuationData(Dialog dialog, SipProvider provider, ServerTransaction serverTransaction, Request request) {
        this.dialog = dialog;
        this.request = request;
        this.provider = provider;
        this.serverTransaction = serverTransaction;
    }

}
