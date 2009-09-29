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
import java.util.Comparator;
import java.util.Map;

public class BackupBean implements Serializable {
    public enum Type {
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

    public static class CompareFolders implements Comparator<Map<Type, BackupBean>> {
        public int compare(Map<Type, BackupBean> o1, Map<Type, BackupBean> o2) {
            BackupBean bean1 = o1.entrySet().iterator().next().getValue();
            BackupBean bean2 = o2.entrySet().iterator().next().getValue();
            return bean1.getParent().compareTo(bean2.getParent());
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

    public BackupBean(File backupFile, String type) {
        m_backupFile = backupFile;
        m_type = Type.typeFromName(type);
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
