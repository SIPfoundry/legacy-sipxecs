/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.setting.SettingSet;

public class UnmanagedServiceSettings extends PersistableSettings {
    private LocationsManager m_locationsManager;

    public UnmanagedServiceSettings() {
        addDefaultBeanSettingHandler(new Defaults());
    }

    class Defaults {
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
        List<Address> addresses = Collections.emptyList();
        Setting s = getSettings().getSetting(setting);
        if (s instanceof SettingSet) {
            SettingSet set = (SettingSet) getSettings().getSetting(setting);
            Collection<Setting> values = set.getValues();
            addresses = new ArrayList<Address>();
            for (Setting server : values) {
                String value = server.getValue();
                if (value != null) {
                    addresses.add(new Address(value));
                }
            }
        } else {
            String value = s.getValue();
            if (value != null) {
                addresses = Collections.singletonList(new Address(value));
            }
        }

        return addresses;
    }

    @Override
    public String getBeanId() {
        return "unmanagedServiceSettings";
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
