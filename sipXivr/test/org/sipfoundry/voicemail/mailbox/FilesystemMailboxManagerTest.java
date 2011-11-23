package org.sipfoundry.voicemail.mailbox;

import java.io.File;
import java.io.IOException;

import junit.framework.TestCase;

import org.apache.commons.io.FileUtils;

public class FilesystemMailboxManagerTest extends TestCase {
    FilesystemMailboxManager m_mailboxManager;

    protected void setUp() throws Exception {
        super.setUp();
        m_mailboxManager = new FilesystemMailboxManager();
        m_mailboxManager.setMailstoreDirectory("/tmp/mailbox/");
        m_mailboxManager.init();
        createUnHeardMessage("201", "01");
        createHeardMessage("201", "02");
        createHeardMessage("201", "03");
        createSavedMessage("201", "04");
        createDeletedMessage("201", "05");
        createHeardMessage("202", "06");
        createDeletedMessage("202", "07");
    }

    protected void tearDown() throws Exception {
        super.tearDown();
        File mailstore = new File("/tmp/mailbox/");
        if (mailstore.isDirectory()) {
            FileUtils.forceDelete(mailstore);
        }
    }

    public void testInit() {
        assertTrue(new File("/tmp/mailbox/").exists());
    }

    public void testGetMailboxDetails() throws IOException {
        MailboxDetails details = m_mailboxManager.getMailboxDetails("200");
        assertEquals(0, details.getInboxCount());
        assertEquals(0, details.getUnheardCount());
        assertEquals(0, details.getHeardCount());
        assertEquals(0, details.getSavedCount());
        assertEquals(0, details.getConferencesCount());
        assertEquals(0, details.getSavedCount());

        details = m_mailboxManager.getMailboxDetails("201");
        assertEquals(3, details.getInboxCount());
        assertEquals(1, details.getUnheardCount());
        assertEquals(2, details.getHeardCount());
        assertEquals(1, details.getSavedCount());
        assertEquals(1, details.getDeletedCount());
        assertTrue(details.getInbox().contains("00000001"));
        assertTrue(details.getInbox().contains("00000002"));
        assertTrue(details.getInbox().contains("00000003"));
        assertTrue(details.getSaved().contains("00000004"));
        assertTrue(details.getDeleted().contains("00000005"));
    }

    private void createUnHeardMessage(String username, String id) throws IOException {
        FileUtils.touch(new File (String.format("/tmp/mailbox/%s/%s/000000%s-00.sta", username, Folder.INBOX, id)));
        FileUtils.touch(new File (String.format("/tmp/mailbox/%s/%s/000000%s-00.wav", username, Folder.INBOX, id)));
        FileUtils.touch(new File (String.format("/tmp/mailbox/%s/%s/000000%s-00.xml", username, Folder.INBOX, id)));
    }

    private void createHeardMessage(String username, String id) throws IOException {
        FileUtils.touch(new File (String.format("/tmp/mailbox/%s/%s/000000%s-00.wav", username, Folder.INBOX, id)));
        FileUtils.touch(new File (String.format("/tmp/mailbox/%s/%s/000000%s-00.xml", username, Folder.INBOX, id)));
    }

    private void createSavedMessage(String username, String id) throws IOException {
        FileUtils.touch(new File (String.format("/tmp/mailbox/%s/%s/000000%s-00.wav", username, Folder.SAVED, id)));
        FileUtils.touch(new File (String.format("/tmp/mailbox/%s/%s/000000%s-00.xml", username, Folder.SAVED, id)));
    }

    private void createDeletedMessage(String username, String id) throws IOException {
        FileUtils.touch(new File (String.format("/tmp/mailbox/%s/%s/000000%s-00.wav", username, Folder.DELETED, id)));
        FileUtils.touch(new File (String.format("/tmp/mailbox/%s/%s/000000%s-00.xml", username, Folder.DELETED, id)));
    }

}
