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

import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.backup.BackupManager;
import org.sipfoundry.sipxconfig.backup.BackupPlan;
import org.sipfoundry.sipxconfig.backup.BackupType;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.mongo.MongoManager;
import org.sipfoundry.sipxconfig.mongo.MongoNode;

public abstract class RestorePage extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "bakup/RestorePage";
    private static final String PRIMARY = "PRIMARY";

    @Persist
    @InitialValue(value = "literal:restoreLocal")
    public abstract String getTab();

    @InjectObject(value = "spring:backupManager")
    public abstract BackupManager getBackupManager();

    @InjectObject(value = "spring:mongoManager")
    public abstract MongoManager getMongoManager();

    @InjectObject(value = "spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    public abstract BackupPlan getLocalPlan();

    public abstract void setLocalPlan(BackupPlan plan);

    public abstract BackupPlan getFtpPlan();

    public abstract void setFtpPlan(BackupPlan plan);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    //A restore can be executed only if the master host is also Mongo PRIMARY
    //Restore cannot be executed on a SECONDARY Mongo node
    public abstract Boolean getCanRestore();

    public abstract void setCanRestore(Boolean canRestore);

    public void pageBeginRender(PageEvent event_) {
        if (getLocalPlan() == null) {
            setLocalPlan(getBackupManager().findOrCreateBackupPlan(BackupType.local));
        }
        if (getFtpPlan() == null) {
            setFtpPlan(getBackupManager().findOrCreateBackupPlan(BackupType.ftp));
        }
        if (getCanRestore() == null) {
            setCanRestore(false);
            Collection<MongoNode> mongoNodes = getMongoManager().getMeta().getNodes();
            for (MongoNode node : mongoNodes) {
                if (StringUtils.equals(node.getFqdn(), getLocationsManager().getPrimaryLocation().getFqdn())) {
                    if (node.getStatus() != null && node.getStatus().contains(PRIMARY)) {
                        setCanRestore(true);
                        break;
                    }
                }
            }
        }
    }
}
