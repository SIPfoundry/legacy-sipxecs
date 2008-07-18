/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.service;

import org.springframework.beans.factory.annotation.Required;

public class SipxIvrService extends SipxService {
    private String m_vxmlDir;
    private String m_mailstoreDir;
    private String m_scriptsDir;

    @Required
    public void setMailstoreDir(String mailstoreDirectory) {
        m_mailstoreDir = mailstoreDirectory;
    }

    public String getMailstoreDir() {
        return m_mailstoreDir;
    }

    @Required
    public void setVxmlDir(String vxmlDirectory) {
        m_vxmlDir = vxmlDirectory;
    }

    public String getVxmlDir() {
        return m_vxmlDir;
    }

    @Required
    public void setScriptsDir(String scriptsDirectory) {
        m_scriptsDir = scriptsDirectory;
    }

    public String getScriptsDir() {
        return m_scriptsDir;
    }
}
