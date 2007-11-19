/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.sbc;

import org.sipfoundry.sipxconfig.device.AbstractProfileManager;
import org.sipfoundry.sipxconfig.device.Device;
import org.springframework.beans.factory.annotation.Required;

public class SbcProfileManagerImpl extends AbstractProfileManager {

    private SbcDeviceManager m_sbcDeviceManager;

    @Required
    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }

    @Override
    protected Device loadDevice(Integer id) {
        return m_sbcDeviceManager.getSbcDevice(id);
    }

    @Override
    protected void restartDevice(Integer id) {
        // TODO: need a way to restart sbcs
    }
}
