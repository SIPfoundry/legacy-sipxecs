/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.freeswitch;

public class ConferenceMember {
    String m_memberId;
    String m_memberName;
    String m_memberNumber;
    boolean m_muted; 
    String m_memberIndex;

    public boolean isMuted() {
        return m_muted;
    }

    public String memberId() {
        return m_memberId;
    }

    public String memberName() {
        return m_memberName;
    } 

    public String memberNumber() {
        return m_memberNumber;
    }
     
    public String memberIndex() {
        return m_memberIndex;
    }
}

