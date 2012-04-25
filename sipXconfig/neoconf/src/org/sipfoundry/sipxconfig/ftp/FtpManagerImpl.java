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

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.BundleConstraint;
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

public class FtpManagerImpl extends SipxHibernateDaoSupport<Object> implements FtpManager, ProcessProvider,
    SetupListener, FeatureProvider, FirewallProvider, AddressProvider {
    private static final List<AddressType> ADDRESSES = Arrays.asList(TFTP_ADDRESS, FTP_ADDRESS, FTP_DATA_ADDRESS);
    private static final List<LocationFeature> FEATURES = Arrays.asList(TFTP_FEATURE, FTP_FEATURE);
    private BeanWithSettingsDao<FtpSettings> m_settingsDao;

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        return (location.isPrimary() ? Collections.singleton(new ProcessDefinition("vsftpd")) : null);
    }

    @Override
    public void setup(SetupManager manager) {
        for (LocationFeature f : FEATURES) {
            if (!manager.isSetup(f.getId())) {
                Location primary = manager.getConfigManager().getLocationManager().getPrimaryLocation();
                manager.getFeatureManager().enableLocationFeature(f, primary, true);
                manager.setSetup(f.getId());
            }
        }
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return FEATURES;
    }

    @Override
    public void getBundleFeatures(Bundle b) {
        if (b.isBasic()) {
            // Primary only because of local manipulation of files by sipxconfig
            for (LocationFeature f : FEATURES) {
                b.addFeature(f, BundleConstraint.PRIMARY_ONLY);
            }
        }
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return DefaultFirewallRule.rules(Arrays.asList(FTP_ADDRESS, FTP_DATA_ADDRESS, TFTP_ADDRESS),
                FirewallRule.SystemId.PUBLIC);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        if (!ADDRESSES.contains(type)) {
            return null;
        }

        LocationFeature f = (type.equals(FTP_ADDRESS) ? FTP_FEATURE : TFTP_FEATURE);
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(f);
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
}
