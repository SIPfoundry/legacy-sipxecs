/**
 * 
 */
package org.sipfoundry.sipxbridge.performance;

import gov.nist.javax.sip.DialogExt;


class DialogContext {
    private DialogExt dialog;

    public DialogContext(DialogExt dialog) {
        this.dialog = dialog;
        PerformanceTester.timer.schedule(new CompletionTimerTask(dialog), PerformanceTester.CALL_TIME);
    }

}