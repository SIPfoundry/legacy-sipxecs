/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.ftp;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.BundleConstraint;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.firewall.CustomFirewallRule;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallCustomRuleProvider;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallRule;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public class FtpManagerImpl extends SipxHibernateDaoSupport<Object> implements FtpManager, ProcessProvider,
        SetupListener, FeatureProvider, FirewallCustomRuleProvider, AddressProvider {
    private static final List<AddressType> ADDRESSES = Arrays.asList(TFTP_ADDRESS, FTP_ADDRESS, FTP_DATA_ADDRESS,
            FTP_PASV_ADDRESS);
    private static final List<LocationFeature> FEATURES = Arrays.asList(TFTP_FEATURE, FTP_FEATURE);
    private BeanWithSettingsDao<FtpSettings> m_settingsDao;
    private FeatureManager m_featureManager;

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        // tftp service is managed thru xinetd
        if (!m_featureManager.isFeatureEnabled(FTP_FEATURE, location)) {
            return null;
        }
        return Collections.singleton(ProcessDefinition.sysv("vsftpd", true));
    }

    @Override
    public boolean setup(SetupManager manager) {
        for (LocationFeature f : FEATURES) {
            if (manager.isFalse(f.getId())) {
                Location primary = manager.getConfigManager().getLocationManager().getPrimaryLocation();
                manager.getFeatureManager().enableLocationFeature(f, primary, true);
                manager.setTrue(f.getId());
            }
        }
        return true;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return FEATURES;
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.PROVISION) {
            // Primary only because of local manipulation of files by sipxconfig
            for (LocationFeature f : FEATURES) {
                b.addFeature(f, BundleConstraint.PRIMARY_ONLY);
            }
        }
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return DefaultFirewallRule.rules(
                Arrays.asList(FTP_ADDRESS, FTP_DATA_ADDRESS, TFTP_ADDRESS, FTP_PASV_ADDRESS),
                FirewallRule.SystemId.PUBLIC);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        if (!ADDRESSES.contains(type)) {
            return null;
        }

        // TFTP addresses
        if (type.equals(TFTP_ADDRESS)) {
            List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(TFTP_FEATURE);
            return Location.toAddresses(type, locations);
        }

        // FTP addresses (ftp, data, pasv)
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FTP_FEATURE);
        if (type.equals(FTP_PASV_ADDRESS)) {
            List<Address> addresses = new ArrayList<Address>(locations.size());
            for (Location location : locations) {
                Address a = new Address(type, location.getAddress(), getSettings().getMinPasvPort());
                a.setEndPort(getSettings().getMaxPasvPort());
                addresses.add(a);
            }
            return addresses;
        }
        return Location.toAddresses(type, locations);
    }

    public void setSettingsDao(BeanWithSettingsDao<FtpSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public FtpSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(FtpSettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
    }

    @Override
    public Collection<CustomFirewallRule> getCustomRules(FirewallManager manager, Location location,
            Map<Object, Object> requestData) {
        return null;
    }

    @Override
    public Collection<String> getRequiredModules(FirewallManager manager, Location location,
            Map<Object, Object> requestData) {
        List<String> mods = new ArrayList<String>();
        if (m_featureManager.isFeatureEnabled(FTP_FEATURE, location)) {
            mods.add("ip_conntrack_ftp");
        }
        if (m_featureManager.isFeatureEnabled(TFTP_FEATURE, location)) {
            mods.add("ip_conntrack_tftp");
        }
        return mods;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }
}
