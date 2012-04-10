/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.ftp;

import java.io.Serializable;

import org.sipfoundry.sipxconfig.common.BeanWithId;

public class FtpExternalServerConfig extends BeanWithId implements Serializable {
    private String m_host;
    private String m_userId;
    private String m_password;

    public String getHost() {
        return m_host;
    }

    public void setHost(String host) {
        m_host = host;
    }

    public String getPassword() {
        return m_password;
    }

    public void setPassword(String password) {
        m_password = password;
    }

    public String getUserId() {
        return m_userId;
    }

    public void setUserId(String userId) {
        m_userId = userId;
    }

    public FtpExternalServer getFtpContext() {
        FtpExternalServerImpl ftpContext = new FtpExternalServerImpl();
        ftpContext.setHost(m_host);
        ftpContext.setUserId(m_userId);
        ftpContext.setPassword(m_password);
        return ftpContext;
    }
}
