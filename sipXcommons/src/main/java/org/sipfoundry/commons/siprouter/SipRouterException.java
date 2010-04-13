/*
 *  Copyright (C) 2008 Nortel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.siprouter;

public class SipRouterException extends RuntimeException {
    
    public SipRouterException() {
        super();
    }
    
    public SipRouterException(String message, Exception ex) {
        super(message,ex);
    }
    
    public SipRouterException(String message) {
        super(message);
    }

}
