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
            logger.info( "WaitingForRefreshTimerState::refreshSubscription caught exception: " + ex );
            dialog.changeState(new Moribund());
        }
    }
}
