/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrest;

public class SipxRestException extends RuntimeException {
    
    public SipxRestException(String message) {
        super(message);
    }
    
    public SipxRestException(String message, Throwable source) {
        super(message,source);
    }
    
    public SipxRestException () {
        super();
    }
    
    public SipxRestException(Exception ex) {
        super(ex);
    }

}
