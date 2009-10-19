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
import org.sipfoundry.sipxconfig.admin.TemplateConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.common.User;

public class AlarmGroupsConfiguration extends TemplateConfigurationFile {
    private List<AlarmGroup> m_alarmGroups;

    public void generate(List<AlarmGroup> alarmGroups) {
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
        VelocityContext context = super.setupContext(location);
        context.put("groups", m_alarmGroups);
        return context;
    }
}
