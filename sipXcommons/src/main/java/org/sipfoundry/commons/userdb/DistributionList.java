/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.userdb;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Vector;

public class DistributionList {
    private String m_id;
    private String m_audioString;
    private Vector<String>m_mailboxStrings;
    private Vector<String>m_systemListStrings;
    
    public DistributionList(String id, String audio) {
        m_id = id;
        m_audioString = audio;
        m_mailboxStrings = new Vector<String>();
        m_systemListStrings = new Vector<String>();
    }
    
    /**
     * Returns a list of all the mailboxes in the list.
     * Plus any mailboxes in system lists, and purges duplicates
     * @param vm
     * @return
     */
    public HashSet<String> getList(HashMap<String, DistributionList> sysDistLists) {
        Vector<String> usedLists = new Vector<String>();
        return getLists(sysDistLists, usedLists);
    }
    
    /**
     * Returns a list of all the mailboxes in the list.
     * Expands any mailboxes in system lists, and purges duplicates
     * @param vm
     * @param usedLists
     * @return
     */
    private HashSet<String> getLists(HashMap<String, DistributionList> sysDistLists, Vector<String> usedLists) {
        HashSet<String> list = new HashSet<String>();
        // Iterate thru all the system list names we haven't already visited
        // and add the mailboxes each list defines.
        for (String sysListId : m_systemListStrings) {
            if (usedLists.contains(sysListId)) {
                // Already used this system list, so don't use it again (breaks recursion)
                continue;
            }
            usedLists.add(sysListId);
            DistributionList sysDl = sysDistLists.get(sysListId);
            if (sysDl != null) {
                list.addAll(sysDl.getLists(sysDistLists, usedLists));
            }
        }
        // Now, add all the mailboxes this list defines
        list.addAll(m_mailboxStrings);
        return list;
    }

    public void addMailboxString(String mailboxString) {
        m_mailboxStrings.add(mailboxString);
    }

    public void addSystemListString(String systemListString) {
        m_systemListStrings.add(systemListString);
    }

    public String getId() {
        return m_id;
    }

    public String getAudioString() {
        return m_audioString;
    }
}
