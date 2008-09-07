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

/**
 * The information we stow away on a pending transaction completion. This is
 * transitory information that is only relevant for the duration of the
 * transaction. Long term data that has to persist for the duration of a Dialog
 * is kept in DialogApplicationData.
 * 
 * @author M. Ranganathan
 * 
 */
class TransactionApplicationData {

    /*
     * The current operation.
     */
    Operation operation;
    
    /*
     * The continuation operation after the current tx is complete
     */

    public Operation continuationOperation;

    /*
     * The incoming session. This is associated with the incoming invite. It is
     * completed when the response is forwarded.
     */
    RtpSession incomingSession;
    
    RtpSession incomingRtcpSession;
    
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
    ServerTransaction serverTransaction;

    /*
     * The provider associated with the server transaction
     */
    SipProvider serverTransactionProvider;

    /*
     * The outgoing client transaction.
     */

    ClientTransaction clientTransaction;

    /*
     * The provider associated with the ct.
     */
    SipProvider clientTransactionProvider;

    /*
     * The tag to assign to responses.
     */
    String toTag;

    /*
     * Tag whether this is a re-invite or an INVITE
     */
    boolean isReInvite;

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

    String itspTransport;

   
    
   

    TransactionApplicationData(Operation operation) {
        this.operation = operation;
    }

}
