/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;


/**
 * Sipxbridge runtime exception specialization.
 *
 * @author M. Ranganathan.
 *
 */
@SuppressWarnings("serial")
public class SipXbridgeException extends RuntimeException {
    public SipXbridgeException(String reason) {
        super(reason);
    }

    public SipXbridgeException(String reason, Throwable cause) {
        super(reason, cause);
    }

	public SipXbridgeException(Exception ex) {
		super(ex);
	}

}
