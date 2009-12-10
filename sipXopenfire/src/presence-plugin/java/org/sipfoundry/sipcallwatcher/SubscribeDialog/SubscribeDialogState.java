package org.sipfoundry.sipcallwatcher.SubscribeDialog;

import java.util.Map;

import javax.sip.Dialog;
import javax.sip.RequestEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipProvider;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.SubscriptionStateHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;
import org.sipfoundry.sipcallwatcher.CallWatcherException;
import org.sipfoundry.sipcallwatcher.ResourceStateEvent;
import org.sipfoundry.sipcallwatcher.RlmiMultipartMessage;
import org.sipfoundry.sipcallwatcher.SipResourceState;
import org.sipfoundry.sipcallwatcher.Subscriber;

public class SubscribeDialogState extends AbstractSubscribeDialogState
{
    private static Logger logger = Logger.getLogger(SubscribeDialogState.class);

    @Override
    public void processNotifyRequest( SubscribeDialog dialog, RequestEvent notifyRequestEvent )
    {
        try{
            // send 200 OK for NOTIFY.
            ServerTransaction serverTransaction = notifyRequestEvent.getServerTransaction();
            Request notify = notifyRequestEvent.getRequest();
            dialog.constructAndSend200OkForNotify( notify, serverTransaction );
            
            // check the subscription state
            SubscriptionStateHeader subscriptionState = (SubscriptionStateHeader) notify.getHeader(SubscriptionStateHeader.NAME);
            // is Subscription in pending or active state?
            String state = subscriptionState.getState();
            if( state.equalsIgnoreCase(SubscriptionStateHeader.ACTIVE)  ||
                state.equalsIgnoreCase(SubscriptionStateHeader.PENDING))
            {
                dialog.processNotifyBody( notify );
            }
            else{
                // subscription is in one of the many failed states - give up on dialog
                dialog.changeState(new Moribund());
            }
        }
        catch( SubscribeDialogStateException ex ){
            logger.info( "SubscribeDialogState::processNotifyRequest caught exception: " + ex );
            dialog.changeState(new Moribund());
        }
    }
    
    @Override
    public void notifySubscriptionExpired( SubscribeDialog dialog )
    {
        dialog.changeState(new Moribund());
    }

    @Override
    public void notifyDialogTerminated( SubscribeDialog dialog )
    {
        dialog.changeState(new Moribund());
    }
    
    
}    
