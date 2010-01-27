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
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.common.SipUri;

/**
 * Statisticall data about the single call handled AcdCallStats
 */
public class AcdCallStats implements AcdStatsItem.AgentName, AcdStatsItem.QueueName, Serializable {

    private static final long serialVersionUID = 1L;

    /**
     * Call status: the state transition is WAITING,{UNANSWERED|(IN_PROGRESS,COMPLETED)} Values
     * need to match constantct in agent/lib/call.rb
     */
    public static final class State extends Enum {
        public static final State WAITING = new State("waiting");
        public static final State IN_PROGRESS = new State("in_progress");
        public static final State COMPLETED = new State("terminate");
        public static final State UNANSWERED = new State("unanswered");

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

    private State m_state;

    /** from field of the initial call? */
    private String m_caller;

    /** agent who picked up the call */
    private String m_agentUri;

    /** interanl id of the call - never shown to the user */
    private String m_callId;

    /** current queue */
    private String m_queueUri;

    /** time the call is waiting in current queue */
    private long m_queueWaitMillis;

    /** total call waiting time */
    private long m_totalWaitMillis;

    /** processing time - how long the agent has been taling to the caller */
    private long m_processingMillis;

    public String getAgentUri() {
        return m_agentUri;
    }

    public void setAgentUri(String agentUri) {
        m_agentUri = agentUri;
    }

    public String getAgentName() {
        return m_agentUri != null ? SipUri.extractUser(m_agentUri) : m_agentUri;
    }

    public String getCaller() {
        return m_caller;
    }

    public void setCaller(String caller) {
        m_caller = caller;
    }

    /**
     * Extracts short user ID from caller URI.
     *
     * @return userId or fuller caller string if it cannot parse caller URI
     */
    public String getCallerName() {
        String shortCaller = SipUri.extractFullUser(m_caller);
        return StringUtils.defaultIfEmpty(shortCaller, m_caller);
    }

    public String getCallId() {
        return m_callId;
    }

    public void setCallId(String callId) {
        m_callId = callId;
    }

    public long getProcessingMillis() {
        return m_processingMillis;
    }

    public void setProcessingMillis(long processingMillis) {
        m_processingMillis = processingMillis;
    }

    public String getQueueUri() {
        return m_queueUri;
    }

    public String getQueueName() {
        return m_queueUri != null ? SipUri.extractUser(m_queueUri) : null;
    }

    public void setQueueUri(String queueUri) {
        m_queueUri = queueUri;
    }

    public long getQueueWaitMillis() {
        return m_queueWaitMillis;
    }

    public void setQueueWaitMillis(long queueWaitMillis) {
        m_queueWaitMillis = queueWaitMillis;
    }

    public long getTotalWaitMillis() {
        return m_totalWaitMillis;
    }

    public void setTotalWaitMillis(long totalWaitMillis) {
        m_totalWaitMillis = totalWaitMillis;
    }

    public State getState() {
        return m_state;
    }

    public void setState(State state) {
        m_state = state;
    }
}
