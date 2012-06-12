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

import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.backup.BackupManager;
import org.sipfoundry.sipxconfig.backup.BackupSettings;
import org.sipfoundry.sipxconfig.backup.ManualRestore;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.admin.WaitingPage;

public abstract class RestoreFinalize extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "backup/RestoreFinalize";

    @Persist
    public abstract Collection<String> getBackupPaths();

    public abstract void setBackupPaths(Collection<String> paths);

    @InjectObject("spring:manualRestore")
    public abstract ManualRestore getManualRestore();

    @InjectPage(value = WaitingPage.PAGE)
    public abstract WaitingPage getWaitingPage();

    @InjectObject(value = "spring:backupManager")
    public abstract BackupManager getBackupManager();

    public Setting getRestoreSettings() {
        return getBackupSettings().getSettings().getSetting("restore");
    }

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

    public IPage restore() {
        Collection<String> restoreFrom = getBackupPaths();
        boolean isAdminRestore = isSelected(restoreFrom, AdminContext.ARCHIVE);
        ManualRestore restore = getManualRestore();
        if (isAdminRestore) {
            restore.restore(restoreFrom, getBackupSettings(), true);
            WaitingPage waitingPage = getWaitingPage();
            waitingPage.setWaitingListener(restore);
            return waitingPage;
        } else {
            restore.restore(restoreFrom, getBackupSettings());
            getValidator().recordSuccess("Need to implement");
        }

        return null;
    }

    boolean isSelected(Collection<String> selected, String id) {
        String find = '/' + id;
        for (String s : selected) {
            if (s.endsWith(find)) {
                return true;
            }
        }
        return false;
    }
}
