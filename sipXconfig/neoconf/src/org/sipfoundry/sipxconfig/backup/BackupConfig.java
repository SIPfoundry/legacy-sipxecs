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
import java.util.Collections;
import java.util.List;
import java.util.Set;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.cfgmgt.CfengineModuleConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.YamlConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;

public class BackupConfig implements ConfigProvider {
    private static final String RESTORE = "restore";
    private static final String AUTO = "auto";
    private static final String MANUAL = "manual";
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
        ConfigUtils.enableCfengineClass(manager.getGlobalDataDirectory(), "archive.cfdat", true, "archive");
        for (Location location : locations) {
            for (BackupPlan plan : plans) {
                writeConfigs(plan, null, location, hosts, settings, null);
            }
        }
    }

    public File writeManualBackupConfigs(BackupType type, BackupSettings manualSettings) {
        BackupPlan plan = m_backupManager.findOrCreateBackupPlan(type);
        return writeManualBackupConfigs(plan, manualSettings);
    }

    public File writeManualBackupConfigs(BackupPlan manualPlan, BackupSettings manualSettings) {
        try {
            BackupPlan autoPlan = m_backupManager.findOrCreateBackupPlan(manualPlan.getType());
            BackupSettings autoSettings = m_backupManager.getSettings();
            File planFile = null;
            List<Location> hosts = m_locationsManager.getLocationsList();
            for (Location location : hosts) {
                File f = writeConfigs(autoPlan, manualPlan, location, hosts, autoSettings, manualSettings);
                if (location.isPrimary()) {
                    planFile = f;
                }
            }
            return planFile;
        } catch (IOException e) {
            throw new RuntimeException("Error writing backup plans", e);
        }
    }

    File writeConfigs(BackupPlan autoPlan, BackupPlan manualPlan, Location location, Collection<Location> hosts,
            BackupSettings autoSettings, BackupSettings manualSettings) throws IOException {
        String fname = format("archive-%s.yaml", autoPlan.getType());
        File dir = m_configManager.getLocationDataDirectory(location);
        Collection<ArchiveDefinition> defs = m_backupManager.getArchiveDefinitions(location, manualSettings);
        Collection<String> auto = autoPlan.getAutoModeDefinitionIds();
        Collection<String> manual;
        if (manualPlan == null) {
            manual = Collections.emptyList();
        } else {
            manual = manualPlan.getAutoModeDefinitionIds();
        }

        File planFile = new File(dir, fname);
        Writer config = new FileWriter(planFile);
        try {
            writeBackupDefinitions(config, defs, auto, manual);
            if (location.isPrimary()) {
                writePrimaryBackupConfig(config, autoPlan, manualPlan, hosts, autoSettings, manualSettings);
            }
        } finally {
            IOUtils.closeQuietly(config);
        }

        if (location.isPrimary() && autoPlan.getSchedules() != null) {
            String datname = format("archive-%s.cfdat", autoPlan.getType());
            Writer cfdat = new FileWriter(new File(dir, datname));
            try {
                writeBackupSchedules(cfdat, autoPlan.getType(), autoPlan.getSchedules());
            } finally {
                IOUtils.closeQuietly(cfdat);
            }
        }

        return planFile;
    }

    void writeBackupSchedules(Writer w, BackupType type, Collection<DailyBackupSchedule> schedules) throws IOException {
        CfengineModuleConfiguration config = new CfengineModuleConfiguration(w);
        List<String> crons = new ArrayList<String>();
        for (DailyBackupSchedule schedule : schedules) {
            if (schedule.isEnabled()) {
                crons.add(schedule.toCronString());
            }
        }
        config.writeList(type.toString() + "_backup_schedule", crons);
    }

    void writePrimaryBackupConfig(Writer w, BackupPlan autoPlan, BackupPlan backupPlan, Collection<Location> hosts,
            BackupSettings auto, BackupSettings manual) throws IOException {
        YamlConfiguration config = new YamlConfiguration(w);

        config.startStruct("hosts");
        for (Location host : hosts) {
            config.write(host.getId().toString(), host.isPrimary() ? "127.0.0.1" : host.getAddress());
        }
        config.endStruct();

        config.write("plan", autoPlan.getType());
        config.write("max", autoPlan.getLimitedCount());
        config.startStruct("settings");
        config.startStruct(AUTO);
        writeSettings(config, autoPlan, auto);
        config.endStruct();
        config.startStruct(MANUAL);
        writeSettings(config, backupPlan, manual);
        config.endStruct();
        config.endStruct();

        config.startStruct("correlate_restore");
        for (Location host : hosts) {
            Collection<ArchiveDefinition> defs = m_backupManager.getArchiveDefinitions(host, null);
            @SuppressWarnings("unchecked")
            Collection<String> defIds = CollectionUtils.collect(defs, ArchiveDefinition.GET_IDS);
            config.writeInlineArray(host.getId().toString(), defIds);
        }
        config.endStruct();
    }

    void writeSettings(YamlConfiguration config, BackupPlan plan, BackupSettings settings)
        throws IOException {
        if (settings == null || plan == null) {
            return;
        }

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

        config.startStruct(RESTORE);
        for (ArchiveDefinition def : defs) {
            writeCommand(config, def, def.getRestoreCommand());
        }
        config.endStruct();

        config.startStruct("selected_backups");
        config.writeArray(AUTO, auto);
        config.writeArray(MANUAL, manual);
        config.endStruct();
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
