/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.ivr;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.backup.ArchiveDefinition;
import org.sipfoundry.sipxconfig.backup.ArchiveProvider;
import org.sipfoundry.sipxconfig.backup.BackupManager;
import org.sipfoundry.sipxconfig.backup.BackupSettings;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
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
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallProvider;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.mwi.Mwi;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;
import org.springframework.beans.factory.annotation.Required;

public class IvrImpl implements FeatureProvider, AddressProvider, Ivr, ProcessProvider, DnsProvider,
        FirewallProvider, ArchiveProvider {
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(new AddressType[] {
        REST_API, SIP_ADDRESS
    });
    private static final String VM = "vm";
    private BeanWithSettingsDao<IvrSettings> m_settingsDao;
    private BeanWithSettingsDao<CallPilotSettings> m_pilotSettingsDao;
    private ConfigManager m_configManager;
    private FeatureManager m_featureManager;
    private DomainManager m_domainManager;
    private FreeswitchFeature m_fsFeature;
    private boolean m_highAvailabilitySupport;
    private String m_archiveScript = "sipxivr-archive";
    private SipxReplicationContext m_sipxReplicationContext;


    public IvrSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    public String getAudioFormat() {
        return getSettings().getAudioFormat();
    }

    public CallPilotSettings getCallPilotSettings() {
        return m_pilotSettingsDao.findOrCreateOne();
    }

    public void saveSettings(IvrSettings settings) {
        m_settingsDao.upsert(settings);
    }

    public void saveCallPilotSettings(CallPilotSettings settings) {
        m_pilotSettingsDao.upsert(settings);
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return Collections.singleton(CALLPILOT);
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return Collections.singleton(FEATURE);
    }

    public void setSettingsDao(BeanWithSettingsDao<IvrSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    public void setCallPilotSettingsDao(BeanWithSettingsDao<CallPilotSettings> settingsDao) {
        m_pilotSettingsDao = settingsDao;
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return Collections.singleton(new DefaultFirewallRule(REST_API));
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        if (!ADDRESSES.contains(type)) {
            return null;
        }
        IvrSettings settings = getSettings();
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        List<Address> addresses = new ArrayList<Address>(locations.size());
        for (Location location : locations) {
            Address address = null;
            if (type.equals(SIP_ADDRESS)) {
                address = new Address(SIP_ADDRESS, location.getAddress(), m_fsFeature.getSettings(location)
                        .getFreeswitchSipPort());
            } else if (type.equals(REST_API)) {
                address = new Address(REST_API, location.getAddress(), settings.getHttpPort());
            }
            addresses.add(address);
        }
        return addresses;
    }

    @Override
    public Address getAddress(DnsManager manager, AddressType t, Collection<Address> addresses, Location whoIsAsking) {
        if (!t.equals(SIP_ADDRESS) || !m_featureManager.isFeatureEnabled(Ivr.FEATURE)) {
            return null;
        }

        if (whoIsAsking != null && m_featureManager.isFeatureEnabled(Ivr.FEATURE, whoIsAsking)) {
            return new Address(t, getAddress(whoIsAsking.getHostnameInSipDomain()));
        }
        return new Address(t, getAddress(m_domainManager.getDomainName()));
    }

    private String getAddress(String host) {
        return String.format("%s.%s", VM, host);
    }

    @Override
    public List<ResourceRecords> getResourceRecords(DnsManager manager, Location whoIsAsking) {
        ResourceRecords tcpRecords = new ResourceRecords("_sip._tcp", VM);
        List<ResourceRecords> records = new LinkedList<ResourceRecords>();
        Collection<Address> addresses = getAvailableAddresses(manager.getAddressManager(), SIP_ADDRESS, whoIsAsking);
        if (addresses != null && addresses.isEmpty()) {
            return records;
        }
        tcpRecords.addAddresses(addresses);
        records.add(tcpRecords);
        return records;
    }

    public void setPilotSettingsDao(BeanWithSettingsDao<CallPilotSettings> pilotSettingsDao) {
        m_pilotSettingsDao = pilotSettingsDao;
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public void setFreeswitchFeature(FreeswitchFeature fsFeature) {
        m_fsFeature = fsFeature;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE, location);
        return (enabled ? Collections.singleton(ProcessDefinition.sysvByRegex("sipxivr",
                ".*\\s-Dprocname=sipxivr\\s.*")) : null);
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE_TELEPHONY) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        validator.requiredOnSameHost(FEATURE, FreeswitchFeature.FEATURE);
        validator.requiredOnSameHost(FEATURE, Mwi.FEATURE);
        if (!m_highAvailabilitySupport) {
            validator.singleLocationOnly(FEATURE);
        }
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        if (request.getAllNewlyEnabledFeatures().contains(Ivr.FEATURE)) {
            IvrSettings settings = getSettings();
            if (settings.isNew()) {
                saveSettings(settings);
            }
            m_sipxReplicationContext.generateAll(DataSet.ALIAS);
        }

        if (request.hasChanged(Ivr.FEATURE)) {
            m_configManager.configureEverywhere(DnsManager.FEATURE, DialPlanContext.FEATURE,
                    FreeswitchFeature.FEATURE);
        }
    }

    /**
     * Setting this to true just relaxes the validator, Stock sipXivr will not work in HA mode
     */
    public void setHighAvailabilitySupport(boolean highAvailabilitySupport) {
        m_highAvailabilitySupport = highAvailabilitySupport;
    }

    public void setArchiveScript(String script) {
        m_archiveScript = script;
    }

    @Override
    public Collection<ArchiveDefinition> getArchiveDefinitions(BackupManager manager, Location location,
            BackupSettings settings) {
        if (!manager.getFeatureManager().isFeatureEnabled(FEATURE, location)) {
            return null;
        }

        String script = "$(sipx.SIPX_BINDIR)/" + m_archiveScript;
        ArchiveDefinition def = new ArchiveDefinition(ARCHIVE, script + " --backup %s", script + " --restore %s");
        return Collections.singleton(def);
    }

    @Required
    public void setSipxReplicationContext(SipxReplicationContext replicationContext) {
        m_sipxReplicationContext = replicationContext;
    }
}
