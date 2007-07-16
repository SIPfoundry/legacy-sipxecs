/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.Serializable;

public class BackupBean implements Serializable {
    private String m_name;
    private String m_parent;
    private String m_path;
    private boolean m_checked;

    public BackupBean() {
    }

    public BackupBean(String name, String parent, String path) {
        m_name = name;
        m_parent = parent;
        m_path = path;
    }

    public String getPath() {
        return m_path;
    }

    public void setPath(String path) {
        m_path = path;
    }

    public String getParent() {
        return m_parent;
    }

    public void setParent(String parent) {
        m_parent = parent;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public boolean isChecked() {
        return m_checked;
    }

    public void setChecked(boolean checked) {
        m_checked = checked;
    }
}
