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

public class BackupConfig implements ConfigProvider {
    private BackupManager m_backupManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(BackupManager.FEATURE)) {
            return;
        }

        BackupSettings settings = m_backupManager.getSettings();
        Collection<BackupPlan> plans = m_backupManager.getBackupPlans();
        Set<Location> locations = request.locations(manager);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            for (BackupPlan plan : plans) {
                Collection<ArchiveDefinition> defs = m_backupManager.getArchiveDefinitions(plan, location);
                String fname = format("backup-%s.yaml", plan.getType());
                Writer config = new FileWriter(new File(dir, fname));
                try {
                    writeBackupConfig(config, defs);
                } finally {
                    IOUtils.closeQuietly(config);
                }
            }

            if (location.isPrimary()) {
                for (BackupPlan plan : plans) {
                    String fname = format("backup-cluster-%.yaml", plan.getType());
                    Writer cluster = new FileWriter(new File(dir, fname));
                    try {
                        List<Location> hosts = manager.getLocationManager().getLocationsList();
                        writeClusterBackupConfig(cluster, plan, hosts, settings);
                    } finally {
                        IOUtils.closeQuietly(cluster);
                    }
                }

                Writer schedules = new FileWriter(new File(dir, "backup-schedules.cfdat"));
                try {
                    writeBackupSchedules(schedules, plans);
                } finally {
                    IOUtils.closeQuietly(schedules);
                }
            }
        }
    }

    void writeBackupSchedules(Writer w, Collection<BackupPlan> plans) throws IOException {
        CfengineModuleConfiguration config = new CfengineModuleConfiguration(w);
        for (BackupPlan plan : plans) {
            List<String> crons = new ArrayList<String>();
            for (DailyBackupSchedule schedule : plan.getSchedules()) {
                crons.add(schedule.toCronString());
            }
            config.writeList("backupSchedule" + plan.getType().toString(), crons);
        }
    }

    public void writeClusterBackupConfig(Writer w, BackupPlan plan, Collection<Location> hosts,
            BackupSettings settings) throws IOException {
        writeClusterBackupConfig(w, plan, plan.getType().toString(), hosts, settings);
    }

    public void writeClusterBackupConfig(Writer w, BackupPlan plan, String planId, Collection<Location> hosts,
            BackupSettings settings) throws IOException {
        YamlConfiguration config = new YamlConfiguration(w);

        List<String> ips = new ArrayList<String>(hosts.size());
        for (Location host : hosts) {
            ips.add(host.getAddress());
        }
        config.writeArray("hosts", ips);

        config.write("plan", planId);
        config.write("max", plan.getLimitedCount());
        if (plan.getType() == BackupType.ftp) {
            String ftp = "ftp";
            config.startStruct(ftp);
            config.writeSettings(settings.getSettings().getSetting(ftp));
            config.endStruct();
        }
    }

    public void writeBackupConfig(Writer w, Collection<ArchiveDefinition> defs) throws IOException {
        YamlConfiguration config = new YamlConfiguration(w);
        config.startStruct("backup");
        for (ArchiveDefinition def : defs) {
            writeCommand(config, def, def.getBackupCommand());
        }
        config.endStruct();
    }

    public void writeRestoreConfig(Writer w, Collection<ArchiveDefinition> defs, String restoreId)
        throws IOException {
        YamlConfiguration config = new YamlConfiguration(w);
        config.startStruct("restore");
        for (ArchiveDefinition def : defs) {
            writeCommand(config, def, def.getRestoreCommand());
        }
        config.endStruct();
    }

    void writeCommand(YamlConfiguration config, ArchiveDefinition def, String command) throws IOException {
        if (StringUtils.isBlank(command)) {
            return;
        }

        config.write(def.getId(), command);
    }

    public void setBackupManager(BackupManager backupManager) {
        m_backupManager = backupManager;
    }
}
