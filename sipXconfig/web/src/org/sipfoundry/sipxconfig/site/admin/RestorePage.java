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
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.request.IUploadFile;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.admin.BackupBean;
import org.sipfoundry.sipxconfig.admin.BackupBean.Type;
import org.sipfoundry.sipxconfig.admin.BackupPlan;
import org.sipfoundry.sipxconfig.admin.Restore;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class RestorePage extends UserBasePage implements PageBeginRenderListener {
    public static final String NO_SCRIPT_FOUND = "message.noScriptFound";
    public static final String SUCCESS = "message.label.success";

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

    public abstract IUploadFile getUploadVoicemailFile();

    public abstract IUploadFile getUploadConfigurationFile();

    @InjectObject(value = "spring:restore")
    public abstract Restore getRestore();

    @Persist
    @InitialValue(value = "literal:restore")
    public abstract String getTab();

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
                    new ValidatorException(getMessages().getMessage(NO_SCRIPT_FOUND)));
            return;
        }

        TapestryUtils.recordSuccess(this, getMessages().getMessage(SUCCESS));
    }

    public void uploadAndRestoreFiles() {
        try {
            List<BackupBean> beans = new ArrayList<BackupBean>();
            BackupBean config;
            config = upload(getUploadConfigurationFile(), BackupPlan.CONFIGURATION_ARCHIVE,
                    "message.wrongConfigurationFileToRestore");
            if (config != null) {
                beans.add(config);
            }
            BackupBean voicemail = upload(getUploadVoicemailFile(), BackupPlan.VOICEMAIL_ARCHIVE,
                    "message.wrongVoicemailFileToRestore");
            if (voicemail != null) {
                beans.add(config);
            }

            if (beans.isEmpty()) {
                throw new ValidatorException(getMessages().getMessage("message.noFileToRestore"));
            }

            if (!getRestore().perform(beans)) {
                throw new ValidatorException(getMessages().getMessage(NO_SCRIPT_FOUND));
            }

            TapestryUtils.recordSuccess(this, getMessages().getMessage(SUCCESS));
        } catch (ValidatorException e) {
            IValidationDelegate validator = TapestryUtils.getValidator(getPage());
            validator.record(e);
        }

    }

    private BackupBean upload(IUploadFile uploadFile, String name, String msgWrongFile)
        throws ValidatorException {

        if (uploadFile == null) {
            return null;
        }
        if (!uploadFile.getFileName().equalsIgnoreCase(name)) {
            String error = getMessages().getMessage(msgWrongFile);
            throw new ValidatorException(error);
        }

        OutputStream os = null;
        try {
            String prefix = StringUtils.substringBefore(uploadFile.getFileName(), ".");
            File tmpFile = File.createTempFile(prefix, ".tar.gz");
            os = new FileOutputStream(tmpFile);
            IOUtils.copy(uploadFile.getStream(), os);
            return new BackupBean(tmpFile, name);
        } catch (IOException ex) {
            String error = getMessages().getMessage("message.failed.uploadConfiguration");
            throw new ValidatorException(error);
        } finally {
            IOUtils.closeQuietly(os);
        }
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
