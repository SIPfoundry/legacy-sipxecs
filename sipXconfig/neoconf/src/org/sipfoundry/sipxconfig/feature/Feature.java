/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
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

    public String toString() {
        return m_id;
    }
}
