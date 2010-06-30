package org.sipfoundry.sipxbridge.performance;


import gov.nist.javax.sip.ListeningPointExt;

import java.util.TimerTask;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.ListeningPoint;
import javax.sip.RequestEvent;
import javax.sip.SipProvider;
import javax.sip.message.Request;

public class ByeTimerTask extends TimerTask {
    private RequestEvent requestEvent;
    private ListeningPoint listeningPoint;

    public ByeTimerTask(RequestEvent requestEvent, ListeningPointExt listeningPoint) {
        this.requestEvent = requestEvent;
        this.listeningPoint = listeningPoint;
    }

    @Override
    public void run() {
        try {
            SipProvider provider = (SipProvider) requestEvent.getSource();
            Dialog dialog = requestEvent.getDialog();
            Request bye = dialog.createRequest(Request.BYE);
            ClientTransaction ct = provider.getNewClientTransaction(bye);
            dialog.sendRequest(ct);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

}
