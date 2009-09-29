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

import java.util.Collections;
import java.util.Map;

import org.apache.commons.beanutils.ConversionException;
import org.apache.commons.beanutils.Converter;
import org.apache.commons.beanutils.converters.IntegerConverter;
import org.apache.commons.collections.map.LinkedMap;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.setting.Setting;

public class EnumSetting extends AbstractSettingType {
    private static final Converter CONVERTER = new IntegerConverter();

    private Map<String, String> m_enums = new LinkedMap();

    private String m_labelKeyPrefix;

    private boolean m_listenOnChange;
    private boolean m_promptSelect;

    public EnumSetting() {
    }

    public String getName() {
        return "enum";
    }

    public void addEnum(String value, String label) {
        m_enums.put(value, StringUtils.defaultString(label, value));
    }

    public Map<String, String> getEnums() {
        return Collections.unmodifiableMap(m_enums);
    }

    public void clearEnums() {
        m_enums.clear();
    }

    public boolean isRequired() {
        return true;
    }

    /**
     * At the moment we do not know if enumeration values are integers or strings. The naive
     * implementation tries to coerce the value to integer, if that fails strings are used.
     */
    public Object convertToTypedValue(Object value) {
        if (!isKey(value)) {
            return null;
        }
        try {
            return CONVERTER.convert(Integer.class, value);

        } catch (ConversionException e) {
            return value;
        }
    }

    public String convertToStringValue(Object value) {
        if (!isKey(value)) {
            return null;
        }
        return value.toString();
    }

    /**
     * Check if the value is known by this enum
     *
     * @param key
     * @return true if this is one of the enumeration values
     */
    boolean isKey(Object key) {
        if (key == null) {
            return false;
        }
        if (m_enums.containsKey(key)) {
            return true;
        }
        // if key is not a string also check for a key converted to string
        if (!(key instanceof String)) {
            return m_enums.containsKey(key.toString());
        }
        return false;
    }

    public String getLabel(Object value) {
        return m_enums.get(value);
    }

    /**
     * Prefix of the property that will be used to look up human representation of value.
     */
    public void setLabelKeyPrefix(String labelKeyPrefix) {
        m_labelKeyPrefix = labelKeyPrefix;
    }

    /**
     * Construct the labelKey that will be used to retrive the localized label for a particular
     * settings value.
     *
     * We are trying to acomodate various usage patterns here: (1) if setting has a special
     * labelPrefix set we use it (2) if setting has a an id the key will start with "type.id." (3)
     * if none of the above the setting name is used to construct label key
     *
     * @param setting used only to retrieve setting name
     * @param value enumeration value for which the key will be created, since this is an
     *        enumeration different values have to render different keys
     * @return labelKey (code) that is to be used against message source
     */
    public String getLabelKey(Setting setting, String value) {
        StringBuilder labelKey = new StringBuilder();
        if (m_labelKeyPrefix != null) {
            labelKey.append(m_labelKeyPrefix);
        } else if (getId() != null) {
            labelKey.append("type.");
            labelKey.append(getId());
        } else {
            labelKey.append(setting.getLabelKey());
        }
        labelKey.append('.');
        labelKey.append(value);
        return labelKey.toString();
    }

    public boolean isListenOnChange() {
        return m_listenOnChange;
    }

    public void setListenOnChange(boolean listenOnChange) {
        m_listenOnChange = listenOnChange;
    }

    public boolean isPromptSelect() {
        return m_promptSelect;
    }

    public void setPromptSelect(boolean promptSelect) {
        m_promptSelect = promptSelect;
    }

}
