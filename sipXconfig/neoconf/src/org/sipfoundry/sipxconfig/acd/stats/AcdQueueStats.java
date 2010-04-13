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

import org.sipfoundry.sipxconfig.common.SipUri;

/**
 * ACD Queue statistic.
 * Average and max values are computed based on the last 30 minutes of calls.
 * Processed calls and abandoned calls are also computed based on 30 minutes rolling window.
 * All other values are computed based on the calls currently in the queue.
 *
 */
public class AcdQueueStats implements AcdStatsItem.QueueName, Serializable {

    private static final long serialVersionUID = 1L;

    /** Queue URI */
    private String m_queueUri;

    /** Waiting calls information: calls not picked up and not abandoned yet */
    private long m_waitingCalls;

    /** Average wait time for all the calls waiting, processed and abandoned */
    private long m_averageWaitMillis;

    /** Maximum wait time for all the calls waiting, processed and abandoned */
    private long m_maxWaitMillis;

    /** Abandoned calls (terminated by caller, no agent pick-up) */
    private long m_averageAbandonedMillis;

    private long m_maxAbandonedMillis;

    private long m_abandonedCalls;

    /** Processed calls */
    private long m_processedCalls;

    private long m_averageProcessingMillis;

    private long m_maxProcessingMillis;

    /** Number of idle agents */
    private int m_idleAgents;

    /** Number of busy agents */
    private int m_busyAgents;

    public long getAverageWaitMillis() {
        return m_averageWaitMillis;
    }

    public void setAverageWaitMillis(long averageWaitMillis) {
        m_averageWaitMillis = averageWaitMillis;
    }

    public int getBusyAgents() {
        return m_busyAgents;
    }

    public void setBusyAgents(int busyAgents) {
        m_busyAgents = busyAgents;
    }

    public int getIdleAgents() {
        return m_idleAgents;
    }

    public void setIdleAgents(int idleAgents) {
        m_idleAgents = idleAgents;
    }

    public String getQueueUri() {
        return m_queueUri;
    }

    public void setQueueUri(String queueUri) {
        m_queueUri = queueUri;
    }

    public String getQueueName() {
        return SipUri.extractUser(m_queueUri);
    }

    public int getSignInAgents() {
        return m_busyAgents + m_idleAgents;
    }

    public long getAbandonedCalls() {
        return m_abandonedCalls;
    }

    public void setAbandonedCalls(long abandonedCalls) {
        m_abandonedCalls = abandonedCalls;
    }

    public long getAverageAbandonedMillis() {
        return m_averageAbandonedMillis;
    }

    public void setAverageAbandonedMillis(long averageAbandonedMillis) {
        m_averageAbandonedMillis = averageAbandonedMillis;
    }

    public long getAverageProcessingMillis() {
        return m_averageProcessingMillis;
    }

    public void setAverageProcessingMillis(long averageProcessingMillis) {
        m_averageProcessingMillis = averageProcessingMillis;
    }

    public long getMaxAbandonedMillis() {
        return m_maxAbandonedMillis;
    }

    public void setMaxAbandonedMillis(long maxAbandonedMillis) {
        m_maxAbandonedMillis = maxAbandonedMillis;
    }

    public long getMaxProcessingMillis() {
        return m_maxProcessingMillis;
    }

    public void setMaxProcessingMillis(long maxProcessingMillis) {
        m_maxProcessingMillis = maxProcessingMillis;
    }

    public long getMaxWaitMillis() {
        return m_maxWaitMillis;
    }

    public void setMaxWaitMillis(long maxWaitMillis) {
        m_maxWaitMillis = maxWaitMillis;
    }

    public long getProcessedCalls() {
        return m_processedCalls;
    }

    public void setProcessedCalls(long processedCalls) {
        m_processedCalls = processedCalls;
    }

    public long getWaitingCalls() {
        return m_waitingCalls;
    }

    public void setWaitingCalls(long waitingCalls) {
        m_waitingCalls = waitingCalls;
    }
}
