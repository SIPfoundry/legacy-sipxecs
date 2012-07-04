/**
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
package org.sipfoundry.sipxconfig.dhcp;


import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class DhcpSettings extends PersistableSettings implements DeployConfigOnEdit {
    private static final String LAST_IPV4_SEGMENT = "\\.\\d+$";
    private static final String UNMANAGED_SETTING = "dhcpd-unmanaged/unmanaged";
    private static final String UNMANAGED_SERVER = "dhcpd-unmanaged/dhcpd-server";
    private LocationsManager m_locationsManager;
    private Location m_primary;

    public DhcpSettings() {
        addDefaultBeanSettingHandler(new Defaults());
    }

    public class Defaults {
        @SettingEntry(path = "dhcpd-config/subnet")
        public String getSubnet() {
            return getPrimary().getAddress().replaceFirst(LAST_IPV4_SEGMENT, ".0");
        }
        @SettingEntry(path = "dhcpd-config/range_begin")
        public String getBeginRange() {
            return getPrimary().getAddress().replaceFirst(LAST_IPV4_SEGMENT, ".50");
        }
        @SettingEntry(path = "dhcpd-config/range_end")
        public String getEndRange() {
            return getPrimary().getAddress().replaceFirst(LAST_IPV4_SEGMENT, ".250");
        }
    }

    Location getPrimary() {
        if (m_primary == null) {
            m_primary = m_locationsManager.getPrimaryLocation();
        }
        return m_primary;
    }

    void setPrimary(Location primary) {
        m_primary = primary;
    }

    @Override
    public String getBeanId() {
        return "dhcpSettings";
    }

    public boolean isServiceUnmanaged() {
        return (Boolean) getSettingTypedValue(UNMANAGED_SETTING);
    }

    public String getUnmanagedDhcpServer() {
        return (String) getSettingTypedValue(UNMANAGED_SERVER);
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("dhcp/dhcp.xml");
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) DhcpManager.FEATURE);
    }
}
