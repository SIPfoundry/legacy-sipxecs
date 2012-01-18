/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.ivr;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;

public class IvrImpl implements FeatureProvider, AddressProvider, Ivr {
    private BeanWithSettingsDao<IvrSettings> m_settingsDao;
    private BeanWithSettingsDao<CallPilotSettings> m_pilotSettingsDao;

    public IvrSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
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
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return Collections.singleton(CALLPILOT);
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return Collections.singleton(FEATURE);
    }

    public void setSettingsDao(BeanWithSettingsDao<IvrSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    public void setCallPilotSettingsDao(BeanWithSettingsDao<CallPilotSettings> settingsDao) {
        m_pilotSettingsDao = settingsDao;
    }

    @Override
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Object requester) {
        if (type.equals(REST_API)) {
            IvrSettings settings = getSettings();
            List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
            List<Address> addresses = new ArrayList<Address>(locations.size());
            for (Location location : locations) {
                Address address = new Address();
                address.setAddress(location.getAddress());
                if (type.equals(REST_API)) {
                    address.setPort(settings.getHttpsPort());
                    address.setFormat("https://%s:%d");
                }
                addresses.add(address);
            }
            return addresses;
        }
        return null;
    }

    public void setPilotSettingsDao(BeanWithSettingsDao<CallPilotSettings> pilotSettingsDao) {
        m_pilotSettingsDao = pilotSettingsDao;
    }

}
