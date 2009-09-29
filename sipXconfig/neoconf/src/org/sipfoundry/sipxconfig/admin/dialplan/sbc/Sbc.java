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

    private SbcRoutes m_routes;

    private SbcDevice m_sbcDevice;

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

    /**
     * Called when SBC device asociated with this SBC gets deleted.
     *
     * @return true if SBC should be deleted as well
     */
    public boolean onDeleteSbcDevice() {
        return true;
    }
}
