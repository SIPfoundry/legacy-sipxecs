/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.alarm;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.common.PrimaryKeySource;

public class Alarm implements Comparable, PrimaryKeySource {
    private String m_alarmId;

    private String m_code;

    private String m_severity;

    private String m_component;

    private boolean m_logEnabled = true;

    private boolean m_emailEnabled = true;

    private int m_minThreshold;

    private String m_shortTitle;

    private String m_description;

    private String m_resolution;

    public String getAlarmId() {
        return m_alarmId;
    }

    public void setAlarmId(String alarmId) {
        m_alarmId = alarmId;
    }

    public String getCode() {
        return m_code;
    }

    public void setCode(String code) {
        m_code = code;
    }

    public String getComponent() {
        return m_component;
    }

    public void setComponent(String component) {
        m_component = component;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public boolean isEmailEnabled() {
        return m_emailEnabled;
    }

    public void setEmailEnabled(boolean emailEnabled) {
        m_emailEnabled = emailEnabled;
    }

    public boolean isLogEnabled() {
        return m_logEnabled;
    }

    public void setLogEnabled(boolean logEnabled) {
        m_logEnabled = logEnabled;
    }

    public int getMinThreshold() {
        return m_minThreshold;
    }

    public void setMinThreshold(int minThreshold) {
        m_minThreshold = minThreshold;
    }

    public String getResolution() {
        return m_resolution;
    }

    public void setResolution(String resolution) {
        m_resolution = resolution;
    }

    public String getSeverity() {
        return m_severity;
    }

    public void setSeverity(String severity) {
        m_severity = severity;
    }

    public String getShortTitle() {
        return m_shortTitle;
    }

    public void setShortTitle(String shortTitle) {
        m_shortTitle = shortTitle;
    }

    public int compareTo(Object alarm) {
        return getAlarmId().compareTo(((Alarm) alarm).getAlarmId());
    }

    public int hashCode() {
        return new HashCodeBuilder().append(m_alarmId).toHashCode();
    }

    public boolean equals(Object other) {
        if (!(other instanceof Alarm)) {
            return false;
        }
        if (this == other) {
            return true;
        }
        Alarm alarm = (Alarm) other;
        return new EqualsBuilder().append(m_alarmId, alarm.m_alarmId).isEquals();
    }

    public String toString() {
        return "Alarm: " + m_alarmId;
    }

    public Object getPrimaryKey() {
        return this;
    }
}
