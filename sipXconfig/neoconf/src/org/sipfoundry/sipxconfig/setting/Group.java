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

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.DataCollectionItem;
import org.sipfoundry.sipxconfig.common.NamedObject;

/**
 * User labeled storage of settings.
 * 
 * @author dhubler
 * 
 */
public class Group extends ValueStorage implements Comparable, DataCollectionItem, NamedObject {
    private String m_name;
    private String m_description;
    private String m_resource;
    private Integer m_weight;

    public String getName() {
        return m_name;
    }

    public void setName(String label) {
        m_name = label;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getResource() {
        return m_resource;
    }

    public void setResource(String resource) {
        m_resource = resource;
    }

    /**
     * When setting values conflict, the setting with the highest weight wins.
     * 
     * @return setting weight
     */
    public Integer getWeight() {
        return m_weight;
    }

    public void setWeight(Integer weight) {
        m_weight = weight;
    }

    public int compareTo(Object arg0) {
        Group b = (Group) arg0;
        int w1 = defaultWeight(m_weight);
        int w2 = defaultWeight(b.getWeight());
        int cmp = w1 - w2;
        if (cmp != 0) {
            return cmp;
        }
        String s1 = StringUtils.defaultString(getName());
        String s2 = StringUtils.defaultString(b.getName());
        return s1.compareTo(s2);
    }

    private int defaultWeight(Integer weight) {
        return weight != null ? weight : -1;
    }

    /**
     * byproduct of DataCollectionItem interface, returns weight - 1
     */
    public int getPosition() {
        int w = (m_weight != null ? m_weight.intValue() : -1);
        return w - 1;
    }

    /**
     * byproduct of DataCollectionItem interface, sets weight to position + 1
     */
    public void setPosition(int position) {
        m_weight = new Integer(position + 1);
    }

    /**
     * Creates a copy of settings that can be used to edit settings for this group.
     * 
     * We use the same group object for all types of groups (phone, lines, attendants etc.).
     * Because of that group does not know which settings it supports. Use this function to pass
     * settings that will become a base for this group.
     * 
     * @param beanSettings settings to inherit from, no model can be set for those settings
     * @return copy of settings to be edited
     */
    public Setting inherhitSettingsForEditing(BeanWithSettings bean) {
        // base settings for the group cannot have any model
        Setting baseSettings = bean.getSettings().copy();
        baseSettings.acceptVisitor(new BeanWithSettingsModel.SetModelReference(null));
        
        // settings have model constructed with baseSettings
        Setting settings = bean.getSettings().copy();
        SettingModel model = new GroupSettingModel(this, bean);
        settings.acceptVisitor(new BeanWithSettingsModel.SetModelReference(model));
        return settings;
    }

    /**
     * Delegate all functions with the exception of setSettingValue to base settings.
     */
    static class GroupSettingModel implements SettingModel {
        private Group m_group;
        private BeanWithSettings m_bean;

        public GroupSettingModel(Group group, BeanWithSettings bean) {
            m_group = group;
            m_bean = bean;
        }

        public void setSettingValue(Setting setting, String value) {
            m_group.setSettingValue(setting, new SettingValueImpl(value),
                    getDefaultSettingValue(setting));
        }

        public SettingValue getSettingValue(Setting setting) {
            SettingValue value = m_group.getSettingValue(setting);
            if (value != null) {
                return value;
            }
            return getDefaultSettingValue(setting);
        }

        public SettingValue getDefaultSettingValue(Setting setting) {
            String settingValue = m_bean.getSettingValue(setting.getPath());
            return new SettingValueImpl(settingValue);
        }

        public SettingValue getProfileName(Setting setting) {
            String profileName = m_bean.getSettings().getSetting(setting.getPath()).getProfileName();
            return new SettingValueImpl(profileName);
        }
    }
}
