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

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.TemplateConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;
import org.springframework.beans.factory.annotation.Required;

public class AlarmGroupsConfiguration extends TemplateConfigurationFile {
    private List<AlarmGroup> m_alarmGroups;

    private MailboxManager m_mailboxManager;

    public void generate(List<AlarmGroup> alarmGroups) {
        String emailAddress;
        for (AlarmGroup group : alarmGroups) {
            List<String> userEmailAddresses = new ArrayList<String>();
            Set<User> users = group.getUsers();
            for (User user : users) {
                Mailbox mailbox = m_mailboxManager.getMailbox(user.getUserName());
                MailboxPreferences mailboxPrefs = m_mailboxManager.loadMailboxPreferences(mailbox);
                emailAddress = mailboxPrefs.getEmailAddress();
                if (emailAddress != null) {
                    userEmailAddresses.add(emailAddress);
                }
                emailAddress = mailboxPrefs.getAlternateEmailAddress();
                if (emailAddress != null) {
                    userEmailAddresses.add(emailAddress);
                }
            }

            group.setUserEmailAddresses(userEmailAddresses);
        }
        this.m_alarmGroups = alarmGroups;
    }

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        context.put("groups", m_alarmGroups);
        return context;
    }

    @Required
    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }
}
