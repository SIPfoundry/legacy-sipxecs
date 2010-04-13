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

import java.util.Map;
import java.util.Map.Entry;

import org.apache.tapestry.form.IPropertySelectionModel;

/**
 * Uses pairs of option->label as a backend for Tapestry implementation
 */
public class NamedValuesSelectionModel implements IPropertySelectionModel {

    private Object[] m_options;
    private String[] m_labels;

    /**
     * @map if instance of commons LinkedMap, use it directly, otherwise copy into a order-aware
     *      map
     */
    public NamedValuesSelectionModel(Map< ? , String> map) {
        final int size = map.size();
        m_options = new Object[size];
        m_labels = new String[size];
        int i = 0;
        for (Entry< ? , String> entry : map.entrySet()) {
            m_options[i] = entry.getKey();
            m_labels[i] = entry.getValue();
            i++;
        }
    }

    public NamedValuesSelectionModel(Object[] options, String[] labels) {
        m_options = options;
        m_labels = labels;
    }

    public void setOptions(Object[] options) {
        m_options = options;
    }

    public void setLabels(String[] labels) {
        m_labels = labels;
    }

    public int getOptionCount() {
        return m_options.length;
    }

    public Object getOption(int index) {
        return m_options[index];
    }

    public String getLabel(int index) {
        return m_labels[index];
    }

    public String getValue(int index) {
        return Integer.toString(index);
    }

    public Object translateValue(String value) {
        int index = Integer.parseInt(value);
        return getOption(index);
    }

    public boolean isDisabled(int index) {
        return false;
    }
}
