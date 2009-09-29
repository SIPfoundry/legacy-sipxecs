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

import java.util.List;

import org.apache.commons.lang.enums.Enum;
import org.apache.commons.lang.enums.EnumUtils;
import org.apache.tapestry.form.IPropertySelectionModel;

/**
 * EnumPropertySelectionModel Similar to the class from Tapestry with the same name, but can be
 * used in as a bean. Does not support bundles. Use either setOptions or setEnumClass but not both
 *
 * @see org.apache.tapestry.form.EnumPropertySelectionModel
 */
public class EnumPropertySelectionModel implements IPropertySelectionModel {

    private Enum[] m_options;

    public void setOptions(Enum[] options) {
        m_options = options;
    }

    public void setEnumClass(Class enumeration) {
        List list = EnumUtils.getEnumList(enumeration);
        m_options = (Enum[]) list.toArray(new Enum[list.size()]);
    }

    public int getOptionCount() {
        return m_options.length;
    }

    public Object getOption(int index) {
        return m_options[index];
    }

    public String getLabel(int index) {
        return m_options[index].getName();
    }

    public String getValue(int index) {
        return new Integer(index).toString();
    }

    public Object translateValue(String value) {
        int i = Integer.parseInt(value);
        return m_options[i];
    }

    public boolean isDisabled(int index) {
        return false;
    }
}
