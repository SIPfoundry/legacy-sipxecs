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
 * Property selection model for Java 1.5 enums
 */
public class NewEnumPropertySelectionModel<E extends Enum<E>> implements IPropertySelectionModel {

    private E[] m_options;

    public void setOptions(E[] options) {
        m_options = options;
    }

    public void setEnumType(Class<E> elementType) {
        m_options = elementType.getEnumConstants();
    }

    public int getOptionCount() {
        return m_options.length;
    }

    public Object getOption(int index) {
        return m_options[index];
    }

    public String getLabel(int index) {
        return m_options[index].name();
    }

    public String getValue(int index) {
        return new Integer(index).toString();
    }

    public Object translateValue(String value) {
        int i = Integer.parseInt(value);
        // This is a robustness handling. If ever run into the indexOutOfBound
        // situation, reset the index to 0.
        if (i >= m_options.length) {
            i = 0;
        }
        return m_options[i];
    }

    public boolean isDisabled(int index) {
        return false;
    }
}
