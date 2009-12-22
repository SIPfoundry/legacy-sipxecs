/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.site.common;

import java.util.ArrayList;
import java.util.List;

import org.apache.tapestry.IComponent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.LocalizationUtils;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.BooleanSetting;

/**
 * Creates option model from settings group. The group needs to contain boolean settings only.
 * Every option corresponds to a boolean setting.
 */
public class GroupBooleanModel implements IPropertySelectionModel {

    private final Setting m_settingSet;
    private final IComponent m_component;
    private final List<Setting> m_settings;

    public GroupBooleanModel(Setting settingSet, IComponent component) {
        m_settingSet = settingSet;
        m_component = component;
        m_settings = new ArrayList<Setting>();
        for (Setting s : m_settingSet.getValues()) {
            if (s.getType() instanceof BooleanSetting) {
                m_settings.add(s);
            }
        }
    }

    @Override
    public String getLabel(int index) {
        return LocalizationUtils.getSettingLabel(m_component, getSetting(index));
    }

    private Setting getSetting(int index) {
        return m_settings.get(index);
    }

    @Override
    public Object getOption(int index) {
        return getSetting(index);
    }

    @Override
    public int getOptionCount() {
        return m_settings.size();
    }

    @Override
    public String getValue(int index) {
        return getSetting(index).getName();
    }

    @Override
    public boolean isDisabled(int index) {
        return false;
    }

    @Override
    public Object translateValue(String value) {
        if (value == null) {
            return null;
        }
        return m_settingSet.getSetting(value);
    }
}
