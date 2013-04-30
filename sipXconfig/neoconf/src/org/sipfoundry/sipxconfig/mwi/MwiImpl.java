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
package org.sipfoundry.sipxconfig.mwi;

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
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.InvalidChange;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallProvider;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public class MwiImpl implements AddressProvider, FeatureProvider, Mwi, DnsProvider, ProcessProvider,
        FirewallProvider {
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(SIP_UDP, SIP_TCP, HTTP_API);
    private static final String PROCESS = "sipxpublisher";
    private BeanWithSettingsDao<MwiSettings> m_settingsDao;
    private ConfigManager m_configManager;
    private FeatureManager m_featureManager;
    private DomainManager m_domainManager;

    public MwiSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    public void saveSettings(MwiSettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return Collections.singleton(FEATURE);
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return DefaultFirewallRule.rules(ADDRESSES);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        if (!ADDRESSES.contains(type)) {
            return null;
        }

        MwiSettings settings = getSettings();
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        List<Address> addresses = new ArrayList<Address>(locations.size());
        for (Location location : locations) {
            Address address;
            if (type.equals(HTTP_API)) {
                address = new Address(HTTP_API, location.getAddress(), settings.getHttpApiPort());
            } else if (type.equals(SIP_UDP)) {
                address = new Address(SIP_UDP, location.getAddress(), settings.getUdpPort());
            } else {
                address = new Address(SIP_TCP, location.getAddress(), settings.getTcp());
            }
            addresses.add(address);
        }
        return addresses;
    }

    public void setSettingsDao(BeanWithSettingsDao<MwiSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Address getAddress(DnsManager manager, AddressType t, Collection<Address> addresses, Location whoIsAsking) {
        if (t.equals(HTTP_API)) {
            return new Address(HTTP_API, whoIsAsking.getAddress(), getSettings().getHttpApiPort());
        }

        if (t.equals(SIP_TCP)) {
            // NOTE: drop port, it's in DNS resource records
            if (m_featureManager.isFeatureEnabled(Mwi.FEATURE, whoIsAsking)) {
                return new Address(t, getAddress(whoIsAsking.getHostnameInSipDomain()));
            } else {
                return new Address(t, getAddress(m_domainManager.getDomainName()));
            }
        }

        return null;
    }

    private String getAddress(String host) {
        return String.format("mwi.%s", host);
    }

    @Override
    public List<ResourceRecords> getResourceRecords(DnsManager manager, Location whoIsAsking) {
        ResourceRecords rr = new ResourceRecords("_sip._tcp", "mwi");
        Collection<Address> addresses = getAvailableAddresses(manager.getAddressManager(), SIP_TCP, whoIsAsking);
        rr.addAddresses(addresses);
        return Collections.singletonList(rr);
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE, location);
        ProcessDefinition def = ProcessDefinition.sipx("sipstatus", PROCESS, PROCESS);
        return (enabled ? Collections.singleton(def) : null);
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE_TELEPHONY) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        // Do not auto resolve to avoid circular dependency
        if (validator.isEnabledSomewhere(FEATURE) && !validator.isEnabledSomewhere(Ivr.FEATURE)) {
            InvalidChange requires = InvalidChange.requires(FEATURE, Ivr.FEATURE);
            requires.setAllowAutoResolve(false);
            validator.getInvalidChanges().add(requires);
        }
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        if (request.hasChanged(FEATURE)) {
            m_configManager.configureEverywhere(DnsManager.FEATURE, DialPlanContext.FEATURE);
        }
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
}
