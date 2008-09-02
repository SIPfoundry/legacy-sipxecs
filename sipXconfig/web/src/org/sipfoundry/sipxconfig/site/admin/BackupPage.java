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
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.admin.BackupPlan;
import org.sipfoundry.sipxconfig.admin.DailyBackupSchedule;
import org.sipfoundry.sipxconfig.admin.FtpBackupPlan;
import org.sipfoundry.sipxconfig.admin.LocalBackupPlan;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.ExtraOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.NamedValuesSelectionModel;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class BackupPage extends BasePage implements PageBeginRenderListener {
    public static final String PAGE = "admin/BackupPage";

    /**
     * Conceivable, available backup limits. Otherwise arbitrary. NOTE : Spring 1.1 couldn't
     * define Integers in lists see DefaultXmlBeanDefinitionParser.java:parsePropertySubelement()
     */
    public static final List<Integer> BACKUP_LIMIT_MODEL = Arrays.asList(1, 2, 3, 4, 5, 10, 20, 30, 40, 50);

    public abstract AdminContext getAdminContext();

    public abstract List getBackupFiles();

    public abstract void setBackupFiles(List files);

    public abstract BackupPlan getBackupPlan();

    public abstract void setBackupPlan(BackupPlan plan);

    public abstract IPropertySelectionModel getBackupLimitSelectionModel();

    public abstract void setBackupLimitSelectionModel(IPropertySelectionModel model);

    @Persist
    @InitialValue(value = LocalBackupPlan.TYPE)
    public abstract String getBackupPlanType();

    @Persist
    @InitialValue(value = "literal:none")
    public abstract String getConfiguration();

    public abstract void setConfiguration(String configuration);

    public abstract void setBackupPlanType(String type);

    public IPropertySelectionModel getBackupPlanTypeModel() {
        return new NamedValuesSelectionModel(new Object[] {LocalBackupPlan.TYPE, FtpBackupPlan.TYPE},
            new String[] {getMessages().getMessage("backupPlan.type.local"),
                getMessages().getMessage("backupPlan.type.ftp")
            }
        );
    }

    public void pageBeginRender(PageEvent event_) {
        //getBackupRestoreConfigurationPage().setLaunchingPage(PAGE);
        List urls = getBackupFiles();
        if (urls == null) {
            setBackupFiles(Collections.EMPTY_LIST);
        }

        // every plan has at least 1 schedule, thought of having this somewhere in
        // library, but you could argue it's application specific.
        BackupPlan plan = null;
        if (getBackupPlanType() != null && getBackupPlanType().equals(FtpBackupPlan.TYPE)) {
            plan = getAdminContext().getFtpConfiguration().getBackupPlan();
            setConfiguration("configuration");
        } else {
            plan = getAdminContext().getLocalBackupPlan();
            setConfiguration("none");
        }

        if (plan.getSchedules().isEmpty()) {
            DailyBackupSchedule schedule = new DailyBackupSchedule();
            plan.addSchedule(schedule);
        }
        setBackupPlan(plan);

        ExtraOptionModelDecorator backupLimitModel = (ExtraOptionModelDecorator) getBackupLimitSelectionModel();
        if (backupLimitModel == null) {
            ObjectSelectionModel numbersOnly = new ObjectSelectionModel();
            numbersOnly.setCollection(BACKUP_LIMIT_MODEL);
            numbersOnly.setLabelExpression("toString()");
            backupLimitModel = new ExtraOptionModelDecorator();
            backupLimitModel.setModel(numbersOnly);
            backupLimitModel.setExtraLabel(getMessages().getMessage("select.unlimited"));
            backupLimitModel.setExtraOption(null);
            setBackupLimitSelectionModel(backupLimitModel);
        }
    }

    public void backup() {

        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
        BackupPlan plan = getBackupPlan();
        if (plan.isEmpty()) {
            throw new EmptySelectionException();
        }

        AdminContext adminContext = getAdminContext();
        File[] backupFiles = adminContext.performBackup(plan);
        if (null == backupFiles) {
            throw new FailedBackupException();
        }
        setBackupFiles(Arrays.asList(backupFiles));
    }

    public void ok() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
        BackupPlan plan = getBackupPlan();
        if (plan.isEmpty()) {
            throw new EmptySelectionException();
        }
        AdminContext adminContext = getAdminContext();
        adminContext.storeBackupPlan(plan);
    }

    private static class EmptySelectionException extends UserException {
        public EmptySelectionException() {
            super(false, "message.emptySelection");
        }
    }

    private static class FailedBackupException extends UserException {
        public FailedBackupException() {
            super(false, "message.backupFailed");
        }
    }
}
