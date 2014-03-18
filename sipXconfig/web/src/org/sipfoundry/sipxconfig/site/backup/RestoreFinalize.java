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
package org.sipfoundry.sipxconfig.site.backup;

import java.util.Collection;
import java.util.Set;
import java.util.TreeSet;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.ValidatorException;
import org.restlet.resource.ResourceException;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.backup.BackupManager;
import org.sipfoundry.sipxconfig.backup.BackupPlan;
import org.sipfoundry.sipxconfig.backup.BackupRunnerImpl;
import org.sipfoundry.sipxconfig.backup.BackupSettings;
import org.sipfoundry.sipxconfig.backup.BackupType;
import org.sipfoundry.sipxconfig.backup.RestoreApi;
import org.sipfoundry.sipxconfig.common.WaitingListener;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.admin.WaitingPage;

public abstract class RestoreFinalize extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "backup/RestoreFinalize";

    private static final Log LOG = LogFactory.getLog(RestoreFinalize.class);

    @Persist
    public abstract void setBackupType(BackupType type);

    public abstract BackupType getBackupType();

    @Persist
    public abstract Collection<String> getSelections();

    public abstract void setSelections(Collection<String> paths);

    @Persist
    public abstract Set<String> getUploadedIds();

    public abstract void setUploadedIds(Set<String> ids);

    @InjectObject("spring:restoreApi")
    public abstract RestoreApi getRestoreApi();

    @InjectPage(value = WaitingPage.PAGE)
    public abstract WaitingPage getWaitingPage();

    @InjectObject(value = "spring:backupManager")
    public abstract BackupManager getBackupManager();

    public abstract BackupSettings getBackupSettings();

    public abstract void setBackupSettings(BackupSettings settings);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Override
    public void pageBeginRender(PageEvent event) {
        if (getBackupSettings() == null) {
            setBackupSettings(getBackupManager().getSettings());
        }
    }

    public Setting getRestoreSettings() {
        return getBackupSettings().getSettings().getSetting("restore");
    }

    public IPage restore() {
        if (!TapestryUtils.isValid(this)) {
            return null;
        }

        //configure plan given selected local/ftp definitions or definitions to upload
        Set<String> selectedDefinitions = CollectionUtils.isEmpty(getSelections())
            ? getUploadedIds() : new TreeSet<String>();
        String[] splittedString = null;
        if (CollectionUtils.isEmpty(selectedDefinitions)) {
            for (String def : getSelections()) {
                splittedString = StringUtils.split(def, '/');
                selectedDefinitions.add(splittedString[splittedString.length - 1]);
            }
        }
        boolean isAdminRestore = isSelected(AdminContext.ARCHIVE);
        //The plan needs to know what archives are to be restored
        //in order to corectly create the configuration .yaml file
        BackupPlan plan = getBackupManager().findOrCreateBackupPlan(getBackupType());
        plan.setDefinitionIds(selectedDefinitions);
        RestoreApi restore = getRestoreApi();
        //When restore files are uploaded, selections are empty, we do no need to stage
        //any archive because the upload process uploads them directly in stage directory
        if (isAdminRestore) {
            WaitingPage waitingPage = getWaitingPage();
            waitingPage.setWaitingListener(new RestoreWaitingListener(restore,
                plan, getBackupSettings(), getSelections()));
            return waitingPage;
        } else {
            try {
                getValidator().recordSuccess(getMessages().getMessage("restore.initiated"));
                restore.restore(plan, getBackupSettings(), getSelections());
                // if we are here, restore is successful
                getValidator().recordSuccess(getMessages().getMessage("restore.success"));

            } catch (ResourceException e) {
                if (e.getCause() instanceof BackupRunnerImpl.TimeoutException) {
                    //BackupRunnerImpl has a background timeout > foreground timeout for restore: it went background
                    getValidator().recordSuccess(getMessages().getMessage("restore.background"));
                } else if (e.getCause() instanceof BackupRunnerImpl.StdErrException) {
                    getValidator().record(new ValidatorException(e.getMessage()));
                }
            }
        }
        return null;
    }

    boolean isSelected(String id) {
        Collection<String> selected = getSelections();
        if (selected != null) {
            String find = '/' + id;
            for (String s : selected) {
                if (s.endsWith(find)) {
                    return true;
                }
            }
        }

        Collection<String> uploaded = getUploadedIds();
        if (uploaded != null) {
            if (uploaded.contains(id)) {
                return true;
            }
        }

        return false;
    }

    private static class RestoreWaitingListener implements WaitingListener {
        private RestoreApi m_restoreApi;
        private BackupPlan m_backupPlan;
        private BackupSettings m_backupSettings;
        private Collection<String> m_selections;

        public RestoreWaitingListener(RestoreApi restoreApi, BackupPlan backupPlan, BackupSettings backupSettings,
            Collection<String> selections) {
            m_restoreApi = restoreApi;
            m_backupPlan = backupPlan;
            m_backupSettings = backupSettings;
            m_selections = selections;
        }

        @Override
        public void afterResponseSent() {
            try {
                LOG.info("Initiate configuration restore...");
                m_restoreApi.restore(m_backupPlan, m_backupSettings, m_selections);
            } catch (ResourceException e) {
                LOG.error("Cannot restore admin backup ", e);
            }
        }
    }
}
