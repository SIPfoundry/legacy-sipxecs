/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.ftp;

import org.sipfoundry.sipxconfig.admin.BackupPlan;
import org.sipfoundry.sipxconfig.common.BeanWithId;

public class FtpConfiguration extends BeanWithId {
    private String m_host;
    private String m_userId;
    private String m_password;
    private BackupPlan m_backupPlan;
    public BackupPlan getBackupPlan() {
        return m_backupPlan;
    }
    public void setBackupPlan(BackupPlan backupPlan) {
        m_backupPlan = backupPlan;
    }
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


}
