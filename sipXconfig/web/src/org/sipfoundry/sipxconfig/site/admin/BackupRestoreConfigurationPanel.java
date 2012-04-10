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
import org.sipfoundry.sipxconfig.backup.BackupManager;
import org.sipfoundry.sipxconfig.backup.BackupPlan;
import org.sipfoundry.sipxconfig.backup.FtpBackupPlan;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.ftp.FtpExternalServerConfig;
import org.sipfoundry.sipxconfig.site.common.IPageWithReset;

@ComponentClass
public abstract class BackupRestoreConfigurationPanel extends BaseComponent implements PageBeginRenderListener {

    @InjectObject(value = "spring:backupManager")
    public abstract BackupManager getBackupManager();

    public abstract void setFtpConfiguration(FtpExternalServerConfig ftpConfiguration);

    public abstract BackupPlan getBackupPlan();

    public abstract void setBackupPlan(BackupPlan plan);

    @Parameter(required = false)
    public abstract IPageWithReset getContainerPage();

    public void pageBeginRender(PageEvent event) {
        if (getBackupPlan() != null) {
            return;
        }
        FtpBackupPlan plan = (FtpBackupPlan) getBackupManager().getBackupPlan(FtpBackupPlan.TYPE);
        setBackupPlan(plan);
        setFtpConfiguration(plan.getFtpConfiguration());
    }

    public void onApply() {
        TapestryUtils.getValidator(this).clear();
        getBackupManager().storeBackupPlan(getBackupPlan());
        if (getContainerPage() != null) {
            getContainerPage().reset();
        }
    }
}
