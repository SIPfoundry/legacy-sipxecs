package org.sipfoundry.voicemail;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;

import javax.sound.sampled.AudioFileFormat;
import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;

import org.apache.commons.io.FileUtils;
import org.sipfoundry.sipxivr.Configuration;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.User;
import org.sipfoundry.voicemail.MessageDescriptor.Priority;

import junit.framework.TestCase;

public class EmailFormatterTest extends TestCase {
    User m_user;
    File m_testdir;
    File m_mailstore;
    Mailbox m_mailbox;
    Configuration m_ivrConfig;

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
        m_ivrConfig = Configuration.getTest();
        m_ivrConfig.setConfigUrl("https://configserver/");
        Emailer.setJustTesting(true);
        if (false) {
            // Test actually sending e-mail.  Don't use normally, as I don't want to get flooded with mail!
            Emailer.init(m_ivrConfig);
            Emailer.setJustTesting(false);
            m_mailbox.getMailboxPreferences().setAttachVoicemailToEmail(true);
            m_mailbox.getMailboxPreferences().setEmailAddress("woof@iwoof.org");
        }
    }

    protected void tearDown() throws Exception {
        super.tearDown();
        if (m_testdir.isDirectory()) {
            FileUtils.forceDelete(m_testdir);
        }
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

    public void testFormats() throws IOException {
        File wavFile = new File(m_testdir.getPath()+"fake.wav");
        makeWaves(wavFile, (byte)0, 8000*60*2);  // two minutes
        Message message = Message.newMessage(m_mailbox, wavFile, 
                "\"Me\" <woof@pingtel.com>", Priority.NORMAL);
        message.storeInInbox();
        EmailFormatter eft = new EmailFormatter(m_ivrConfig, m_mailbox, message.getVmMessage());
        assertEquals("2:00 Voice Message from woof", eft.getSubject());
        assertEquals("Voicemail Notification Service <postmaster@localhost>", eft.getSender());
    }

}
