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
package org.sipfoundry.sipxconfig.cfgmgt;

import java.util.Collection;

import org.sipfoundry.sipxconfig.commserver.Location;

public class RunRequest {
    private String m_label;
    private String[] m_bundles = new String[0];
    private String[] m_defines = new String[0];
    private Collection<Location> m_locations;

    public RunRequest(String label, Collection<Location> locations) {
        m_label = label;
        m_locations = locations;
    }

    public String[] getDefines() {
        return m_defines;
    }

    public void setDefines(String... defines) {
        m_defines = defines;
    }

    public String getLabel() {
        return m_label;
    }

    public void setBundles(String... bundles) {
        m_bundles = bundles;
    }

    public String[] getBundles() {
        return m_bundles;
    }

    public Collection<Location> getLocations() {
        return m_locations;
    }
}
