/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipcallwatcher.SubscribeDialog;

import java.util.Timer;
import java.util.TimerTask;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.DialogTerminatedEvent;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.ServerTransaction;
import javax.sip.Transaction;
import javax.sip.header.ContactHeader;
import javax.sip.header.ExpiresHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;
import org.sipfoundry.sipcallwatcher.CallWatcherException;
import org.sipfoundry.sipcallwatcher.Subscriber;

public class SubscribeDialog
{
    // //////////////// //
    // static variables //
    // //////////////// //
    private static Logger logger = Logger.getLogger(SubscribeDialog.class);
    
    // ////////////////// //
    // instance variables //
    // ////////////////// //
    private Dialog establishedDialog;
    private int expectedRlmiVersion = -1;
    private Timer refreshTimer; // tracks the time when a re-subscribe is required to refresh the subscription
    private Timer expiryTimer; // tracks the time when the subscription expires 
    private Subscriber subscriber;
    private AbstractSubscribeDialogState currentState = new WaitingForStartState();
    private ClientTransaction clientTransaction;
    private Dialog currentDialog;
     
    public SubscribeDialog(Subscriber subscriber)
    {
        this.subscriber = subscriber;
    }
    
    // ////////// //
    // FSM Events //
    // ////////// //
    public synchronized void start()
    {
        logger.info("[SubscribeDialog FSM:" + this + " " + this.currentDialog + "] Incoming event 'start'");
        currentState.start(this);
    }

    public synchronized void processNotifyRequest(RequestEvent notifyRequest)
    {
        try
        {
            logger.info("[SubscribeDialog FSM:" + this + " " + this.currentDialog + "] Incoming event 'processNotifyRequest'");
            validateServerTransaction( notifyRequest.getServerTransaction() );
            currentState.processNotifyRequest(this, notifyRequest);
        } 
        catch( UnknownTransactionException ex )
        {
            logger.info(ex);
            subscriber.constructAndSendResponseForRequest(notifyRequest.getRequest(), 481, notifyRequest.getServerTransaction());
        }
    }
    
    public synchronized void processResponse(ResponseEvent responseReceivedEvent) 
    {
        logger.info("[SubscribeDialog FSM:" + this + " " + this.currentDialog + "] Incoming event 'processResponse'");
        try{
            validateClientTransaction(responseReceivedEvent.getClientTransaction());
            Response response = responseReceivedEvent.getResponse();
            ClientTransaction tempClientTransaction = responseReceivedEvent.getClientTransaction();
            logger.info("Response received on client transaction id " + tempClientTransaction + ": " + response.getStatusCode());
            // this is a response for the client transaction we initiated, process it.
            if( response.getStatusCode() >= Response.OK && response.getStatusCode() < Response.MULTIPLE_CHOICES )
            {
                currentState.processSuccessfulResponse( this, responseReceivedEvent );
            }
            else if( response.getStatusCode() >= Response.BAD_REQUEST )
            {
                currentState.processFailureResponse( this, responseReceivedEvent );
            }
            else
            {
                logger.info("Received unexpected Response:  status code=" + response.getStatusCode() );
            }
        }
        catch( UnknownTransactionException ex){
            logger.info(ex);            
        }
    }
        
    private synchronized void notifyRefreshTimerFired(){
        logger.info("[SubscribeDialog FSM:" + this + " " + this.currentDialog + "] Incoming event 'refreshSubscription'");
        currentState.refreshSubscription(this);
    }
 
    private synchronized void notifyExpiryTimerTimerFired(){
        logger.info("[SubscribeDialog FSM:" + this + " " + this.currentDialog + "] Incoming event 'notifySubscriptionExpired'");
        currentState.notifySubscriptionExpired(this);
    }
    
    public synchronized void processDialogTerminated( DialogTerminatedEvent dialogTerminatedEvent ) 
    {
        try
        {
            logger.info("[SubscribeDialog FSM:" + this + " " + this.currentDialog + "] Incoming event 'processDialogTerminated' for dialog: " + dialogTerminatedEvent.getDialog());
            validateDialog(dialogTerminatedEvent.getDialog());
            currentState.notifyDialogTerminated(this);
        }
        catch( UnknownTransactionException ex ){
            logger.info(ex);            
        }
    }
    
    // JAIN-SIP does not send a DialogTerminatedEvent when a dialog-forming request
    // times out.  Instead it sends a TransactionTerminated event.  If such an
    // event is received while the dialog state appears to be 'terminated' 
    // (that is the state that JAIN-SIP sends before the first dialog-forming response)
    // then treat that event as though it was a dialog terminated.
    public synchronized void processTransactionTerminated( TransactionTerminatedEvent transactionTerminatedEvent ) 
    {
        try{
            if( this.currentDialog != null && this.currentDialog.getState() == DialogState.TERMINATED ){
                logger.info("[SubscribeDialog FSM:" + this + " " + this.currentDialog + "] Incoming event 'processTransactionTerminated' for tx: " + transactionTerminatedEvent.getClientTransaction() );
                validateDialog(transactionTerminatedEvent.getClientTransaction());
                currentState.notifyDialogTerminated(this);
            }
        }
        catch( UnknownTransactionException ex ){
            logger.info(ex);            
        }
    }   

    // /////////////////// //
    // Misc Public Methods //
    // /////////////////// //
    public String getCurrentStateName()
    {
        return currentState.toString();
    }
    
