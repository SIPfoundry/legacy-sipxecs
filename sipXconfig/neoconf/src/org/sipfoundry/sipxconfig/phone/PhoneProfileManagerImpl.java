/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import org.sipfoundry.sipxconfig.device.AbstractProfileManager;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.RestartManager;

public class PhoneProfileManagerImpl extends AbstractProfileManager {
    private PhoneContext m_phoneContext;

    private RestartManager m_restartManager;

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void setRestartManager(RestartManager restartManager) {
        m_restartManager = restartManager;
    }

    protected Device loadDevice(Integer id) {
        return m_phoneContext.loadPhone(id);
    }

    protected void restartDevice(Integer id) {
        m_restartManager.restart(id);
    }
}
