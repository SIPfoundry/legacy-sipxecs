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
import java.io.IOException;
import java.util.List;

import junit.framework.TestCase;
import org.apache.commons.io.FileUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.test.TestUtil;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

public class MailboxManagerTest extends TestCase {
    private MailboxManagerImpl m_mgr;
    private User user = new User();
    private static final String FILE_SEPARATOR = "file.separator";
    public static final File READONLY_MAILSTORE = new File(TestUtil.getTestSourceDirectory(MailboxManagerTest.class));

    @Override
    protected void setUp() {
        m_mgr = new MailboxManagerImpl();

        String thisDir = TestUtil.getTestSourceDirectory(getClass());
        m_mgr.setMailstoreDirectory(thisDir);

        PermissionManager pManager = createMock(PermissionManager.class);
        pManager.getPermissionModel();
        expectLastCall().andReturn(TestHelper.loadSettings("commserver/user-settings.xml")).anyTimes();

        AddressBookEntry abe = new AddressBookEntry();
        abe.setEmailAddress("myemail@gmail.com");
        abe.setAlternateEmailAddress("myotheremail@gmail.com");
        user = new User();
        user.setUserName("300");
        user.setAddressBookEntry(abe);
        user.setPermissionManager(pManager);

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUserByUserName("300");
        expectLastCall().andReturn(user).anyTimes();
        coreContext.saveUser(user);
        expectLastCall().andReturn(true);
        replay(pManager, coreContext);
        m_mgr.setCoreContext(coreContext);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        File mailstore300 = new File(new StringBuilder(TestUtil.getTestSourceDirectory(MailboxManagerTest.class))
                .append(System.getProperty(FILE_SEPARATOR)).append("300").toString());
        File mailboxprefs300 = new File(new StringBuilder(TestUtil.getTestSourceDirectory(MailboxManagerTest.class))
                .append(System.getProperty(FILE_SEPARATOR)).append("300").append(System.getProperty(FILE_SEPARATOR))
                .append("mailboxprefs.xml").toString());
        if (mailboxprefs300.exists()) {
            mailboxprefs300.delete();
        }
        if (mailstore300.exists()) {
            mailstore300.delete();
        }
    }

    public static File createTestMailStore() throws IOException {
        File testMailstore = new File(TestHelper.getTestDirectory() + '/' + System.currentTimeMillis());
        testMailstore.mkdirs();
        FileUtils.copyDirectory(new File(READONLY_MAILSTORE, "200"), new File(testMailstore, "200"));
        FileUtils.copyDirectory(new File(READONLY_MAILSTORE, "200"), new File(testMailstore, "201"));
        return testMailstore;
    }

    public void testEnabled() {
        assertTrue(m_mgr.isEnabled());
        m_mgr.setMailstoreDirectory("bogus-mogus");
        assertFalse(m_mgr.isEnabled());
    }

    public void testDeleteMailbox() throws IOException {
        File mailstore = MailboxManagerTest.createTestMailStore();
        MailboxManagerImpl mgr = new MailboxManagerImpl();
        mgr.setMailstoreDirectory(mailstore.getAbsolutePath());

        Mailbox mbox = mgr.getMailbox("200");
        assertTrue(mbox.getUserDirectory().exists());
        mgr.deleteMailbox("200");
        assertFalse(mbox.getUserDirectory().exists());
        mgr.deleteMailbox("200");
        assertFalse(mbox.getUserDirectory().exists());

        Mailbox mbox1 = mgr.getMailbox("201");
        Mailbox mbox2 = mgr.getMailbox("202");
        assertTrue(mbox1.getUserDirectory().exists());
        mgr.renameMailbox("201", "202");
        assertFalse(mbox1.getUserDirectory().exists());
        assertTrue(mbox2.getUserDirectory().exists());
        mgr.deleteMailbox("202");
        assertFalse(mbox2.getUserDirectory().exists());

        Mailbox nombox = mgr.getMailbox("non-existing-user");
        assertFalse(nombox.getUserDirectory().exists());
        mgr.deleteMailbox("non-existing-user");
        assertFalse(nombox.getUserDirectory().exists());

        // nice, not critical
        FileUtils.deleteDirectory(mailstore);
    }

    public void testGetVoicemailWhenInvalid() {
        MailboxManagerImpl mgr = new MailboxManagerImpl();
        Mailbox mbox = mgr.getMailbox("200");
        try {
            mgr.getVoicemail(mbox, "inbox").size();
            fail();
        } catch (UserException expected) {
            assertTrue(true);
        }

        try {
            mgr.setMailstoreDirectory("bogus");
            mgr.getVoicemail(mbox, "inbox").size();
            fail();
        } catch (UserException expected) {
            assertTrue(true);
        }
    }

    public void testGetVoicemailWhenEmpty() {
        assertEquals(0, m_mgr.getVoicemail(m_mgr.getMailbox("200"), "inbox-bogus").size());
        assertEquals(0, m_mgr.getVoicemail(m_mgr.getMailbox("200-bogus"), "inbox").size());
    }

    public void testGetInboxVoicemail() {
        List<Voicemail> vm = m_mgr.getVoicemail(m_mgr.getMailbox("200"), "inbox");
        assertEquals(2, vm.size());
        assertEquals("00000001", vm.get(0).getMessageId());
        assertTrue(vm.get(0).getMediaFile().exists());
    }

    public void testBasename() {
        assertEquals("bird", MailboxManagerImpl.basename("bird-00.xml"));
        assertEquals("bird", MailboxManagerImpl.basename("bird"));
    }

    public void testGetFolders() {
        List<String> folderIds = m_mgr.getMailbox("200").getFolderIds();
        assertEquals(4, folderIds.size());
    }

    public void testGetDeletedVoicemail() {
        List<Voicemail> deleted = m_mgr.getVoicemail(m_mgr.getMailbox("200"), "deleted");
        assertEquals(1, deleted.size());
        assertEquals("00000002", deleted.get(0).getMessageId());
    }
}
