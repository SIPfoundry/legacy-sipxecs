/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.conference;

import org.sipfoundry.sipxconfig.common.PrimaryKeySource;

/**
 * Represents a Conference that is currently in progress.
 */
public class ActiveConference implements PrimaryKeySource {

    private static final String EMPTY = "";

    /** The name of the conference. */
    private String m_name;

    /** The number of members connected to the conference. */
    private int m_members;

    /** Whether or not this conference is currently locked. */
    private boolean m_isLocked;

    /** The Conference object corresponding to this active conference. */
    private Conference m_conference;

    public ActiveConference(String name, int members, boolean isLocked) {
        m_name = name;
        m_members = members;
        m_isLocked = isLocked;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public int getMembers() {
        return m_members;
    }

    public void setMembers(int members) {
        m_members = members;
    }

    public boolean isLocked() {
        return m_isLocked;
    }

    public void setLocked(boolean isLocked) {
        m_isLocked = isLocked;
    }

    public void setConference(Conference conference) {
        m_conference = conference;
    }

    public Conference getConference() {
        return m_conference;
    }

    public String getExtension() {
        return (m_conference != null) ? m_conference.getExtension() : EMPTY;
    }

    public String getDescription() {
        return (m_conference != null) ? m_conference.getDescription() : EMPTY;
    }

    public Object getPrimaryKey() {
        return (m_conference != null) ? m_conference.getId() : -1;
    }

}
