/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.forwarding;

import java.util.Formatter;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing;

/**
 * Ring - represents one stage in a call forwaring sequence
 */
public class Ring extends AbstractRing {
    private static final String VALID_TIME_PARAM = ";sipx-ValidTime=\"%s\"";

    private String m_number = StringUtils.EMPTY;
    private CallSequence m_callSequence;
    private Schedule m_schedule;

    /**
     * Default "bean" constructor
     */
    public Ring() {
        // empty
    }

    /**
     * @param number phone number or SIP url to which call is to be transfered
     * @param expiration number of seconds that call will ring
     * @param type if the call should wait for the previous call failure or start ringing at the
     *        same time
     * @param enabled flag indicating whether this rule is active
     */
    public Ring(String number, int expiration, Type type, boolean enabled) {
        m_number = number;
        setExpiration(expiration);
        setType(type);
        setEnabled(enabled);
    }

    /**
     * Retrieves the user part of the contact used to calculate contact
     *
     * @return String or object implementing toString method
     */
    @Override
    protected Object getUserPart() {
        return m_number;
    }

    public synchronized String getNumber() {
        return m_number;
    }

    public synchronized void setNumber(String number) {
        m_number = number;
    }

    public synchronized CallSequence getCallSequence() {
        return m_callSequence;
    }

    public synchronized void setCallSequence(CallSequence callSequence) {
        m_callSequence = callSequence;
    }

    public Schedule getSchedule() {
        return m_schedule;
    }

    public void setSchedule(Schedule schedule) {
        m_schedule = schedule;
    }

    @Override
    protected void addUrlParams(StringBuilder params) {
        if (m_schedule != null) {
            Formatter formatter = new Formatter(params);
            formatter.format(VALID_TIME_PARAM, m_schedule.calculateValidTime());
        }
    }
}
