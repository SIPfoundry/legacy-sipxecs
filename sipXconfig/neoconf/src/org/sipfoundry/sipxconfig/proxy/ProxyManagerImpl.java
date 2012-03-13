/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.proxy;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alarm.AlarmDefinition;
import org.sipfoundry.sipxconfig.alarm.AlarmProvider;
import org.sipfoundry.sipxconfig.alarm.AlarmServerManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dns.DnsManager;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public class ProxyManagerImpl implements ProxyManager, FeatureProvider, AddressProvider, ProcessProvider,
    FeatureListener, AlarmProvider {
    public static final LocationFeature FEATURE = new LocationFeature("proxy");
    public static final AddressType TCP_ADDRESS = AddressType.sip("proxyTcp");
    public static final AddressType UDP_ADDRESS = AddressType.sip("procyUdp");
    public static final AddressType TLS_ADDRESS = AddressType.sip("proxyTls");
    private static final Collection<AddressType> ADDRESS_TYPES = Arrays.asList(new AddressType[] {
        TCP_ADDRESS, UDP_ADDRESS, TLS_ADDRESS
    });
    private FeatureManager m_featureManager;
    private BeanWithSettingsDao<ProxySettings> m_settingsDao;
    private ConfigManager m_configManager;

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return Collections.singleton(FEATURE);
    }

    public ProxySettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    public void saveSettings(ProxySettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        return ADDRESS_TYPES;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            Object requester) {
        if (!ADDRESS_TYPES.contains(type)) {
            return null;
        }
        Collection<Address> addresses = null;
        Collection<Location> locations = m_featureManager.getLocationsForEnabledFeature(FEATURE);
        addresses = new ArrayList<Address>(locations.size());
        for (Location location : locations) {
            Address address = null;
            if (type.equals(TCP_ADDRESS)) {
                address = new Address(TCP_ADDRESS, location.getAddress(), 5060);
            } else if (type.equals(UDP_ADDRESS)) {
                address = new Address(UDP_ADDRESS, location.getAddress(), 5060);
            } else if (type.equals(TLS_ADDRESS)) {
                address = new Address(UDP_ADDRESS, location.getAddress(), 5061);
            }
            addresses.add(address);
        }

        return addresses;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public void setSettingsDao(BeanWithSettingsDao<ProxySettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = m_featureManager.isFeatureEnabled(FEATURE, location);
        return (enabled ? Collections.singleton(new ProcessDefinition("sipXproxy")) : null);
    }

    @Override
    public void enableLocationFeature(FeatureManager manager, FeatureEvent event, LocationFeature feature,
            Location location) {
        if (!feature.equals(FEATURE)) {
            return;
        }

        switch (event) {
        case POST_DISABLE:
        case POST_ENABLE:
            m_configManager.configureEverywhere(DnsManager.FEATURE, DialPlanContext.FEATURE);
            break;
        default:
            break;
        }
    }

    @Override
    public void enableGlobalFeature(FeatureManager manager, FeatureEvent event, GlobalFeature feature) {
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    @Override
    public Collection<AlarmDefinition> getAvailableAlarms(AlarmServerManager manager) {
        return Collections.singleton(EMERG_NUMBER_DIALED);
    }

    @Override
    public void getBundleFeatures(Bundle b) {
        if (b.isRouter()) {
            b.addFeature(FEATURE);
        }
    }
}
