package org.sipfoundry.sipcallwatcher.SubscribeDialog;

import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;

import org.apache.log4j.Logger;

public abstract class AbstractSubscribeDialogState
{
    private static Logger logger = Logger.getLogger(AbstractSubscribeDialogState.class);

    // entry and exit actions
    public void doEntryAction( SubscribeDialog dialog ){};
    public void doExitAction( SubscribeDialog dialog ){};
    
    // input events of state machine
    public void start( SubscribeDialog dialog )
    {
        logger.info( "Received unexpected event 'start' while in state " + dialog.getCurrentStateName());
    };
    public void processNotifyRequest( SubscribeDialog dialog, RequestEvent notifyRequestEvent )
    {
        logger.info( "Received unexpected event 'processNotifyRequest' while in state " + dialog.getCurrentStateName());
    };
    public void processSuccessfulResponse( SubscribeDialog dialog, ResponseEvent responseEvent )
    {
        logger.info( "Received unexpected event 'processSuccessfulResponse' while in state " + dialog.getCurrentStateName());
    };
    public void processFailureResponse( SubscribeDialog dialog, ResponseEvent responseEvent )
    {
        logger.info( "Received unexpected event 'processFailureResponse' while in state " + dialog.getCurrentStateName());
    };
    public void refreshSubscription( SubscribeDialog dialog )
    {
        logger.info( "Received unexpected event 'refreshSubscription' while in state " + dialog.getCurrentStateName());
    };
    public void notifySubscriptionExpired( SubscribeDialog dialog )
    {
        logger.info( "Received unexpected event 'notifySubscriptionExpired' while in state " + dialog.getCurrentStateName());
    };    
    public void notifyDialogTerminated( SubscribeDialog dialog )
    {
        logger.info( "Received unexpected event 'notifyDialogTerminated' while in state " + dialog.getCurrentStateName());
    };    
}
