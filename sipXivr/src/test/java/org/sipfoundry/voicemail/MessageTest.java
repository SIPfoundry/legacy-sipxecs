/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.voicemail;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;

import javax.sound.sampled.AudioFileFormat;
import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;

import junit.framework.TestCase;

import org.apache.commons.io.FileUtils;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.sipxivr.Mailbox;
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
        m_mailstore = new File(m_testdir.getPath()+"/mailstore/") ;
        m_mailstore.mkdirs();
        m_mailbox = new Mailbox(m_user, m_mailstore.getPath());
        Mailbox.createDirsIfNeeded(m_mailbox);
        Mwi.setJustTesting(true);
        Emailer.setJustTesting(true);
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
        
        String messageId1 = VmMessage.nextMessageId(m_mailstore.getPath());
        String messageId2 = VmMessage.nextMessageId(m_mailstore.getPath());
        assertEquals("00000001", messageId1);
        assertEquals("00000002", messageId2);
    }

    private void makeWaves(File wavFile, byte filler, int length) throws IOException {
        byte[] fill = new byte[length];
        for(int i=0; i<length; i++) {
            fill[i] = filler;
        }
        AudioInputStream ais = new AudioInputStream(new ByteArrayInputStream(fill), 
                new AudioFormat(8000,16, 1, true, false), fill.length);
        AudioSystem.write(ais, AudioFileFormat.Type.WAVE, wavFile);
    }

    public void testMessage() throws IOException {
        File wavFile = new File(m_testdir.getPath()+"fake.wav");
        makeWaves(wavFile, (byte)0, 8000);
        Message message = Message.newMessage(m_mailbox, wavFile, 
                "\"Me\" <woof@pingtel.com>", Priority.NORMAL, null);
        message.storeInInbox();
        assertEquals("00000001", message.getMessageId());
        assertEquals(m_mailbox.getInboxDirectory()+"00000001-00.wav", message.getWavPath());
        assertTrue("mailstore/woof/inbox/00000001-00.wav doesn't exist!", message.getWavFile().exists());
    }


}
