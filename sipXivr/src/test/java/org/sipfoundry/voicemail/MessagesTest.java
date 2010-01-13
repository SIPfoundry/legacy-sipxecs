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
import java.util.HashMap;
import java.util.List;
import java.util.Properties;

import javax.sound.sampled.AudioFileFormat;
import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;

import junit.framework.TestCase;

import org.apache.commons.io.FileUtils;
import org.apache.log4j.PropertyConfigurator;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.voicemail.MessageDescriptor.Priority;

public class MessagesTest extends TestCase {
    File m_testDir;
    File m_mailstoreDir;
    File m_userDir;
    File m_inboxDir;
    File m_savedDir;
    File m_deletedDir;

    MessageDescriptorWriter m_mw;
    MessageDescriptor m_md;
    
    protected void setUp() throws Exception {
        super.setUp();
        // Configure log4j
        Properties props = new Properties();
        props.setProperty("log4j.rootLogger", "debug, cons");
        props.setProperty("log4j.appender.cons", "org.apache.log4j.ConsoleAppender");
        props.setProperty("log4j.appender.cons.layout", "org.sipfoundry.commons.log4j.SipFoundryLayout");
        props.setProperty("log4j.appender.cons.layout.facility", "sipXivr");

        PropertyConfigurator.configure(props);

        m_testDir = new File("/tmp/MessagesTest/");
        if (m_testDir.isDirectory()) {
            FileUtils.forceDelete(m_testDir);
        }
        m_testDir.mkdir();
        m_mailstoreDir = new File(m_testDir, "mailstore");
        m_mailstoreDir.mkdir();
        m_userDir = new File(m_mailstoreDir, "user"); m_userDir.mkdir();
        m_inboxDir = new File(m_userDir, "inbox"); m_inboxDir.mkdir();
        m_savedDir = new File(m_userDir, "saved"); m_savedDir.mkdir();
        m_deletedDir = new File(m_userDir, "deleted"); m_deletedDir.mkdir();

        // Tell MWI & email not to bother contacting an external server
        Mwi.setJustTesting(true);
        Emailer.setJustTesting(true);
        
        m_mw = new MessageDescriptorWriter();
        m_md = new MessageDescriptor();
        m_md.setId("woof");
        m_md.setFromUri("user@dog");
        m_md.setDurationSecs(42);
        m_md.setTimestamp(System.currentTimeMillis());
        m_md.setSubject("Voice Message");
        m_md.setPriority(Priority.NORMAL);

    }

    protected void tearDown() throws Exception {
        super.tearDown();
        if (m_testDir.isDirectory()) {
            FileUtils.forceDelete(m_testDir);
        }
    }

    protected void makeMd(File file) {
        m_mw.writeObject(m_md, file);
    }
    
    public void testMultiAccess() {
        User user = new User();
        user.setUserName("user");
        user.setIdentity("user@dog");

        Mailbox mbox = new Mailbox(user, m_mailstoreDir.getPath());


        Messages m1 = Messages.newMessages(mbox);
        Messages m2 = Messages.newMessages(mbox);
        assertSame(m1, m2);
        Messages.releaseMessages(m2);
        Messages m3 = Messages.newMessages(mbox);
        assertSame(m1, m3);
        Messages.releaseMessages(m3);
        Messages.releaseMessages(m1);
        Messages m4 = Messages.newMessages(mbox);
        assertNotSame(m1, m4);
    }
    
