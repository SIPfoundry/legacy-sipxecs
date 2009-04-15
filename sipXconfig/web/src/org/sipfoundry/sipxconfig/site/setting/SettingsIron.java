/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.setting;

import java.util.ArrayList;
import java.util.Collection;

import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingArray;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.setting.SettingVisitor;

/**
 * Strips hidden settings, checks for advanced flag. Please note that it's not a good idea to
 * strip advanced settings here. We need to render advanced settings as hidden components on
 * the page if user modifies them and then switches between 'advanced' and 'normal' views.
 */
public class SettingsIron implements SettingVisitor {

    private Collection<Setting> m_flat = new ArrayList<Setting>();
    private boolean m_isAdvanced;

    public Collection<Setting> getFlat() {
        return m_flat;
    }

    public boolean isAdvanced() {
        return m_isAdvanced;
    }

    public void visitSetting(Setting setting) {
        if (setting.isHidden()) {
            return;
        }
        m_isAdvanced = m_isAdvanced || setting.isAdvanced();
        m_flat.add(setting);
    }

    public boolean visitSettingArray(SettingArray array) {
        visitSetting(array);
        // do not flatten array members
        return false;
    }

    public boolean visitSettingGroup(SettingSet group) {
        if (group.isHidden()) {
            return false;
        }
        visitSetting(group);
        return true;
    }

}
