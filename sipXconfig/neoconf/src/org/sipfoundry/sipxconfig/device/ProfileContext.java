/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import org.apache.velocity.tools.generic.MathTool;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.setting.SettingUtil;

/**
 * State used by profile generator
 */
public class ProfileContext<T extends Device> {
    /**
     * Shows all settings and groups in a flat collection
     */
    private static final SettingFilter RECURSIVE_SETTINGS = new SettingFilter() {
        public boolean acceptSetting(Setting root_, Setting setting) {
            boolean group = SettingSet.class.isAssignableFrom(setting.getClass());
            return !group;
        }
    };

    private static final SettingFilter SETTINGS = new SettingFilter() {
        public boolean acceptSetting(Setting root, Setting setting) {
            boolean firstGeneration = (setting.getParent() == root);
            boolean isLeaf = setting.getValues().isEmpty();
            return firstGeneration && isLeaf;
        }
    };

    private T m_device;

    private String m_profileTemplate;

    public ProfileContext(T device, String profileTemplate) {
        m_device = device;
        m_profileTemplate = profileTemplate;
    }

    public Map<String, Object> getContext() {
        HashMap<String, Object> context = new HashMap<String, Object>();
        context.put("phone", m_device);
        context.put("gateway", m_device);
        context.put("sbc", m_device);
        context.put("cfg", this);
        context.put("math", new MathTool());
        return context;
    }

    public T getDevice() {
        return m_device;
    }

    public String getProfileTemplate() {
        return m_profileTemplate;
    }

    /**
     * Velocity macro convenience method. Recursive list of all settings, ignoring groups
     */
    public Collection getRecursiveSettings(Setting group) {
        return SettingUtil.filter(RECURSIVE_SETTINGS, group);
    }

    public Collection getSettings(Setting group) {
        return SettingUtil.filter(SETTINGS, group);
    }

    /**
     * Velocity macro convenience method for accessing end point settings
     */
    public Setting getEndpointSettings() {
        return m_device.getSettings();
    }
}