    public void testLoadFolder() throws IOException, InterruptedException {
        Messages m = new Messages();
        m.loadFolder(m_inboxDir, m.m_inbox, true, null);
        assertEquals(0, m.getInboxCount());

        FileUtils.touch(new File(m_inboxDir, "0001-00.wav"));
        makeMd(new File(m_inboxDir, "0001-00.xml"));
        FileUtils.touch(new File(m_inboxDir, "0002-00.wav"));
        makeMd(new File(m_inboxDir, "0002-00.xml"));
        FileUtils.touch(new File(m_inboxDir, "0003-00.wav"));
        makeMd(new File(m_inboxDir, "0003-00.xml"));
        FileUtils.touch(new File(m_inboxDir, "0003-00.sta"));
        FileUtils.touch(new File(m_inboxDir, "0004-00.wav"));
        makeMd(new File(m_inboxDir, "0004-00.xml"));
        FileUtils.touch(new File(m_inboxDir, "0004-00.sta"));
        
        FileUtils.touch(new File(m_inboxDir, "0005-00.dog")); // .dog doesn't count
        makeMd(new File(m_inboxDir, "0006-00.xml")); // .xml with no .wav, doesn't count.
        FileUtils.touch(new File(m_inboxDir, "0007-00.wav")); // .wav with no .xml, doesn't count.
        
        makeMd(new File(m_inboxDir, "0008-00.xml"));
        FileUtils.touch(new File(m_inboxDir, "0008-00.wav"));
        FileUtils.touch(new File(m_inboxDir, "0008-00.sta"));
        makeMd(new File(m_inboxDir, "0008-01.xml"));
        FileUtils.touch(new File(m_inboxDir, "0008-01.wav"));
        FileUtils.touch(new File(m_inboxDir, "0008-FW.wav"));

        FileUtils.touch(new File(m_inboxDir, "hey there"));
        FileUtils.touch(new File(m_inboxDir, "this.is.a.file"));

        m.loadFolder(m_inboxDir, m.m_inbox, true, null);
        assertEquals(5, m.getInboxCount());
        assertEquals(3, m.getUnheardCount());

        m.loadFolder(m_inboxDir, m.m_saved, false, null);
        assertEquals(5, m.getSavedCount());
        assertEquals(3, m.getUnheardCount());

    }
 
    public void testGetInbox() {
        Messages m = new Messages();
        m.m_inbox = new HashMap<String, VmMessage>();
        long timestamp = System.currentTimeMillis();
        String id;
        VmMessage m1 = VmMessage.testMessage(id = "woof", false, timestamp);
        m.m_inbox.put(id, m1);

        VmMessage m2 = VmMessage.testMessage(id = "dog", false, timestamp+1000);
        m.m_inbox.put(id, m2);

        VmMessage m3 = VmMessage.testMessage(id = "cat", false, timestamp+2000);
        m.m_inbox.put(id, m3);

        VmMessage m4 = VmMessage.testMessage(id = "firstHeard", false, timestamp-1000);
        m.m_inbox.put(id, m4);

        List<VmMessage> l = m.getInbox();
        assertEquals("firstHeard", l.get(0).getMessageId());
        assertEquals("woof", l.get(1).getMessageId());
        assertEquals("dog", l.get(2).getMessageId());
        assertEquals("cat", l.get(3).getMessageId());

        VmMessage m5 = VmMessage.testMessage(id = "firstUnheard", true, timestamp+3000);
        m.m_inbox.put(id, m5);

        VmMessage m6 = VmMessage.testMessage(id = "lastUnheard", true, timestamp+5000);
        m.m_inbox.put(id, m6);

        VmMessage m7 = VmMessage.testMessage(id = "nextUnheard", true, timestamp+4000);
        m.m_inbox.put(id, m7);

        l = m.getInbox();
        assertEquals("firstUnheard", l.get(0).getMessageId());
        assertEquals("nextUnheard", l.get(1).getMessageId());
        assertEquals("lastUnheard", l.get(2).getMessageId());
        assertEquals("firstHeard", l.get(3).getMessageId());
        assertEquals("woof", l.get(4).getMessageId());
        assertEquals("dog", l.get(5).getMessageId());
        assertEquals("cat", l.get(6).getMessageId());
    }


