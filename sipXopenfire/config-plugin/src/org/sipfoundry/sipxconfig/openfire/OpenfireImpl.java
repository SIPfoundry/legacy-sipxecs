/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.openfire;

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
import org.sipfoundry.sipxconfig.cfgmgt.RunRequest;
import org.sipfoundry.sipxconfig.commserver.Location;
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
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.rls.Rls;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;
import org.springframework.beans.factory.annotation.Required;

public class OpenfireImpl extends ImManager implements FeatureProvider, AddressProvider, ProcessProvider, Openfire,
    FirewallProvider {
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(new AddressType[] {
        XMPP_ADDRESS, XMPP_SECURE_ADDRESS, XMPP_FEDERATION_ADDRESS, XMLRPC_ADDRESS, XMLRPC_VCARD_ADDRESS,
        WATCHER_ADDRESS, XMPP_FILE_TRANSFER_PROXY_ADDRESS, XMPP_ADMIN_CONSOLE_ADDRESS
    });
    private BeanWithSettingsDao<OpenfireSettings> m_settingsDao;
    private ConfigManager m_configManager;
    private boolean m_highAvailabilitySupport;

    @Override
    public OpenfireSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(OpenfireSettings settings) {
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
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            Location whoIsAsking) {
        if (!ADDRESSES.contains(type)) {
            return null;
        }
        Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        // only zero or one at this time allowed
        if (locations.isEmpty()) {
            return null;
        }

        OpenfireSettings settings = getSettings();
        List<Address> addresses = new ArrayList<Address>(locations.size());
        for (Location location : locations) {
            Address address = null;
            if (type.equals(XMPP_ADDRESS) || type.equals(XMPP_SECURE_ADDRESS)) {
                address = new Address(type, location.getAddress());
            } else if (type.equals(XMPP_FEDERATION_ADDRESS)) {
                address = new Address(XMPP_FEDERATION_ADDRESS, location.getAddress(), settings.getXmppFederationPort());
            } else if (type.equals(XMLRPC_ADDRESS)) {
                address = new Address(XMLRPC_ADDRESS, location.getAddress(), settings.getXmlRpcPort());
            } else if (type.equals(XMLRPC_VCARD_ADDRESS)) {
                address = new Address(XMLRPC_VCARD_ADDRESS, location.getAddress(), settings.getXmlRpcVcardPort());
            } else if (type.equals(WATCHER_ADDRESS)) {
                address = new Address(WATCHER_ADDRESS, location.getAddress(), settings.getWatcherPort());
            } else if (type.equals(XMPP_FILE_TRANSFER_PROXY_ADDRESS)) {
                address = new Address(XMPP_FILE_TRANSFER_PROXY_ADDRESS, location.getAddress(), settings.getFileTransferProxyPort());
            } else if (type.equals(XMPP_ADMIN_CONSOLE_ADDRESS)) {
                address = new Address(XMPP_ADMIN_CONSOLE_ADDRESS, location.getAddress(), XMPP_ADMIN_CONSOLE_ADDRESS.getCanonicalPort());
            }
            addresses.add(address);
        }

        return addresses;
    }

    public void setSettingsDao(BeanWithSettingsDao<OpenfireSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    /**
     * Setting this to true just relaxes the validator, Stock openfire will not work in HA mode
     */
    public void setHighAvailabilitySupport(boolean highAvailabilitySupport) {
        m_highAvailabilitySupport = highAvailabilitySupport;
    }

    @Override
	public void touchXmppUpdate(Collection<Location> locations) {
        RunRequest touchXmppUpdate = new RunRequest("touch xmpp update", locations);
        touchXmppUpdate.setBundles("touch_xmpp_update");
        m_configManager.run(touchXmppUpdate);
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.IM) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        if (!manager.getFeatureManager().isFeatureEnabled(FEATURE, location)) {
            return null;
        }
        return Collections.singleton(ProcessDefinition.sysvByRegex("openfire",
                ".*\\s-Dprovider.properties.className=org.jivesoftware.util.FilePropertiesProvider\\s.*"));
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        // cluster-only by default
        List<DefaultFirewallRule> rules = DefaultFirewallRule.rules(Arrays.asList(XMLRPC_ADDRESS, WATCHER_ADDRESS, XMPP_FILE_TRANSFER_PROXY_ADDRESS, XMPP_ADMIN_CONSOLE_ADDRESS));
        // public by default
        rules.add(new DefaultFirewallRule(XMPP_ADDRESS, FirewallRule.SystemId.PUBLIC));
        rules.add(new DefaultFirewallRule(XMPP_SECURE_ADDRESS, FirewallRule.SystemId.PUBLIC));
        rules.add(new DefaultFirewallRule(XMPP_FEDERATION_ADDRESS, FirewallRule.SystemId.PUBLIC));
        return rules;
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        if (!m_highAvailabilitySupport) {
            validator.singleLocationOnly(FEATURE);
        }
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        if (request.hasChanged(ImManager.FEATURE)) {
            m_configManager.configureEverywhere(Rls.FEATURE);
        }
    }

    @Override
    public boolean isPresenceEnabled() {
        return getSettings().isPresenceEnabled();
    }

    @Required
    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }
}
