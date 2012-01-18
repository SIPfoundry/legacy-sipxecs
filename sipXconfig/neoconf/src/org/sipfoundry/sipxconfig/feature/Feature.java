/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.feature;

public abstract class Feature {
    private String m_id;

    public Feature(String id) {
        m_id = id;
    }

    public String getId() {
        return m_id;
    }

    @Override
    public boolean equals(Object o) {
        if (o == null) {
            return false;
        }
        return m_id.equals(((Feature) o).getId());
    }

    @Override
    public int hashCode() {
        return m_id.hashCode();
    }
}
