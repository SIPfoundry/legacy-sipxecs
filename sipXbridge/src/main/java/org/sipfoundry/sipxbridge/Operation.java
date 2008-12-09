/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

/**
 * The operation that is currently being done for a given ClientTransaction.
 * This is context information that is stored in the transaction data pointer so
 * that when the response comes back we know what to do. This is essentially
 * the operator that drives the SBC state machine.
 * 
 * @author M. Ranganathan
 * 
 */
public enum Operation {
    REFER_INVITE_TO_SIPX_PROXY, PROCESS_BYE, SEND_DEREGISTER,
    SEND_INVITE_TO_ITSP, SEND_INVITE_TO_SIPX_PROXY, 
    SEND_REGISTER_QUERY, SEND_REGISTER, 
    SPIRAL_BLIND_TRANSFER_INVITE_TO_ITSP, 
    HANDLE_SPIRAL_INVITE_WITH_REPLACES,
    SEND_INVITE_TO_MOH_SERVER, SOLICIT_SDP_OFFER_FROM_PEER_DIALOG, 
    SESSION_TIMER, FORWARD_REINVITE, SEND_BYE_TO_MOH_SERVER,
    HANDLE_INVITE_WITH_REPLACES, PROCESS_INVITE, 
    SEND_SDP_RE_OFFER, FORWARD_REFER, SEND_BYE_TO_REPLACED_DIALOG,
    FORWARD_SDP_SOLICITIATION, NONE;

}

