/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrelay;

import org.apache.xmlrpc.XmlRpcException;

@SuppressWarnings("serial")
public class SymmitronException extends RuntimeException {
    public SymmitronException(String msg ) {
        super( msg);
    }
    
    public SymmitronException(String msg, Exception ex) {
        super( msg, ex);
    }

    public SymmitronException(Exception ex) {
       super(ex);
    }
    
    
    public SymmitronException(XmlRpcException ex) {
        super("Error contacting xml RPC server",ex);
     }
    

}