    public void changeState( AbstractSubscribeDialogState newState )
    {
        logger.info("[SubscribeDialog FSM:" + this.currentDialog + "] Transitioning from " + this.currentState.toString() + " to " + newState.toString() );
        this.currentState.doExitAction(this);
        currentState = newState;
        this.currentState.doEntryAction(this);
    }

    // /////////////////////////////////////////////////////// //
    // helper methods to be invoked by subscribe dialog states //
    // /////////////////////////////////////////////////////// //
    void sendDialogFormingSubscribe() throws SubscribeDialogStateException
    {
        // this will initiate a new subscription that will
        // come it with its own RLMI version number - reset
        // our RLMI version tracker count to force a resync
        // once the subscription gets re-established
        try{
            expectedRlmiVersion = -1;
            this.clientTransaction = subscriber.sendDialogFormingSubscribe();
            this.currentDialog = clientTransaction.getDialog();
            logger.info("New Subscribe Dialog = " + this.currentDialog);
        }
        catch( Exception e ){
            throw new SubscribeDialogStateException( e );            
        }
    }
    
    void sendInDialogSubscribe() throws SubscribeDialogStateException
    {
        try{
            this.clientTransaction = subscriber.sendInDialogSubscribe( this.currentDialog, 3600 );
        }
        catch( Exception e ){
            throw new SubscribeDialogStateException( e );
        }
    }

    void terminateSubscription()
    {
        this.clientTransaction = subscriber.sendInDialogSubscribe( this.currentDialog, 0 );
    }
    
    void handleChallenge( Response response ) throws SubscribeDialogStateException
    {
        try
        {
            this.clientTransaction = this.subscriber.getSipStackBean().handleChallenge(response, this.clientTransaction );
            logger.debug("Dialog before challenge = " + this.currentDialog + "; Dialog after challenge = " + this.clientTransaction.getDialog() );
            this.currentDialog = this.clientTransaction.getDialog();
        }
        catch( Exception e ){
            throw new SubscribeDialogStateException(e);
        }
    }
    
    void armRefreshTimer(int delayInSecs) {
        if (refreshTimer != null) {
            refreshTimer.cancel();
        }
        refreshTimer = new Timer();
        TimerTask task = new TimerTask() {
            public void run() {
                notifyRefreshTimerFired();
            }
        };
        logger.debug("arming refreshTimer for " + delayInSecs);
        refreshTimer.schedule(task, delayInSecs * 1000);
    }
    
    void cancelRefreshTimer() {
        if (refreshTimer != null) {
            refreshTimer.cancel();
        }
        logger.debug("cancelling refreshTimer");
    }
    
    void armExpiryTimer(int delayInSecs) {
        if (expiryTimer != null) {
            expiryTimer.cancel();
        }
        expiryTimer = new Timer();
        TimerTask task = new TimerTask() {
            public void run() {
                notifyExpiryTimerTimerFired();
            }
        };
        logger.debug("arming expiryTimer for " + delayInSecs);
        expiryTimer.schedule(task, delayInSecs * 1000);
    }
    
    void cancelExpiryTimer() {
        if (expiryTimer != null) {
            expiryTimer.cancel();
        }
        logger.debug("cancelling expiryTimer");
    }
    
    void constructAndSend200OkForNotify( Request notify, ServerTransaction serverTransaction ) throws SubscribeDialogStateException{
        try{
            subscriber.constructAndSendResponseForRequest( notify, 200, serverTransaction );
        }
        catch( Exception e ){
            throw new SubscribeDialogStateException(e);
        }
    }
    
    void processNotifyBody( Request notify ) throws SubscribeDialogStateException
    {
        try{
            this.expectedRlmiVersion = 1 + subscriber.processNotifyBody( notify, this.expectedRlmiVersion );
        }
        catch( CallWatcherException e ){
            throw new SubscribeDialogStateException(e);
        }
    }
    
    DialogState getCurrentDialogState() 
    {
        return this.currentDialog == null ? DialogState.TERMINATED : this.currentDialog.getState();
    }
    
    void deleteCurrentDialog() 
    {
        if( this.currentDialog != null )
        {
            this.currentDialog.delete();
            this.currentDialog = null;
        }
        this.subscriber.notifySubscriptionDialogTerminated(this);
    }

    // /////////////// //
    // private methods //
    // /////////////// //
 
    private void validateClientTransaction( ClientTransaction clientTransaction ) throws UnknownTransactionException
    {
        if( clientTransaction == null || 
            clientTransaction != this.clientTransaction )
        {
            throw new UnknownTransactionException("SubscribeDialog::validateClientTransaction: Response received for foreign client transaction id: " 
                    + clientTransaction + "; was expecting "
                    + this.clientTransaction );
        }
        else
        {
            validateDialog( clientTransaction );
        }
    }

    private void validateServerTransaction( ServerTransaction serverTransaction ) throws UnknownTransactionException
    {
        if( serverTransaction == null ) 
        {
            throw new UnknownTransactionException("SubscribeDialog::validateServerTransaction: Request associated with null server transaction");
        }
        else
        {
            validateDialog( serverTransaction );
        }
    }

    private void validateDialog( Transaction transaction ) throws UnknownTransactionException
    {
        validateDialog( transaction.getDialog() );
    }

    private void validateDialog( Dialog dialog ) throws UnknownTransactionException
    {
        if( dialog != this.currentDialog )
        {
            
            throw new UnknownTransactionException("SubscribeDialog::validateDialog: event received for foreign dialog: " 
                   + dialog + "; was expecting " + this.currentDialog );
        }
    }

}
