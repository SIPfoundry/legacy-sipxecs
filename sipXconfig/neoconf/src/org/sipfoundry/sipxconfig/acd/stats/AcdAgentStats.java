/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd.stats;

import java.io.Serializable;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.common.SipUri;

public class AcdAgentStats implements AcdStatsItem.AgentName, Serializable {

    private static final long serialVersionUID = 1L;

    public static final class State extends Enum {
        public static final State IDLE = new State("idle");
        public static final State BUSY = new State("busy");

        private State(String name) {
            super(name);
        }

        public static List getEnumList() {
            return Enum.getEnumList(State.class);
        }

        public static State getEnum(String name) {
            return (State) Enum.getEnum(State.class, name);
        }
    }

    /** Agent id */
    private String m_agentUri;

    /** Time since last agent transition */
    private long m_currentStateMillis;

    /** current state */
    private State m_state;

    /** set of queues for this agent */
    private Set m_queues;

    public String getAgentUri() {
        return m_agentUri;
    }

    public void setAgentUri(String agentUri) {
        m_agentUri = agentUri;
    }

    public String getAgentName() {
        return m_agentUri != null ? SipUri.extractUser(m_agentUri) : null;
    }

    public long getCurrentStateMillis() {
        return m_currentStateMillis;
    }

    public void setCurrentStateMillis(long currentStateMillis) {
        m_currentStateMillis = currentStateMillis;
    }

    public State getState() {
        return m_state;
    }

    public void setState(State state) {
        m_state = state;
    }

    public void setQueues(String[] queues) {
        m_queues = new HashSet(Arrays.asList(queues));
    }

    public boolean isInQueue(String queue) {
        return m_queues.contains(queue);
    }
}
