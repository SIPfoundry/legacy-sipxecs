package org.sipfoundry.sipxrest;

public class SipxRestException extends RuntimeException {
    
    public SipxRestException(String message) {
        super(message);
    }
    
    public SipxRestException(String message, Throwable source) {
        super(message,source);
    }
    
    public SipxRestException () {
        super();
    }
    
    public SipxRestException(Exception ex) {
        super(ex);
    }

}
