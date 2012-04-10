/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.callcontroller;

public class SipxSipException extends RuntimeException {
    public SipxSipException(Throwable cause) {
        super(cause);
    }
    
    public SipxSipException() {
        super();
    }
    
    public SipxSipException(String error) {
        super(error);
    }
    
    public SipxSipException(String error, Throwable cause) {
        super(error,cause);
    }
  
}
