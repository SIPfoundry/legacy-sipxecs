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
 * @see CallControlManager for how this is used.
 *
 * <ul>
 *  <li> REFER_INVITE_TO_SIPX_PROXY  -- Send an INVITE to sipx proxy when a REFER is
 *                                      received from the pbx. </li>
 *  <li> PROCESS_BYE -- Forward a BYE ( using a client transaction ) </li>
 *   <li> SEND_DEREGISTER -- Send a De-Register</li>
 *   <li> SEND_INVITE_TO_ITSP -- Send an INVITE ( dialog creating ) to the ITSP
 *   <li> SEND_INVITE_TO_SIPX_PROXY -- Send an INVITE ( Dialog Creating ) to sipx proxy
 *   <li> SEND_REGISTER_QUERY -- Send a register with no expires.
 *   <li> SEND_REGISTER -- send  a REGISTER
 *   <li> SPIRAL_BLIND_TRANSFER_INVITE_TO_ITSP -- Blind Hairpinned transfer to ITSP
 *   <li> HANDLE_SPIRAL_INVITE_WITH_REPLACES -- Blind consultative transfer to ITSP
 *   <li> SEND_INVITE_TO_MOH_SERVER -- send an INVITE to the MOH server
 *   <li> SOLICIT_SDP_OFFER_FROM_PEER_DIALOG - send an INVITE ( no SDP ) to peer dialog.
 *   <li> SESSION_TIMER -- send a Session Timer INVITE
 *   <li> FORWARD_REINVITE -- Forward re-INVITE from peer dialog.
 *   <li> SEND_BYE_TO_MOH_SERVER -- Send BYE  to park Server
 *   <li> HANDLE_INVITE_WITH_REPLACES -- Handle INVITE with Replaces Header.
 *   <li> PROCESS_INVITE -- Controls processing for 491 timer.
 *   <li> SEND_SDP_RE_OFFER -- Send Re-INVITE ( with SDP )
 *   <li> FORWARD_REFER -- forward REFER request.
 *   <li> SEND_BYE_TO_REPLACED_DIALOG -- send BYE to Replaced dialog during call xfer.
 *   <li> FORWARD_SDP_SOLICITIATION -- Forward SDP solicitation from UA ( phone )
 *   <li> NONE -- No OP
 *   <li> CANCEL_REPLACED_INVITE -- CANCEL INVITE when error is encountered during call
 *           xfer. </li>
 *   <li> SEND_PRACK -- send a PRACK. </li>
 *   <li>PROXY_REGISTER_REQUEST -- proxy the REGISTER request when sipxbridge </li>
 *  </ul>
 *
 * @author M. Ranganathan
 *
 */
public enum Operation {
    REFER_INVITE_TO_SIPX_PROXY,
    PROCESS_BYE,
    SEND_DEREGISTER,
    SEND_INVITE_TO_ITSP,
    SEND_INVITE_TO_SIPX_PROXY,
    SEND_REGISTER_QUERY,
    SEND_REGISTER,
    SPIRAL_BLIND_TRANSFER_INVITE_TO_ITSP,
    HANDLE_SPIRAL_INVITE_WITH_REPLACES,
    SEND_INVITE_TO_MOH_SERVER,
    SOLICIT_SDP_OFFER_FROM_PEER_DIALOG,
    SESSION_TIMER, FORWARD_REINVITE,
    SEND_BYE_TO_MOH_SERVER,
    HANDLE_INVITE_WITH_REPLACES,
    PROCESS_INVITE,
    SEND_SDP_RE_OFFER,
    FORWARD_REFER,
    SEND_BYE_TO_REPLACED_DIALOG,
    FORWARD_SDP_SOLICITIATION,
    NONE,
    CANCEL_REPLACED_INVITE,
    SEND_PRACK,
    CANCEL_MOH_INVITE,
    CANCEL_INVITE, SEND_BYE_FOR_TEARDOWN,
    PROXY_REGISTER_REQUEST, RE_INVITE_REMOVE_RELAY, 
    RE_INVITE_REMOVE_RELAY_CONTINUATION;

}
