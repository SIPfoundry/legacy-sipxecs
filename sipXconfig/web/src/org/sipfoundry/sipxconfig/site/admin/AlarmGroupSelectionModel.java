/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.List;

import org.apache.tapestry.form.IPropertySelectionModel;

public class AlarmGroupSelectionModel implements IPropertySelectionModel {
    private static final String GROUP_NAME_DISABLED = "disabled";
    private List m_groups;

    /**
     * Bean style initialization; you can only call it once
     *
     * @param alarmGroups list of gateways for the model
     */
    public void setGroups(List groups) {
        if (m_groups != null) {
            throw new IllegalStateException("Object has been already initialized");
        }
        m_groups = groups;
    }

    public int getOptionCount() {
        return m_groups.size() + 1;
    }

    public Object getOption(int index) {
        if (index == 0) {
            return GROUP_NAME_DISABLED;
        }
        return m_groups.get(index - 1);
    }

    public String getLabel(int index) {
        String alarmGroupString;
        if (index == 0) {
            alarmGroupString = GROUP_NAME_DISABLED;
        } else {
            alarmGroupString = (String) getOption(index);
        }
        return alarmGroupString;
    }

    public String getValue(int index) {
        return getLabel(index);
    }

    public Object translateValue(String value) {
        return value;
    }

    public boolean isDisabled(int index) {
        return false;
    }
}
