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

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.admin.BackupBean;
import org.sipfoundry.sipxconfig.admin.BackupBean.Type;
import org.sipfoundry.sipxconfig.admin.Restore;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class RestorePage extends UserBasePage implements PageBeginRenderListener {
    @InjectObject(value = "spring:adminContext")
    public abstract AdminContext getAdminContext();

    @Bean
    public abstract SelectMap getSelections();

    public abstract void setBackups(List<Map<Type, BackupBean>> list);

    public abstract List<Map<Type, BackupBean>> getBackups();

    public abstract Type getCurrentType();

    public abstract void setCurrentType(Type type);

    public abstract Map<Type, BackupBean> getCurrentFolder();

    public abstract void setCurrentFolder(Map<Type, BackupBean> folder);

    @InjectObject(value = "spring:restore")
    public abstract Restore getRestore();

    public void pageBeginRender(PageEvent event_) {
        if (getBackups() != null) {
            return;
        }
        AdminContext context = getAdminContext();
        setBackups(context.getBackups());
    }

    public String getCurrentBackupName() {
        return getMessages().getMessage("backup." + getCurrentType());
    }

    public String getCurrentFolderName() {
        for (Map.Entry<Type, BackupBean> entry : getCurrentFolder().entrySet()) {
            return entry.getValue().getParent();
        }
        return StringUtils.EMPTY;
    }

    public Type[] getBackupTypes() {
        return Type.values();
    }

    public void restore() {
        Collection<File> selectedFiles = getSelections().getAllSelected();
        List<BackupBean> selectedBackups = new ArrayList<BackupBean>();
        for (File file : selectedFiles) {
            selectedBackups.add(new BackupBean(file));
        }

        if (!validateSelections(selectedBackups)) {
            TapestryUtils.getValidator(getPage()).record(
                    new ValidatorException(getMessages().getMessage("message.invalidSelection")));
            return;
        }

        if (!getRestore().perform(selectedBackups)) {
            TapestryUtils.getValidator(getPage()).record(
                    new ValidatorException(getMessages().getMessage("message.noScriptFound")));
            return;
        }

        TapestryUtils.recordSuccess(this, getMessages().getMessage("message.label.success"));
    }

    static boolean validateSelections(List<BackupBean> list) {
        final int size = list.size();
        if (size == 1) {
            // a single selection is OK
            return true;
        }
        if (size == 2 && (list.get(0).getType() != list.get(1).getType())) {
            // 2 selections are OK if they are of different types
            return true;
        }
        return false;
    }
}
