/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;


/**
 * System was unable to restart phone  
 */
public class RestartException extends RuntimeException {
    
    public RestartException(String msg) {
        super(msg);
    }
    
    public RestartException(String msg, Throwable cause) {
        super(msg, cause);
    }
}
