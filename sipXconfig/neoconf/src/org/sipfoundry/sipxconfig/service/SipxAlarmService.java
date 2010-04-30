/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.common.AlarmContext;
import org.springframework.beans.factory.annotation.Required;

public class SipxAlarmService extends SipxService {

    public static final String BEAN_ID = "sipxAlarmService";

    private AlarmContext m_alarmContext;

    @Required
    public void setAlarmContext(AlarmContext alarmContext) {
        m_alarmContext = alarmContext;
    }
}
