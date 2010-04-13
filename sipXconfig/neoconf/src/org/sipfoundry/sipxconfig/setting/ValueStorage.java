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

import java.util.HashMap;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.BeanWithId;

/**
 * Basic layer of settings decoration that captures just setting values.
 */
public class ValueStorage extends BeanWithId implements Storage {
    private Map m_databaseValues = new HashMap();

    public int size() {
        return m_databaseValues.size();
    }

    public Map getDatabaseValues() {
        return m_databaseValues;
    }

    public void setDatabaseValues(Map databaseValues) {
        m_databaseValues = databaseValues;
    }

    public int getSize() {
        return getDatabaseValues().size();
    }

    public SettingValue getSettingValue(Setting setting) {
        if (getDatabaseValues() == null) {
            return null;
        }

        SettingValue settingValue = null;
        // null is legal value so test for key existance
        if (getDatabaseValues().containsKey(setting.getPath())) {
            String value = getSettingValue(setting.getPath());
            settingValue = new SettingValueImpl(blankToNull(value));
        }
        return settingValue;
    }

    public String getSettingValue(String path) {
        return (String) getDatabaseValues().get(path);
    }

    public void setSettingValue(String path, String value) {
        getDatabaseValues().put(path, nullToBlank(value));
    }

    private static String nullToBlank(String value) {
        return value == null ? StringUtils.EMPTY : value;
    }

    private static String blankToNull(String value) {
        return StringUtils.isEmpty(value) ? null : value;
    }

    public void setSettingValue(Setting setting, SettingValue value, SettingValue defaultValue) {
        if (value.equals(defaultValue)) {
            revertSettingToDefault(setting);
        } else {
            setSettingValue(setting.getPath(), value.getValue());
        }
    }

    public void revertSettingToDefault(Setting setting) {
        getDatabaseValues().remove(setting.getPath());
    }
}
