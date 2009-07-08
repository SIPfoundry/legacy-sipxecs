package org.sipfoundry.openfire.client;

public class OpenfireClientException extends Exception {
    
    public OpenfireClientException(String message) {
        super(message);
    }
    
    public OpenfireClientException(Exception ex) {
        super(ex);
    }

}
