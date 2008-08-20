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
 * that when the response comes back we know what to do.
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
    SEND_INVITE_TO_MOH_SERVER, QUERY_SDP_FROM_PEER_DIALOG, SESSION_TIMER, SEND_MOH_REINIVTE_TO_ITSP, SEND_BYE_TO_MOH_SERVER;

}
