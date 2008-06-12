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
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
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
import org.sipfoundry.sipxconfig.admin.FtpRestore;
import org.sipfoundry.sipxconfig.admin.Restore;
import org.sipfoundry.sipxconfig.admin.ftp.FtpConfiguration;
import org.sipfoundry.sipxconfig.admin.ftp.FtpContext;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.AssetSelector;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class RestorePage extends UserBasePage implements PageBeginRenderListener {
    public static final String PAGE = "admin/RestorePage";

    private static final String FILE_TYPE = ".tar.gz";

    private static final String MESSAGE = "message.";

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

    @InjectObject(value = "spring:ftpRestore")
    public abstract FtpRestore getFtpRestore();

    @InjectPage(value = WaitingPage.PAGE)
    public abstract WaitingPage getWaitingPage();

    @InjectPage(value = BackupRestoreConfigurationPage.PAGE)
    public abstract BackupRestoreConfigurationPage getBackupRestoreConfigurationPage();

    @Persist
    @InitialValue(value = "literal:restore")
    public abstract String getTab();

    @Persist(value = "session")
    public abstract boolean isFtp();

    public abstract void setFtp(boolean ftp);

    public void pageBeginRender(PageEvent event_) {
        if (getBackups() != null) {
            return;
        }
        getBackupRestoreConfigurationPage().setLaunchingPage(PAGE);
        if (getTab() != null && getTab().equals("restore")) {
            backupSetting();
        }
    }

    private void backupSetting() {
        AdminContext context = getAdminContext();
        // get corresonding backups depending on getBackupRestoreConfigurationPage setting
        try {
            if (isFtp()) {
                setBackups(context.getFtpBackups());
            } else {
                setBackups(context.getBackups());
            }
        } catch (UserException ex) {
            setBackups(new ArrayList<Map<Type, BackupBean>>());
            TapestryUtils.getValidator(this).record(
                    new ValidatorException(getMessages().getMessage(ex.getMessage())));

        }
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

    public void refreshBackupSource() {
        //remove previous errors since the restore source is changed (FTP or local)
        TapestryUtils.getValidator(this).clear();
        backupSetting();
    }

    public IPage restore() {
        Collection<File> selectedFiles = getSelections().getAllSelected();
        List<BackupBean> selectedBackups = new ArrayList<BackupBean>();
        for (File file : selectedFiles) {
            selectedBackups.add(new BackupBean(file));
        }

        if (!validateSelections(selectedBackups)) {
            TapestryUtils.getValidator(getPage()).record(
                    new ValidatorException(getMessages().getMessage(
                            "message.invalidSelection")));
            return null;
        }
        return setupWaitingPage(selectedBackups, isFtp());
    }

    public IPage uploadAndRestoreFiles() {
        IValidationDelegate validator = TapestryUtils.getValidator(this);
        try {
            List<BackupBean> selectedBackups = new ArrayList<BackupBean>();
            BackupBean config;
            config = upload(getUploadConfigurationFile(), BackupPlan.CONFIGURATION_ARCHIVE);
            if (config != null) {
                selectedBackups.add(config);
            }
            BackupBean voicemail = upload(getUploadVoicemailFile(), BackupPlan.VOICEMAIL_ARCHIVE);
            if (voicemail != null) {
                selectedBackups.add(voicemail);
            }

            if (selectedBackups.isEmpty()) {
                throw new ValidatorException(getMessages().getMessage("message.noFileToRestore"));
            }
            return setupWaitingPage(selectedBackups, false);
        } catch (ValidatorException e) {
            validator.record(e);
            return null;
        } catch (UserException ex) {
            validator.record(new ValidatorException(getMessages().getMessage(
                    MESSAGE + ex.getMessage())));
            return null;
        }

    }

    private IPage setupWaitingPage(List<BackupBean> selectedBackups, boolean fromFtp) {
        Restore restore = null;
        FtpConfiguration configuration = getAdminContext().getFtpConfiguration();
        if (fromFtp) {
            FtpRestore ftpRestore = getFtpRestore();
            FtpContext ftpContext = ftpRestore.getFtpContext();
            ftpContext.setHost(configuration.getHost());
            ftpContext.setPassword(configuration.getPassword());
            ftpContext.setUserId(configuration.getUserId());
            restore = ftpRestore;
        } else {
            restore = getRestore();
        }

        restore.validate(selectedBackups);
        // set selected backups in order to be used when Waiting page notifies the restore
        // bean
        restore.setSelectedBackups(selectedBackups);

        // sets the waiting listener: it'll be notified by waiting page when this is
        // requested by the client (browser) - after it loads the waiting page
        WaitingPage waitingPage = getWaitingPage();
        waitingPage.setWaitingListener(restore);
        return waitingPage;
    }

    private BackupBean upload(IUploadFile uploadFile, String name) throws ValidatorException {

        if (uploadFile == null) {
            return null;
        }
        String fileName = AssetSelector.getSystemIndependentFileName(uploadFile.getFilePath());
        if (!fileName.endsWith(FILE_TYPE)) {
            String error = getMessages().getMessage("message.wrongFileToRestore");
            throw new ValidatorException(error);
        }

        OutputStream os = null;
        try {
            String prefix = StringUtils.substringBefore(fileName, ".");
            File tmpFile = File.createTempFile(prefix, FILE_TYPE);
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

    public String getLog() {
        try {
            return getRestore().getRestoreLogContent();
        } catch (UserException ex) {
            return getMessages().getMessage(ex.getMessage());
        }
    }

}
