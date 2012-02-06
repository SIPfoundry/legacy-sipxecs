/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
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
        @SettingEntry(paths = { "services/syslog", "services/ntp/0", "services/dns/0" })
        public String getPrimaryServer() {
            return m_locationsManager.getPrimaryLocation().getAddress();
        }
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("unmanaged/services.xml");
    }

    public String getNtpServer() {
        return getSettingValue("services/ntp/0");
    }

    public String getSyslogServer() {
        return getSettingValue("services/syslog");
    }

    public List<Address> getAddresses(String setting) {
        return SettingUtil.getAddresses(getSettings(), setting);
    }

    @Override
    public String getBeanId() {
        return "unmanagedServiceSettings";
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
