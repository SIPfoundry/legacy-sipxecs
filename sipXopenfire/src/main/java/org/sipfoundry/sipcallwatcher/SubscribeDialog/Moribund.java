package org.sipfoundry.sipcallwatcher.SubscribeDialog;
import javax.sip.DialogState;


public class Moribund extends SubscribeDialogState
{

    @Override
    public void doEntryAction( SubscribeDialog dialog )
    {
        // clean up any left-over timer
        dialog.cancelRefreshTimer();
        dialog.cancelExpiryTimer();

        // check if it is the established dialog that is being terminated
        if( dialog.getCurrentDialogState() == DialogState.CONFIRMED )
        {
            // terminate the subscription
            dialog.terminateSubscription();
        }
        dialog.deleteCurrentDialog();
    }
}
