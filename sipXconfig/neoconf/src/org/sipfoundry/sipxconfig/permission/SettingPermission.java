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

import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.context.MessageSource;

public class SettingPermission extends Permission {
    private MessageSource m_messageSource;

    private String m_name;

    private String m_labelKey;

    private String m_descriptionKey;

    private boolean m_builtIn;

    public SettingPermission(Setting setting) {
        initSettingPermission(setting);
    }

    public SettingPermission(Setting setting, boolean builtIn) {
        setBuiltIn(builtIn);
        initSettingPermission(setting);
    }

    public SettingPermission(Setting setting, Type type) {
        setType(type);
        initSettingPermission(setting);
    }

    public SettingPermission(Setting setting, Type type, boolean builtIn) {
        setType(type);
        setBuiltIn(builtIn);
        initSettingPermission(setting);
    }

    @SuppressWarnings("deprecation")
    private void initSettingPermission(Setting setting) {
        m_name = setting.getName();
        setLabel(setting.getLabel());
        setDescription(setting.getDescription());
        setDefaultValue(isEnabled(setting.getValue()));
        m_messageSource = setting.getMessageSource();
        m_labelKey = setting.getLabelKey();
        m_descriptionKey = setting.getDescriptionKey();
    }

    public void setBuiltIn(boolean builtIn) {
        m_builtIn = builtIn;
    }

    public boolean isBuiltIn() {
        return m_builtIn;
    }

    public Object getPrimaryKey() {
        return m_name;
    }

    public String getName() {
        return m_name;
    }

    public String getLabel(Locale locale) {
        if (m_messageSource == null) {
            return super.getLabel(locale);
        }
        return m_messageSource.getMessage(m_labelKey, null, getLabel(), locale);
    }

    public String getDescription(Locale locale) {
        if (m_messageSource == null) {
            return super.getDescription(locale);
        }
        return m_messageSource.getMessage(m_descriptionKey, null, getDescription(), locale);
    }
}
