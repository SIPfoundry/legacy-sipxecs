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
 * Implementation of IPropertySelectionModel Tapestry interface, which allows two custom Strings
 * to be used as labels for a property of type Boolean. Can be used as a bean
 *
 * @see org.apache.tapestry.form.IPropertySelectionModel
 */
public class BooleanPropertySelectionModel implements IPropertySelectionModel {

    private boolean[] m_values = {
        false, true
    };
    private String[] m_labels = {
        "False", "True"
    };

    public BooleanPropertySelectionModel() {

    }

    public BooleanPropertySelectionModel(String first, String second) {

        m_labels[0] = first;
        m_labels[1] = second;
    }

    public void setFalseLabel(String label) {

        m_labels[0] = label;
    }

    public void setTrueLabel(String label) {

        m_labels[1] = label;
    }

    public String getLabel(int index) {

        return m_labels[index];
    }

    public Object getOption(int index) {

        return m_values[index];
    }

    public int getOptionCount() {

        return m_values.length;
    }

    public String getValue(int index) {

        return Integer.toString(index);
    }

    public Object translateValue(String value) {

        int index = Integer.parseInt(value);
        return m_values[index];
    }

    public boolean isDisabled(int arg0) {
        return false;
    }
}
