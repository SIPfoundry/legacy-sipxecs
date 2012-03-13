/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.registrar;

import static java.lang.String.format;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dns.DnsManager;
import org.sipfoundry.sipxconfig.dns.DnsProvider;
import org.sipfoundry.sipxconfig.dns.ResourceRecords;
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
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class RegistrarImpl implements FeatureProvider, AddressProvider, BeanFactoryAware, Registrar,
        FeatureListener, DnsProvider, ProcessProvider {
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(new AddressType[] {
        TCP_ADDRESS, UDP_ADDRESS, PRESENCE_MONITOR_ADDRESS, EVENT_ADDRESS
    });
    private BeanWithSettingsDao<RegistrarSettings> m_settingsDao;
    private ListableBeanFactory m_beanFactory;
    private ConfigManager m_configManager;

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return Collections.singleton(FEATURE);
    }

    @Override
    public RegistrarSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(RegistrarSettings settings) {
        m_settingsDao.upsert(settings);
    }

    public void initialize() {
        RegistrarSettings settings = getSettings();
        if (settings != null) {
            return;
        }

        settings = m_beanFactory.getBean(RegistrarSettings.class);
        m_settingsDao.upsert(settings);
    }

    @Override
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        return ADDRESSES;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            Object requester) {
        if (!ADDRESSES.contains(type) || !manager.getFeatureManager().isFeatureEnabled(FEATURE)) {
            return null;
        }

        RegistrarSettings settings = getSettings();
        Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        List<Address> addresses = new ArrayList<Address>(locations.size());
        for (Location location : locations) {
            Address address = null;
            if (type.equals(TCP_ADDRESS)) {
                address = new Address(TCP_ADDRESS, location.getAddress(), settings.getSipTcpPort());
            } else if (type.equals(UDP_ADDRESS)) {
                address = new Address(UDP_ADDRESS, location.getAddress(), settings.getSipUdpPort());
            } else if (type.equals(EVENT_ADDRESS)) {
                address = new Address(EVENT_ADDRESS, location.getAddress(), settings.getMonitorPort());
            } else if (type.equals(XMLRPC_ADDRESS)) {
                address = new Address(XMLRPC_ADDRESS, location.getAddress(), settings.getXmlRpcPort());
            } else if (type.equals(PRESENCE_MONITOR_ADDRESS)) {
                address = new Address(PRESENCE_MONITOR_ADDRESS, location.getAddress(), settings.getPresencePort());
            }
            addresses.add(address);
        }

        return addresses;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public void setSettingsDao(BeanWithSettingsDao<RegistrarSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public void enableLocationFeature(FeatureManager manager, FeatureEvent event, LocationFeature feature,
            Location location) {
        if (!feature.equals(Registrar.FEATURE)) {
            return;
        }

        switch (event) {
        case PRE_ENABLE:
            RegistrarSettings settings = getSettings();
            if (settings.isNew()) {
                saveSettings(settings);
            }
            break;
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

    @Override
    public Address getAddress(DnsManager manager, AddressType t, Collection<Address> addresses, Location whoIsAsking) {
        if (!t.equals(Registrar.TCP_ADDRESS)) {
            return null;
        }

        // NOTE: drop port, it's in DNS resource records
        return new Address(t, format("rr.%s", whoIsAsking.getFqdn()));
    }

    @Override
    public List<ResourceRecords> getResourceRecords(DnsManager manager, Location whoIsAsking) {
        ResourceRecords rr = new ResourceRecords("_sip._tcp", "rr");
        Collection<Address> addresses = getAvailableAddresses(manager.getAddressManager(), TCP_ADDRESS, whoIsAsking);
        rr.addAddresses(addresses);
        return Collections.singletonList(rr);
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE, location);
        return (enabled ? Collections.singleton(new ProcessDefinition("sipregistrar")) : null);
    }

    @Override
    public void getBundleFeatures(Bundle b) {
        if (b.isRouter()) {
            b.addFeature(FEATURE);
        }
    }
}
