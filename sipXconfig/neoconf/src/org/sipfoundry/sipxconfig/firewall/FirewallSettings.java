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
package org.sipfoundry.sipxconfig.firewall;


import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class FirewallSettings extends PersistableSettings implements DeployConfigOnEdit {
    private boolean m_unmanagedDefault;

    public FirewallSettings() {
        addDefaultBeanSettingHandler(new Defaults());
    }

    public class Defaults {
        @SettingEntry(path = "sys/unmanaged")
        public boolean unmanaged() {
            return m_unmanagedDefault;
        }
    }

    @Override
    public String getBeanId() {
        return "firewallSettings";
    }

    public Setting getSystemSettings() {
        return getSettings().getSetting("sys");
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("firewall/firewall.xml");
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) FirewallManager.FEATURE);
    }

    public void setUnmanagedDefault(boolean unmanagedDefault) {
        m_unmanagedDefault = unmanagedDefault;
    }
}
