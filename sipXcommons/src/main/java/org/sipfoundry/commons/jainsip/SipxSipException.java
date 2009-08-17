package org.sipfoundry.commons.jainsip;

public class SipxSipException extends RuntimeException {
    public SipxSipException(Throwable cause) {
        super(cause);
    }
    
    public SipxSipException() {
        super();
    }
    
    public SipxSipException(String error) {
        super(error);
    }
    
    public SipxSipException(String error, Throwable cause) {
        super(error,cause);
    }
}
