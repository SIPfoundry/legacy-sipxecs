/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.util.Collection;
import java.util.Random;

import gov.nist.javax.sip.TransactionExt;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.RequestEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipProvider;
import javax.sip.Transaction;
import javax.sip.address.Hop;
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
class TransactionContext {

    private static Logger logger = Logger.getLogger(TransactionContext.class);

    /*
     * The current operation.
     */
    private Operation operation;


    /*
     * The Pending outgoing session ( awaiting completion after the response
     * comes in ). This is associated with the outgoing invite and is completed
     * when the response containing the sdp answer comes in.
     */
    private RtpSession outgoingRtpSession;


    /*
     * The ITSP account information.
     */
    private ItspAccountInfo itspAccountInfo;

    /*
     * The incoming server transaction.
     */
    private ServerTransaction serverTransaction;


    /*
     * The outgoing client transaction.
     */

    private ClientTransaction clientTransaction;


    /*
     * The Refering DIALOG if any.
     */
    private Dialog referingDialog;

    /*
     * The ReferedTo dialog if any.
     */
    private Dialog replacedDialog;

    /*
     * The Back to back UA that is shared by both the client and server
     * transaction.
     */
    private BackToBackUserAgent backToBackUa;

    /*
     * The Dialog that gets an ACK with the answer from an SDP offer solicitation.
     */
    private Dialog dialogPendingSdpAnswer;


    /*
     * State for the next step of the SBC state machine.
     */
    private ContinuationData continuationData;

    /*
     * The Refer request.
     */
    private Request referRequest;


    /*
     * To tag for client/server pair.
     */
    private String toTag;

    /*
     * Counter for tracking # of times we have redispatched a new transaction for
     * a spurious error response ( ITSP Hack ).
     */
	protected int counter;

	/*
	 * Client transaction pending for Music On Hold.
	 */
	private ClientTransaction mohClientTransaction;

    private RequestEvent requestEvent;

    private Collection<Hop> proxyAddresses;

    static TransactionContext attach(Transaction transaction,
            Operation operation) {
        if ( transaction.getApplicationData() != null ) {
            logger.warn("RESETTING Transaction Pointer");
        }
        TransactionContext retval = new TransactionContext(transaction,operation);
        return retval;
    }

    /*
     * Get the TAD associated with a Transaction.
     */
    static TransactionContext get(
            Transaction  transaction ) {

        return (TransactionContext) transaction.getApplicationData();
    }

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

