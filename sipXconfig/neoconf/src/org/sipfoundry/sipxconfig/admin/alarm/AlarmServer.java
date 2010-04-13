/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.alarm;

import org.sipfoundry.sipxconfig.common.BeanWithId;

public class AlarmServer extends BeanWithId {
    private boolean m_alarmNotificationEnabled = true; // default enabled

    private String m_fromEmailAddress;

    public boolean isAlarmNotificationEnabled() {
        return m_alarmNotificationEnabled;
    }

    public void setAlarmNotificationEnabled(boolean alarmNotificationEnabled) {
        m_alarmNotificationEnabled = alarmNotificationEnabled;
    }

    public void setFromEmailAddress(String fromEmailAddress) {
        m_fromEmailAddress = fromEmailAddress;
    }

    public String getFromEmailAddress() {
        return m_fromEmailAddress;
    }

}
