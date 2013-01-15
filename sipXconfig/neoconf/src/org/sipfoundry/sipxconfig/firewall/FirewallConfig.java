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
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.cfgmgt.CfengineModuleConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.YamlConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.ftp.FtpManager;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.setting.Setting;

public class FirewallConfig implements ConfigProvider, FeatureListener {
    private static final Logger LOG = Logger.getLogger(FirewallConfig.class);
    private FirewallManager m_firewallManager;
    private AddressManager m_addressManager;
    private ConfigManager m_configManager;
    private CallRateManager m_callRateManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(FirewallManager.FEATURE, LocationsManager.FEATURE, ImManager.FEATURE,
                FtpManager.FTP_FEATURE)) {
            return;
        }

        FirewallSettings settings = m_firewallManager.getSettings();
        Set<String> blackList = settings.getBlackListSet();
        Set<String> whiteList = settings.getWhiteListSet();
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FirewallManager.FEATURE);
        List<FirewallRule> rules = m_firewallManager.getFirewallRules();
        List<ServerGroup> groups = m_firewallManager.getServerGroups();
        List<Location> locations = manager.getLocationManager().getLocationsList();
        List<CallRateRule> rateRules = m_callRateManager.getCallRateRules();
        for (Location location : request.locations(manager)) {
            File dir = manager.getLocationDataDirectory(location);
            Map<Object, Object> configRequest = request.getRequestData();

            Writer sysconf = new FileWriter(new File(dir, "firewall.cfdat"));
            try {
                Set<String> mods = m_firewallManager.getRequiredModules(location, configRequest);
                writeCfdat(sysconf, enabled, settings.getSystemSettings(), mods);
            } finally {
                IOUtils.closeQuietly(sysconf);
            }

            if (!enabled) {
                continue;
            }

            Writer sysctl = new FileWriter(new File(dir, "sysctl.part"));
            try {
                writeSysctl(sysctl, settings);
            } finally {
                IOUtils.closeQuietly(sysctl);
            }

            List<CustomFirewallRule> custom = m_firewallManager.getCustomRules(location, configRequest);
            Writer config = new FileWriter(new File(dir, "firewall.yaml"));
            try {
                writeIptables(config, whiteList, blackList, settings, rateRules, rules, custom,
                        groups, locations, location);
            } finally {
                IOUtils.closeQuietly(config);
            }
        }
    }

    void writeCfdat(Writer w, boolean enabled, Setting sysSettings, Collection<String> mods) throws IOException {
        CfengineModuleConfiguration cfg = new CfengineModuleConfiguration(w);
        cfg.writeClass("firewall", enabled);
        cfg.writeSettings("firewall_", sysSettings);
        cfg.write("firewall_modules", StringUtils.join(mods, ' '));
    }

    void writeSysctl(Writer w, FirewallSettings settings) throws IOException {
        KeyValueConfiguration c = KeyValueConfiguration.equalsSeparated(w);
        c.writeSettings(settings.getSettings().getSetting("sysctl"));
    }

    void writeIptables(Writer w, Set<String> whiteList, Set<String> blackList, FirewallSettings settings,
            List<CallRateRule> rateRules, List<FirewallRule> rules, List<CustomFirewallRule> custom,
            List<ServerGroup> groups, List<Location> cluster, Location thisLocation)
        throws IOException {
        YamlConfiguration c = new YamlConfiguration(w);

        Collection< ? > ips = CollectionUtils.collect(cluster, Location.GET_ADDRESS);
        c.write("logdropped", settings.isLogDroppedPacketsEnabled());
        c.write("logdos", settings.isLogDosPacketsEnabled());
        c.write("lograte", settings.isLogRatePacketsEnabled());
        c.write("logregister", settings.isLogSipRegisterEnabled());
        c.write("loginvite", settings.isLogSipInviteEnabled());
        c.write("logack", settings.isLogSipAckEnabled());
        c.write("logoptions", settings.isLogSipOptionsEnabled());
        c.write("logsubscribe", settings.isLogSipSubscribeEnabled());
        c.write("loglimit", settings.getLogLimitNumber());
        c.write("loginterval", settings.getLogLimitInterval());

        c.writeInlineArray("cluster", ips);

        c.startArray("chains");
        for (ServerGroup group : groups) {
            c.nextElement();
            c.write(":name", group.getName());
            List<String> sourceIPs = new ArrayList<String>();
            String servers = group.getServerList();
            if (StringUtils.isNotBlank(servers)) {
                sourceIPs = Arrays.asList(StringUtils.split(servers, " "));
            }
            c.writeArray(":ipv4s", sourceIPs);
        }
        c.endArray();

        c.writeArray("whitelist", whiteList);
        c.writeArray("blacklist", blackList);
        c.writeArray("deniedsip", settings.getDeniedSipUAs());

        c.startArray("raterules");
        for (CallRateRule rule : rateRules) {
            c.nextElement();
            c.write(":rule", StringUtils.deleteWhitespace(rule.getName()));
            c.write(":startIp", rule.getStartIp());
            if (rule.getEndIp() != null) {
                c.write(":endIp", rule.getEndIp());
            }
            c.startArray(":limits");
            for (CallRateLimit limit : rule.getCallRateLimits()) {
                c.nextElement();
                c.write(":method", limit.getSipMethod());
                c.write(":rate", limit.getRate());
                c.write(":interval", limit.getInterval());
            }
            c.endArray();
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
                    if (FirewallRule.SystemId.CLUSTER == rule.getSystemId() && rule.getServerGroup() == null) {
                        continue;
                    }

                    c.write(":port", port);
                    c.write(":protocol", atype.getProtocol());
                    c.write(":sip", atype.isExternalSip());
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

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public void setCallRateManager(CallRateManager callRateManager) {
        m_callRateManager = callRateManager;
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        // cannot tell what features would effect firewall, so need to re-evaluate
        m_configManager.configureEverywhere(FirewallManager.FEATURE);
    }
}
