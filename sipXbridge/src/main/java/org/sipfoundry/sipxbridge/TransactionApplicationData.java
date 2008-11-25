/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.ServerTransaction;
import javax.sip.SipProvider;
import javax.sip.Transaction;
import javax.sip.message.Request;

import org.apache.log4j.Logger;

/**
 * Transaction context data. This pairs server/client tx pairs.
 * The information we stow away on a pending transaction completion. This is
 * transitory information that is only relevant for the duration of the
 * transaction. Long term data that has to persist for the duration of a Dialog
 * is kept in DialogApplicationData.
 * 
 * @author M. Ranganathan
 * 
 */
class TransactionApplicationData {
    
    private static Logger logger = Logger.getLogger(TransactionApplicationData.class);

    /*
     * The current operation.
     */
    Operation operation;
    
    /*
     * The continuation operation after the current tx is complete
     */

    Operation continuationOperation;

    /*
     * The incoming session. This is associated with the incoming invite. It is
     * completed when the response is forwarded.
     */
    RtpSession incomingSession;
    
    
    /*
     * The Pending outgoing session ( awaiting completion after the response
     * comes in ). This is associated with the outgoing invite and is completed
     * when the response containing the sdp answer comes in.
     */
    RtpSession outgoingSession;
    
    
    /*
     * The ITSP account information.
     */
    ItspAccountInfo itspAccountInfo;

    /*
     * The incoming server transaction.
     */
    private ServerTransaction serverTransaction;

    /*
     * The provider associated with the server transaction
     */
    SipProvider serverTransactionProvider;

    /*
     * The outgoing client transaction.
     */

    private ClientTransaction clientTransaction;

    /*
     * The provider associated with the ct.
     */
    SipProvider clientTransactionProvider;

    /*
     * The tag to assign to responses.
     */
    String toTag;

  
    /*
     * The Refering DIALOG if any.
     */
    Dialog referingDialog;

    /*
     * The ReferedTo dialog if any.
     */
    Dialog replacedDialog;

    /*
     * The Back to back UA that is shared by both the client and server
     * transaction.
     */
    BackToBackUserAgent backToBackUa;

    
    /*
     * For the next step of the SBC state machine.
     */
    ContinuationData continuationData;

    /*
     * The Refer request.
     */
    Request referRequest;

    /**
     * The server side of the pairing.
     * @param stx
     */
    void setServerTransaction(ServerTransaction stx) {
        
        if ( this.serverTransaction != null) {
            logger.debug("serverTransactionPointer already set");
            this.serverTransaction.setApplicationData(null);
        }
        stx.setApplicationData(this);
        this.serverTransaction = stx;
        
    }
    
    /**
     * The client side of the pairing.
     * 
     * @param ctx
     */
    void setClientTransaction(ClientTransaction ctx) {
       if (this.clientTransaction != null ) {
           logger.debug("Tx pointer already set.");
           this.clientTransaction.setApplicationData(null);
       }
       ctx.setApplicationData(this);
       this.clientTransaction = ctx;
        
    }

    TransactionApplicationData(Transaction transaction , Operation operation) {
        this.operation = operation;
        if ( transaction instanceof ServerTransaction )  {
            this.setServerTransaction((ServerTransaction)transaction);
        } else {
            this.setClientTransaction((ClientTransaction) transaction);
        }
       
    }

    /**
     * @return the serverTransaction
     */
    ServerTransaction getServerTransaction() {
        return serverTransaction;
    }

    /**
     * @return the clientTransaction
     */
    ClientTransaction getClientTransaction() {
        return clientTransaction;
    }

    public static TransactionApplicationData attach(Transaction transaction,
            Operation operation) {
        if ( transaction.getApplicationData() != null ) {
            logger.warn("RESETTING Transaction Pointer");
        }
        TransactionApplicationData retval = new TransactionApplicationData(transaction,operation);
        return retval;
    }

    



   

}
