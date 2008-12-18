/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

public class SipxMediaService extends SipxService {
    public static final String BEAN_ID = "sipxMediaService";

    private int m_httpPort;

    public int getHttpPort() {
        return m_httpPort;
    }

    public void setHttpPort(int httpPort) {
        m_httpPort = httpPort;
    }

    @Override
    public String getBeanId() {
        return BEAN_ID;
    }
}
