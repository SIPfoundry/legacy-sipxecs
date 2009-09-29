/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.Map;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;

/**
 * Provides special equality testing for SettingMaps.
 *
 * It is intended to eliminate duplicate name=value settings from a set or SettingArray.
 * Overriding equals() and hashCode() in AbstractSetting itself causes problems with the unit
 * test.
 *
 * To use simply wrap each SettingMap that should not have any equivalent settings in an
 * EquivalentSettingMapWrapper, and add the wrapper to your list, set, or other data structure.
 *
 * The main point is that class determines equality by looking at the setting name, type, path,
 * and String value.
 */
public class EquivalentSettingMapWrapper {

    private SettingMap m_map;

    public EquivalentSettingMapWrapper(final SettingMap map) {
        m_map = map;
    }

    public SettingMap getMap() {
        return m_map;
    }

    public boolean equals(Object o) {
        if (!(o instanceof EquivalentSettingMapWrapper)) {
            return false;
        }

        EquivalentSettingMapWrapper wrapper = (EquivalentSettingMapWrapper) o;
        SettingMap map = wrapper.getMap();
        SettingMap ap = getMap();

        if ((ap != map) && (ap.size() != map.size())) {
            return false;
        }

        for (Map.Entry<String, Setting> setting : ap.entrySet()) {
            String key = setting.getKey();
            Setting value = setting.getValue();
            if (!map.containsKey(key) || !equal(value, map.get(key))) {
                return false;
            }
        }
        return true;
    }

    public int hashCode() {
        int h = 0;
        for (Map.Entry<String, Setting> s : m_map.entrySet()) {
            h += settingHashCode(s.getValue());
        }
        return h;
    }

    private int settingHashCode(Setting s) {
        HashCodeBuilder builder = new HashCodeBuilder();
        builder.append(s.getType());
        builder.append(s.getName());
        builder.append(s.getValue());
        return builder.toHashCode();
    }

    private boolean equal(Setting s1, Setting s2) {
        EqualsBuilder builder = new EqualsBuilder();
        builder.append(s1.getType(), s2.getType());
        builder.append(s1.getName(), s2.getName());
        builder.append(s1.getValue(), s2.getValue());
        return builder.isEquals();
    }
}
