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
 * An interface for continuation operations to store data away.
 * 
 * @author M. Ranganathan
 */
public interface ContinuationData {
    
    /**
     * Get the RequestEvent for the continuation.
     * 
     * @return the requestEvent that was previously stored in the continuation data.
     */
    public RequestEvent getRequestEvent();
    
    
    /**
     * Get the operator for the continuation.
     * 
     * @return the operation for the continuation.
     */
    public Operation getOperation();
    
    
    /**
     * Dialog to which the operation belongs
     */
    public Dialog getDialog();

}
