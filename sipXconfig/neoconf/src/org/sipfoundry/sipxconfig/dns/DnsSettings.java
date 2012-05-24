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
package org.sipfoundry.sipxconfig.dns;


import java.util.Collection;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingUtil;

public class DnsSettings extends PersistableSettings implements DeployConfigOnEdit {
    private static final String UNMANAGED_SETTING = "named-unmanaged/unmanaged";
    private static final String DNS_UNMANAGED_SERVER_0 = "named-unmanaged/0";
    private static final String DNS_UNMANAGED_SERVER_1 = "named-unmanaged/1";
    private static final String DNS_UNMANAGED_SERVER_2 = "named-unmanaged/2";
    private static final String DNS_UNMANAGED_SERVER_3 = "named-unmanaged/3";

    @Override
    public String getBeanId() {
        return "dnsSettings";
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxdns/sipxdns.xml");
    }

    public List<Address> getDnsForwarders() {
        return SettingUtil.getAddresses(DnsManager.DNS_ADDRESS, getSettings(), "named-config/dnsForwarders");
    }

    public boolean isServiceUnmanaged() {
        return (Boolean) getSettingTypedValue(UNMANAGED_SETTING);
    }

    public List<String> getUnmanagedDnsServers() {
        List<String> servers = new LinkedList<String>();
        if (isServiceUnmanaged()) {
            addDnsServer(DNS_UNMANAGED_SERVER_0, servers);
            addDnsServer(DNS_UNMANAGED_SERVER_1, servers);
            addDnsServer(DNS_UNMANAGED_SERVER_2, servers);
            addDnsServer(DNS_UNMANAGED_SERVER_3, servers);
        }
        return servers;
    }

    private void addDnsServer(String setting, List<String> servers) {
        String server = getSettingValue(setting);
        if (StringUtils.isNotBlank(server)) {
            servers.add(server);
        }
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) DnsManager.FEATURE);
    }
}
