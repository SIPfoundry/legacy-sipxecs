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
package org.sipfoundry.sipxconfig.service;

import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.setting.SettingUtil;

public class UnmanagedServiceSettings extends PersistableSettings {
    private LocationsManager m_locationsManager;

    public UnmanagedServiceSettings() {
        addDefaultBeanSettingHandler(new Defaults());
    }

    public class Defaults {
        @SettingEntry(paths = { "services/syslog", "services/dns/0" })
        public String getPrimaryServer() {
            return m_locationsManager.getPrimaryLocation().getAddress();
        }
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("unmanaged/services.xml");
    }

    public String getSyslogServer() {
        return getSettingValue("services/syslog");
    }

    public List<Address> getAddresses(AddressType t, String setting) {
        return SettingUtil.getAddresses(t, getSettings(), setting);
    }

    @Override
    public String getBeanId() {
        return "unmanagedServiceSettings";
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