    TransactionContext(Transaction transaction , Operation operation) {
        this.setOperation(operation);
        if ( transaction instanceof ServerTransaction )  {
            this.setServerTransaction((ServerTransaction)transaction);
        } else {
            this.setClientTransaction((ClientTransaction) transaction);
        }
        if ( transaction.getDialog() != null &&
                DialogContext.get(transaction.getDialog()) != null ) {
            this.itspAccountInfo = DialogContext.get(transaction.getDialog()).getItspInfo();
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


	/**
	 * Get the continuation operation.
	 *
	 * @return
	 */
    Operation getContinuationOperation() {
        if ( this.getContinuationData() == null) {
            return Operation.NONE;
        } else {
            return this.getContinuationData().getOperation();
        }
    }

    /**
     * @param dialogPendingSdpAnswer the dialogPendingSdpAnswer to set
     */
    void setDialogPendingSdpAnswer(Dialog dialogPendingSdpAnswer) {
        this.dialogPendingSdpAnswer = dialogPendingSdpAnswer;
    }

    /**
     * @return the dialogPendingSdpAnswer
     */
    Dialog getDialogPendingSdpAnswer() {
        return dialogPendingSdpAnswer;
    }


    /**
     * @return the serverTransactionProvider
     */
    SipProvider getServerTransactionProvider() {
        return this.serverTransaction == null ? null :
            ((TransactionExt) serverTransaction).getSipProvider();

    }

    /**
     * @param referingDialog the referingDialog to set
     */
    void setReferingDialog(Dialog referingDialog) {
        this.referingDialog = referingDialog;
    }

    /**
     * @return the referingDialog
     */
    Dialog getReferingDialog() {
        return referingDialog;
    }

    /**
     * @param replacedDialog the replacedDialog to set
     */
    void setReplacedDialog(Dialog replacedDialog) {
        this.replacedDialog = replacedDialog;
    }

    /**
     * @return the replacedDialog
     */
    Dialog getReplacedDialog() {
        return replacedDialog;
    }

    /**
     * @param operation the operation to set
     */
    void setOperation(Operation operation) {
        this.operation = operation;
    }

    /**
     * @return the operation
     */
    Operation getOperation() {
        return operation;
    }

    /**
     * @param outgoingRtpSession the outgoingRtpSession to set
     */
    void setOutgoingRtpSession(RtpSession outgoingRtpSession) {
        this.outgoingRtpSession = outgoingRtpSession;
    }

    /**
     * @return the outgoingRtpSession
     */
    RtpSession getOutgoingRtpSession() {
        return outgoingRtpSession;
    }

    /**
     * @param backToBackUa the backToBackUa to set
     */
    void setBackToBackUa(BackToBackUserAgent backToBackUa) {
        this.backToBackUa = backToBackUa;
    }

    /**
     * @return the backToBackUa
     */
    BackToBackUserAgent getBackToBackUa() {
        return backToBackUa;
    }

    /**
     * @param continuationData the continuationData to set
     */
    void setContinuationData(ContinuationData continuationData) {
        this.continuationData = continuationData;
    }

    /**
     * @return the continuationData
     */
    ContinuationData getContinuationData() {
        return continuationData;
    }

    /**
     * @param referRequest the referRequest to set
     */
    void setReferRequest(Request referRequest) {
        this.referRequest = referRequest;
    }

    /**
     * @return the referRequest
     */
    Request getReferRequest() {
        return referRequest;
    }

    /**
     * possibly create and return a fresh to tag.
     *
     * @return toTag
     */
    String createToTag() {
        if (toTag == null) {
            toTag = Integer.toString(Math.abs(new Random().nextInt()));

        }
        return toTag;
    }

    /**
     * @param itspAccountInfo the itspAccountInfo to set
     */
    void setItspAccountInfo(ItspAccountInfo itspAccountInfo) {
        if ( itspAccountInfo == null ) {
            logger.warn("Setting NULL ItspAccountInfo");
        }
        this.itspAccountInfo = itspAccountInfo;
    }

    /**
     * @return the itspAccountInfo
     */
    ItspAccountInfo getItspAccountInfo() {
        return itspAccountInfo;
    }

	public void setMohClientTransaction(ClientTransaction mohClientTransaction) {
		this.mohClientTransaction = mohClientTransaction;
	}

	public ClientTransaction getMohClientTransaction() {
		return mohClientTransaction;
	}

    public void setRequestEvent(RequestEvent requestEvent) {
        this.requestEvent = requestEvent;
    }

    /**
     * @return the requestEvent
     */
    RequestEvent getRequestEvent() {
        return requestEvent;
    }

    public void setProxyAddresses(Collection<Hop> addresses) {
       this.proxyAddresses = addresses;
    }

    public Collection<Hop> getProxyAddresses() {
        return this.proxyAddresses;
    }

    public void copyTo(TransactionContext newContext) {


        newContext.operation = operation;
        newContext.outgoingRtpSession = outgoingRtpSession;
        newContext.itspAccountInfo = itspAccountInfo;
        newContext.serverTransaction = serverTransaction;
        newContext.referingDialog = referingDialog;
        newContext.replacedDialog = replacedDialog;
        newContext.backToBackUa = backToBackUa;
        newContext.dialogPendingSdpAnswer = dialogPendingSdpAnswer ;
        newContext.continuationData = continuationData;
        newContext.referRequest = referRequest;
        newContext.counter = counter;
        newContext.mohClientTransaction = mohClientTransaction;
        newContext.requestEvent = requestEvent;
        newContext.proxyAddresses = proxyAddresses;

    }



}
