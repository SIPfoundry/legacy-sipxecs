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

import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.admin.BackupPlan;
import org.sipfoundry.sipxconfig.admin.DailyBackupSchedule;
import org.sipfoundry.sipxconfig.components.ExtraOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class BackupPage extends BasePage implements PageBeginRenderListener {

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

    public void pageBeginRender(PageEvent event_) {
        List urls = getBackupFiles();
        if (urls == null) {
            setBackupFiles(Collections.EMPTY_LIST);
        }

        // every plan has at least 1 schedule, thought of having this somewhere in
        // library, but you could argue it's application specific.
        BackupPlan plan = getBackupPlan();
        if (plan == null) {
            plan = getAdminContext().getBackupPlan();
            if (plan.getSchedules().isEmpty()) {
                DailyBackupSchedule schedule = new DailyBackupSchedule();
                plan.addSchedule(schedule);
            }
            setBackupPlan(plan);
        }

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

    public void submit() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
    }

    public void backup() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
        AdminContext adminContext = getAdminContext();
        BackupPlan plan = getBackupPlan();
        if (!plan.isConfigs() && !plan.isVoicemail()) {
            IValidationDelegate delegate = TapestryUtils.getValidator(getPage());
            delegate.record(getMessages().getMessage("message.emptySelection"), ValidationConstraint.REQUIRED);
        } else {
            File[] backupFiles = adminContext.performBackup(plan);
            if (null != backupFiles) {
                setBackupFiles(Arrays.asList(backupFiles));
            } else {
                IValidationDelegate validator = TapestryUtils.getValidator(this);
                validator.record("Backup operation failed.", ValidationConstraint.CONSISTENCY);
            }
        }
    }

    public void ok() {
        AdminContext adminContext = getAdminContext();
        BackupPlan plan = getBackupPlan();
        adminContext.storeBackupPlan(plan);
    }
}
