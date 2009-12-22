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
