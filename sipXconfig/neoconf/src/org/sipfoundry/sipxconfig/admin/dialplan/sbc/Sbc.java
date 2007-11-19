/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * 
 */
package org.sipfoundry.sipxconfig.admin.dialplan.sbc;

import org.sipfoundry.sipxconfig.common.BeanWithId;

public abstract class Sbc extends BeanWithId {
    private boolean m_enabled;

    private String m_address;

    private SbcRoutes m_routes;

    private SbcDevice m_sbcDevice;

    public String getAddress() {
        return m_address;
    }

    public void setAddress(String address) {
        m_address = address;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public SbcRoutes getRoutes() {
        return m_routes;
    }

    public void setRoutes(SbcRoutes routes) {
        m_routes = routes;
    }

    public SbcDevice getSbcDevice() {
        return m_sbcDevice;
    }

    public void setSbcDevice(SbcDevice sbcDevice) {
        m_sbcDevice = sbcDevice;
    }
}
