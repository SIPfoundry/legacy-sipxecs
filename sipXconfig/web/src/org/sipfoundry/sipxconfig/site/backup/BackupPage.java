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
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.backup.BackupManager;
import org.sipfoundry.sipxconfig.backup.BackupPlan;
import org.sipfoundry.sipxconfig.backup.BackupType;
import org.sipfoundry.sipxconfig.components.SipxBasePage;

public abstract class BackupPage extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "backup/BackupPage";

    @Persist
    @InitialValue("literal:localBackup")
    public abstract String getTab();
    @InjectObject("spring:backupManager")
    public abstract BackupManager getBackupManager();
    public abstract BackupPlan getLocalPlan();
    public abstract void setLocalPlan(BackupPlan plan);
    public abstract BackupPlan getFtpPlan();
    public abstract void setFtpPlan(BackupPlan plan);

//    public abstract List<File> getBackupFiles();
//
//    public abstract void setBackupFiles(List<File> files);
//
//    public abstract File getBackupFile();
//
//
//    public abstract IPropertySelectionModel getBackupLimitSelectionModelCached();
//
//    public abstract void setBackupLimitSelectionModelCached(IPropertySelectionModel model);
//
//    @Persist
//    @InitialValue(value = LocalBackupPlan.TYPE)
//    public abstract String getBackupPlanType();
//
//    public abstract DailyBackupSchedule getSchedule();
//
//    public boolean isFtpBackupPlanActive() {
//        return FtpBackupPlan.TYPE.equals(getBackupPlanType());
//    }
//
//    public IPropertySelectionModel getBackupPlanTypeModel() {
//        return new NamedValuesSelectionModel(new Object[] {
//            LocalBackupPlan.TYPE, FtpBackupPlan.TYPE
//        }, new String[] {
//            getMessages().getMessage("backupPlan.type.local"), getMessages().getMessage("backupPlan.type.ftp")
//        });
//    }
//

    public void pageBeginRender(PageEvent event_) {
        if (getLocalPlan() == null) {
            setLocalPlan(getBackupManager().findOrCreateBackupPlan(BackupType.local));
        }
        if (getFtpPlan() == null) {
            setFtpPlan(getBackupManager().findOrCreateBackupPlan(BackupType.ftp));
        }
//        List urls = getBackupFiles();
//        if (urls == null) {
//            setBackupFiles(Collections.<File> emptyList());
//        }
//        if (getBackupPlan() != null) {
//            return;
//        }
//        configure();
    }
//
//    /**
//     * When user changes the backup plan type
//     */
//    public void formSubmit() {
//        configure();
//    }
//
//    private void configure() {
//        BackupPlan plan = getBackupManager().getBackupPlan(getBackupPlanType());
//
//        // every plan has exactly 1 schedule
//        if (plan.getSchedules().isEmpty()) {
//            DailyBackupSchedule schedule = new DailyBackupSchedule();
//            plan.addSchedule(schedule);
//            getBackupManager().storeBackupPlan(plan);
//        }
//
//        Locale locale = getPage().getLocale();
//        plan.setLocale(locale);
//        setBackupPlan(plan);
//    }
//
//
//    public void backup() {
//        if (!TapestryUtils.isValid(this)) {
//            // do nothing on errors
//            return;
//        }
//        BackupPlan plan = getBackupPlan();
//        if (plan.isEmpty()) {
//            throw new EmptySelectionException();
//        }
//
//        BackupManager adminContext = getBackupManager();
//        File[] backupFiles = adminContext.performBackup(plan);
//        if (null == backupFiles) {
//            throw new FailedBackupException();
//        }
//        setBackupFiles(Arrays.asList(backupFiles));
//    }
//
//    public void ok() {
//        if (!TapestryUtils.isValid(this)) {
//            // do nothing on errors
//            return;
//        }
//        BackupPlan plan = getBackupPlan();
//        if (plan.isEmpty()) {
//            throw new EmptySelectionException();
//        }
//        BackupManager adminContext = getBackupManager();
//        adminContext.storeBackupPlan(plan);
//    }
//
//    private static class EmptySelectionException extends UserException {
//        public EmptySelectionException() {
//            super("&message.emptySelection");
//        }
//    }
//
//    private static class FailedBackupException extends UserException {
//        public FailedBackupException() {
//            super("&message.backupFailed");
//        }
//    }
}
