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

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.setting.type.SettingType;

/**
 * User labeled storage of settings.
 *
 * @author dhubler
 *
 */
public class Group extends ValueStorage implements Comparable, NamedObject {
    private String m_name;
    private String m_description;
    private String m_resource;
    private Integer m_weight;
    private Branch m_branch;

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

    public Branch getBranch() {
        return m_branch;
    }

    public void setBranch(Branch branch) {
        m_branch = branch;
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
        Setting beanSettings = bean.getSettings();
        // base settings for the group cannot have any model
        Setting baseSettings = beanSettings.copy();
        baseSettings.acceptVisitor(new BeanWithSettingsModel.SetModelReference(null));

        // settings have model constructed with baseSettings
        Setting settings = beanSettings.copy();
        SettingModel model = new GroupSettingModel(this, baseSettings);
        settings.acceptVisitor(new BeanWithSettingsModel.SetModelReference(model));
        return settings;
    }

    /**
     * Calculate a typed value of a setting.
     *
     * Beans can easily calculate typed value of a setting since they have access to setting
     * models. Groups only have access to raw value of a setting kept in the database.
     *
     * @param type setting type
     * @param setting path to the setting
     * @return converted value of the setting
     */
    public Object getSettingTypedValue(SettingType type, String setting) {
        String settingValue = getSettingValue(setting);
        return type.convertToTypedValue(settingValue);
    }

    /**
     * Delegate all functions with the exception of setSettingValue to base settings.
     */
    static class GroupSettingModel implements SettingModel {
        private final Group m_group;
        private final Setting m_baseSetting;

        public GroupSettingModel(Group group, Setting baseSetting) {
            m_group = group;
            m_baseSetting = baseSetting;
        }

        public void setSettingValue(Setting setting, String value) {
            m_group.setSettingValue(setting, new SettingValueImpl(value), getDefaultSettingValue(setting));
        }

        public SettingValue getSettingValue(Setting setting) {
            SettingValue value = m_group.getSettingValue(setting);
            if (value != null) {
                return value;
            }
            return getDefaultSettingValue(setting);
        }

        public SettingValue getDefaultSettingValue(Setting setting) {
            Setting baseSetting = m_baseSetting.getSetting(setting.getPath());
            return new SettingValueImpl(baseSetting.getDefaultValue());
        }

        public SettingValue getProfileName(Setting setting) {
            Setting baseSetting = m_baseSetting.getSetting(setting.getPath());
            return new SettingValueImpl(baseSetting.getProfileName());
        }
    }

    /**
     * Returns the group with the highest weight from the list. Returns null if the list of groups
     * is empty
     *
     * @param groups A list of groups
     * @return The group with the highest weight
     */
    public static Group selectGroupWithHighestWeight(List<Group> groups) {
        if (groups.size() == 0) {
            return null;
        }

        List<Group> localGroupList = new ArrayList<Group>(groups.size());
        localGroupList.addAll(groups);
        Collections.<Group> sort(localGroupList);

        return localGroupList.get(0);
    }
}
