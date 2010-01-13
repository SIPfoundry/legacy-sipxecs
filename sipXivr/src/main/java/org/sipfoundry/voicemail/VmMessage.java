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
import java.io.SequenceInputStream;
import java.util.Vector;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.mail.MessagingException;
import javax.mail.Flags.Flag;
import javax.sound.sampled.AudioFileFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;

import org.apache.commons.io.FileUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.voicemail.MessageDescriptor.Priority;

import com.sun.mail.imap.IMAPMessage;

/**
 * Represents the message in a mailbox
 */
public class VmMessage {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    static Pattern namePattern = Pattern.compile("^(\\d+)-(00|01|FW)\\.(xml|wav|sta)$");

    String m_messageId; // A machine wide unique incremented "number"
    File m_audioFile;
    File m_originalAudioFile;
    File m_combinedAudioFile;
    File m_descriptorFile;
    File m_originalDescriptorFile;
    File m_statusFile;
    File m_urgentFile;
    MessageDescriptor m_messageDescriptor;
    boolean m_unHeard ;
    boolean m_urgent;
    
    /**
     * Private constructor for factory method to call
     */
    private VmMessage() {
        
    }
    
    /**
     * LEGEND INTO FILE DIRECTORY ========================================= 
     * For a message in a voicemail directory
     *
     * Message id = "00000018"
     * 
     * 00000018-00.sta zero length file, exists only if message is unheard
     *
     * 00000018-00.wav if forwarded represent the comment, NOTE: can be zero if no comment was left
     * else voicemail message media
     *
     * 00000018-00.xml message or comment details
     *
     * 00000018-01.wav original message without comment NOTE: only exists for forwarded messages
     *
     * 00000018-01.xml original message details NOTE: only exists for forwarded messages
     *
     * 00000018-FW.wav original message plus comment NOTE: only exists for forwarded messages
     *
     */
    
    /**
     * Factory method to Load a message with the existing files from the directory
     * @param directory
     * @param id
     */
    public static VmMessage loadMessage(File directory, String id) {
        VmMessage me = new VmMessage();
        me.m_messageId = id;
        me.m_descriptorFile = new File(directory, id+"-00.xml");
        if (!me.m_descriptorFile.exists()) {
            LOG.error(String.format("Voice message descriptor file %s does not exist",
                    me.m_descriptorFile.getPath()));
            return null;
        }

        // Check for -00.wav
        File audio = new File(directory, id+"-00.wav");
        if (!audio.exists()) {
            LOG.error(String.format("Voice message descriptor file %s exists with no corresponding audio file",
                    me.m_descriptorFile.getPath()));
            return null;
        }
        
        me.m_audioFile = audio;

        // Check for -01.wav
        File audio1 = new File(directory, id+"-01.wav");
        if (audio1.exists()) {
            me.m_originalAudioFile = audio1;
        }

        // Check for -01.xml
        File descriptor1 = new File(directory, id+"-01.xml");
        if (descriptor1.exists()) {
            me.m_originalDescriptorFile = descriptor1;
        }
        
        // See if -FW.wav exists 
        File audioFw = new File(directory, id+"-FW.wav");
        if (audioFw.exists()) {
            me.m_combinedAudioFile = audioFw;
        }
                 
        // Check for -00.sta
        File status = new File(directory, id+"-00.sta");
        me.m_statusFile = status;
        if (status.exists()) {
            me.m_unHeard = true ;
        } 

        // Check for -00.urg
        File urgent = new File(directory, id+"-00.urg");
        me.m_urgentFile = urgent;
        if (urgent.exists()) {
            me.m_urgent = true ;
        } 
        
        return me;
    }

