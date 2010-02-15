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

import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.ACTIVE_GREETING;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.BUSY_PROMPT;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.IMAP_HOST;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.IMAP_PASSWORD;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.IMAP_PORT;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.IMAP_TLS;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.PRIMARY_EMAIL_NOTIFICATION;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.VOICEMAIL_TUI;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.io.StringWriter;

import org.custommonkey.xmlunit.XMLTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.ActiveGreeting;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.AttachType;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.MailFormat;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.VoicemailTuiType;

public class MailboxPreferencesTest extends XMLTestCase {
    private MailboxPreferencesWriter m_writer;

    protected void setUp() {
        m_writer = new MailboxPreferencesWriter();
        m_writer.setVelocityEngine(TestHelper.getVelocityEngine());
    }

    public void testGetValueOfById() {
        MailboxPreferences.ActiveGreeting actual = MailboxPreferences.ActiveGreeting.fromId("none");
        assertSame(MailboxPreferences.ActiveGreeting.NONE, actual);
    }

    public void testFromUser() {
        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        User user = new User();
        user.setPermissionManager(pm);
        user.setEmailAddress("first@example.com");
        user.setAlternateEmailAddress("second@example.com");
        user.setSettingValue(IMAP_HOST, "imap.host.exampl.com");
        user.setSettingValue(IMAP_PASSWORD, "4321");

        MailboxPreferences mailboxPrefs = new MailboxPreferences(user);

        assertEquals("first@example.com", mailboxPrefs.getEmailAddress());
        assertEquals("second@example.com", mailboxPrefs.getAlternateEmailAddress());
        assertEquals(MailboxPreferences.AttachType.NO, mailboxPrefs.getAttachVoicemailToEmail());
        assertEquals("imap.host.exampl.com", mailboxPrefs.getImapHost());
        assertEquals("143", mailboxPrefs.getImapPort());
        assertFalse(mailboxPrefs.getImapTLS());
        assertEquals("4321", mailboxPrefs.getImapPassword());
    }

    public void testUpdateUser() {
        MailboxPreferences mailboxPrefs = new MailboxPreferences();
        mailboxPrefs.setEmailAddress("first@example.com");
        mailboxPrefs.setAlternateEmailAddress("second@example.com");
        mailboxPrefs.setAttachVoicemailToEmail(MailboxPreferences.AttachType.NO);
        mailboxPrefs.setImapHost("imap.host.exampl.com");
        mailboxPrefs.setImapPort("143");
        mailboxPrefs.setImapTLS(true);
        mailboxPrefs.setImapPassword("4321");
        mailboxPrefs.setActiveGreeting(ActiveGreeting.EXTENDED_ABSENCE);

        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());
        User user = new User();
        user.setPermissionManager(pm);
        mailboxPrefs.updateUser(user);

        assertEquals("first@example.com", user.getEmailAddress());
        assertEquals("second@example.com", user.getAlternateEmailAddress());
        assertEquals(MailboxPreferences.AttachType.NO.getValue(), user.getSettingValue(PRIMARY_EMAIL_NOTIFICATION));
        assertEquals("imap.host.exampl.com", user.getSettingValue(IMAP_HOST));
        assertEquals("143", user.getSettingValue(IMAP_PORT));
        assertTrue((Boolean) user.getSettingTypedValue(IMAP_TLS));
        assertEquals("4321", user.getSettingValue(IMAP_PASSWORD));
        assertEquals(ActiveGreeting.EXTENDED_ABSENCE.getId(), user.getSettingValue(ACTIVE_GREETING));
    }

    public void testUserTui() {
        MailboxPreferences mailboxPrefs = new MailboxPreferences();
        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());
        User user = new User();
        user.setPermissionManager(pm);

        mailboxPrefs.setBusyPrompt("true");
        mailboxPrefs.updateUser(user);
        assertEquals(VoicemailTuiType.STANDARD.getValue(), user.getSettingValue(VOICEMAIL_TUI));
        assertEquals("true", user.getSettingValue(BUSY_PROMPT));

        mailboxPrefs.setBusyPrompt("false");
        mailboxPrefs.setVoicemailTui(VoicemailTuiType.CALLPILOT);
        mailboxPrefs.updateUser(user);
        assertEquals(VoicemailTuiType.CALLPILOT.getValue(), user.getSettingValue(VOICEMAIL_TUI));
        assertEquals("false", user.getSettingValue(BUSY_PROMPT));
    }

    public void testWritePreferences() throws Exception {
        StringWriter actual = new StringWriter();
        MailboxPreferences prefs = new MailboxPreferences();
        prefs.setActiveGreeting(ActiveGreeting.EXTENDED_ABSENCE);

        m_writer.writeObject(prefs, actual);
        InputStream expectedIn = getClass().getResourceAsStream("expected-mailboxprefs.xml");
        assertXMLEqual(new InputStreamReader(expectedIn), new StringReader(actual.toString()));
    }
}
