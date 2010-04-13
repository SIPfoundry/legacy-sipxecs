/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxbridge;

import javax.sip.Dialog;
import javax.sip.RequestEvent;
import javax.sip.ServerTransaction;

/**
 * Continuation data for re-invite processing.
 */
class SendInviteToMohServerContinuationData implements ContinuationData {
    
  
    private RequestEvent requestEvent;

    private Operation operation;
    
    public SendInviteToMohServerContinuationData(RequestEvent requestEvent) {
      
        this.requestEvent = requestEvent;     
        this.operation = Operation.SEND_INVITE_TO_MOH_SERVER;
    }

  
    public RequestEvent getRequestEvent() {
      return this.requestEvent;
    }
    
    /*
     * (non-Javadoc)
     * @see org.sipfoundry.sipxbridge.ContinuationData#getOperation()
     */
    public Operation getOperation() {
        return operation;
    }


    /*
     * (non-Javadoc)
     * @see org.sipfoundry.sipxbridge.ContinuationData#getDialog()
     */
    public Dialog getDialog() {
       
        return requestEvent.getDialog();
    }


	ServerTransaction getServerTransaction() {
		return requestEvent.getServerTransaction();
	}


}
