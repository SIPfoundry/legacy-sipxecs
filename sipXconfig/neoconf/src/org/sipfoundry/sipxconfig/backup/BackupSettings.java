/**
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.backup;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.Collection;
import java.util.Collections;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.codehaus.jackson.annotate.JsonIgnore;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.rest.RestUtilities;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.beans.factory.annotation.Required;

public class BackupSettings extends PersistableSettings implements DeployConfigOnEdit {
    private static final Log LOG = LogFactory.getLog(BackupSettings.class);
    private String m_localBackupPath;
    private Setting m_dbSettings;

    @Override
    public String getBeanId() {
        return "backupSettings";
    }

    public String getLink(BackupPlan plan, Integer userId, String backupId, String backupEntry) {
        String fmt = "%s/%s/%s";
        switch (plan.getType()) {
        case ftp:
            return String.format(fmt, getFtpUrl(), backupId, backupEntry);
        default:
        case local:
            String file = String.format(fmt, m_localBackupPath, backupId, backupEntry);
            try {
                return RestUtilities.getLink(new File(file), userId, "tar/x-gzip");
            } catch (FileNotFoundException impossible) {
                // file really should exist because list is built from current file listing
                LOG.error(impossible);
                return "file is missing";
            }
        }
    }

    public String getPath(BackupPlan plan) {
        switch (plan.getType()) {
        case ftp:
            return getFtpUrl();
        default:
        case local:
            return getLocalBackupPath();
        }
    }

    @Required
    public void setLocalBackupPath(String localBackupPath) {
        m_localBackupPath = localBackupPath;
    }

    public String getLocalBackupPath() {
        return m_localBackupPath;
    }

    public Setting getDbSettings() {
        if (m_dbSettings != null) {
            return m_dbSettings;
        }
        m_dbSettings = loadDbSettings();
        return m_dbSettings;
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("backup/backup.xml");
    }
    /**
     * This holds database specific settings. the user saved values are kept in database: backup_plan table
     * and this setting will keep only default values
     * We always need to keep setting in sync with database (see BackupApi.java)
     * @return
     */
    protected Setting loadDbSettings() {
        return getModelFilesContext().loadModelFile("backup/backup-db.xml");
    }

    public Setting getIncludeDeviceFiles() {
        return m_dbSettings.getSetting("db/includeDeviceFiles");
    }

    public Setting getDb() {
        return m_dbSettings.getSetting("db");
    }

    @Override
    @JsonIgnore
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) BackupManager.FEATURE);
    }

    @JsonIgnore
    public String getFtpUrl() {
        return (String) getSettingTypedValue("ftp/url");
    }

    @JsonIgnore
    public boolean isKeepDomain() {
        return (Boolean) getSettingTypedValue("restore/keepDomain");
    }

    @JsonIgnore
    public boolean isKeepFqdn() {
        return (Boolean) getSettingTypedValue("restore/keepFqdn");
    }

    @JsonIgnore
    public boolean isDecodePins() {
        return (Boolean) getSettingTypedValue("restore/decodePins");
    }

    @JsonIgnore
    public int getDecodePinLen() {
        return (Integer) getSettingTypedValue("restore/decodePinMaxLen");
    }

    @JsonIgnore
    public String getResetPin() {
        return (String) getSettingTypedValue("restore/resetPin");
    }

    @JsonIgnore
    public String getResetPassword() {
        return (String) getSettingTypedValue("restore/resetPassword");
    }
}
