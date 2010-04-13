/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.alarm;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.service.SipxServiceConfiguration;
import org.springframework.beans.factory.annotation.Required;

public class AlarmGroupsConfiguration extends SipxServiceConfiguration {
    private AlarmServerManager m_alarmServerManager;

    private List<AlarmGroup> m_alarmGroups;

    private void generate() {
        List<AlarmGroup> alarmGroups = m_alarmServerManager.getAlarmGroups();
        for (AlarmGroup group : alarmGroups) {
            List<String> userEmailAddresses = new ArrayList<String>();
            Set<User> users = group.getUsers();
            for (User user : users) {
                String emailAddress = user.getEmailAddress();
                if (emailAddress != null) {
                    userEmailAddresses.add(emailAddress);
                }
                String altEmailAddress = user.getAlternateEmailAddress();
                if (altEmailAddress != null) {
                    userEmailAddresses.add(altEmailAddress);
                }
            }

            group.setUserEmailAddresses(userEmailAddresses);
        }
        m_alarmGroups = alarmGroups;
    }

    @Override
    protected VelocityContext setupContext(Location location) {
        generate();
        VelocityContext context = super.setupContext(location);
        context.put("groups", m_alarmGroups);
        return context;
    }

    @Required
    public void setAlarmServerManager(AlarmServerManager alarmServerManager) {
        m_alarmServerManager = alarmServerManager;
    }
}
