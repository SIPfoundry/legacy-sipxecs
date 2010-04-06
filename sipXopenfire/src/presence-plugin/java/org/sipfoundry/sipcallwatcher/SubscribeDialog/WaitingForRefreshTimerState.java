/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipcallwatcher.SubscribeDialog;

import org.apache.log4j.Logger;

public class WaitingForRefreshTimerState extends SubscribeDialogState
{
    private static Logger logger = Logger.getLogger(WaitingForRefreshTimerState.class);

    @Override
    public void refreshSubscription( SubscribeDialog dialog )
    {
        try{
            dialog.sendInDialogSubscribe();
            dialog.changeState(new WaitingForSubscribeResponseState());
        }
        catch( SubscribeDialogStateException ex ){
            logger.error( "WaitingForRefreshTimerState::refreshSubscription caught exception: ", ex );
            dialog.changeState(new Moribund());
        }
    }
}
