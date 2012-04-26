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
package org.sipfoundry.sipxconfig.firewall;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Collection;
import java.util.List;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.YamlConfiguration;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public class FirewallConfig implements ConfigProvider, FeatureListener, DaoEventListener {
    private static final Logger LOG = Logger.getLogger(FirewallConfig.class);
    private FirewallManager m_firewallManager;
    private AddressManager m_addressManager;
    private ConfigManager m_configManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(FirewallManager.FEATURE)) {
            return;
        }

        File gdir = manager.getGlobalDataDirectory();
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FirewallManager.FEATURE);
        ConfigUtils.enableCfengineClass(gdir, "firewall.cfdat", enabled, "firewall");
        if (!enabled) {
            return;
        }

        FirewallSettings settings = m_firewallManager.getSettings();
        Writer sysctl = new FileWriter(new File(gdir, "sysctl.part"));
        try {
            writeSysctl(sysctl, settings);
        } finally {
            IOUtils.closeQuietly(sysctl);
        }

        List<FirewallRule> rules = m_firewallManager.getFirewallRules();
        List<ServerGroup> groups = m_firewallManager.getServerGroups();
        List<Location> locations = manager.getLocationManager().getLocationsList();
        for (Location location : request.locations(manager)) {
            List<CustomFirewallRule> custom = m_firewallManager.getCustomRules(location, request.getRequestData());
            File dir = manager.getLocationDataDirectory(location);
            Writer config = new FileWriter(new File(dir, "firewall.yaml"));
            try {
                writeIptables(config, rules, custom, groups, locations, location);
            } finally {
                IOUtils.closeQuietly(config);
            }
        }
    }

    void writeSysctl(Writer w, FirewallSettings settings) throws IOException {
        KeyValueConfiguration c = KeyValueConfiguration.equalsSeparated(w);
        c.write(settings.getSettings().getSetting("sysctl"));
    }

    void writeIptables(Writer w, List<FirewallRule> rules, List<CustomFirewallRule> custom,
            List<ServerGroup> groups, List<Location> cluster, Location thisLocation) throws IOException {
        YamlConfiguration c = new YamlConfiguration(w);

        Collection< ? > ips = CollectionUtils.collect(cluster, Location.GET_ADDRESS);
        c.writeInlineArray("cluster", ips);

        c.startArray("chains");
        for (ServerGroup group : groups) {
            c.nextElement();
            c.write(":name", group.getName());
            c.write(":ipv4s", group.getServerList().replaceAll("\\s", ", "));
        }
        c.endArray();

        c.startArray("rules");
        for (FirewallRule rule : rules) {
            AddressType type = rule.getAddressType();
            List<Address> addresses = m_addressManager.getAddresses(type, thisLocation);
            if (addresses != null) {
                for (Address address : addresses) {

                    // not a rule for this server
                    if (!address.getAddress().equals(thisLocation.getAddress())) {
                        continue;
                    }

                    AddressType atype = address.getType();
                    String id = atype.getId();
                    int port = address.getCanonicalPort();
                    // internal error
                    if (port == 0) {
                        LOG.error("Cannot open up port zero for service id " + id);
                        continue;
                    }

                    // blindly allowed
                    if (FirewallRule.SystemId.CLUSTER == rule.getSystemId()) {
                        continue;
                    }

                    c.write(":port", port);
                    c.write(":protocol", atype.getProtocol());
                    c.write(":service", id);
                    c.write(":priority", rule.isPriority());
                    if (address.getEndPort() != 0) {
                        c.write(":end_port", address.getEndPort());
                    }
                    ServerGroup group = rule.getServerGroup();
                    String chain;
                    if (group != null) {
                        chain = group.getName();
                    } else if (rule.getSystemId() == FirewallRule.SystemId.PUBLIC) {
                        chain = "ACCEPT";
                    } else {
                        chain = rule.getSystemId().name();
                    }
                    c.write(":chain", chain);
                    c.nextElement();
                }
            }
        }
        c.endArray();

        for (FirewallTable table : FirewallTable.values()) {
            Collection< ? > tableRules = CollectionUtils.select(custom, CustomFirewallRule.byTable(table));
            if (!tableRules.isEmpty()) {
                c.writeArray(table.toString(), tableRules);
            }
        }
    }

    public void setFirewallManager(FirewallManager firewallManager) {
        m_firewallManager = firewallManager;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    @Override
    public void enableLocationFeature(FeatureManager manager, FeatureEvent event, LocationFeature feature,
            Location location) {
        // every feature enable/disable will trigger firewall rules to reconfig
        // because cannot tell what this affects
        m_configManager.configureEverywhere(FirewallManager.FEATURE);
    }

    @Override
    public void enableGlobalFeature(FeatureManager manager, FeatureEvent event, GlobalFeature feature) {
        // see enableLocationFeature
        m_configManager.configureEverywhere(FirewallManager.FEATURE);
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    @Override
    public void onDelete(Object entity) {
        onChange(entity);
    }

    @Override
    public void onSave(Object entity) {
        onChange(entity);
    }

    void onChange(Object entity) {
        if (entity instanceof Location) {
            m_configManager.configureEverywhere(FirewallManager.FEATURE);
        }
    }
}
