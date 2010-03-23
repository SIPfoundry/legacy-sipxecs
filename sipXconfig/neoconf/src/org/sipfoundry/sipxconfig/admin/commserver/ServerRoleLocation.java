/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.Collection;

import org.sipfoundry.sipxconfig.service.SipxServiceBundle;

public class ServerRoleLocation {

    private Location m_location;
    private Collection<SipxServiceBundle> m_modifiedBundles;

    public Collection<SipxServiceBundle> getModifiedBundles() {
        return m_modifiedBundles;
    }

    public void setModifiedBundles(Collection<SipxServiceBundle> modifiedBundles) {
        this.m_modifiedBundles = modifiedBundles;
    }

    public Location getLocation() {
        return m_location;
    }

    public void setLocation(Location location) {
        m_location = location;
    }

    public boolean isBundleModified(String bundleName) {
        for (SipxServiceBundle bundle : m_modifiedBundles) {
            if (bundle.getName().equals(bundleName)) {
                return true;
            }
        }
        return false;
    }
}
