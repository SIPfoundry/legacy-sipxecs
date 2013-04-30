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
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallProvider;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class RegistrarImpl implements FeatureProvider, AddressProvider, BeanFactoryAware, Registrar,
        DnsProvider, ProcessProvider, FirewallProvider {
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(new AddressType[] {
        TCP_ADDRESS, UDP_ADDRESS, PRESENCE_MONITOR_ADDRESS, EVENT_ADDRESS
    });
    private static final String PROCESS = "sipxregistrar";
    private BeanWithSettingsDao<RegistrarSettings> m_settingsDao;
    private ListableBeanFactory m_beanFactory;
    private ConfigManager m_configManager;

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
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
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return DefaultFirewallRule.rules(ADDRESSES);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            Location requester) {
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
    public Address getAddress(DnsManager manager, AddressType t, Collection<Address> addresses, Location whoIsAsking) {
        if (!t.equals(Registrar.TCP_ADDRESS)) {
            return null;
        }

        // NOTE: drop port, it's in DNS resource records
        return new Address(t, format("rr.%s", whoIsAsking.getHostnameInSipDomain()));
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
        if (!manager.getFeatureManager().isFeatureEnabled(FEATURE, location)) {
            return null;
        }
        ProcessDefinition def = ProcessDefinition.sipx("sipregistrar", PROCESS, PROCESS);
        return Collections.singleton(def);
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE_TELEPHONY) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        validator.requiredOnSameHost(FEATURE, ProxyManager.FEATURE);
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        if (request.getAllNewlyEnabledFeatures().contains(FEATURE)) {
            RegistrarSettings settings = getSettings();
            if (settings.isNew()) {
                saveSettings(settings);
            }
        }
        if (request.hasChanged(FEATURE)) {
            m_configManager.configureEverywhere(DnsManager.FEATURE, DialPlanContext.FEATURE);
        }
    }
}
