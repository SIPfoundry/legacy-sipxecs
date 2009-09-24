/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.forwarding;

import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.callgroup.AbstractCallSequence;
import org.sipfoundry.sipxconfig.common.User;

/**
 * CallSequence
 */
public class CallSequence extends AbstractCallSequence {
    private static final String CALL_FWD_TIMER_SETTING = "callfwd/timer";

    private User m_user;
    private boolean m_withVoicemail;

    public CallSequence() {
        // empty default constructor
    }

    /**
     * @param calls list of ring objects
     * @param user original phone number
     * @param withVoicemail
     */
    CallSequence(List calls, User user, boolean withVoicemail) {
        m_user = user;
        m_withVoicemail = withVoicemail;
        setRings(calls);
    }

    public Ring insertRing() {
        Ring ring = new Ring();
        ring.setCallSequence(this);
        insertRing(ring);
        return ring;
    }

    public List generateAliases(String domain) {
        String identity = m_user.getUserName() + "@" + domain;
        // pass true to never route this to voicemail
        return generateAliases(identity, domain, true);
    }

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public boolean isWithVoicemail() {
        return m_withVoicemail;
    }

    public void setWithVoicemail(boolean withVoicemail) {
        m_withVoicemail = withVoicemail;
    }

    public int getCfwdTime() {
        return (Integer) m_user.getSettingTypedValue(CALL_FWD_TIMER_SETTING);
    }

    public void setCfwdTime(int cfwdtime) {
        m_user.setSettingTypedValue(CALL_FWD_TIMER_SETTING, cfwdtime);
    }

    @Override
    public void insertRings(Collection rings) {
        super.insertRings(rings);
        for (Iterator iter = rings.iterator(); iter.hasNext();) {
            Ring ring = (Ring) iter.next();
            ring.setCallSequence(this);
        }
    }
}