    public void testSaveDeletePurge() throws IOException, InterruptedException {
        User user = new User();
        user.setUserName("user");
        user.setIdentity("user@dog");

        Mailbox mbox = new Mailbox(user, m_mailstoreDir.getPath());

        FileUtils.touch(new File(m_inboxDir, "0001-00.wav"));
        makeMd(new File(m_inboxDir, "0001-00.xml"));
        FileUtils.touch(new File(m_inboxDir, "0001-00.sta"));
        FileUtils.touch(new File(m_inboxDir, "0001-01.wav"));
        makeMd(new File(m_inboxDir, "0001-01.xml"));
        FileUtils.touch(new File(m_inboxDir, "0001-FW.wav"));
        FileUtils.touch(new File(m_inboxDir, "0002-00.wav"));
        makeMd(new File(m_inboxDir, "0002-00.xml"));

        Messages m = Messages.newMessages(mbox);
        m.loadFolder(m_inboxDir, m.m_inbox, true, null);
        m.loadFolder(m_savedDir, m.m_saved, false, null);
        m.loadFolder(m_deletedDir, m.m_deleted, false, null);
        
        assertEquals(2, m.getInboxCount());
        assertEquals(1, m.getUnheardCount());
        assertEquals(0, m.getSavedCount());
        assertEquals(0, m.getDeletedCount());
        
        // Find 0001
        VmMessage msg=null;
        for (VmMessage msg1 : m.getInbox()) {
            if (msg1.getMessageId().equals("0001")) {
                msg = msg1;
                break;
            }
        }

        // Mark 0001 as heard
        m.markMessageHeard(msg, true);
        // Prove it 
        assertEquals(2, m.getInboxCount());
        assertEquals(0, m.getUnheardCount());
        assertEquals(0, m.getSavedCount());
        assertEquals(0, m.getDeletedCount());
        assertFalse(new File(m_inboxDir, "0001-00.sta").exists());
        
        // Reload and make sure we get the same result
        Messages.releaseMessages(m);
        m = Messages.newMessages(mbox);
        m.loadFolder(m_inboxDir, m.m_inbox, true, null);
        m.loadFolder(m_savedDir, m.m_saved, false, null);
        m.loadFolder(m_deletedDir, m.m_deleted, false, null);

        assertEquals(2, m.getInboxCount());
        assertEquals(0, m.getUnheardCount());
        assertEquals(0, m.getSavedCount());
        assertEquals(0, m.getDeletedCount());

        // Find 0001 (again)
        msg=null;
        for (VmMessage msg1 : m.getInbox()) {
            if (msg1.getMessageId().equals("0001")) {
                msg = msg1;
                break;
            }
        }

        // Save the message
        m.saveMessage(msg);
        assertEquals(1, m.getInboxCount());
        assertEquals(0, m.getUnheardCount());
        assertEquals(1, m.getSavedCount());
        assertEquals(0, m.getDeletedCount());
        assertFalse(new File(m_inboxDir, "0001-00.wav").exists());
        assertFalse(new File(m_inboxDir, "0001-00.xml").exists());
        assertFalse(new File(m_inboxDir, "0001-00.sta").exists());
        assertFalse(new File(m_inboxDir, "0001-01.wav").exists());
        assertFalse(new File(m_inboxDir, "0001-01.xml").exists());
        assertFalse(new File(m_inboxDir, "0001-FW.wav").exists());

        File xml = new File(m_savedDir, "0001-00.xml");
        File wav = new File(m_savedDir, "0001-FW.wav");
        assertEquals(xml.getPath(), msg.m_descriptorFile.getPath());
        assertEquals(wav.getPath(), msg.getAudioFile().getPath());
        
        assertTrue(xml.exists());
        assertTrue(wav.exists());
        
        // Delete the message
        m.deleteMessage(msg);
        assertEquals(1, m.getInboxCount());
        assertEquals(0, m.getUnheardCount());
        assertEquals(0, m.getSavedCount());
        assertEquals(1, m.getDeletedCount());
        assertFalse(new File(m_savedDir, "0001-00.xml").exists());
        assertFalse(new File(m_savedDir, "0001-FW.wav").exists());

        xml = new File(m_deletedDir, "0001-00.xml");
        wav = new File(m_deletedDir, "0001-FW.wav");
        assertEquals(xml.getPath(), msg.m_descriptorFile.getPath());
        assertEquals(wav.getPath(), msg.getAudioFile().getPath());
        
        assertTrue(xml.exists());
        assertTrue(wav.exists());

        // Revive the message
        m.saveMessage(msg);
        assertEquals(2, m.getInboxCount());
        assertEquals(0, m.getUnheardCount());
        assertEquals(0, m.getSavedCount());
        assertEquals(0, m.getDeletedCount());
        assertFalse(new File(m_deletedDir, "0001-00.xml").exists());
        assertFalse(new File(m_deletedDir, "0001-FW.wav").exists());

        assertTrue(new File(m_inboxDir, "0001-00.xml").exists());
        assertTrue(new File(m_inboxDir, "0001-FW.wav").exists());

        // Delete the message
        m.deleteMessage(msg);
        // Purge the message
        m.deleteMessage(msg);
        assertEquals(1, m.getInboxCount());
        assertEquals(0, m.getUnheardCount());
        assertEquals(0, m.getSavedCount());
        assertEquals(0, m.getDeletedCount());
        assertFalse("File shouldn't exist", new File(m_deletedDir, "0001-00.xml").exists());
        assertFalse("File shouldn't exist", new File(m_deletedDir, "0001-FW.wav").exists());
        Messages.releaseMessages(m);
    }

