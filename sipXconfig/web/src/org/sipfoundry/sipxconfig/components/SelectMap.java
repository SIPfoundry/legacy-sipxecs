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

import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

/**
 * SelectMap Map used to keep the status of selected row. It is initialized from
 * the collection of Java beans with id property. It is used similarly to
 * ListEditMap but can be used outside of list edit components.
 */
public class SelectMap {
    /**
     * Only keeps the id's for which selected is set
     */
    private Set m_selections = new HashSet();

    public boolean getSelected(Object id) {
        return m_selections.contains(id);
    }

    public void setAllSelected(Collection ids, boolean selected) {
        if (selected) {
            m_selections.addAll(ids);
        } else {
            m_selections.removeAll(ids);
        }
    }

    public void setSelected(Object id, boolean selected) {
        if (selected) {
            m_selections.add(id);
        } else {
            m_selections.remove(id);
        }
    }

    public Collection getAllSelected() {
        return Collections.unmodifiableCollection(m_selections);
    }
}
