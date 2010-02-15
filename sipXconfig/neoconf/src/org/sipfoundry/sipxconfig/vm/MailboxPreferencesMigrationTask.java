/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.vm;

import java.io.File;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.InitTaskListener;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.springframework.beans.factory.annotation.Required;

public class MailboxPreferencesMigrationTask extends InitTaskListener {

    private static final Log LOG = LogFactory.getLog(MailboxPreferencesMigrationTask.class);
    private MailboxManager m_mailboxManager;
    private CoreContext m_coreContext;
    private MailboxPreferencesReader m_mailboxPreferencesReader;
    private PermissionManager m_permissionManager;

    @Override
    public void onInitTask(String task) {
        LOG.debug("Starting mailbox preferences migration");
        Closure<User> closure = new Closure<User>() {
            @Override
            public void execute(User user) {
                // HACK: setting permission manager is only needed in the tests
                user.setPermissionManager(m_permissionManager);

                Mailbox mailbox = m_mailboxManager.getMailbox(user.getUserName());
                File prefsFile = mailbox.getVoicemailPreferencesFile();
                if (!prefsFile.exists()) {
                    return;
                }
                MailboxPreferences preferences = m_mailboxPreferencesReader.readObject(prefsFile);
                if (preferences == null) {
                    return;
                }
                preferences.updateUser(user);
                m_coreContext.saveUser(user);
                m_mailboxManager.writePreferencesFile(user);
                LOG.debug("Saved preferences for user: " + user.getUserName());
            }
        };
        DaoUtils.forAllUsersDo(m_coreContext, closure);
    }

    @Required
    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setMailboxPreferencesReader(MailboxPreferencesReader mailboxPreferencesReader) {
        m_mailboxPreferencesReader = mailboxPreferencesReader;
    }

    @Required
    public void setPermissionManager(PermissionManager permissionManager) {
        m_permissionManager = permissionManager;
    }
}
