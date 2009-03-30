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

import org.apache.commons.io.FileUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.voicemail.MessageDescriptor.Priority;

public class Message {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private Mailbox m_mailbox;
    private String m_messageId; // A machine wide unique incremented "number"
    private String m_fromUri;
    private Priority m_priority;
    private MessageDescriptor m_messageDescriptor;
    private String m_wavName;
    private boolean m_stored;
    private boolean m_isTempWav;
    private boolean m_isToBeStored;
    
    public enum Reason {
        SAVED,
        TOO_SHORT,
        FAILED
    }
    /**
     * Create a new message in the mailbox from the given wav file
     * Message isn't physically stored in the inbox until storeInInbox is invoked.
     * @param mailbox
     */
    Message(Mailbox mailbox, String wavName, String fromUri, Priority priority) {
        m_mailbox = mailbox;
        
        m_wavName = wavName;
        m_isTempWav = true ;
        m_fromUri = fromUri;
        m_priority = priority;
        m_stored = false;
        m_isToBeStored = true;
    }

    /**
     * Copy a new message in the mailbox from an other Message
     * Message isn't physically stored in the inbox until storeInInbox is invoked.
     * @param mailbox
     */
    Message(Mailbox mailbox, Message existingMessage) {
        m_mailbox = mailbox;
        
        m_wavName = existingMessage.getWavName();
        m_isTempWav = false;
        m_fromUri = existingMessage.getFromUri();
        m_priority = existingMessage.getPriority();
        m_stored = false;
        m_isToBeStored = true;
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
        if (m_isTempWav = true) {
            FileUtils.deleteQuietly(new File(m_wavName)) ;
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
        
        // Generate the next message ID
        m_messageId = Message.nextMessageId(m_mailbox);
        
        // Generate the MessageDescriptor;
        m_messageDescriptor = new MessageDescriptor();
        m_messageDescriptor.setId(m_mailbox.getUser().getIdentity());
        m_messageDescriptor.setFromUri(m_fromUri);
        m_messageDescriptor.setDurationSecs("0"); //TODO
        m_messageDescriptor.setTimestamp(System.currentTimeMillis());
        m_messageDescriptor.setSubject("Voice Message "+m_messageId);
        m_messageDescriptor.setPriority(m_priority);
        
        File temp = new File(m_wavName);
        String baseName = m_messageId+"-00"; //TODO the -00 means something...
        File wavFile = new File(m_mailbox.getInboxDirectory()+baseName+".wav");
        File xmlFile = new File(m_mailbox.getInboxDirectory()+baseName+".xml");
        File staFile = new File(m_mailbox.getInboxDirectory()+baseName+".sta");
        
        String operation = "storeInInbox";
        try {
            if (m_isTempWav) {
                operation = "storeInInbox moving temp .wav file to "+wavFile.getPath();
                LOG.debug("Message::storeInInbox "+operation);
                FileUtils.moveFile(temp, wavFile);
                m_isTempWav = false; // temp file is no more
                
            } else {
                operation = "storeInInbox copy orig .wav file to "+wavFile.getPath();
                LOG.debug("Message::storeInInbox "+operation);
                FileUtils.copyFile(temp, wavFile, true);
            }
            // Now has a new name!
            m_wavName = wavFile.getPath();
            
            operation = "storeInInbox creating messageDescriptor "+xmlFile.getPath();
            LOG.debug("Message::storeInInbox "+operation);
            new MessageDescriptorWriter().writeObject(m_messageDescriptor, xmlFile);

            operation = "storeInInbox creating status file "+staFile.getPath();
            LOG.debug("Message::storeInInbox "+operation);

            FileUtils.touch(staFile);
            
        } catch (IOException e) {
            LOG.error("Message::storeInInbox  error during "+operation, e);
            return Reason.FAILED;
        }
        m_stored = true;
        LOG.info("Message::storeInInbox stored messageId "+m_messageId);
        return Reason.SAVED;
    }
    
    /**
     * Generate the next message Id
     * static synchronized as it's machine wide
     * @param mailbox
     */
    static synchronized String nextMessageId(Mailbox mailbox) {
        long numericMessageId = 1;
        String format = "%08d";
        String messageId = String.format(format,numericMessageId);
        
        // messageid.txt file is one level up from mailstore
        String messageIdFileName = mailbox.getUserDirectory()+"../../messageid.txt";
        File midFile = new File(messageIdFileName);
        if (midFile.exists()) {
            try {
                // The messageid in the file is the NEXT one
                messageId = FileUtils.readFileToString(midFile);
                numericMessageId = Long.parseLong(messageId);
            } catch (IOException e) {
                LOG.error("Message::nextMessageId cannot read "+messageIdFileName, e);
                throw new RuntimeException(e);
            }
        }
        // Increment message id, store for another day
        numericMessageId++;
        try {
            FileUtils.writeStringToFile(midFile, String.format(format, numericMessageId));
        } catch (IOException e) {
            LOG.error("Message::nextMessageId cannot write "+messageIdFileName, e);
            throw new RuntimeException(e);
        }
        
        return messageId;
    }
    
    public String getMessageId() {
        return m_messageId;
    }
    
    public String getWavName() {
        return m_wavName;
    }
    
}
