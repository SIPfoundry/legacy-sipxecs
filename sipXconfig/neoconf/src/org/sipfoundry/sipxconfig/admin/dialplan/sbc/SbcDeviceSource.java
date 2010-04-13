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

import org.sipfoundry.sipxconfig.device.DeviceSource;
import org.springframework.beans.factory.annotation.Required;

public class SbcDeviceSource implements DeviceSource<SbcDevice> {

    private SbcDeviceManager m_sbcDeviceManager;

    @Required
    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }

    public SbcDevice loadDevice(Integer id) {
        return m_sbcDeviceManager.getSbcDevice(id);
    }
}
