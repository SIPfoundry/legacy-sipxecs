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
