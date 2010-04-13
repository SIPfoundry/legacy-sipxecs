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
import java.io.FilenameFilter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.io.FileUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.Mailbox;

/**
 * Represents the messages in a mailbox
 */
public class Messages {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    
    // Static holding tank for Messages, so multiple threads always get the same 
    // Messages object for the same mailstore
    static HashMap<String, Messages> s_messages = new HashMap<String, Messages>();
    int m_usage;    // Counter to know when it's safe to remove Messages from the tank.
    
    Mailbox m_mailbox;
    String m_mbxid; 
    
    public enum Folders {
        INBOX, SAVED, DELETED;
    }
    
    HashMap<String, VmMessage> m_inbox = new HashMap<String, VmMessage>();
    HashMap<String, VmMessage> m_saved = new HashMap<String, VmMessage>();
    HashMap<String, VmMessage> m_deleted = new HashMap<String, VmMessage>();
    
    List<String> m_MsgIds;

    int m_numUnheard;
    int m_numUnheadUrgent;

    File m_inboxDir;
    File m_savedDir;
    File m_deletedDir;
    
    // Private so only newMessages factory method can access
    private Messages(Mailbox mailbox) {
        m_mailbox = mailbox;
        m_usage = 0;
        // Create the mailbox if it isn't there
        Mailbox.createDirsIfNeeded(m_mailbox);
        m_inboxDir = new File(mailbox.getInboxDirectory());
        m_savedDir = new File(mailbox.getSavedDirectory());
        m_deletedDir = new File(mailbox.getDeletedDirectory());
        m_mbxid = mailbox.getUser().getUserName();
        m_MsgIds = null;
    }

    /*
     * Factory method to create a Messages object for a mailbox.
     * Ensures only one object per mailstore
     */
    public static Messages newMessages(Mailbox mailbox) {
        Messages messages;
        synchronized (s_messages) {                                     // lock on s_messages so multithreads to mess it up
            messages = s_messages.get(mailbox.getUserDirectory());      // See if Messages for this mailstore already exists
            if (messages == null) {
                messages = new Messages(mailbox);                       // Create a new one
                s_messages.put(mailbox.getUserDirectory(), messages);   // Store it.
            }
        }
        synchronized (messages) {
            messages.m_usage++; // Increment usage count
            messages.update();
            LOG.debug(String.format("Messages::newMessages m_usage for %s is %d",
                    mailbox.getUserDirectory(), messages.m_usage));
        }
        return messages;
    }
    
    public static void releaseMessages(Messages messages) {
        synchronized (s_messages) { // lock on s_messages so multithreads don't mess it up
            if (messages != null) {
                synchronized (messages) {
                    messages.m_usage--; // One less user
                    LOG.debug(String.format("Messages::releaseMessages m_usage for %s is %d",
                            messages.m_mailbox.getUserDirectory(), messages.m_usage));
                    if (messages.m_usage <= 0) {
                        s_messages.remove(messages.m_mailbox.getUserDirectory());
                    }
                }
            }
        }
    }
    
    public Messages() {
        // Just for test cases, without need for a file system full of messages
    }
    
    public synchronized void update() {
        
        List<String> msgIds = new ArrayList<String>();
        
        // Load the inbox folder and count unheard messages there
        loadFolder(m_inboxDir, m_inbox, true, msgIds);
        // Load the saved folder (don't count unheard...shouldn't be any but why take chances!)
        loadFolder(m_savedDir, m_saved, false, msgIds);
        // Load the deleted folder (same with unheard)
        loadFolder(m_deletedDir, m_deleted, false, null);
        
        // build a mailbox wide list of messageIds as loadFolder is called
        // then sort the list. this list allows us to easily map a CallPilot Msg Number 
        // to a message Id
        
        m_MsgIds = msgIds;
          
        Collections.sort(m_MsgIds, new Comparator<String>(){
            public int compare(String o1, String o2) {
                
                if(Integer.parseInt(o1) > Integer.parseInt(o2)) {
                    return 1;
                } else { 
                    return -1;
                }    
            }
        });
    }
    
    /**
     * Load the map with the files from the directory
     * @param directory
     * @param map
     * @param countUnheard count number of unheard messages while doing this
     */
    
