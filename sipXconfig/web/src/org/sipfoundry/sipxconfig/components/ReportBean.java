/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import java.io.Serializable;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;

public class ReportBean implements OptionAdapter, Serializable {
    private String m_reportName;

    private String m_reportLabel;

    public ReportBean(String reportName, String reportLabel) {
        m_reportName = reportName;
        m_reportLabel = reportLabel;
    }

    public String getLabel(Object option, int index) {
        return m_reportLabel;
    }

    public Object getValue(Object option, int index) {
        return this;
    }

    public String squeezeOption(Object option, int index) {
        return m_reportName;
    }

    public String getReportName() {
        return m_reportName;
    }

    public void setReportName(String reportName) {
        m_reportName = reportName;
    }

    public String getReportLabel() {
        return m_reportLabel;
    }

    public void setReportLabel(String reportLabel) {
        m_reportLabel = reportLabel;
    }

    public int hashCode() {
        return new HashCodeBuilder().append(m_reportName).toHashCode();
    }

    public boolean equals(Object other) {
        if (!(other instanceof ReportBean)) {
            return false;
        }
        if (this == other) {
            return true;
        }
        ReportBean bean = (ReportBean) other;
        return new EqualsBuilder().append(m_reportName, bean.m_reportName).isEquals();
    }
}
