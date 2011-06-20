/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipcallwatcher;

public class CallWatcherException extends RuntimeException {

	public CallWatcherException(String message) {
		super(message);
	}
	
	public CallWatcherException(String message, Exception ex) {
		super(message,ex);
	}

	public CallWatcherException(Exception ex) {
		super(ex);
	}
}
