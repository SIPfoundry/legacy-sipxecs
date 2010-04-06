/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.callcontroller;

public class CallControllerException extends RuntimeException {
    
    public CallControllerException (Exception ex) {
        super(ex);
    }
    
    public CallControllerException(String msg, Throwable th) {
        super(msg,th);
    }
    
    public CallControllerException(String msg) {
        super(msg);
    }

}
