package org.sipfoundry.sipcallwatcher.SubscribeDialog;

import javax.sip.ResponseEvent;
import javax.sip.header.ExpiresHeader;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

public class WaitingForSubscribeResponseState extends SubscribeDialogState
{
    private static Logger logger = Logger.getLogger(WaitingForSubscribeResponseState.class);
    
    @Override
    public void processFailureResponse( SubscribeDialog dialog, ResponseEvent responseEvent )
    {
        try{
            Response response = responseEvent.getResponse();
            if (response.getStatusCode() == Response.UNAUTHORIZED) {
                dialog.handleChallenge( response );
                dialog.changeState(new WaitingForSubscribeResponseWithChallengeState());
            }
            else
            {
                dialog.changeState(new Moribund());
            }
        }
        catch( SubscribeDialogStateException ex )
        {
            logger.info( "WaitingForSubscribeResponseState::processFailureResponse caught exception: " + ex );            
            dialog.changeState(new Moribund());
        }
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
