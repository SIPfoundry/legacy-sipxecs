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

import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;

public class SipxMediaService extends SipxService {
    public static final String BEAN_ID = "sipxMediaService";

    private static final ProcessName PROCESS_NAME = ProcessName.MEDIA_SERVER;
    private int m_httpPort;

    @Override
    public ProcessName getProcessName() {
        return PROCESS_NAME;
    }

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
