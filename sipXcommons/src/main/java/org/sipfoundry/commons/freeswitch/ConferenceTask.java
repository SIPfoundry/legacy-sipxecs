/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.freeswitch;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import org.sipfoundry.commons.userdb.User;

public class ConferenceTask {
    // maps member id to member object
    private Map<String, ConferenceMember> m_members;
    private boolean             m_isLocked;
    private String              m_lastMemberTalking;
    private boolean             m_noOneTalking;
    private User                m_owner;
    private String              m_wavName;
    private int                 m_participantIndex;
    
    ConferenceTask() {
        m_members = new HashMap<String, ConferenceMember>();
        m_isLocked = false;
        m_lastMemberTalking = null;
        m_noOneTalking = true;
        m_wavName = null;
        m_participantIndex = 1;
    }
    
    public int getSize() {
        return m_members.size();
    }
    
    public Collection<ConferenceMember> getMembers() {
        return m_members.values();
    }
    
    public String getNextParticipantIndex() {
        return String.valueOf(m_participantIndex++);
    }
    
    public User getOwner() {
        return m_owner;
    }
    
    public void setOwner(User owner) {
        m_owner = owner;
    }
    
    public String getWavName() {
        return m_wavName;
    }
    
    public void setWavName(String wavName) {
        m_wavName = wavName;
    }
    
    public void setLocked(boolean isLocked) {
        m_isLocked = isLocked;
    }
    
    public boolean isLocked() {
        return m_isLocked;
    }

    public ConferenceMember get(String memberId) {
        return m_members.get(memberId);
    }
    
    public void setLastMemberTalking(String lastMemberTalking) {
        m_lastMemberTalking = lastMemberTalking;
    }
    
    public String getWhosTalking() {
        return m_lastMemberTalking;
    }
    
    public void setNoOneTalking(boolean noOneTalking) {
        m_noOneTalking = noOneTalking;
    }
    
    public boolean isNoOneTalking() {
        return m_noOneTalking;
    }
    
    public void add(String memberId, ConferenceMember member) {
        m_members.put(memberId, member);
    }
    
    public void delete(String memberId) {
        if(memberId.equals(m_lastMemberTalking)) {
            m_lastMemberTalking = null;
            m_noOneTalking = true;
        }
        m_members.remove(memberId);
    }
}

