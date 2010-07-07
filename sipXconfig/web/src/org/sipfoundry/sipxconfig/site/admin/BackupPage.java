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
import java.util.Locale;

import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.admin.BackupPlan;
import org.sipfoundry.sipxconfig.admin.DailyBackupSchedule;
import org.sipfoundry.sipxconfig.admin.FtpBackupPlan;
import org.sipfoundry.sipxconfig.admin.LocalBackupPlan;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.ExtraOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.NamedValuesSelectionModel;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class BackupPage extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "admin/BackupPage";

    /**
     * Conceivable, available backup limits. Otherwise arbitrary. NOTE : Spring 1.1 couldn't
     * define Integers in lists see DefaultXmlBeanDefinitionParser.java:parsePropertySubelement()
     */
    public static final List<Integer> BACKUP_LIMIT_MODEL = Arrays.asList(1, 2, 3, 4, 5, 10, 20, 30, 40, 50);

    @InjectObject("spring:adminContext")
    public abstract AdminContext getAdminContext();

    public abstract List<File> getBackupFiles();

    public abstract void setBackupFiles(List<File> files);

    public abstract File getBackupFile();

    public abstract BackupPlan getBackupPlan();

    public abstract void setBackupPlan(BackupPlan plan);

    public abstract IPropertySelectionModel getBackupLimitSelectionModelCached();

    public abstract void setBackupLimitSelectionModelCached(IPropertySelectionModel model);

    @Persist
    @InitialValue(value = LocalBackupPlan.TYPE)
    public abstract String getBackupPlanType();

    public abstract DailyBackupSchedule getSchedule();

    public boolean isFtpBackupPlanActive() {
        return FtpBackupPlan.TYPE.equals(getBackupPlanType());
    }

    public IPropertySelectionModel getBackupPlanTypeModel() {
        return new NamedValuesSelectionModel(new Object[] {
            LocalBackupPlan.TYPE, FtpBackupPlan.TYPE
        }, new String[] {
            getMessages().getMessage("backupPlan.type.local"), getMessages().getMessage("backupPlan.type.ftp")
        });
    }

    public void pageBeginRender(PageEvent event_) {
        List urls = getBackupFiles();
        if (urls == null) {
            setBackupFiles(Collections.<File> emptyList());
        }
        if (getBackupPlan() != null) {
            return;
        }
        configure();
    }

    /**
     * When user changes the backup plan type
     */
    public void formSubmit() {
        configure();
    }

    private void configure() {
        BackupPlan plan = getAdminContext().getBackupPlan(getBackupPlanType());

        // every plan has exactly 1 schedule
        if (plan.getSchedules().isEmpty()) {
            DailyBackupSchedule schedule = new DailyBackupSchedule();
            plan.addSchedule(schedule);
            getAdminContext().storeBackupPlan(plan);
        }

        Locale locale = getPage().getLocale();
        plan.setLocale(locale);
        setBackupPlan(plan);
    }

    public IPropertySelectionModel getBackupLimitSelectionModel() {
        IPropertySelectionModel modelCached = getBackupLimitSelectionModelCached();
        if (modelCached != null) {
            return modelCached;
        }

        ObjectSelectionModel numbersOnly = new ObjectSelectionModel();
        numbersOnly.setCollection(BACKUP_LIMIT_MODEL);
        numbersOnly.setLabelExpression("toString()");

        ExtraOptionModelDecorator backupLimitModel = new ExtraOptionModelDecorator();
        backupLimitModel.setModel(numbersOnly);
        backupLimitModel.setExtraLabel(getMessages().getMessage("select.unlimited"));
        backupLimitModel.setExtraOption(null);
        setBackupLimitSelectionModelCached(backupLimitModel);
        return backupLimitModel;
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
            super("&message.emptySelection");
        }
    }

    private static class FailedBackupException extends UserException {
        public FailedBackupException() {
            super("&message.backupFailed");
        }
    }
}