    public static VmMessage newMessageFromMime(Mailbox mailbox, IMAPMessage msg) {
        VmMessage me = new VmMessage();

        // Generate the next message ID
        me.m_messageId = ExtMailStore.GetMsgId(msg);
        if (me.m_messageId == null) {
            me.m_messageId = nextMessageId(mailbox.getMailstoreDirectory()+"/..");
        }

        // Generate the MessageDescriptor;
        me.m_messageDescriptor = new MessageDescriptor();
        me.m_messageDescriptor.setId(mailbox.getUser().getIdentity());
        //me.m_messageDescriptor.setFromUri(msg.getf);

        try {
            me.m_messageDescriptor.setDurationSecs(msg.getSize());
            me.m_unHeard = !msg.isSet(Flag.SEEN);
            me.m_messageDescriptor.setTimestamp(msg.getReceivedDate().toString());
            me.m_messageDescriptor.setSubject(msg.getSubject());

            Priority pr = Priority.NORMAL;

            if (msg.isSet(Flag.FLAGGED)) {
                pr = Priority.URGENT;
                me.m_urgent = true;

            }
            me.m_messageDescriptor.setPriority(pr);

        } catch (MessagingException e1) {
        // TODO Auto-generated catch block
            e1.printStackTrace();
        }

        String baseName = mailbox.getInboxDirectory()+me.m_messageId+"-00";
        me.m_audioFile = new File(baseName+".wav");
        me.m_descriptorFile = new File(baseName+".xml");
        me.m_statusFile = new File(baseName+".sta");
        me.m_urgentFile = new File(baseName+".urg");
        String operation= "storing stuff";
        try {
            if(me.m_unHeard) {
                operation = "creating status file "+me.m_statusFile.getPath();
                LOG.debug("VmMessage::newMessage "+operation);
                FileUtils.touch(me.m_statusFile);
            }
            
            if(me.m_urgent) {
                operation = "creating urgent file "+ me.m_urgentFile.getPath();
                LOG.debug("VmMessage::newMessage "+ operation);
                FileUtils.touch(me.m_urgentFile);
            }
            
            operation = "creating messageDescriptor "+me.m_descriptorFile.getPath();
            LOG.debug("VmMessage::newMessage "+operation);
            new MessageDescriptorWriter().writeObject(me.m_messageDescriptor, me.m_descriptorFile);
        } catch (IOException e) {
            LOG.error("VmMessage::newMessage error while "+operation, e);
            return null;
        }
        LOG.info("VmMessage::newMessage created message "+me.m_descriptorFile.getPath());
        if(me.m_unHeard) {
            Mwi.sendMWI(mailbox);
        }

        return me;
    }

    
    public static VmMessage newMessage(Mailbox mailbox, Message recording) {
        VmMessage me = new VmMessage();
        
        // Generate the next message ID
        me.m_messageId = nextMessageId(mailbox.getMailstoreDirectory()+"/..");
        me.m_unHeard = true;

        // Generate the MessageDescriptor;
        me.m_messageDescriptor = new MessageDescriptor();
        me.m_messageDescriptor.setId(mailbox.getUser().getIdentity());
        me.m_messageDescriptor.setFromUri(recording.getFromUri());
        me.m_messageDescriptor.setDurationSecs(recording.getDuration());
        me.m_messageDescriptor.setTimestamp(recording.getTimestamp());
        me.m_messageDescriptor.setSubject("Voice Message "+me.m_messageId);
        me.m_messageDescriptor.setPriority(recording.getPriority());
        
        if(recording.getPriority() == Priority.URGENT) {
            me.m_urgent = true;           
        }
        
        Vector<User> otherRecipients = recording.getOtherRecipeints();
        if(otherRecipients != null) {
            for(User recipient : otherRecipients) {
                if(!recipient.getUserName().equals(mailbox.getUser().getUserName())) {
                    me.m_messageDescriptor.addOtherRecipient(recipient.getUserName());
                }
            }            
        }

        File tempFile = recording.getWavFile();
        String baseName = mailbox.getInboxDirectory()+me.m_messageId+"-00"; 
        me.m_audioFile = new File(baseName+".wav");
        me.m_descriptorFile = new File(baseName+".xml");
        me.m_statusFile = new File(baseName+".sta");
        me.m_urgentFile = new File(baseName+".urg");
        String operation= "storing stuff";
        try {
            operation = "creating status file "+me.m_statusFile.getPath();
            LOG.debug("VmMessage::newMessage "+operation);
            FileUtils.touch(me.m_statusFile);
            
            if(me.m_urgent) {
                operation = "creating urgent file "+ me.m_urgentFile.getPath();
                LOG.debug("VmMessage::newMessage "+ operation);
                FileUtils.touch(me.m_urgentFile);
            }
            
            operation = "copying recording .wav file to "+me.m_audioFile.getPath();
            LOG.debug("VmMessage::newMessage "+operation);
            FileUtils.copyFile(tempFile, me.m_audioFile, true);
            
            operation = "creating messageDescriptor "+me.m_descriptorFile.getPath();
            LOG.debug("VmMessage::newMessage "+operation);
            new MessageDescriptorWriter().writeObject(me.m_messageDescriptor, me.m_descriptorFile);
        } catch (IOException e) {
            LOG.error("VmMessage::newMessage error while "+operation, e);
            return null;
        }
        LOG.info("VmMessage::newMessage created message "+me.m_descriptorFile.getPath());
        Mwi.sendMWI(mailbox);
        me.sendToEmail(mailbox);
        
        return me;
    }
        
