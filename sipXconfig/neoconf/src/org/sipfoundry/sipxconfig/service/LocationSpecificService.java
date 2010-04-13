/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.common.BeanWithId;

public class LocationSpecificService extends BeanWithId {

    private Location m_location;
    private SipxService m_sipxService;
    private boolean m_enableOnNextUpgrade;

    /**
     * No-arg constructor
     */
    public LocationSpecificService() {
        // nothing to do...
    }

    /**
     * Convenience constructor to create LocationSpecificService from a SipService definition
     */
    public LocationSpecificService(SipxService sipxService) {
        m_sipxService = sipxService;
    }

    public Location getLocation() {
        return m_location;
    }

    public void setLocation(Location location) {
        m_location = location;
    }

    public SipxService getSipxService() {
        return m_sipxService;
    }

    public void setSipxService(SipxService sipxService) {
        m_sipxService = sipxService;
    }

    public boolean getEnableOnNextUpgrade() {
        return m_enableOnNextUpgrade;
    }

    public void setEnableOnNextUpgrade(boolean enableOnNextUpgrade) {
        m_enableOnNextUpgrade = enableOnNextUpgrade;
    }

    public String toString() {
        StringBuffer strBuffer = new StringBuffer();
        strBuffer.append("LocationSpecificService: ");
        strBuffer.append("locationId=");
        strBuffer.append(m_location.getId());
        strBuffer.append(", ");
        strBuffer.append("sipxServiceId=");
        strBuffer.append(m_sipxService.getId());
        return strBuffer.toString();
    }
}
