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

import org.apache.commons.beanutils.Converter;
import org.apache.commons.beanutils.converters.DoubleConverter;

public class RealSetting extends AbstractSettingType {
    private static final Converter CONVERTER = new DoubleConverter(null);

    private float m_min;

    private float m_max = Float.MAX_VALUE;

    private boolean m_required;

    public float getMax() {
        return m_max;
    }

    public void setMax(float max) {
        m_max = max;
    }

    public float getMin() {
        return m_min;
    }

    public void setMin(float min) {
        m_min = min;
    }

    public String getName() {
        return "real";
    }

    public boolean isRequired() {
        return m_required;
    }

    public void setRequired(boolean required) {
        m_required = required;
    }

    public Object convertToTypedValue(Object value) {
        return CONVERTER.convert(Double.class, value);
    }

    public String convertToStringValue(Object value) {
        if (value == null) {
            return null;
        }
        return value.toString();
    }

    public String getLabel(Object value) {
        return convertToStringValue(value);
    }
}
