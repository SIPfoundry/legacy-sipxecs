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

import org.apache.commons.io.IOUtils;
import org.apache.poi.util.TempFile;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;

public class RestoreApi extends Resource {
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
        try {
            planFile = TempFile.createTempFile("restore", "yaml");
            planWtr = new FileWriter(planFile);
            Collection<Location> hosts = m_locationsManager.getLocationsList();
            m_backupConfig.writeConfig(planWtr, plan, hosts, settings);
            IOUtils.closeQuietly(planWtr);
            planWtr = null;
            m_backupRunner.restore(planFile, selections);
        } catch (Exception e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        } finally {
            IOUtils.closeQuietly(planWtr);
            if (planFile != null) {
                planFile.delete();
            }
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
