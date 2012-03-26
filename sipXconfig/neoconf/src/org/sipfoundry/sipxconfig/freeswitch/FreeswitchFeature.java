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
package org.sipfoundry.sipxconfig.freeswitch;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.SettingsWithLocationDao;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public class FreeswitchFeature implements FeatureProvider, AddressProvider, ProcessProvider {
    public static final LocationFeature FEATURE = new LocationFeature("freeSwitch");
    public static final AddressType SIP_ADDRESS = AddressType.sip("freeswitch-sip");
    public static final AddressType XMLRPC_ADDRESS = new AddressType("freeswitch-xmlrpc", "http://%s:%d/RPC2");
    public static final AddressType EVENT_ADDRESS = new AddressType("freeswitch-event");
    public static final AddressType ACC_EVENT_ADDRESS = new AddressType("acc-freeswitch-event");
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(SIP_ADDRESS, XMLRPC_ADDRESS,
            EVENT_ADDRESS, ACC_EVENT_ADDRESS);

    private SettingsWithLocationDao<FreeswitchSettings> m_settingsDao;
    private String m_name = "freeswitch";

    public FreeswitchSettings getSettings(Location location) {
        return m_settingsDao.findOrCreate(location);
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return Collections.singleton(FEATURE);
    }

    @Override
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        return Collections.singleton(XMLRPC_ADDRESS);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Object requester) {
        if (!ADDRESSES.contains(type)) {
            return null;
        }

        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        if (locations.isEmpty()) {
            return null;
        }

        List<Address> addresses = new ArrayList<Address>();
        for (Location location : locations) {
            FreeswitchSettings settings = getSettings(location);
            Address address = null;
            if (type.equals(XMLRPC_ADDRESS)) {
                address = new Address(XMLRPC_ADDRESS, location.getAddress(), settings.getXmlRpcPort());
            } else if (type.equals(EVENT_ADDRESS)) {
                address = new Address(EVENT_ADDRESS, location.getAddress(), settings.getEventSocketPort());
            } else if (type.equals(ACC_EVENT_ADDRESS)) {
                address = new Address(ACC_EVENT_ADDRESS, location.getAddress(), settings.getAccEventSocketPort());
            } else if (type.equals(SIP_ADDRESS)) {
                address = new Address(SIP_ADDRESS, location.getAddress(), settings.getFreeswitchSipPort());
            }
            addresses.add(address);
        }

        return addresses;
    }

    public void setSettingsDao(SettingsWithLocationDao<FreeswitchSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE, location);
        return (enabled ? Collections.singleton(new ProcessDefinition(m_name)) : null);
    }

    @Override
    public void getBundleFeatures(Bundle b) {
        if (b.isBasic() || b.basedOn(OpenAcdContext.CALL_CENTER)) {
            b.addFeature(FEATURE);
        }
    }
}
