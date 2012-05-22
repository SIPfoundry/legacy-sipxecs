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
package org.sipfoundry.sipxconfig.time;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.RunRequest;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallProvider;
import org.sipfoundry.sipxconfig.firewall.FirewallRule;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public class NtpManagerImpl implements NtpManager, ProcessProvider, FeatureProvider, SetupListener,
        FirewallProvider, AddressProvider {
    private static final List<AddressType> ADDRESSES = Arrays.asList(NTP_SERVER);
    private static final Log LOG = LogFactory.getLog(NtpManagerImpl.class);
    private BeanWithSettingsDao<NtpSettings> m_settingsDao;
    private ConfigManager m_configManager;
    private LocationsManager m_locationsManager;

    @Override
    public NtpSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(NtpSettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public void setSystemTimezone(String timezone) {
        Writer wtr = null;
        try {
            File f = new File(m_configManager.getGlobalDataDirectory(), "timezone.ini");
            wtr = new FileWriter(f);
            wtr.write(timezone);
            wtr.flush();
            RunRequest changeTimezone = new RunRequest("change timezone", m_locationsManager.getLocationsList());
            changeTimezone.setBundles("change_timezone");
            m_configManager.run(changeTimezone);
        } catch (IOException ex) {
            LOG.error(String.format("Failed to change timezone %s", ex.getMessage()));
            throw new UserException("&error.changetimezone.failed");
        } finally {
            IOUtils.closeQuietly(wtr);
        }
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE);
        return (enabled ? Collections.singleton(ProcessDefinition.sysvDefault("ntpd")) : null);
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return Collections.singleton(FEATURE);
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return null;
    }

    @Override
    public void setup(SetupManager manager) {
        if (!manager.isSetup(FEATURE.getId())) {
            manager.getFeatureManager().enableGlobalFeature(FEATURE, true);
            manager.setSetup(FEATURE.getId());
        }
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return Collections.singleton(new DefaultFirewallRule(NTP_ADDRESS, FirewallRule.SystemId.PUBLIC));
    }

    public void setSettingsDao(BeanWithSettingsDao<NtpSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        if (!ADDRESSES.contains(type)) {
            return null;
        }
        if (type.equals(NTP_SERVER)) {
            List<String> ntpServers = getSettings().getNtpServers();
            List<Address> addresses = new ArrayList<Address>();
            for (String server : ntpServers) {
                Address address = new Address(NTP_SERVER, server);
                addresses.add(address);
            }
            return addresses;
        }
        return null;
    }
}
