/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.plugin.presence;

public class SipXOpenfirePluginException extends RuntimeException {
    
    public SipXOpenfirePluginException(Exception cause) {
        super(cause);
    }
    
    public SipXOpenfirePluginException(String msg, Exception cause) {
        super(msg,cause);
    }
    
    public SipXOpenfirePluginException(String msg) {
        super(msg);
    }

}
