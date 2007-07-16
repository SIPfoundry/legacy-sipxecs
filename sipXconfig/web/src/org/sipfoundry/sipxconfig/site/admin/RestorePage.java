/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.admin.BackupBean;
import org.sipfoundry.sipxconfig.admin.Restore;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class RestorePage extends UserBasePage implements PageBeginRenderListener {
    public abstract void setBackups(BackupBean[] backups);

    public abstract BackupBean[] getBackups();

    @InjectObject(value = "spring:adminContext")
    public abstract AdminContext getAdminContext();

    public abstract List getBackupFolders();

    public abstract void setBackupFolders(List list);

    public abstract String getFolder();

    public abstract void setFolder(String folder);

    public abstract BackupBean getCurrentBackup();

    public abstract void setCurrentBackup(BackupBean folder);

    @InjectObject(value = "spring:restore")
    public abstract Restore getRestore();

    public void pageBeginRender(PageEvent event_) {
        if (getBackups() != null) {
            return;
        }
        AdminContext context = getAdminContext();
        setBackups(context.getBackups());

        List<String> backupFolders = new ArrayList<String>();
        for (BackupBean backupBean : getBackups()) {
            if (!backupFolders.contains(backupBean.getParent())) {
                backupFolders.add(backupBean.getParent());
            }
        }
        Collections.sort(backupFolders);
        setBackupFolders(backupFolders);
    }

    public boolean isSameBackupFolder() {
        return getCurrentBackup().getParent().equalsIgnoreCase(getFolder());
    }

    public void restore() {

        List<BackupBean> selectedBackups = new ArrayList<BackupBean>();
        for (BackupBean backupBean : getBackups()) {
            if (backupBean.isChecked()) {
                selectedBackups.add(backupBean);
            }
        }

        if (!validateSelections(selectedBackups)) {
            TapestryUtils.getValidator(getPage()).record(
                    new ValidatorException(getMessages().getMessage("message.invalidSelection")));
            return;
        }

        if (!getRestore().perform(selectedBackups)) {
            TapestryUtils.getValidator(getPage()).record(
                    new ValidatorException(getMessages().getMessage("message.noScriptFound")));
        }
        TapestryUtils.recordSuccess(this, getMessages().getMessage("message.label.success"));
    }

    private boolean validateSelections(List<BackupBean> list) {
        int listSize = list.size();
        if (listSize == 1
                || (listSize == 2 && (!list.get(0).getName().equalsIgnoreCase(
                        list.get(1).getName())))) {
            return true;
        }
        return false;
    }
}