    @SuppressWarnings("unchecked") // FileUtls.itereateFiles isn't type safe
    synchronized void loadFolder(File directory, HashMap<String, VmMessage> map, 
                                 boolean countUnheard, List<String> msgIds) {
        Pattern p = Pattern.compile("^(\\d+)-00\\.xml$");
        // Load the directory and count unheard
        Iterator<File> fileIterator = FileUtils.iterateFiles(directory, null, false);
        while(fileIterator.hasNext()) {
            File file = fileIterator.next();
            String name = file.getName();
            Matcher m = p.matcher(name);

            if (m.matches()) {
                String id = m.group(1);     // The ID
                
                if(msgIds != null) {
                    msgIds.add(id);
                } 
                
                // If this message is already in the map, skip it
                VmMessage vmMessage = map.get(id);
                if (vmMessage != null) {
                    continue;
                }

                // Otherwise make a new one.
                vmMessage = VmMessage.loadMessage(directory, id);
                if (vmMessage == null) {
                    continue;
                }
                
                if (countUnheard && vmMessage.isUnHeard()) {
                    m_numUnheard++;
                    if(vmMessage.isUrgent()) {
                        m_numUnheadUrgent++; 
                    }
                }
                                                          
                map.put(id, vmMessage);
            }
        }
    }

    /**
     * Return the inbox messages as a sorted list (UnHeard/Heard then Date (earliest first))
     * @return
     */
    public List<VmMessage> getInbox() {
        return getFolder(m_inbox);
    }

    /**
     * Return the saved messages sorted by Date (earliest first)
     * @return
     */
    public List<VmMessage> getSaved() {
        return getFolder(m_saved);
    }

    /**
     * Return the deleted messages sorted by Date (earliest first)
     * @return
     */
    public List<VmMessage> getDeleted() {
        return getFolder(m_deleted);
    }

    /**
     * Sort the folder as a list (UnHeard/Heard then Date (earliest first))
     * @param folder
     * @return
     */
    public synchronized List<VmMessage> getFolder(HashMap<String, VmMessage> folder) {
        List<VmMessage> l = Arrays.asList(folder.values().toArray(new VmMessage[0]));
        
        // Then put all unheard messages first
        // Then sort by date (earliest first)
        Collections.sort(l, new Comparator<VmMessage>(){

            public int compare(VmMessage o1, VmMessage o2) {
                if (o1.isUnHeard() == o2.isUnHeard()) {
                    if (o1.getTimestamp() < o2.getTimestamp()) {
                        return -1;
                    } else if (o1.getTimestamp() > o2.getTimestamp()) {
                        return 1;
                    }
                    return 0 ;
                }
                if (o1.isUnHeard() && !o2.isUnHeard()) {
                    return -1;
                }
                return 1;
            }
        });
        return l;
    }
    
    public synchronized int getInboxCount() {
        return m_inbox.size();
    }
    
    public synchronized int getSavedCount() {
        return m_saved.size();
    }
    
    public synchronized int getDeletedCount() {
        return m_deleted.size();
    }
    
    public synchronized int getUnheardCount() {
        return m_numUnheard;
    }
    
    public synchronized int getUnhreadUrgentCount() {
        return m_numUnheadUrgent;
    }
    
    public synchronized int getHeardCount() {
        return m_inbox.size() - m_numUnheard ;
    }
    
    /**
     * Find the messageId in the mailbox, no matter what directory it's in
     * @param messageId
     * @return
     */
    public synchronized VmMessage getMessage(String messageId) {
        VmMessage msg = m_inbox.get(messageId) ;
        if (msg == null) {
            msg = m_saved.get(messageId);
            if (msg == null) {
                msg = m_deleted.get(messageId);
            }
        }
        return msg;
    }
    
    /**
     * Mark the message represented by messageId heard
     * @param messageId
     * @param sendMwi
     */
    public synchronized void markMessageHeard(String messageId, boolean sendMwi) {
        VmMessage msg = getMessage(messageId);
        if (msg != null) {
            markMessageHeard(msg, sendMwi);
        }
    }
    
    public synchronized boolean isDeleted(VmMessage vmMsg) {
        return m_deleted.containsKey(vmMsg.getMessageId());
    }
    
    /**
     * Mark the message heard
     * @param msg
     * @param sendMwi
     */
    public synchronized void markMessageHeard(VmMessage msg, boolean sendMwi) {
        if (msg.isUnHeard()) {
            msg.markHeard();
            m_numUnheard--;
            if (sendMwi) {
                Mwi.sendMWI(m_mailbox, this);
            }
        }        
        ExtMailStore.MarkSaved(m_mbxid, msg.getMessageId());    
    }

    /**
     * Mark the message represented by messageId Unheard
     * @param messageId
     * @param sendMwi
     */
    public synchronized void markMessageUnheard(String messageId, boolean sendMwi) {
        VmMessage msg = getMessage(messageId);
        if (msg != null) {
            markMessageUnheard(msg, sendMwi);
        }
    }
    
    /**
     * Mark the message Unheard
     * @param msg
     * @param sendMwi
     */
    public synchronized void markMessageUnheard(VmMessage msg, boolean sendMwi) {
        if (!msg.isUnHeard()) {
            msg.markUnheard();
            m_numUnheard++;
            if (sendMwi) {
                Mwi.sendMWI(m_mailbox, this);
            }
        }        
        ExtMailStore.MarkNew(m_mbxid, msg.getMessageId());    
    }

