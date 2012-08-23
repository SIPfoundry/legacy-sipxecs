/**
 *
 *
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
package org.sipfoundry.sipxconfig.snmp;

import static java.lang.String.format;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Collection;
import java.util.List;
import java.util.Set;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.SystemUtils;
import org.sipfoundry.sipxconfig.alarm.Alarm;
import org.sipfoundry.sipxconfig.alarm.AlarmServerManager;
import org.sipfoundry.sipxconfig.alarm.Alarms;
import org.sipfoundry.sipxconfig.cfgmgt.CfengineModuleConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;

public class SnmpConfig implements ConfigProvider, FeatureListener, SetupListener {
    private SnmpManager m_snmp;
    private ConfigManager m_configManager;
    private AlarmServerManager m_alarmServerManager;

    public boolean setup(SetupManager manager) {
        if (!manager.isTrue(SnmpManager.FEATURE.getId())) {
            // SNMP is pretty core to the system, enable it by default
            manager.getFeatureManager().enableGlobalFeature(SnmpManager.FEATURE, true);
            manager.setTrue(SnmpManager.FEATURE.getId());
        }
        return true;
    }

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(SnmpManager.FEATURE, LocationsManager.FEATURE)) {
            return;
        }
        Set<Location> locations = request.locations(manager);
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(SnmpManager.FEATURE);
        boolean alarmsEnabled = manager.getFeatureManager().isFeatureEnabled(Alarms.FEATURE);
        File gdir = manager.getGlobalDataDirectory();
        SnmpSettings settings = m_snmp.getSettings();
        List<Alarm> alarms = null;
        if (alarmsEnabled) {
            alarms = m_alarmServerManager.getAlarms();
        }

        Writer cfdat = new FileWriter(new File(gdir, "snmpd.cfdat"));
        try {
            CfengineModuleConfiguration config = new CfengineModuleConfiguration(cfdat);
            config.writeClass(SnmpManager.FEATURE.getId(), enabled);
            config.writeSettings("snmp_", settings.getSettings().getSetting("cfdat"));
        } finally {
            IOUtils.closeQuietly(cfdat);
        }

        File fixDeadProcs = new File(gdir, "snmp_fix_dead_processes");
        if (enabled && settings.isFixDeadProcesses()) {
            FileUtils.touch(fixDeadProcs);
        } else if (fixDeadProcs.exists()) {
            fixDeadProcs.delete();
        }

        if (enabled) {
            for (Location location : locations) {
                File dir = manager.getLocationDataDirectory(location);
                List<ProcessDefinition> defs = m_snmp.getProcessDefinitions(location);
                Writer wtr = new FileWriter(new File(dir, "snmpd.conf.part"));
                try {
                    if (alarmsEnabled) {
                        writeActiveAlarms(wtr, alarms, location);
                    }
                    writeProcesses(wtr, defs);
                } finally {
                    IOUtils.closeQuietly(wtr);
                }
            }
        }
    }

    void writeActiveAlarms(Writer w, Collection<Alarm> alarms, Location location) throws IOException {
        for (String config : m_alarmServerManager.getActiveMonitorConfiguration(m_snmp, alarms, location)) {
            w.write(config);
            w.write(SystemUtils.LINE_SEPARATOR);
        }
    }

    void writeProcesses(Writer w, List<ProcessDefinition> defs) throws IOException {
        String eol = SystemUtils.LINE_SEPARATOR;
        for (ProcessDefinition def : defs) {
            w.write("proc ");
            w.write(def.getProcess());
            String regexp = def.getRegexp();
            if (StringUtils.isNotBlank(regexp)) {
                // max min (max of 0 means unlimited)
                w.write(" 0 1 ");
                w.write(regexp);
            }
            w.write(eol);
            if (StringUtils.isNotBlank(def.getRestartCommand())) {
                String fix = format("procfix %s $(sipx.SIPX_LIBEXECDIR)/snmp-fix-process %s %s\n", def.getProcess(),
                        def.getProcess(), def.getRestartCommand());
                w.write(fix);
            }
        }
    }

    public void setSnmpManager(SnmpManager snmp) {
        m_snmp = snmp;
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        // cannot tell what processes will become alive/dead
        m_configManager.configureEverywhere(SnmpManager.FEATURE);
    }

    public void setAlarmServerManager(AlarmServerManager alarmServerManager) {
        m_alarmServerManager = alarmServerManager;
    }
}
