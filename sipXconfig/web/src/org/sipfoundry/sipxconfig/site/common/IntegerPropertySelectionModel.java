/*
 * Copyright (C) 2013 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.common;

import org.apache.tapestry.AbstractComponent;
import org.apache.tapestry.form.IPropertySelectionModel;

public class IntegerPropertySelectionModel implements IPropertySelectionModel {

    private final AbstractComponent m_component;
    private final int[] m_options;

    public IntegerPropertySelectionModel(AbstractComponent component, int[] options) {
        m_component = component;
        this.m_options = new int[options.length];
        System.arraycopy(options, 0, this.m_options, 0, options.length);
    }

    @Override
    public String getLabel(int index) {
        return m_component.getMessages().getMessage("label." + String.valueOf(m_options[index]));
    }

    @Override
    public Object getOption(int index) {
        return m_options[index];
    }

    @Override
    public int getOptionCount() {
        return m_options.length;
    }

    @Override
    public String getValue(int index) {
        return String.valueOf(m_options[index]);
    }

    @Override
    public boolean isDisabled(int index) {
        return false;
    }

    @Override
    public Object translateValue(String value) {
        Integer translatedValue = null;
        if (value != null) {
            try {
                translatedValue = Integer.valueOf(value);
            } catch (NumberFormatException nfe) {
                // ignore. will just return null
                nfe.getMessage(); // precommit demands at least one statement
            }
        }

        return translatedValue;
    }
}
