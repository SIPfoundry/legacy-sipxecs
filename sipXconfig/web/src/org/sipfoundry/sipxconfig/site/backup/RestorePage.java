/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.backup;

import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.backup.BackupManager;
import org.sipfoundry.sipxconfig.components.SipxBasePage;

public abstract class RestorePage extends SipxBasePage {
    public static final String PAGE = "bakup/RestorePage";

    @InjectObject(value = "spring:backupManager")
    public abstract BackupManager getBackupManager();

//    @Bean
//    public abstract SelectMap getSelections();
//
//    public abstract void setBackups(List<Map<Type, BackupBean>> list);
//
//    public abstract List<Map<Type, BackupBean>> getBackups();
//
//    public abstract Type getCurrentType();
//
//    public abstract void setCurrentType(Type type);
//
//    public abstract Map<Type, BackupBean> getCurrentFolder();
//
//    public abstract void setCurrentFolder(Map<Type, BackupBean> folder);
//
//    public abstract IUploadFile getUploadVoicemailFile();
//
//    public abstract IUploadFile getUploadConfigurationFile();
//
//    public abstract IUploadFile getUploadCdrFile();
//
//    public abstract IUploadFile getUploadDeviceConfigFile();
//
//    @InjectObject(value = "spring:restore")
//    public abstract Restore getRestore();
//
//    @InjectObject(value = "spring:locationsManager")
//    public abstract LocationsManager getLocationsManager();
//
//    @InjectObject(value = "spring:ftpRestore")
//    public abstract FtpRestore getFtpRestore();
//
//    @InjectPage(value = WaitingPage.PAGE)
//    public abstract WaitingPage getWaitingPage();
//
    @Persist
    @InitialValue(value = "literal:restoreLocal")
    public abstract String getTab();
//
//    @Persist
//    @InitialValue(value = LocalBackupPlan.TYPE)
//    public abstract String getBackupPlanType();
//
//    public IPropertySelectionModel getBackupPlanTypeModel() {
//        return new NamedValuesSelectionModel(new Object[] {
//            LocalBackupPlan.TYPE, FtpBackupPlan.TYPE
//        }, new String[] {
//            getMessages().getMessage("backupPlan.type.local"), getMessages().getMessage("backupPlan.type.ftp")
//        });
//    }
//
//    @Override
    public void pageBeginRender(PageEvent event_) {
//        if (getTab() != null && getTab().equals("restore")) {
//            if (getBackups() == null) {
//                backupSetting();
//            }
//        }
    }
//
//    public void formSubmit() {
//        backupSetting();
//    }
//
//    @Override
//    public void reset() {
//        setBackups(null);
//    }
//
//    private void backupSetting() {
//        BackupManager context = getBackupManager();
//        // get corresponding backups depending on getBackupRestoreConfigurationPage setting
//        try {
//            BackupPlan backupPlan = context.getBackupPlan(getBackupPlanType());
//            setBackups(backupPlan.getBackups());
//        } catch (UserException ex) {
//            setBackups(new ArrayList<Map<Type, BackupBean>>());
//            SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils
//                    .getValidator(this);
//            validator.record(ex, getMessages());
//        }
//    }
//
//    public String getCurrentBackupName() {
//        return getMessages().getMessage("backup." + getCurrentType());
//    }
//
//    public String getCurrentFolderName() {
//        for (Map.Entry<Type, BackupBean> entry : getCurrentFolder().entrySet()) {
//            return entry.getValue().getParent();
//        }
//        return StringUtils.EMPTY;
//    }
//
//    public Type[] getBackupTypes() {
//        return Type.values();
//    }
//
//    public IPage restore() {
//        Collection<File> selectedFiles = getSelections().getAllSelected();
//        List<BackupBean> selectedBackups = new ArrayList<BackupBean>();
//        boolean restoreVoicemail = false;
//        boolean restoreConfig = false;
//        boolean restoreCdr = false;
//        boolean restoreDeviceConfig = false;
//        for (File file : selectedFiles) {
//            BackupBean backup = new BackupBean(file);
//            selectedBackups.add(backup);
//            if (backup.getType() == BackupBean.Type.CONFIGURATION) {
//                restoreConfig = true;
//            } else if (backup.getType() == BackupBean.Type.DEVICE_CONFIG) {
//                restoreDeviceConfig = true;
//            } else if (backup.getType() == BackupBean.Type.CDR) {
//                restoreCdr = true;
//            } else if (backup.getType() == BackupBean.Type.VOICEMAIL) {
//                restoreVoicemail = true;
//            }
//        }
//
//        if (!validateSelections(selectedBackups)) {
//            TapestryUtils.getValidator(getPage()).record(
//                    new ValidatorException(getMessages().getMessage("message.invalidSelection")));
//            return null;
//        }
//        Restore restore = prepareRestore(selectedBackups, getBackupPlanType());
//        return setupWaitingPage(restore, restoreConfig, restoreVoicemail, restoreCdr, restoreDeviceConfig);
//    }
//
//    public IPage uploadAndRestoreFiles() {
//        IValidationDelegate validator = TapestryUtils.getValidator(this);
//        try {
//            List<BackupBean> selectedBackups = new ArrayList<BackupBean>();
//            BackupBean config;
//            boolean restoreVoicemail = false;
//            boolean restoreConfig = false;
//            boolean restoreCdr = false;
//            boolean restoreDeviceConfig = false;
//            config = upload(getUploadConfigurationFile(), BackupPlan.CONFIGURATION_ARCHIVE);
//            if (config != null) {
//                selectedBackups.add(config);
//                restoreConfig = true;
//            }
//            BackupBean deviceConfig = upload(getUploadDeviceConfigFile(), BackupPlan.DEVICE_CONFIG);
//            if (deviceConfig != null) {
//                selectedBackups.add(deviceConfig);
//                restoreDeviceConfig = true;
//            }
//            BackupBean cdr = upload(getUploadCdrFile(), BackupPlan.CDR_ARCHIVE);
//            if (cdr != null) {
//                selectedBackups.add(cdr);
//                restoreCdr = true;
//            }
//            BackupBean voicemail = upload(getUploadVoicemailFile(), BackupPlan.VOICEMAIL_ARCHIVE);
//            if (voicemail != null) {
//                selectedBackups.add(voicemail);
//                restoreVoicemail = true;
//            }
//
//            if (selectedBackups.isEmpty()) {
//                throw new ValidatorException(getMessages().getMessage("message.noFileToRestore"));
//            }
//            Restore restore = prepareRestore(selectedBackups, LocalBackupPlan.TYPE);
//            return setupWaitingPage(restore, restoreConfig, restoreVoicemail, restoreCdr, restoreDeviceConfig);
//        } catch (ValidatorException e) {
//            validator.record(e);
//            return null;
//        }
//
//    }
//
//    private IPage setupWaitingPage(Restore restore, boolean restoreConfig, boolean restoreVoicemail,
//        boolean restoreCdr, boolean restoreDeviceConfig) {
//        if (restoreConfig) {
//            // sets the waiting listener: it'll be notified by waiting page when this is
//            // requested by the client (browser) - after it loads the waiting page
//            WaitingPage waitingPage = getWaitingPage();
//            waitingPage.setWaitingListener(restore);
//            return waitingPage;
//        } else if (restoreVoicemail || restoreCdr || restoreDeviceConfig) {
//            restore.afterResponseSent();
//            if (restoreVoicemail && !restoreCdr) {
//                TapestryUtils.recordSuccess(this, getMessages().getMessage("message.remoteRestore.started"));
//            } else if (restoreCdr && !restoreVoicemail) {
//                TapestryUtils.recordSuccess(this, getMessages().getMessage("message.cdr.started"));
//            } else if (restoreCdr && restoreVoicemail) {
//                TapestryUtils.recordSuccess(this, getMessages().getMessage("message.cdr.ivr.started"));
//            } else if (restoreDeviceConfig) {
//                TapestryUtils.recordSuccess(this, getMessages().getMessage("message.deviceconfig.restored"));
//            }
//            return null;
//        } else {
//            return null;
//        }
//    }
//
//    private BackupBean upload(IUploadFile uploadFile, String name) throws ValidatorException {
//
//        if (uploadFile == null) {
//            return null;
//        }
//        String fileName = AssetSelector.getSystemIndependentFileName(uploadFile.getFilePath());
//        if (!fileName.endsWith(FILE_TYPE)) {
//            String error = getMessages().getMessage("message.wrongFileToRestore");
//            throw new ValidatorException(error);
//        }
//
//        OutputStream os = null;
//        try {
//            String prefix = StringUtils.substringBefore(fileName, ".");
//            File tmpFile = File.createTempFile(prefix, FILE_TYPE);
//            os = new FileOutputStream(tmpFile);
//            IOUtils.copy(uploadFile.getStream(), os);
//            return new BackupBean(tmpFile, name);
//        } catch (IOException ex) {
//            String error = getMessages().getMessage("message.failed.uploadConfiguration");
//            throw new ValidatorException(error);
//        } finally {
//            IOUtils.closeQuietly(os);
//        }
//    }
//
//    static boolean validateSelections(List<BackupBean> list) {
//        final int size = list.size();
//        if (size == 1) {
//            // a single selection is OK
//            return true;
//        }
//        if (size == 2 && (list.get(0).getType() != list.get(1).getType())) {
//            // 2 selections are OK if they are of different types
//            return true;
//        }
//        if (size == 3 && (list.get(0).getType() != list.get(1).getType())
//                && (list.get(0).getType() != list.get(2).getType())
//                && (list.get(1).getType() != list.get(2).getType())) {
//            // 3 selections are OK if they are of different types
//            return true;
//        }
//        if (size == 4 && (list.get(0).getType() != list.get(1).getType())
//                && (list.get(0).getType() != list.get(2).getType())
//                && (list.get(1).getType() != list.get(2).getType())
//                && (list.get(0).getType() != list.get(3).getType())
//                && (list.get(1).getType() != list.get(3).getType())
//                && (list.get(2).getType() != list.get(3).getType())) {
//            // 3 selections are OK if they are of different types
//            return true;
//        }
//        return false;
//    }
//
//    public String getLog() {
//        try {
//            return getRestore().getRestoreLogContent();
//        } catch (UserException ex) {
//            return LocalizationUtils.localizeException(getMessages(), ex);
//        }
//    }
//
//    private Restore prepareRestore(List<BackupBean> selectedBackups, String backupPlanType) {
//        Restore restore = null;
//        if (backupPlanType.equals(FtpBackupPlan.TYPE)) {
//            FtpRestore ftpRestore = getFtpRestore();
//            FtpBackupPlan plan = (FtpBackupPlan) getBackupManager().getBackupPlan(backupPlanType);
//            ftpRestore.setFtpConfiguration(plan.getFtpConfiguration());
//            restore = ftpRestore;
//        } else {
//            restore = getRestore();
//        }
//
//        restore.validate(selectedBackups);
//        // set selected backups in order to be used when Waiting page notifies the restore bean
//        restore.setSelectedBackups(selectedBackups);
//        return restore;
//    }
}
