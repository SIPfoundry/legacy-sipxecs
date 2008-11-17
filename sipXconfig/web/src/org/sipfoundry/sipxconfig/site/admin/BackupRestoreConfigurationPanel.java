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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.admin.BackupPlan;
import org.sipfoundry.sipxconfig.admin.FtpBackupPlan;
import org.sipfoundry.sipxconfig.admin.ftp.FtpConfiguration;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.common.IPageWithReset;

@ComponentClass
public abstract class BackupRestoreConfigurationPanel extends BaseComponent implements PageBeginRenderListener {

    @InjectObject(value = "spring:adminContext")
    public abstract AdminContext getAdminContext();

    public abstract void setFtpConfiguration(FtpConfiguration ftpConfiguration);

    public abstract BackupPlan getBackupPlan();

    public abstract void setBackupPlan(BackupPlan plan);

    @Parameter(required = false)
    public abstract IPageWithReset getContainerPage();

    public void pageBeginRender(PageEvent event) {
        if (getBackupPlan() != null) {
            return;
        }
        FtpBackupPlan plan = (FtpBackupPlan) getAdminContext().getBackupPlan(FtpBackupPlan.TYPE);
        setBackupPlan(plan);
        setFtpConfiguration(plan.getFtpConfiguration());
    }

    public void onApply() {
        TapestryUtils.getValidator(this).clear();
        getAdminContext().storeBackupPlan(getBackupPlan());
        if (getContainerPage() != null) {
            getContainerPage().reset();
        }
    }
}
