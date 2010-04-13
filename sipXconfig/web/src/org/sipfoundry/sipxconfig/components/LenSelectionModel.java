/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.form.IPropertySelectionModel;

/**
 * LenSelectionModel
 *
 * Supports selecting number of digits from withing the specified, inclusive range
 */
public class LenSelectionModel implements IPropertySelectionModel {
    private int m_min;
    private int m_max;

    public int getOptionCount() {
        if (m_max < m_min) {
            throw new IllegalStateException("MAX < MIN: model not configured properly. MAX: "
                    + m_max + " MIN: " + m_min);
        }
        return m_max - m_min + 1;
    }

    public Object getOption(int index) {
        return new Integer(index + m_min);
    }

    public String getLabel(int index) {
        return getOption(index) + " digits";
    }

    public String getValue(int index) {
        return getOption(index).toString();
    }

    public Object translateValue(String value) {
        int index = Integer.parseInt(value);
        return new Integer(index);
    }

    public int getMax() {
        return m_max;
    }

    public void setMax(int max) {
        m_max = max;
    }

    public int getMin() {
        return m_min;
    }

    public void setMin(int min) {
        m_min = min;
    }

    public boolean isDisabled(int index) {
        return false;
    }
}
