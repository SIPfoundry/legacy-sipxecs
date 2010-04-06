/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
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
