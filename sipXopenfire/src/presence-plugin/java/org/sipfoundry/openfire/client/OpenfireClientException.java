/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
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
