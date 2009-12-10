package org.sipfoundry.openfire.client;

import org.apache.xmlrpc.XmlRpcException;

public class OpenfireClientException extends XmlRpcException {
    
    public OpenfireClientException(String message) {
        super(message);
    }
    
    public OpenfireClientException(Exception ex) {
        super("OpenfireClientException",ex);
    }

}
