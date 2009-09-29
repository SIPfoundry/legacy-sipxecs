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

/**
 * Option representing optgroup element
 *
 * Has to be used with special selection renderer that recognizes it
 */
public class OptGroup implements OptionAdapter {
    private String m_label;

    public OptGroup(String label) {
        m_label = label;
    }

    public Object getValue(Object option_, int index_) {
        return this;
    }

    public String getLabel(Object option_, int index_) {
        return m_label;
    }

    public String squeezeOption(Object option_, int index_) {
        return "optgroup." + m_label;
    }
}
