/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.voicemail;

import java.io.File;
import java.io.IOException;

import junit.framework.TestCase;

import org.apache.commons.io.FileUtils;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.User;
import org.sipfoundry.voicemail.MessageDescriptor.Priority;

public class MessageTest extends TestCase {
    User m_user;
    File m_testdir;
    File m_mailstore;
    Mailbox m_mailbox;
    
    protected void setUp() throws Exception {
        super.setUp();
        m_user = new User();
        m_user.setUserName("woof");
        m_user.setDisplayName("Fuzzy Puppy");
        m_user.setIdentity("woof@pingtel.com");
        m_user.setUri("\"Fuzzy Puppy\" <woof@pingtel.com>");
        m_testdir = new File("/tmp/MessageTest/");
        if (m_testdir.isDirectory()) {
            FileUtils.forceDelete(m_testdir);
        }
        m_mailstore = new File(m_testdir.getPath()+"/mailstore") ;
        m_mailstore.mkdirs();
        m_mailbox = new Mailbox(m_user, m_mailstore.getPath());
        Mailbox.createDirsIfNeeded(m_mailbox);

    }

    protected void tearDown() throws Exception {
        super.tearDown();
        if (m_testdir.isDirectory()) {
            FileUtils.forceDelete(m_testdir);
        }
    }

    public void testNextMessageId() throws IOException {
        assertTrue("mailstore/woof doesn't exist!", 
                new File(m_mailstore.getPath()+"/woof").isDirectory());
        assertTrue("mailstore/woof/inbox doesn't exist!", 
                new File(m_mailstore.getPath()+"/woof/inbox").isDirectory());
        
        String messageId1 = Message.nextMessageId(m_mailbox);
        String messageId2 = Message.nextMessageId(m_mailbox);
        assertEquals("00000001", messageId1);
        assertEquals("00000002", messageId2);
    }

    public void testMessage() throws IOException {
        File wavFile = new File(m_testdir.getPath()+"fake.wav");
        FileUtils.touch(wavFile);
        Message message = new Message(m_mailbox, wavFile.getPath(), 
                "\"Me\" <woof@pingtel.com>", Priority.NORMAL);
        message.storeInInbox();
        assertEquals(message.getMessageId(), "00000001");
        assertEquals(m_mailbox.getInboxDirectory()+"00000001-00.wav", message.getWavName());
        assertTrue("mailstore/woof/inbox/00000001-00.wav doesn't exist!",
                new File(message.getWavName()).exists());
    }


}
