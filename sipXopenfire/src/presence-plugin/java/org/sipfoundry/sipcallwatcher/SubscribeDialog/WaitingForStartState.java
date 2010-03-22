package org.sipfoundry.sipcallwatcher.SubscribeDialog;

import org.apache.log4j.Logger;
import org.sipfoundry.sipcallwatcher.CallWatcherException;

public class WaitingForStartState extends SubscribeDialogState
{
    private static Logger logger = Logger.getLogger(WaitingForStartState.class);
    
    @Override
    public void start( SubscribeDialog dialog )
    {
        try{
            dialog.sendDialogFormingSubscribe();
            dialog.changeState(new WaitingForSubscribeResponseState());
        }
        catch( SubscribeDialogStateException ex )
        {
            logger.info( "WaitingForStartState::start caught exception: ", ex );            
            dialog.changeState(new Moribund());
        }
    }
}
