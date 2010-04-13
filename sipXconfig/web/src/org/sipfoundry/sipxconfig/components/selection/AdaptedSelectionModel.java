/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components.selection;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import org.apache.tapestry.form.IPropertySelectionModel;

/**
 * Is using option adaptor to create selection model from an array of objects.
 * AdaptedSelectionModel
 */
public class AdaptedSelectionModel implements IPropertySelectionModel {
    private Object[] m_options;

    private OptionAdapter m_adapter;

    private Map m_valueCache;

    public void setCollection(Collection options) {
        m_options = options.toArray();
        m_valueCache = null;
    }

    public void setOptions(Object[] options) {
        m_options = options;
        m_valueCache = null;
    }

    public void setAdapter(OptionAdapter adapter) {
        m_adapter = adapter;
    }

    public int getOptionCount() {
        return m_options.length;
    }

    public Object getOption(int index) {
        OptionAdapter adapter = getAdapter(m_options[index]);
        return adapter.getValue(m_options[index], index);
    }

    public String getLabel(int index) {
        OptionAdapter adapter = getAdapter(m_options[index]);
        return adapter.getLabel(m_options[index], index);
    }

    public String getValue(int index) {
        OptionAdapter adapter = getAdapter(m_options[index]);
        return adapter.squeezeOption(m_options[index], index);
    }

    public Object translateValue(String value) {
        if (m_valueCache == null) {
            m_valueCache = new HashMap(m_options.length);
            for (int i = 0; i < m_options.length; i++) {
                m_valueCache.put(getValue(i), m_options[i]);
            }
        }
        return m_valueCache.get(value);
    }

    private OptionAdapter getAdapter(Object option) {
        if (option instanceof OptionAdapter) {
            return (OptionAdapter) option;
        }
        return m_adapter;
    }

    public boolean isDisabled(int arg0) {
        return false;
    }
}
