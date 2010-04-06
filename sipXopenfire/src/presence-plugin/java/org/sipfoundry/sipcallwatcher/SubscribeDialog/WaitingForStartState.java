/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
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
