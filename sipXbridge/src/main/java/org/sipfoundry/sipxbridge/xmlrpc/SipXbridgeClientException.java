/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */

package org.sipfoundry.sipxbridge.xmlrpc;


@SuppressWarnings("serial")
public class SipXbridgeClientException extends RuntimeException {
    public SipXbridgeClientException(Exception ex) {
        super(ex);
    }
    
    public SipXbridgeClientException(String message, Exception ex) {
        super(message,ex);
    }

    public SipXbridgeClientException(String message) {
        super(message);
    }

}
