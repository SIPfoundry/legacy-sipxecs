/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */

package org.sipfoundry.sipxbridge;

import javax.sip.Dialog;
import javax.sip.RequestEvent;

/**
 * Continuation data for Invite Processing.
 * This is used to avoid collisions (requestPending) from
 * the ITSP.
 * 
 */
public class ProcessInviteContinuationData implements ContinuationData {
    
    private RequestEvent requestEvent;

    public ProcessInviteContinuationData(RequestEvent requestEvent) {
        this.requestEvent = requestEvent;
    }

    /* (non-Javadoc)
     * @see org.sipfoundry.sipxbridge.ContinuationData#getOperation()
     */
  
    public Operation getOperation() {
        return Operation.PROCESS_INVITE;
    }

    /* (non-Javadoc)
     * @see org.sipfoundry.sipxbridge.ContinuationData#getRequestEvent()
     */
   
    public RequestEvent getRequestEvent() {
       
        return requestEvent;
    }
    
    public Dialog getDialog() {
        return requestEvent.getDialog();
    }

}
