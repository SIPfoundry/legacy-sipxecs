/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.snmp;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.List;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;

public class SnmpConfig implements ConfigProvider, FeatureListener, SetupListener {
    private SnmpManager m_snmp;
    private ConfigManager m_configManager;

    public void setup(SetupManager manager) {
        if (!manager.isSetup(SnmpManager.FEATURE.getId())) {
            // SNMP is pretty core to the system, enable it by default
            manager.getFeatureManager().enableGlobalFeature(SnmpManager.FEATURE, true);
            manager.setSetup(SnmpManager.FEATURE.getId());
        }
    }

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(SnmpManager.FEATURE, LocationsManager.FEATURE)) {
            return;
        }
        Set<Location> locations = request.locations(manager);
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(SnmpManager.FEATURE);
        File gdir = manager.getGlobalDataDirectory();
        ConfigUtils.enableCfengineClass(gdir, "snmpd.cfdat", enabled, SnmpManager.FEATURE.getId());
        if (enabled) {
            for (Location location : locations) {
                File dir = manager.getLocationDataDirectory(location);
                List<ProcessDefinition> defs = m_snmp.getProcessDefinitions(location);
                Writer wtr = new FileWriter(new File(dir, "snmpd.conf.part"));
                try {
                    writeProcesses(wtr, defs);
                } finally {
                    IOUtils.closeQuietly(wtr);
                }
            }
        }
    }

    void writeProcesses(Writer w, List<ProcessDefinition> defs) throws IOException {
        for (ProcessDefinition def : defs) {
            w.write("regexp_proc ");
            w.write(def.getProcess());
            String regexp = def.getRegexp();
            if (StringUtils.isNotBlank(regexp)) {
                // max min  (max of 0 means unlimited)
                w.write(" 0 1 ");
                w.write(regexp);
            }
            w.write("\n");
        }
    }

    public void setSnmpManager(SnmpManager snmp) {
        m_snmp = snmp;
    }

    @Override
    public void enableLocationFeature(FeatureManager manager, FeatureEvent event, LocationFeature feature,
            Location location) {
        // cannot tell what processes will become alive/dead
        m_configManager.configureEverywhere(SnmpManager.FEATURE);
    }

    @Override
    public void enableGlobalFeature(FeatureManager manager, FeatureEvent event, GlobalFeature feature) {
        // cannot tell what processes will become alive/dead
        m_configManager.configureEverywhere(SnmpManager.FEATURE);
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }
}