    public void testNewMessage() throws IOException {
        User user = new User();
        user.setUserName("user");
        user.setIdentity("user@dog");

        Mailbox mbox = new Mailbox(user, m_mailstoreDir.getPath());
        File tempFile = new File(m_testDir, "temp.wav");
        makeWaves(tempFile, (byte)0, 4);
        Message m = Message.newMessage(mbox, tempFile, "woof@dog", Priority.NORMAL, null);
        m.storeInInbox();
        assertFalse("temp file was not deleted", tempFile.exists());
        VmMessage vm = m.getVmMessage();
        assertTrue("vmMessage wasn't created", vm != null);
        assertTrue("Audio File not created", vm.m_audioFile.exists());
        assertTrue("Descriptor File not created", vm.m_descriptorFile.exists());
        assertTrue("Status File not created", vm.m_statusFile.exists());
        assertTrue("Descriptor File not created", vm.m_descriptorFile.exists());
    }
    
    public void testCopyMessage() throws IOException {
        User user = new User();
        user.setUserName("user");
        user.setIdentity("user@dog");
        
        Mailbox mbox = new Mailbox(user, m_mailstoreDir.getPath());
        File tempFile = new File(m_testDir, "temp.wav");
        makeWaves(tempFile, (byte)0, 4);
        Message m = Message.newMessage(mbox, tempFile, "woof@dog", Priority.NORMAL, null);
        m.storeInInbox();
        assertFalse("temp file was not deleted", tempFile.exists());
        VmMessage vm = m.getVmMessage();
        VmMessage vm2 = vm.copy(mbox);
        assertFalse("Message ID didn't change", vm.getMessageId().equals(vm2.getMessageId()));
        assertTrue("vmMessage wasn't created", vm2 != null);
        assertTrue("Audio File not created", vm2.m_audioFile.exists());
        assertTrue("Descriptor File not created", vm2.m_descriptorFile.exists());
        assertTrue("Status File not created", vm2.m_statusFile.exists());
        assertFalse("Subject didn't change", vm.m_messageDescriptor.getSubject().equals(vm2.m_messageDescriptor.getSubject()));
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

    public void testConcatAudio() throws Exception {
        File first = new File(m_testDir, "first.wav");
        makeWaves(first, (byte)0, 4);
        File second = new File(m_testDir, "second.wav");
        makeWaves(second, (byte)-1, 4);
        
        File combined = new File(m_testDir, "combined.wav");
        VmMessage.concatAudio(combined, first, second);
        assertTrue("combined.wav doesn't exist", combined.exists());
        
        AudioInputStream ais = AudioSystem.getAudioInputStream(combined);
        byte [] octets = new byte[42];
        int len = ais.read(octets);
        assertEquals(8, len);
        assertEquals(0, octets[0]);
        assertEquals(-1, octets[4]);
    }

    public void testForwardMessage() throws IOException {
        User user = new User();
        user.setUserName("user");
        user.setIdentity("user@dog");
        
        Mailbox mbox = new Mailbox(user, m_mailstoreDir.getPath());
        File tempFile = new File(m_testDir, "temp.wav");
        makeWaves(tempFile, (byte)0, 42);
        
        Message m = Message.newMessage(mbox, tempFile, "woof@dog", Priority.NORMAL, null);
        m.storeInInbox();
        assertFalse("temp file was not deleted", tempFile.exists());
        VmMessage vm = m.getVmMessage();
        
        File fwdWavFile = new File(m_testDir, "fwd.wav");
        makeWaves(fwdWavFile, (byte)-1, 42);
        Message m2 = Message.newMessage(mbox, fwdWavFile, "knight@dog", Priority.NORMAL, null);
        VmMessage vm2 = vm.forward(mbox, m2);
        assertFalse("Message ID didn't change", vm.getMessageId().equals(vm2.getMessageId()));
        assertTrue("vmMessage wasn't created", vm2 != null);
        assertTrue("comments Audio File not created", vm2.m_audioFile.exists());
        assertTrue("Descriptor File not created", vm2.m_descriptorFile.exists());
        assertTrue("Status File not created", vm2.m_statusFile.exists());
        assertTrue("Orig Descriptor File not created", vm2.m_originalDescriptorFile.exists());
        assertTrue("Orig Audio File not created", vm2.m_originalAudioFile.exists());
        assertTrue("Combined audio File not created", vm2.m_combinedAudioFile.exists());
        assertFalse("Subject didn't change", vm.m_messageDescriptor.getSubject().equals(vm2.m_messageDescriptor.getSubject()));
        
        File comment = new File(m_testDir, "comment.wav");
        makeWaves(comment, (byte)-1, 42);
        Message m3 = Message.newMessage(mbox, comment, "knight@dog", Priority.NORMAL, null);
        VmMessage vm3 = vm.forward(mbox, m3);
        assertTrue("Combined audio File not created", vm3.m_combinedAudioFile.exists());
    }
}
