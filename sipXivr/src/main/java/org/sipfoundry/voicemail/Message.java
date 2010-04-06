/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.voicemail;

import java.io.EOFException;
import java.io.File;
import java.util.Vector;

import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;

import org.apache.commons.io.FileUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.voicemail.MessageDescriptor.Priority;

public class Message {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private Mailbox m_mailbox;
    private String m_fromUri;
    private Priority m_priority;
    private Vector<User> m_otherRecipients;
    private File m_wavFile;
    private boolean m_stored;
    private boolean m_isToBeStored;
    private long m_duration;
    private long m_timestamp;
    private VmMessage m_vmMessage; // Once converted to a VmMessage, use this
    
    public enum Reason {
        SAVED,
        TOO_SHORT,
        FAILED
    }
    
    private Message() {
        // Private constructor for internal use only
    }
    
    /**
     * Factory method to create a new message from a wav file
     * 
     * Create a new message in the mailbox from the given wav file
     * Message isn't physically stored in the inbox until storeInInbox is invoked.
     * 
     * @param mailbox
     * @param wavPath
     * @param fromUri
     * @param priority
     * @return
     */
    public static Message newMessage(Mailbox mailbox, File wavFile, String fromUri, 
                                     Priority priority, Vector<User> otherRecipients) {
        Message me = new Message();
        me.m_mailbox = mailbox;
        
        me.m_wavFile = wavFile;
        me.m_fromUri = fromUri;
        me.m_priority = priority;
        me.m_stored = false;
        me.m_isToBeStored = true;
        me.m_duration = 0;
        me.m_timestamp = System.currentTimeMillis();
        me.m_otherRecipients = otherRecipients;
        return me;
    }
        
    
    Priority getPriority() {
        return m_priority;
    }
    
    public void togglePriority() {
        if (m_priority == Priority.NORMAL) {
            m_priority = Priority.URGENT;
        } else {
            m_priority = Priority.NORMAL;
        }
    }

    public String getFromUri() {
        return m_fromUri;
    }
    
    public Vector<User> getOtherRecipeints() {
        return m_otherRecipients;
    }

    public boolean isToBeStored() {
        return m_isToBeStored;
    }
    
    public void setIsToBeStored(boolean isToBeStored) {
        m_isToBeStored = isToBeStored;
    }
    
    public void deleteTempWav() {
        if (m_wavFile != null && m_wavFile.exists()) {
            LOG.debug("Message:deleteTempWav deleting "+ m_wavFile.getPath());
            FileUtils.deleteQuietly(m_wavFile) ;
        }
    }
    
    public Reason storeInInbox() {
        // Not this one, just delete any temp file
        if (!m_isToBeStored) {
            deleteTempWav();
            return Reason.SAVED;  // Lier!
        }
        
        // Already stored
        if (m_stored) { 
            return Reason.SAVED;  // Well, it was already, so it still is!
        }
        
        // Create a new VoiceMail message out of this 
        m_vmMessage = VmMessage.newMessage(m_mailbox, this);
        if (m_vmMessage == null) {
            // Oops, something went wrong
            deleteTempWav();
            return Reason.FAILED;
        }
                
        m_stored = true;
        deleteTempWav();
        
        LOG.info("Message::storeInInbox stored messageId "+getMessageId());
        long secs = getDuration();
        if (secs == 0) {
            LOG.warn("Message::storeInInbox messageId "+getMessageId()+" has zero length");
        }
        return Reason.SAVED;
    }
    
    
    public String getMessageId() {
        if (m_vmMessage != null) {
            return m_vmMessage.getMessageId();
        } else {
            return null;
        }
    }
    
    public File getWavFile() {
        if (m_vmMessage != null) {
            return m_vmMessage.getAudioFile();
        } else {
            return m_wavFile;
        }
    }
        
    public String getWavPath() {
        return getWavFile().getPath();
    }

    public long getDuration() {
        // Calculate the duration (in seconds) from the Wav file
        File wavFile = getWavFile();
        if (wavFile != null) {
            m_duration = getDuration(wavFile);
        }
        return m_duration;
    }

    public static long getDuration(File wavFile) {
        try {
            AudioInputStream ais = AudioSystem.getAudioInputStream(wavFile);
            float secs =  ais.getFrameLength() / ais.getFormat().getFrameRate();
            return Math.round(secs); // Round up.
            } catch (EOFException e) {
                return 0;
              } 
            catch (Exception e) {
                String trouble = "Message::getDuration Problem determining duration of "+wavFile.getPath();
                LOG.error(trouble, e);
                throw new RuntimeException(trouble, e);
            }
   }

    public long getTimestamp() {
        return m_timestamp;
    }

    public VmMessage getVmMessage() {
        return m_vmMessage;
    }
    
}