    /**
     * Copy this message into the inbox of a Mailbox, with a new message ID.
     * @param directory
     * @return newly copied message
     */
    public VmMessage copy(Mailbox mailbox) {
        VmMessage me = new VmMessage();
        
        // Generate the next message ID
        me.m_messageId = nextMessageId(mailbox.getMailstoreDirectory()+"/..");
        
        // Copy into the inbox of the mailbox
        File directory = new File(mailbox.getInboxDirectory());
        String baseName = mailbox.getInboxDirectory()+me.m_messageId+"-00"; 
        me.m_statusFile = new File(baseName+".sta");
        me.m_urgentFile = new File(baseName+".urg");
        
        // Copy (with the new message ID in the names) all the files
        String operation= "copying stuff";
        try {
            me.m_unHeard = true;
            operation = "creating status file "+me.m_statusFile.getPath();
            FileUtils.touch(me.m_statusFile);

            operation = "copying audio file "+m_audioFile.getPath();
            me.m_audioFile = CopyMessageFileToDirectory(m_audioFile, me.m_messageId, directory);
            if (m_originalAudioFile != null) {
                // Deal with the forwarded stuff
                operation = "copying original audio file "+m_originalAudioFile.getPath();
                me.m_originalAudioFile = CopyMessageFileToDirectory(m_originalAudioFile, me.m_messageId, directory);
                operation = "copying original descriptor file "+m_originalDescriptorFile.getPath();
                me.m_originalDescriptorFile = CopyMessageFileToDirectory(m_originalDescriptorFile, me.m_messageId, directory);
                operation = "copying combined audio file "+m_combinedAudioFile.getPath();
                me.m_combinedAudioFile = CopyMessageFileToDirectory(m_combinedAudioFile, me.m_messageId, directory);
            }
            // Including the descriptor file (which will be wrong for a moment, but it gives us the new name)
            operation = "copying descriptor file "+m_descriptorFile.getPath();
            me.m_descriptorFile = CopyMessageFileToDirectory(m_descriptorFile, me.m_messageId, directory);

        } catch (IOException e) {
            LOG.error("VmMessage::copy error while "+operation, e);
            return null;        
        }

        // Read the new (but wrong) message descriptor file
        me.m_messageDescriptor = new MessageDescriptorReader().readObject(me.m_descriptorFile);
        
        // correct the other recipient list
        if(me.m_messageDescriptor.getOtherRecipients() != null) {
            me.m_messageDescriptor.addOtherRecipient(ValidUsersXML.getUserPart(me.m_messageDescriptor.getId()));
            me.m_messageDescriptor.removeOtherRecipient(mailbox.getUser().getUserName());
        }
       
        // Correct the identity in the descriptor file to the correct user
        me.m_messageDescriptor.setId(mailbox.getUser().getIdentity());
        // Correct the subject to the correct subject
        me.m_messageDescriptor.setSubject("Voice Message "+me.m_messageId);
        // Write the new file out.
        new MessageDescriptorWriter().writeObject(me.m_messageDescriptor, me.m_descriptorFile);
        
        LOG.info("VmMessage::copy created message "+me.m_descriptorFile.getPath());
        Mwi.sendMWI(mailbox);
        me.sendToEmail(mailbox);

        return me;
    }
 
