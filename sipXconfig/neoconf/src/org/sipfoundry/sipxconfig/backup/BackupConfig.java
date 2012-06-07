/**
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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

import static java.lang.String.format;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.cfgmgt.CfengineModuleConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.YamlConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;

public class BackupConfig implements ConfigProvider {
    private BackupManager m_backupManager;
    private ConfigManager m_configManager;
    private LocationsManager m_locationsManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(BackupManager.FEATURE)) {
            return;
        }

        BackupSettings settings = m_backupManager.getSettings();
        Collection<BackupPlan> plans = m_backupManager.getBackupPlans();
        List<Location> hosts = manager.getLocationManager().getLocationsList();
        Set<Location> locations = request.locations(manager);
        for (Location location : locations) {
            for (BackupPlan plan : plans) {
                writeConfigs(plan, location, hosts, settings);
            }
        }
    }

    public File writeConfigs(BackupPlan plan, BackupSettings settings) {
        try {
            File planFile = null;
            List<Location> hosts = m_locationsManager.getLocationsList();
            for (Location location : hosts) {
                File f = writeConfigs(plan, location, hosts, settings);
                if (location.isPrimary()) {
                    planFile = f;
                }
            }
            return planFile;
        } catch (IOException e) {
            throw new RuntimeException("Error writing backup plans", e);
        }
    }

    public File writeConfigs(BackupPlan plan, Location location, Collection<Location> hosts, BackupSettings settings)
        throws IOException {
        String fname = format("archive-%s.yaml", plan.getType());
        File dir = m_configManager.getLocationDataDirectory(location);
        Collection<ArchiveDefinition> defs = m_backupManager.getArchiveDefinitions(location);
        Collection<String> auto = plan.getAutoModeDefinitionIds();
        Collection<String> manual = plan.getManualModeDefinitionIds();

        File planFile = new File(dir, fname);
        Writer config = new FileWriter(planFile);
        try {
            writeBackupDefinitions(config, defs, auto, manual);
            if (location.isPrimary()) {
                writePrimaryBackupConfig(config, plan, hosts, settings);
            }
        } finally {
            IOUtils.closeQuietly(config);
        }

        String datname = format("archive-%s.cfdat", plan.getType());
        Writer cfdat = new FileWriter(new File(dir, datname));
        try {
            writeCfengineConfig(cfdat, plan.getType(), auto, manual);
            if (location.isPrimary() && plan.getSchedules() != null) {
                writeBackupSchedules(cfdat, plan.getType(), plan.getSchedules());
            }
        } finally {
            IOUtils.closeQuietly(cfdat);
        }

        return planFile;
    }

    void writeCfengineConfig(Writer w, BackupType type, Collection<String> autoMode,
            Collection<String> manualMode) throws IOException {
        CfengineModuleConfiguration config = new CfengineModuleConfiguration(w);
        // no reason to disable backups, just don't configure any schedules
        config.writeClass("archive", true);
        config.writeList(type.toString() + "_auto_backup_definitions", autoMode);
        config.writeList(type.toString() + "_manual_backup_definitions", manualMode);
    }

    void writeBackupSchedules(Writer w, BackupType type, Collection<DailyBackupSchedule> schedules) throws IOException {
        CfengineModuleConfiguration config = new CfengineModuleConfiguration(w);
        List<String> crons = new ArrayList<String>();
        for (DailyBackupSchedule schedule : schedules) {
            crons.add(schedule.toCronString());
        }
        config.writeList(type.toString() + "BackupSchedule", crons);
    }

    public void writePrimaryBackupConfig(Writer w, BackupPlan plan, Collection<Location> hosts, BackupSettings settings)
        throws IOException {
        YamlConfiguration config = new YamlConfiguration(w);

        List<String> ips = new ArrayList<String>(hosts.size());
        for (Location host : hosts) {
            ips.add(host.getAddress());
        }
        config.writeArray("hosts", ips);

        config.write("plan", plan.getType());
        config.write("max", plan.getLimitedCount());
        if (plan.getType() == BackupType.ftp) {
            String ftp = "ftp";
            config.startStruct(ftp);
            config.writeSettings(settings.getSettings().getSetting(ftp));
            config.endStruct();
        }
    }

    public void writeBackupDefinitions(Writer w, Collection<ArchiveDefinition> defs, Collection<String> auto,
            Collection<String> manual) throws IOException {
        YamlConfiguration config = new YamlConfiguration(w);
        config.startStruct("backup");
        for (ArchiveDefinition def : defs) {
            writeCommand(config, def, def.getBackupCommand());
        }
        config.endStruct();

        config.startStruct("restore");
        for (ArchiveDefinition def : defs) {
            writeCommand(config, def, def.getRestoreCommand());
        }
        config.endStruct();

        config.writeArray("auto", auto);
        config.writeArray("manual", manual);
    }

    void writeCommand(YamlConfiguration config, ArchiveDefinition def, String command) throws IOException {
        if (StringUtils.isNotBlank(command)) {
            config.write(def.getId(), command);
        }
    }

    public void setBackupManager(BackupManager backupManager) {
        m_backupManager = backupManager;
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
