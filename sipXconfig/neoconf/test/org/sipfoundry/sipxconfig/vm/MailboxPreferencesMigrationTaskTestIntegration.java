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

import org.apache.commons.io.FileUtils;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

public class MailboxPreferencesMigrationTaskTestIntegration extends IntegrationTestCase {

    private MailboxManagerImpl m_mailboxManager;
    private CoreContext m_coreContext;
    private MailboxPreferencesMigrationTask m_mailboxPreferencesMigrationTask;
    private File m_mailstore;

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        m_mailstore = MailboxManagerTest.createTestMailStore();
        m_mailboxManager.setMailstoreDirectory(m_mailstore.getPath());
    }

    @Override
    protected void onTearDownInTransaction() throws Exception {
        FileUtils.deleteDirectory(m_mailstore);
        super.onTearDownInTransaction();
    }

    public void testMigrateMailboxPrefs() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        loadDataSet("vm/seed_user.db.xml");

        User user = m_coreContext.loadUserByUserName("200");
        assertNotNull(user);
        assertNull(user.getEmailAddress());
        m_mailboxPreferencesMigrationTask.onInitTask("mailbox_prefs_migration");
        user = m_coreContext.loadUserByUserName("200");
        assertNotNull(user);
        assertEquals("dhubler@pingtel.com", user.getEmailAddress());
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setMailboxPreferencesMigrationTask(MailboxPreferencesMigrationTask mailboxPreferencesMigrationTask) {
        m_mailboxPreferencesMigrationTask = mailboxPreferencesMigrationTask;
    }

    public void setMailboxManagerImpl(MailboxManagerImpl mailboxManager) {
        m_mailboxManager = mailboxManager;
    }
}