    /**
     * Forward this message (with optional recorded comments) to the inbox of a mailbox
     * 
     * @param mailbox
     * @param recording
     * @return
     */
    public VmMessage forward(Mailbox mailbox, Message recording) {
        VmMessage me = new VmMessage();
        
        
        // Generate the next message ID
        me.m_messageId = nextMessageId(mailbox.getMailstoreDirectory()+"/..");
        
        // Generate the MessageDescriptor;
        me.m_messageDescriptor = new MessageDescriptor();
        me.m_messageDescriptor.setId(mailbox.getUser().getIdentity());
        me.m_messageDescriptor.setFromUri(recording.getFromUri());
        me.m_messageDescriptor.setDurationSecs(recording.getDuration()+getDuration());
        me.m_messageDescriptor.setTimestamp(recording.getTimestamp());
        me.m_messageDescriptor.setSubject("Fwd:Voice Message "+m_messageId);
        me.m_messageDescriptor.setPriority(recording.getPriority());
        
        Vector<User> otherRecipients = recording.getOtherRecipeints();
        if(otherRecipients != null) {
            for(User recip: otherRecipients) {
                if(!recip.getUserName().equals(mailbox.getUser().getUserName())) {
                    me.m_messageDescriptor.addOtherRecipient(recip.getUserName());
                }
            }
        }

        if(recording.getPriority() == Priority.URGENT) {
            me.m_urgent = true;           
        }
        
        // Copy into the inbox of the mailbox
        File directory = new File(mailbox.getInboxDirectory());
        String baseName = mailbox.getInboxDirectory()+me.m_messageId; 
        me.m_audioFile = new File(baseName+"-00.wav");
        me.m_descriptorFile = new File(baseName+"-00.xml");
        me.m_statusFile = new File(baseName+"-00.sta");
        me.m_urgentFile = new File(baseName+"-00.urg");
        me.m_combinedAudioFile = new File(baseName+"-FW.wav");

        
        // Copy (with the new message ID in the names) all the files
        String operation= "copying stuff";
        try {
            me.m_unHeard = true;
            operation = "creating status file "+me.m_statusFile.getPath();
            FileUtils.touch(me.m_statusFile);
            
            if(me.m_urgent) {
                operation = "creating urgent file "+ me.m_urgentFile.getPath();
                LOG.debug("VmMessage::newMessage "+ operation);
                FileUtils.touch(me.m_urgentFile);
            }

            operation = "copying audio file "+m_audioFile.getPath();
            // The old audio file becomes "originalAudioFile" as NewID-01.wav
            String newName = String.format("%s-%s.%s", me.m_messageId, "01", "wav");
            me.m_originalAudioFile = new File(directory, newName);
            FileUtils.copyFile(m_audioFile, me.m_originalAudioFile, true);
            
            operation = "copying descriptor file "+m_audioFile.getPath();
            // The old descriptor file becomes "originalDescriptorFile" as NewID-01.xml
            newName = String.format("%s-%s.%s", me.m_messageId, "01", "xml");
            me.m_originalDescriptorFile = new File(directory, newName);
            FileUtils.copyFile(m_descriptorFile, me.m_originalDescriptorFile, true);
            
            // The recording (if any) becomes the audioFile as NewID-00.wav
            if (recording.getWavFile() != null) {
                File tempFile = recording.getWavFile();
                operation = "copying comments .wav file to "+me.m_audioFile.getPath();
                FileUtils.copyFile(tempFile, me.m_audioFile, true);

                // Combine the new and the old file into one bigger .wav as NewID-FW.wav
                operation = "combining "+me.m_audioFile.getPath()+" with "+me.m_originalAudioFile.getPath();
                concatAudio(me.m_combinedAudioFile, me.m_audioFile, me.m_originalAudioFile);
            } else {
                FileUtils.touch(me.m_audioFile); // Create an empty NewID-00.wav file
                // Copy the old audiofile by itself to NewID-FW.wav
                FileUtils.copyFile(me.m_originalAudioFile, me.m_combinedAudioFile, true);
            }
            
            
            operation = "creating messageDescriptor "+me.m_descriptorFile.getPath();
            new MessageDescriptorWriter().writeObject(me.m_messageDescriptor, me.m_descriptorFile);
                
        } catch (Exception e) {
            LOG.error("VmMessage::copyMessage error while "+operation, e);
            return null;        
        }
        LOG.info("VmMessage::forward created message "+me.m_descriptorFile.getPath());
        Mwi.sendMWI(mailbox);
        me.sendToEmail(mailbox);
        
        return me;

    }
    

