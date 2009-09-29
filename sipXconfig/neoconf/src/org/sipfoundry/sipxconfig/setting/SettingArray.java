/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.Collection;
import java.util.LinkedHashSet;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Container for an array of setting groups: support for indexing.
 */
public class SettingArray extends AbstractSetting {
    private static final Pattern INDEX_RE = Pattern.compile("(.+)\\[(\\d+)\\]");

    private SettingMap[] m_elements;

    public SettingArray() {
    }

    public SettingArray(String name, int size) {
        super(name);
        setSize(size);
    }

    public void setSize(int size) {
        m_elements = new SettingMap[size];
        for (int i = 0; i < m_elements.length; i++) {
            m_elements[i] = new SettingMap();
        }
    }

    public int getSize() {
        if (m_elements == null) {
            return 0;
        }
        return m_elements.length;
    }

    /**
     * @return names of the settings that compose
     */
    public String[] getSettingNames() {
        return m_elements[0].keySet().toArray(new String[0]);
    }

    public String getDefaultValue() {
        throw new UnsupportedOperationException();
    }

    public Collection<Setting> getValues() {
        throw new UnsupportedOperationException();
    }

    public Setting addSetting(Setting setting) {
        Setting s = setting;
        for (int i = 0; i < m_elements.length; i++) {
            if (i > 0) {
                s = setting.copy();
            }
            s.setParent(this);
            s.setIndex(i);
            m_elements[i].addSetting(s);
        }
        return setting;
    }

    /**
     * Please note that it does not visit this, the rest of the world does not really know
     * anything about it
     */
    @Override
    public void acceptVisitor(SettingVisitor visitor) {
        if (visitor.visitSettingArray(this)) {
            for (SettingMap setting : m_elements) {
                setting.acceptVisitor(visitor);
            }
        }
    }

    @Override
    protected Setting findChild(String name) {
        Matcher matcher = INDEX_RE.matcher(name);
        if (!matcher.matches()) {
            return null;
        }
        String childName = matcher.group(1);
        int childIndex = Integer.parseInt(matcher.group(2));

        return getSetting(childIndex, childName);
    }

    public Setting getSetting(int index, String column) {
        return m_elements[index].get(column);
    }

    public boolean isLeaf() {
        return false;
    }

    public SettingArray copy() {
        SettingArray copy = (SettingArray) super.copy();
        copy.m_elements = new SettingMap[m_elements.length];
        for (int i = 0; i < m_elements.length; i++) {
            SettingMap sm = m_elements[i];
            copy.m_elements[i] = sm.copy();
        }
        return copy;
    }

    public void removeDuplicates() {
        Set<EquivalentSettingMapWrapper> uniqueSettingMaps = new LinkedHashSet<EquivalentSettingMapWrapper>();

        for (SettingMap settingMap : m_elements) {
            uniqueSettingMaps.add(new EquivalentSettingMapWrapper(settingMap));
        }

        m_elements = new SettingMap[uniqueSettingMaps.size()];
        int index = 0;
        for (EquivalentSettingMapWrapper wrapper : uniqueSettingMaps) {
            m_elements[index++] = wrapper.getMap();
        }
    }
}
