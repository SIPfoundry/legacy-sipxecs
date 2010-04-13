/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.xmlrpc;

import org.apache.xmlrpc.XmlRpcException;

/**
 * This is Runtime exception that mirrors XmlRpcException Runtime exceptions are better suited for
 * proxied interfaces. If methods implementing such interface throw checked exceptions we need to
 * deal with ugly UndeclaredThrowableException beast.
 */
public class XmlRpcRemoteException extends RuntimeException {
    /**
     * The fault code of the exception.
     */
    private final int m_faultCode;

    public XmlRpcRemoteException(XmlRpcException e) {
        super(e.getMessage(), e.getCause());
        m_faultCode = e.code;
    }

    public XmlRpcRemoteException(Exception e) {
        super(e.getMessage(), e);
        m_faultCode = 0;
    }

    public int getFaultCode() {
        return m_faultCode;
    }
}