    /**
     * Factory method for testing (no actual files involved)
     * @param id
     * @param unHeard
     * @param timestamp
     * @return
     */
    public static VmMessage testMessage(String id, boolean unHeard, long timestamp) {
        VmMessage me = new VmMessage();
        me.m_messageId = id;
        me.m_unHeard = unHeard;
        me.m_messageDescriptor = new MessageDescriptor();
        me.m_messageDescriptor.setTimestamp(timestamp);
        return me;
    }

    /**
     * Copy a file from where it is into a new directory with a new message id
     * @param file existing file
     * @param newMessageId
     * @param newDirectory
     * @throws IOException
     */
    private static File CopyMessageFileToDirectory(File file, String newMessageId, File newDirectory) throws IOException  {
        if (file != null) {
            Matcher m = namePattern.matcher(file.getName());
            if (m.matches()) {
                String newName = String.format("%s-%s.%s", newMessageId, m.group(2), m.group(3));
                File newFile = new File(newDirectory, newName);
                FileUtils.copyFile(file, newFile);
                return new File(newDirectory, newFile.getName());
            }
        }
        return null;
    }

    /**
     * Move a file from where it is into a new directory
     * @param file
     * @param newDirectory
     * @throws IOException
     */
    private static File moveFileToDirectory(File file, File newDirectory) throws IOException {
        if (file != null) {
            if (file.exists()) {
                FileUtils.moveFileToDirectory(file, newDirectory, false);
            }
            return new File(newDirectory, file.getName());
        }
        return null;
    }
    
    
    /**
     * Combine two wav files into one bigger one
     * 
     * @param newFile
     * @param orig1
     * @param orig2
     * @throws Exception
     */
    static void concatAudio(File newFile, File orig1, File orig2) throws Exception {
        String operation = "dunno";
        try {
            operation = "getting AudioInputStream from "+ orig1.getPath();
            AudioInputStream clip1 = AudioSystem.getAudioInputStream(orig1);
            operation = "getting AudioInputStream from "+ orig2.getPath();
            AudioInputStream clip2 = AudioSystem.getAudioInputStream(orig2);

            operation = "building SequnceInputStream";
            AudioInputStream concatStream = 
                    new AudioInputStream(
                        new SequenceInputStream(clip1, clip2),     
                        clip1.getFormat(), 
                        clip1.getFrameLength() + clip2.getFrameLength());

            operation = "writing SequnceInputStream to "+newFile.getPath();
            AudioSystem.write(concatStream, AudioFileFormat.Type.WAVE, newFile);
            LOG.info("VmMessage::concatAudio created combined file "+newFile.getPath());
        } catch (Exception e) {
            String trouble = "VmMessage::concatAudio Problem while "+operation;
 //           LOG.error(trouble, e);
            throw new Exception(trouble, e);
        }
    }
    
    /**
     * Moves all the files associated with this message to the specified directory
     * @param newDirectory
     */
    public void moveToDirectory(File newDirectory) {

        try {
            m_audioFile = moveFileToDirectory(m_audioFile, newDirectory);
            m_originalAudioFile = moveFileToDirectory(m_originalAudioFile, newDirectory);
            m_combinedAudioFile = moveFileToDirectory(m_combinedAudioFile, newDirectory);
            m_originalDescriptorFile = moveFileToDirectory(m_originalDescriptorFile, newDirectory);
            m_statusFile = moveFileToDirectory(m_statusFile, newDirectory);
            m_urgentFile = moveFileToDirectory(m_urgentFile, newDirectory);
            m_descriptorFile = moveFileToDirectory(m_descriptorFile, newDirectory);
        } catch (IOException e) {
            LOG.error(String.format("Failed to move message %s to directory %s", m_messageId, newDirectory.getPath()), e);
        }
    }
    
