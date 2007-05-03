/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import java.util.ArrayList;
import java.util.Collection;

import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingArray;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.setting.SettingVisitor;

class SettingsIron implements SettingVisitor {
    public static final String IGNORE = "$$";

    private Collection<Setting> m_flat = new ArrayList<Setting>();

    public Collection<Setting> getFlat() {
        return m_flat;
    }

    public void visitSetting(Setting setting) {
        if (!IGNORE.equals(setting.getProfileName())) {
            m_flat.add(setting);
        }
    }

    public boolean visitSettingArray(SettingArray array) {
        // skip arrays
        return false;
    }

    public boolean visitSettingGroup(SettingSet group) {
        // skip empty groups
        if (group.isLeaf()) {
            return false;
        }
        if (group.getParent() != null) {
            visitSetting(group);
        }
        return true;
    }
}
