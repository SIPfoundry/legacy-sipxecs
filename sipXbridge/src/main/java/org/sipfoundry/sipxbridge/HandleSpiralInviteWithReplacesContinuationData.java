/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import javax.sip.Dialog;
import javax.sip.RequestEvent;
import javax.sip.ServerTransaction;

/**
 * Continuation data to continue the call for Handling Spiral Invite with
 * the Replaces Header. This data is used to continue the processing
 * after the SDP response to the INVITE with no SDP has been received.
 */
class HandleSpiralInviteWithReplacesContinuationData {
    RequestEvent requestEvent;
    Dialog replacedDialog;
    ServerTransaction serverTransaction;
    String toDomain;
    boolean isPhone;

    public HandleSpiralInviteWithReplacesContinuationData(
            RequestEvent requestEvent, Dialog replacedDialog,
            ServerTransaction serverTransaction, String toDomain,
            boolean isphone) {
        this.requestEvent = requestEvent;
        this.replacedDialog = replacedDialog;
        this.serverTransaction = serverTransaction;
        this.toDomain = toDomain;
        this.isPhone = isphone;

    }
}