    /**
     * "Delete" the message.
     * If it is in the inbox, move it to the deleted folder, and the files into the deleted directory.
     * If it is in the saved folder, move it to the deleted folder and the files into the deleted directory.
     * If it is in the deleted folder, remove it and delete the files.
     * @param msg
     */
    public synchronized void deleteMessage(VmMessage msg) {
        String id = msg.getMessageId();
        // Find which folder it was previously in
        if (m_inbox.containsKey(id)) {
            markMessageHeard(msg, false);
            // Move files from inbox to deleted
            msg.moveToDirectory(m_deletedDir);
            m_inbox.remove(id);
            m_deleted.put(id, msg);
            removeRemains(m_inboxDir, id);
            LOG.info(String.format("Messages::deleted moved %s from inbox to deleted Folder", id));
            Mwi.sendMWI(m_mailbox, this);
        } else if (m_saved.containsKey(id)) {
            // Move files from saved to deleted
            msg.moveToDirectory(m_deletedDir);
            m_saved.remove(id);
            m_deleted.put(id, msg);
            removeRemains(m_savedDir, id);
            LOG.info(String.format("Messages::deleted moved %s from saved to deleted Folder", id));
        } else if (m_deleted.containsKey(id)) {
            // Delete the files
            removeRemains(m_deletedDir, id);
            m_deleted.remove(id);
            LOG.info(String.format("Messages::deleted destroyed %s", id));
        }
        ExtMailStore.MarkDeleted(m_mbxid, id);    
    }

    /**
     * "Save" the message.
     * If it is in the inbox, move it to the saved folder, and the files to the saved directory.
     * If it is already in the saved folder, do nothing.
     * If it is in the deleted folder, move it to the inbox folder (yes, not saved) and the files to the inbox directory.
     * 
     * @param msg
     */
    public synchronized void saveMessage(VmMessage msg) {
        String id = msg.getMessageId() ;
        // Find which folder it was previously in
        if (m_inbox.containsKey(id)) {
            // Move files from inbox to saved
            markMessageHeard(msg, false);
            msg.moveToDirectory(m_savedDir);
            m_inbox.remove(id);
            m_saved.put(id, msg);
            removeRemains(m_inboxDir, id);
            LOG.info(String.format("Messages::saveMessage moved %s from inbox to saved Folder", id));
            Mwi.sendMWI(m_mailbox, this);
        } else if (m_saved.containsKey(id)) {
            // It's already saved!  do nothing.
        } else if (m_deleted.containsKey(id)) {
            // Move files from deleted to inbox
            msg.moveToDirectory(m_inboxDir);
            m_deleted.remove(id);
            m_inbox.put(id, msg);
            removeRemains(m_deletedDir, id);
            LOG.info(String.format("Messages::saveMessage moved %s from deleted to inbox Folder", id));
            Mwi.sendMWI(m_mailbox, this);
        }
    }
    
    /**
     * Destroy all the messages in the deleted folder.
     */
    public synchronized void destroyDeletedMessages() {
        for (VmMessage msg : getDeleted()) {
            String id = msg.getMessageId() ;
            // Delete the files
            removeRemains(m_deletedDir, id);
            m_deleted.remove(id);
            LOG.info(String.format("Messages::deleted destroyed %s", id));
        }
    }
    
    void removeRemains(File dir, String id) {
        File[] files = dir.listFiles(new FileFilterByMessageId(id));
        for (File file : files) {
            FileUtils.deleteQuietly(file);
        }
    }
    
    protected static class FileFilterByMessageId implements FilenameFilter {
        private final String m_messageIdPrefix;

        FileFilterByMessageId(String messageId) {
            m_messageIdPrefix = messageId + "-";
        }

        public boolean accept(File dir, String name) {
            return name.startsWith(m_messageIdPrefix);
        }
    }

    public void LoadWaveFile(VmMessage msg) {
        ExtMailStore.FetchBody(m_mbxid, msg.getMessageId(), msg.getAudioFile());
    }

    public String getMsgNumber(VmMessage vmMessage) {
        for(int i=0; i < m_MsgIds.size(); i++) {
            if(m_MsgIds.get(i).equals(vmMessage.getMessageId())) { 
                return String.valueOf(i+1);
            }           
        }
        return null;
    }
    
    public boolean noMessages() {
        return m_MsgIds.size() == 0;
    }
    
    public int getLastMsgNumber() {
        return m_MsgIds.size();    
    }
    
    public VmMessage getMsgByNumber(int msgNumber) {
        
        // make is zero based
        msgNumber--;
        
        if(msgNumber < 0) {
            return null;
        }
        
        if(msgNumber >= m_MsgIds.size()) {
            return null;
        }
        
        String messageId = m_MsgIds.get(msgNumber);
        if(messageId != null) {
            return getMessage(messageId);   
        }
        
        return null;
    }    
}
