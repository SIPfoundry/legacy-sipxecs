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
import java.util.HashSet;
import java.util.Set;

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
        String arrayName = array.getProfileName();

        if (AudioCodesGateway.PARAMETER_TABLE_SETTINGS.contains(arrayName)) {
            array.removeDuplicates();
            m_flat.add(array);
        } else {
            // Scan the array and add settings (non duplicate) to the flattened collection.
            for (String settingName : array.getSettingNames()) {
                Set<Object> values = new HashSet<Object>();
                for (int i = 0; i < array.getSize(); i++) {
                    Object elName = array.getSetting(i, settingName).getTypedValue();
                    if (!values.contains(elName)) {
                        // Non duplicate. Add to flattened collection.
                        values.add(elName);
                        m_flat.add(array.getSetting(i, settingName));
                    }
                }
            }
        }
        // returning false indicates we've processed the array.
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
