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

import org.apache.commons.io.FileUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.voicemail.MessageDescriptor.Priority;

public class Message {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private Mailbox m_mailbox;
    private String m_fromUri;
    private Priority m_priority;
    private String m_wavPath;
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
    public static Message newMessage(Mailbox mailbox, String wavPath, String fromUri, Priority priority) {
        Message me = new Message();
        me.m_mailbox = mailbox;
        
        me.m_wavPath = wavPath;
        me.m_fromUri = fromUri;
        me.m_priority = priority;
        me.m_stored = false;
        me.m_isToBeStored = true;
        me.m_duration = 42; // TODO correct duration
        me.m_timestamp = System.currentTimeMillis();
        return me;
    }
        
    
    Priority getPriority() {
        return m_priority;
    }

    public String getFromUri() {
        return m_fromUri;
    }

    public boolean isToBeStored() {
        return m_isToBeStored;
    }
    
    public void setIsToBeStored(boolean isToBeStored) {
        m_isToBeStored = isToBeStored;
    }
    
    public void deleteTempWav() {
        FileUtils.deleteQuietly(new File(m_wavPath)) ;
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
            return Reason.FAILED;
        }
        
        m_stored = true;
        deleteTempWav();
        
        LOG.info("Message::storeInInbox stored messageId "+getMessageId());
        return Reason.SAVED;
    }
    
    
    public String getMessageId() {
        if (m_vmMessage != null) {
            return m_vmMessage.getMessageId();
        } else {
            return null;
        }
    }
    
    public String getWavPath() {
        if (m_vmMessage != null) {
            return m_vmMessage.getAudioFile().getPath();
        } else {
            return m_wavPath;
        }
    }

    public long getDuration() {
        return m_duration;
    }

    public long getTimestamp() {
        return m_timestamp;
    }

    public VmMessage getVmMessage() {
        return m_vmMessage;
    }
    
}
