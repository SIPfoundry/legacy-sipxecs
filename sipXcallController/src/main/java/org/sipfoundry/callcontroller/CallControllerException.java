package org.sipfoundry.callcontroller;

public class CallControllerException extends RuntimeException {
    
    public CallControllerException (Exception ex) {
        super(ex);
    }
    
    public CallControllerException(String msg, Throwable th) {
        super(msg,th);
    }
    
    public CallControllerException(String msg) {
        super(msg);
    }

}
