/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.device;

import org.sipfoundry.sipxconfig.common.InitTaskListener;
import org.springframework.beans.factory.annotation.Required;

public class TimeZoneInit extends InitTaskListener {
    private TimeZoneManager m_timeZoneManager;

    @Override
    public void onInitTask(String task) {
        m_timeZoneManager.saveDefault();
    }

    @Required
    public void setTimeZoneManager(TimeZoneManager timeZoneManager) {
        m_timeZoneManager = timeZoneManager;
    }
}
