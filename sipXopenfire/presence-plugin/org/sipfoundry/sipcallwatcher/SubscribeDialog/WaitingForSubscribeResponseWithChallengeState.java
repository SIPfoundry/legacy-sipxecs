/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipcallwatcher.SubscribeDialog;

import javax.sip.ResponseEvent;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

public class WaitingForSubscribeResponseWithChallengeState extends SubscribeDialogState
{
    private static Logger logger = Logger.getLogger(WaitingForSubscribeResponseWithChallengeState.class);
    
    @Override
    public void processFailureResponse( SubscribeDialog dialog, ResponseEvent responseEvent )
    {
        dialog.changeState(new Moribund());
    }

    @Override
    public void processSuccessfulResponse( SubscribeDialog dialog, ResponseEvent responseEvent )
    {
        // subscription is accepted - kick off timer task that will
        // refresh it before it expires and another that will
        // warn us if it ever does.  Also arm the the expiry timer so that the 
        // we get a notification if and when the subscription expires.
        dialog.armRefreshTimer( responseEvent.getResponse().getExpires().getExpires() / 2 );
        dialog.armExpiryTimer( responseEvent.getResponse().getExpires().getExpires() );
        dialog.changeState(new WaitingForRefreshTimerState());
    }
}
