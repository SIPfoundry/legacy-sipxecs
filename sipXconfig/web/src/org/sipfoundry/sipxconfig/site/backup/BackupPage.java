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

    public void pageBeginRender(PageEvent event_) {
        if (getLocalPlan() == null) {
            setLocalPlan(getBackupManager().findOrCreateBackupPlan(BackupType.local));
        }
        if (getFtpPlan() == null) {
            setFtpPlan(getBackupManager().findOrCreateBackupPlan(BackupType.ftp));
        }
    }
}
