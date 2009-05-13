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
    Mailbox m_mailbox;
    
    public enum Folders {
        INBOX, SAVED, DELETED;
    }
    
    HashMap<String, VmMessage> m_inbox = new HashMap<String, VmMessage>();
    HashMap<String, VmMessage> m_saved = new HashMap<String, VmMessage>();
    HashMap<String, VmMessage> m_deleted = new HashMap<String, VmMessage>();

    int m_numUnheard;

    File m_inboxDir;
    File m_savedDir;
    File m_deletedDir;
    
    public Messages(Mailbox mailbox) {
        m_mailbox = mailbox;
        m_inboxDir = new File(mailbox.getInboxDirectory());
        m_savedDir = new File(mailbox.getSavedDirectory());
        m_deletedDir = new File(mailbox.getDeletedDirectory());
        
        // Load the inbox folder and count unheard messages there
        loadFolder(m_inboxDir, m_inbox, true);
        // Load the saved folder (don't count unheard...shouldn't be any but why take chances!)
        loadFolder(m_savedDir, m_saved, false);
        // Load the deleted folder (same with unheard)
        loadFolder(m_deletedDir, m_deleted, false);
    }

    
    public Messages() {
        // Just for test cases, without need for a file system full of messages
    }
    
    /**
     * Load the map with the files from the directory
     * @param directory
     * @param map
     * @param countUnheard count number of unheard messages while doing this
     */
    
    @SuppressWarnings("unchecked") // FileUtls.itereateFiles isn't type safe
    void loadFolder(File directory, HashMap<String, VmMessage> map, boolean countUnheard) {
        Pattern p = Pattern.compile("^(\\d+)-00\\.xml$");
        // Load the directory and count unheard
        Iterator<File> fileIterator = FileUtils.iterateFiles(directory, null, false);
        while(fileIterator.hasNext()) {
            File file = fileIterator.next();
            String name = file.getName();
            Matcher m = p.matcher(name);

            if (m.matches()) {
                String id = m.group(1);     // The ID
                
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
    public List<VmMessage> getFolder(HashMap<String, VmMessage> folder) {
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
    
    public int getInboxCount() {
        return m_inbox.size();
    }
    
    public int getSavedCount() {
        return m_saved.size();
    }
    
    public int getDeletedCount() {
        return m_deleted.size();
    }
    
    public int getUnheardCount() {
        return m_numUnheard;
    }
    
    public int getHeardCount() {
        return m_inbox.size() - m_numUnheard ;
    }
    
    /**
     * Mark the message unheard
     * @param msg
     */
    public void markMessageHeard(VmMessage msg) {
        if (msg.isUnHeard()) {
            msg.markHeard();
            m_numUnheard--;
            Mwi.sendMWI(m_mailbox, this);
        }
    }
    
    /**
     * "Delete" the message.
     * If it is in the inbox, move it to the deleted folder, and the files into the deleted directory.
     * If it is in the saved folder, move it to the deleted folder and the files into the deleted directory.
     * If it is in the deleted folder, remove it and delete the files.
     * @param msg
     */
    public void deleteMessage(VmMessage msg) {
        String id = msg.getMessageId();
        // Find which folder it was previously in
        if (m_inbox.containsKey(id)) {
            markMessageHeard(msg);
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
    }

    /**
     * "Save" the message.
     * If it is in the inbox, move it to the saved folder, and the files to the saved directory.
     * If it is already in the saved folder, do nothing.
     * If it is in the deleted folder, move it to the inbox folder (yes, not saved) and the files to the inbox directory.
     * 
     * @param msg
     */
    public void saveMessage(VmMessage msg) {
        String id = msg.getMessageId() ;
        // Find which folder it was previously in
        if (m_inbox.containsKey(id)) {
            // Move files from inbox to saved
            markMessageHeard(msg);
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
    public void destroyDeletedMessages() {
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

}
