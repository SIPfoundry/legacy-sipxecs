/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.device;

import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.springframework.beans.factory.annotation.Required;

public class TimeZoneInit implements SetupListener {
    private TimeZoneManager m_timeZoneManager;

    @Required
    public void setTimeZoneManager(TimeZoneManager timeZoneManager) {
        m_timeZoneManager = timeZoneManager;
    }

    @Override
    public boolean setup(SetupManager manager) {
        String id = "default-time-zone";
        if (manager.isFalse(id)) {
            m_timeZoneManager.saveDefault();
            manager.setTrue(id);
        }
        return true;
    }
}