    /**
     * Send this message to the e-mail addrs specified in the mailbox
     * @param mailbox
     */
    void sendToEmail(Mailbox mailbox) {
        // Queue it in a background thread
        Emailer.queueVm2Email(mailbox, this);
    }
    
    /**
     * Generate the next message Id
     * static synchronized as it's machine wide
     * @param directory which holds the messageid.txt file
     */
    static synchronized String nextMessageId(String directory) {
        long numericMessageId = 1;
        String format = "%08d";
        String messageId = String.format(format,numericMessageId);
        
        // messageid.txt file is (hopefully) in the directory
        File midFile = new File(directory, "messageid.txt");
        String messageIdFilePath = midFile.getPath();
        if (midFile.exists()) {
            try {
                // The messageid in the file is the NEXT one
                messageId = FileUtils.readFileToString(midFile);
                numericMessageId = Long.parseLong(messageId);
            } catch (IOException e) {
                LOG.error("Message::nextMessageId cannot read "+messageIdFilePath, e);
                throw new RuntimeException(e);
            }
        }
        // Increment message id, store for another day
        numericMessageId++;
        try {
            FileUtils.writeStringToFile(midFile, String.format(format, numericMessageId));
        } catch (IOException e) {
            LOG.error("Message::nextMessageId cannot write "+messageIdFilePath, e);
            throw new RuntimeException(e);
        }
        
        return messageId;
    }

    public String getMessageId() {
        return m_messageId;
    }

    public boolean isUnHeard() {
        return m_unHeard;
    }

    public boolean isUrgent() {
        return m_urgent;
    }
    
    public void toggleUrgency() {         
        getMessageDescriptor();
        if(m_messageDescriptor.getPriority() == Priority.URGENT) {
            m_messageDescriptor.setPriority(Priority.NORMAL);
        } else { 
            m_messageDescriptor.setPriority(Priority.URGENT);
        }
        new MessageDescriptorWriter().writeObject(m_messageDescriptor, m_descriptorFile);
    }
    
    public void markNormal() { 
        getMessageDescriptor();
        m_messageDescriptor.setPriority(Priority.NORMAL);
        new MessageDescriptorWriter().writeObject(m_messageDescriptor, m_descriptorFile);
    }
    
    public void markHeard() {
        if (m_unHeard) {
            if (m_statusFile != null) {
                FileUtils.deleteQuietly(m_statusFile);
            }
            LOG.info(String.format("VmMessage::markHeard %s marked heard", m_descriptorFile.getPath()));
            m_unHeard = false;
        }
    }

    public void markUnheard() {
        if (!m_unHeard) {
            if (m_statusFile != null) {
                String operation= "update status";
                try {
                    operation = "creating status file "+m_statusFile.getPath();
                    FileUtils.touch(m_statusFile);
                    LOG.info(String.format("VmMessage::markHeard %s marked unheard", m_descriptorFile.getPath()));
                    m_unHeard = false;
                } catch (IOException e) {
                    LOG.error("VmMessage::markUnHeard error while "+operation, e);
                }
            } 
        }
    }

    public MessageDescriptor getMessageDescriptor() {
        if (m_messageDescriptor == null) {
            // Lazily load the message descriptor
            try {
                MessageDescriptorReader mr = new MessageDescriptorReader();
                m_messageDescriptor = mr.readObject(m_descriptorFile);
            } catch (Throwable t) {
                LOG.error("Somthing amiss reading message descriptor "+m_descriptorFile.getPath(), t);
                m_messageDescriptor = null;
            }
        }
        return m_messageDescriptor;
    }
    
    public long getTimestamp() {
        getMessageDescriptor();
        if (m_messageDescriptor != null) {
            return m_messageDescriptor.getTimeStampDate().getTime();
        } else {
            return 0;
        }
    }

    public long getDuration() {
        getMessageDescriptor();
        if (m_messageDescriptor != null) {
            return m_messageDescriptor.getDurationSecsLong();
        } else {
            return 0L;
        }
    }

    public File getAudioFile() {
        if (m_combinedAudioFile == null) {
            return m_audioFile;
        } else {
            return m_combinedAudioFile;
        }
    }

    public File getDescriptorFile() {
        return m_descriptorFile;
    }

}
