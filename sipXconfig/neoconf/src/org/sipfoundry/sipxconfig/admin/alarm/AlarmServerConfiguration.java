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

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.service.SipxServiceConfiguration;
import org.springframework.beans.factory.annotation.Required;

import static org.apache.commons.lang.StringUtils.defaultIfEmpty;

public class AlarmServerConfiguration extends SipxServiceConfiguration {
    private AlarmServerManager m_alarmServerManager;

    private boolean m_alarmNotificationEnabled;

    private String m_fromEmailAddress;

    private String m_logDirectory;

    private String m_hostName;

    private void generate() {
        AlarmServer alarmServer = m_alarmServerManager.getAlarmServer();
        String host = m_alarmServerManager.getHost();
        m_alarmNotificationEnabled = alarmServer.isAlarmNotificationEnabled();
        m_fromEmailAddress = defaultIfEmpty(alarmServer.getFromEmailAddress(), "postmaster@" + host);
        m_logDirectory = m_alarmServerManager.getLogDirectory();
        m_hostName = host;
    }

    @Override
    protected VelocityContext setupContext(Location location) {
        generate();
        VelocityContext context = super.setupContext(location);
        context.put("enabled", m_alarmNotificationEnabled);
        context.put("fromEmailAddress", m_fromEmailAddress);
        context.put("logDirectory", m_logDirectory);
        context.put("hostName", m_hostName);

        return context;
    }

    @Required
    public void setAlarmServerManager(AlarmServerManager alarmServerManager) {
        m_alarmServerManager = alarmServerManager;
    }
}
