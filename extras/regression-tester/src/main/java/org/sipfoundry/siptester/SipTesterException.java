/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.siptester;

public class SipTesterException extends RuntimeException {
    public SipTesterException(String reason) {
        super(reason);
    }

    public SipTesterException(String reason, Throwable cause) {
        super(reason, cause);
    }

    public SipTesterException(Exception ex) {
        super(ex);
    }
}
