/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting.type;

public class StringSetting extends AbstractSettingType {
    public static final StringSetting DEFAULT = new StringSetting();

    private static final int DEFAULT_MAX_LEN = 255;

    private boolean m_required;

    private int m_maxLen = DEFAULT_MAX_LEN;

    private int m_minLen;

    private String m_pattern;

    private boolean m_password;

    public int getMaxLen() {
        return m_maxLen;
    }

    public void setMaxLen(int maxLen) {
        m_maxLen = maxLen;
    }

    public int getMinLen() {
        return m_minLen;
    }

    public void setMinLen(int minLen) {
        m_minLen = minLen;
    }

    public String getPattern() {
        return m_pattern;
    }

    public void setPattern(String pattern) {
        m_pattern = pattern;
    }

    public boolean isRequired() {
        return m_required;
    }

    public void setRequired(boolean required) {
        m_required = required;
    }

    public boolean isPassword() {
        return m_password;
    }

    public void setPassword(boolean password) {
        m_password = password;
    }

    public String getName() {
        return "string";
    }

    public Object convertToTypedValue(Object value) {
        return value;
    }

    public String convertToStringValue(Object value) {
        return (String) value;
    }

    public String getLabel(Object value) {
        if (isPassword()) {
            return null;
        }
        return convertToStringValue(value);
    }
}
