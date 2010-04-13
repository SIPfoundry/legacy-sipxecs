/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.permission;

import java.util.Locale;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.PrimaryKeySource;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.type.BooleanSetting;

/**
 * Permission Copy of permission setting names exist in user-setting.xml
 */
public class Permission implements Comparable<Permission>, PrimaryKeySource {
    public enum Type {
        APPLICATION(APPLICATION_NAME, APPLICATION_PATH), // application permissions
        CALL(CALLHANDLING_NAME, CALL_PERMISSION_PATH), // call permissions
        VOICEMAIL_SERVER(VOICEMAIL_NAME, VOICEMAIL_SERVER_PATH); // used to select vm server

        private String m_name;
        private String m_path;

        Type(String name, String path) {
            m_name = name;
            m_path = path;
        }

        public String getName() {
            return m_name;
        }

        public String getPath() {
            return m_path;
        }
    }

    public static final String ENABLE = "ENABLE";
    public static final String DISABLE = "DISABLE";

    public static final String APPLICATION_NAME = "application";
    public static final String CALLHANDLING_NAME = "call-handling";
    public static final String VOICEMAIL_NAME = "voicemail-server";

    public static final String PATH_PREFIX = "permission/";
    public static final String APPLICATION_PATH = PATH_PREFIX + APPLICATION_NAME;
    public static final String CALL_PERMISSION_PATH = PATH_PREFIX + CALLHANDLING_NAME;
    public static final String VOICEMAIL_SERVER_PATH = PATH_PREFIX + VOICEMAIL_NAME;

    private Type m_type = Type.CALL;

    private String m_label;

    private String m_description;

    private boolean m_defaultValue;

    private Integer m_id = BeanWithId.UNSAVED_ID;

    public Permission() {
    }

    void setId(Integer id) {
        m_id = id;
    }

    public Integer getId() {
        return m_id;
    }

    public static boolean isEnabled(String value) {
        return (Boolean) PermissionSetting.INSTANCE.convertToTypedValue(value);
    }

    public boolean isBuiltIn() {
        return false;
    }

    /**
     * Returns the path in user settings to this permission group or setting
     */
    public String getSettingPath() {
        return getSettingsPath(m_type, getName());
    }

    public static String getSettingsPath(Type type, String name) {
        StringBuilder sb = new StringBuilder(PATH_PREFIX);
        sb.append(type.getName()).append(Setting.PATH_DELIM);
        sb.append(name);

        return sb.toString();
    }

    public void setLabel(String label) {
        m_label = label;
    }

    public String getLabel() {
        return m_label;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDefaultValue(boolean defaultValue) {
        m_defaultValue = defaultValue;
    }

    public boolean getDefaultValue() {
        return m_defaultValue;
    }

    public String getLabel(@SuppressWarnings("unused") Locale locale) {
        return getLabel();
    }

    public String getDescription(@SuppressWarnings("unused") Locale locale) {
        return getDescription();
    }

    public String getName() {
        return "perm_" + getId();
    }

    public Object getPrimaryKey() {
        return m_id;
    }

    public Type getType() {
        return m_type;
    }

    public void setType(Type type) {
        m_type = type;
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof Permission)) {
            return false;
        }
        if (this == obj) {
            return true;
        }
        Permission rhs = (Permission) obj;
        // only compare name and type - make sure that you compare names using getName (and not
        // m_name)
        return new EqualsBuilder().append(m_type, rhs.m_type).append(getName(), rhs.getName()).isEquals();
    }

    @Override
    public int hashCode() {
        return new HashCodeBuilder().append(getName()).append(m_type).hashCode();
    }

    public int compareTo(Permission o) {
        return getName().compareTo(o.getName());
    }

    /**
     * Create setting that corresponds to this permission
     *
     * @return newly created setting
     */
    Setting getSetting() {
        SettingImpl setting = new SettingImpl(getName());
        setting.setType(PermissionSetting.INSTANCE);
        setting.setDescription(m_description);
        setting.setLabel(m_label);
        setting.setTypedValue(m_defaultValue);
        return setting;
    }

    private static class PermissionSetting extends BooleanSetting {
        public static final PermissionSetting INSTANCE = new PermissionSetting();

        public PermissionSetting() {
            setTrueValue(ENABLE);
            setFalseValue(DISABLE);
        }
    }
}
