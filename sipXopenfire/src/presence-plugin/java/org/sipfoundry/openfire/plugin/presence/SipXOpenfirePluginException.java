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
