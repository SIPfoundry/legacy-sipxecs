/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;

/**
 * Provides special equality testing for SettingMaps. This is intended to eliminate duplicate
 * name=value settings from a set or SettingArray. Overriding equals() and hashCode() in
 * AbstractSetting itself causes problems with the unit test.
 * 
 * To use this, simply wrap each SettingMap that should not have any equivalent settings in an
 * EquivalentSettingMapWrapper, and add the wrapper to your list, set, or other data structure.
 * 
 * The main point is that this class determines equality by looking at the setting name, type,
 * path, and String value.
 */
public class EquivalentSettingMapWrapper {

    private SettingMap m_map;

    public EquivalentSettingMapWrapper(final SettingMap map) {
        this.m_map = map;
    }

    public SettingMap getMap() {
        return m_map;
    }

    public boolean equals(Object o) {
        boolean isEqual = true;

        if (!(o instanceof EquivalentSettingMapWrapper)) {
            isEqual = false;
        } else {
            EquivalentSettingMapWrapper wrapper = (EquivalentSettingMapWrapper) o;
            SettingMap map = wrapper.getMap();
            SettingMap thisMap = getMap();

            if ((thisMap != map) && (thisMap.size() != map.size())) {
                isEqual = false;
            } else {
                Iterator<Map.Entry<String, Setting>> settings = thisMap.entrySet().iterator();
                while (isEqual && settings.hasNext()) {
                    Map.Entry<String, Setting> setting = settings.next();
                    if (!map.containsKey(setting.getKey()) || !equal(setting.getValue(), map.get(setting.getKey()))) {
                        isEqual = false;
                    }
                }
            }
        }
        return isEqual;
    }

    public int hashCode() {
        int h = 0;
        Iterator<Entry<String, Setting>> i = m_map.entrySet().iterator();
        while (i.hasNext()) {
            h += settingHashCode(i.next().getValue());
        }
        return h;
    }

    private int settingHashCode(Setting s) {
        HashCodeBuilder builder = new HashCodeBuilder();
        builder.append(s.getType());
        builder.append(s.getName());
        builder.append(s.getValue());
        builder.append(s.getParent());
        return builder.toHashCode();
    }

    private boolean equal(Setting s1, Setting s2) {
        EqualsBuilder builder = new EqualsBuilder();
        builder.append(s1.getType(), s2.getType());
        builder.append(s1.getName(), s2.getName());
        builder.append(s1.getValue(), s2.getValue());
        builder.append(s1.getParent(), s2.getParent());
        return builder.isEquals();
    }
}
