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

import java.io.File;
import java.io.Serializable;

public class BackupBean implements Serializable {
    enum Type {
        CONFIGURATION("-c"), VOICEMAIL("-v");

        private String m_option;

        Type(String option) {
            m_option = option;
        }

        String getOption() {
            return m_option;
        }

        static Type typeFromName(String name) {
            if (BackupPlan.CONFIGURATION_ARCHIVE.equalsIgnoreCase(name)) {
                return CONFIGURATION;
            }
            return VOICEMAIL;
        }
    }

    private Type m_type;
    private File m_backupFile;

    public BackupBean() {
    }

    public BackupBean(File backupFile) {
        m_backupFile = backupFile;
        m_type = Type.typeFromName(m_backupFile.getName());
    }

    public String getPath() {
        return m_backupFile.getAbsolutePath();
    }

    public String getParent() {
        return m_backupFile.getParentFile().getName();
    }

    public File getFile() {
        return m_backupFile;
    }

    public Type getType() {
        return m_type;
    }
}
