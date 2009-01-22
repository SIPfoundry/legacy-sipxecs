/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import java.io.File;
import java.io.IOException;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.FileUtils;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.service.SipxMediaService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class MailboxManagerTest extends TestCase {
    private MailboxManagerImpl m_mgr;

    public static final File READONLY_MAILSTORE = new File(TestUtil
            .getTestSourceDirectory(MailboxManagerTest.class));

    protected void setUp() {
        m_mgr = new MailboxManagerImpl();
        String thisDir = TestUtil.getTestSourceDirectory(getClass());
        m_mgr.setMailstoreDirectory(thisDir);
    }

    public static File createTestMailStore() throws IOException {
        File testMailstore = new File(TestHelper.getTestDirectory() + '/'
                + System.currentTimeMillis());
        testMailstore.mkdirs();
        FileUtils.copyDirectory(new File(READONLY_MAILSTORE, "200"), new File(testMailstore,
                "200"));
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
        assertEquals(3, folderIds.size());
    }

    public void testGetDeletedVoicemail() {
        List<Voicemail> deleted = m_mgr.getVoicemail(m_mgr.getMailbox("200"), "deleted");
        assertEquals(1, deleted.size());
        assertEquals("00000002", deleted.get(0).getMessageId());
    }

    public void testLoadPreferencesWhenEmpty() {
        Mailbox mailbox = m_mgr.getMailbox("300");
        MailboxPreferencesReader reader = new MailboxPreferencesReader();
        m_mgr.setMailboxPreferencesReader(reader);
        MailboxPreferences preferences = m_mgr.loadMailboxPreferences(mailbox);
        assertNotNull(preferences);
    }

    public void testSavePreferencesWhenEmpty() {
        m_mgr.setMailstoreDirectory(TestHelper.getTestDirectory());
        MailboxPreferencesWriter writer = new MailboxPreferencesWriter();
        writer.setVelocityEngine(TestHelper.getVelocityEngine());
        m_mgr.setMailboxPreferencesWriter(writer);
        Mailbox mailbox = m_mgr.getMailbox("save-prefs-" + System.currentTimeMillis());
        m_mgr.saveMailboxPreferences(mailbox, new MailboxPreferences());
    }

    public void testSavePreferencesWhenNullPreferences() {
        m_mgr.setMailstoreDirectory(TestHelper.getTestDirectory());
        MailboxPreferencesWriter writer = new MailboxPreferencesWriter();
        writer.setVelocityEngine(TestHelper.getVelocityEngine());
        m_mgr.setMailboxPreferencesWriter(writer);
        Mailbox mailbox = m_mgr.getMailbox("save-prefs-" + System.currentTimeMillis());
        m_mgr.saveMailboxPreferences(mailbox, null);
    }

    public void testGetMediaServerCgiUrl() {
        LocationsManager locationsManager = EasyMock.createMock(LocationsManager.class);
        locationsManager.getPrimaryLocation();
        EasyMock.expectLastCall().andReturn(TestUtil.createDefaultLocation());
        EasyMock.replay(locationsManager);

        SipxMediaService mediaService = new SipxMediaService();
        mediaService.setBeanId(SipxMediaService.BEAN_ID);
        mediaService.setVoicemailHttpsPort(9905);
        SipxServiceManager sipxServiceManager = TestUtil.getMockSipxServiceManager(true, mediaService);

        MailboxManagerImpl out = new MailboxManagerImpl();
        out.setLocationsManager(locationsManager);
        out.setSipxServiceManager(sipxServiceManager);
        String expectedCgiUrl = "https://sipx.example.org:9905/cgi-bin/voicemail/mediaserver.cgi";
        assertEquals(expectedCgiUrl, out.getMediaServerCgiUrl());
    }
}
