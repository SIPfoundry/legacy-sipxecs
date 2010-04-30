/**
 * 
 */
package org.sipfoundry.sipxbridge.performance;

import gov.nist.javax.sip.DialogExt;

import java.util.TimerTask;

import javax.sip.ClientTransaction;
import javax.sip.SipProvider;
import javax.sip.message.Request;

class CompletionTimerTask extends TimerTask {

    private DialogExt dialog;

    public CompletionTimerTask(DialogExt dialog) {
        this.dialog = dialog;
    }

    @Override
    public void run() {
        try {
            Request byeRequest = dialog.createRequest(Request.BYE);
            SipProvider provider = dialog.getSipProvider();
            ClientTransaction byeCtx = provider
                    .getNewClientTransaction(byeRequest);
            dialog.sendRequest(byeCtx);
        } catch (Exception ex) {
            ex.printStackTrace();
            System.exit(0);
        }
    }

}