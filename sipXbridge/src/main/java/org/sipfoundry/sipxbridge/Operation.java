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
    SEND_REGISTER_QUERY, SEND_REGISTER, SPIRAL_BLIND_TRANSFER_INVITE_TO_ITSP,
    SPIRAL_CONSULTATION_TRANSFER_INVITE_TO_ITSP, SEND_INVITE_TO_MOH_SERVER;

    @Override
    public String toString() {
        if (this.equals(Operation.REFER_INVITE_TO_SIPX_PROXY)) {
            return "ReferInviteToSipxProxy";
        } else if (this.equals(Operation.PROCESS_BYE)) {
            return "ProcessBye";
        } else if (this.equals(Operation.SEND_DEREGISTER)) {
            return "SendDeregister";
        } else if (this.equals(Operation.SEND_INVITE_TO_ITSP)) {
            return "SendInviteToItsp";
        } else if (this.equals(Operation.SEND_INVITE_TO_SIPX_PROXY)) {
            return "SendInviteToSipxProxy";
        } else if (this.equals(Operation.SEND_REGISTER_QUERY)) {
            return "SendRegisterQuery";
        } else if (this.equals(Operation.SEND_REGISTER)) {
            return "SendRegister";
        } else if (this.equals(Operation.SPIRAL_BLIND_TRANSFER_INVITE_TO_ITSP)) {
            return "SpiralBlindTransferInviteToItsp";
        } else if (this
                .equals(Operation.SPIRAL_CONSULTATION_TRANSFER_INVITE_TO_ITSP)) {
            return "SpiralConsultationTransferInviteToItsp";
        } else if ( this.equals(Operation.SEND_INVITE_TO_MOH_SERVER)) {
            return "SendInviteToMohServer";
        } else {
            return null;
        }
    }

}
