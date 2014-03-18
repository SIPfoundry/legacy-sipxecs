/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.backup;

import java.io.File;
import java.io.FileWriter;
import java.io.Writer;
import java.util.Collection;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;

public class RestoreApi extends Resource {
    private static final Log LOG = LogFactory.getLog(RestoreApi.class);

    private BackupRunner m_backupRunner;
    private LocationsManager m_locationsManager;
    private BackupConfig m_backupConfig;
    private BackupApi m_backupApi;

    @Override
    public boolean allowPost() {
        return true;
    }

    /**
     * POST : Restore
     */
    @Override
    public void acceptRepresentation(Representation entity) throws ResourceException {
        m_backupApi.putOrPost(entity);
    }

    public void restore(BackupPlan plan, BackupSettings settings, Collection<String> selections)
        throws ResourceException {

        File planFile = null;
        Writer planWtr = null;
        String configuration = StringUtils.EMPTY;
        try {
            //write user selection in a dedicated temp file : archive-backup-tmp-local.yaml/archive-backup-tmp-ftp.yaml
            //we cannot use a temp file because if HA setup
            //when backup runs on many nodes, a timeout may be returned and the temp file gets silently deleted
            planFile = m_backupApi.getBackupManager().getTmpRestoreFile(plan);
            planWtr = new FileWriter(planFile);
            Collection<Location> hosts = m_locationsManager.getLocationsList();
            m_backupConfig.writeConfig(planWtr, plan, hosts, settings);
            IOUtils.closeQuietly(planWtr);
            planWtr = null;
            configuration = FileUtils.readFileToString(planFile);
            m_backupRunner.restore(planFile, selections);
            //if we are here, than restore finished successful
            LOG.info("Restore SUCCEEDED for configuration: " + configuration);
        } catch (Exception e) {
            LOG.error("Restore FAILED for configuration: " + configuration, e);
            if (e instanceof BackupRunnerImpl.TimeoutException) {
                throw new ResourceException(Status.CLIENT_ERROR_REQUEST_TIMEOUT, e);
            } else {
                throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e);
            }
        } finally {
            IOUtils.closeQuietly(planWtr);
        }
    }

    public void setBackupRunner(BackupRunner backupRunner) {
        m_backupRunner = backupRunner;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setBackupConfig(BackupConfig backupConfig) {
        m_backupConfig = backupConfig;
    }

    public void setBackupApi(BackupApi backupApi) {
        m_backupApi = backupApi;
    }
}
