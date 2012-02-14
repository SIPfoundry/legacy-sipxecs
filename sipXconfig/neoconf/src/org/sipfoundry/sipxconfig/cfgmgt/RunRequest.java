/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import java.util.Collection;

import org.sipfoundry.sipxconfig.commserver.Location;

public class RunRequest {
    private String m_label;
    private String[] m_bundles;
    private String[] m_defines;
    private Collection<Location> m_locations;

    public RunRequest(String label, String... bundles) {
        m_label = label;
        m_bundles = bundles;
    }

    public RunRequest(String label, String[] defines, String... bundles) {
        this(label, bundles);
        m_defines = defines;
    }

    public RunRequest(String label, Collection<Location> locations, String... bundles) {
        this(label, bundles);
        m_locations = locations;
    }

    public RunRequest(String label, Collection<Location> locations, String[] defines, String... bundles) {
        this(label, locations, bundles);
        m_defines = defines;
    }

    public String[] getDefines() {
        return m_defines;
    }

    public void setDefines(String[] defines) {
        m_defines = defines;
    }

    public String getLabel() {
        return m_label;
    }

    public String[] getBundles() {
        return m_bundles;
    }

    public Collection<Location> getLocations() {
        return m_locations;
    }
}
