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

/**
 * Setting is true or false, If you need true, false and not specified, use EnumSetting.
 */
public class BooleanSetting extends AbstractSettingType {
    private String m_trueValue = "1";

    private String m_falseValue = "0";

    public String getName() {
        return "boolean";
    }

    public String getFalseValue() {
        return m_falseValue;
    }

    public void setFalseValue(String false1) {
        m_falseValue = false1;
    }

    public String getTrueValue() {
        return m_trueValue;
    }

    public void setTrueValue(String true1) {
        m_trueValue = true1;
    }

    public boolean isRequired() {
        return false;
    }

    public Object convertToTypedValue(Object value) {
        if (value == null) {
            return null;
        }
        return value.equals(m_trueValue);
    }

    public String convertToStringValue(Object value) {
        if (value == null) {
            return null;
        }
        Boolean booleanValue = (Boolean) value;
        return booleanValue.booleanValue() ? m_trueValue : m_falseValue;
    }

    public String getLabel(Object value) {
        return getResourceLabel(value);
    }

    String getResourceLabel(Object value) {
        Boolean b = (Boolean) convertToTypedValue(value);
        if (b == null) {
            return null;
        }
        return b ? "true.value" : "false.value";
    }
}
