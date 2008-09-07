/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import javax.sip.Dialog;
import javax.sip.RequestEvent;
import javax.sip.message.Request;

/**
 * A private class that stores continuation data when we re-invite the referred party to 
 * determine the SDP codec or during 491 processing so the request can be reissued later.
 * 
 * @author M. Ranganathan
 *
 */
final class ReferInviteToSipxProxyContinuationData implements ContinuationData {

    private RequestEvent requestEvent;
    private Operation operation;
   
    public ReferInviteToSipxProxyContinuationData(RequestEvent requestEvent) {
        this.requestEvent = requestEvent;
        this.operation = Operation.REFER_INVITE_TO_SIPX_PROXY;
    }

   
    public RequestEvent getRequestEvent() {
        return requestEvent;
    }
 
    public Operation getOperation() {
         return operation;
    }

    Dialog getDialog() {
        return requestEvent.getDialog();
    }

    Request getRequest() {
        return requestEvent.getRequest();
    }

}
