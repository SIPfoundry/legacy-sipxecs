/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.logging;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import org.apache.commons.lang.StringEscapeUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.log4j.MDC;
import org.sipfoundry.sipxconfig.common.ReplicationsFinishedEvent;
import org.sipfoundry.sipxconfig.commserver.FailedReplicationsState;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

public class AuditLogContextImpl implements AuditLogContext, ApplicationContextAware  {

    private static final String EVENT_TYPE = "eventType";
    private static final String REPLICATION_EVENT = "Replication";
    private static final String CONFIG_CHANGE_EVENT = "ConfigChange";
    private static final String PROCESS_STATE_CHANGE_EVENT = "ProcessStateChange";
    private static final String TO = " to ";
    private static final int DEFAULT_SLEEP_INTERVAL = 7;

    private static final Log AUDIT_LOG = LogFactory.getLog("org.sipfoundry.sipxconfig.auditlog");

    private final FailedReplicationsState m_failedState = new FailedReplicationsState();

    private final ScheduledExecutorService m_scheduler = Executors.newScheduledThreadPool(1);

    private final List<String> m_tasks = new ArrayList<String>();

    private int m_sleepInterval = DEFAULT_SLEEP_INTERVAL;

    private ApplicationContext m_applicationContext;

    public void init() {
        m_scheduler.scheduleAtFixedRate(new Worker(), 0, m_sleepInterval, TimeUnit.SECONDS);
    }

    @Override
    public  void logReplication(String dataName, Location location) {
        MDC.put(EVENT_TYPE, REPLICATION_EVENT);
        m_failedState.unmark(location.getFqdn(), dataName);
        log("Replicated " + dataName + TO + location.getFqdn());
        m_tasks.add(dataName);
    }

    @Override
    public void logReplicationMongo(String dataName, Location location) {
        MDC.put(EVENT_TYPE, REPLICATION_EVENT);
        m_failedState.unmark(location.getFqdn(), dataName);
        log("Replicated in Mongo DB" + dataName + TO + location.getFqdn());
        m_tasks.add(dataName);
    }

    @Override
    public void logReplicationFailed(String dataName, Location location, Exception ex) {
        m_failedState.mark(location.getFqdn(), dataName);
        logError("Replication failed " + dataName + TO + location.getFqdn(), ex);
        m_tasks.add(dataName);
    }

    @Override
    public void logReplicationMongoFailed(String dataName, Location location,
            Exception ex) {
        m_failedState.mark(location.getFqdn(), dataName);
        logError("Replication in Mongo DB failed " + dataName + TO
                + location.getFqdn(), ex);
        m_tasks.add(dataName);
    }

    @Override
    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }

    @Override
    public Set<String> getReplicationFailedList(String fqdn) {
        return m_failedState.getFailedList(fqdn);
    }

    @Override
    public Set<String> getReplicationSuccededList(String fqdn) {
        return m_failedState.getSuccededList(fqdn);
    }

    private int getProgressNumber(Location location) {
        Set<String> failedList = getReplicationFailedList(location.getFqdn());
        Set<String> successfulList = getReplicationSuccededList(location
                .getFqdn());
        int progress = ((failedList == null) ? 0 : failedList.size())
                + ((successfulList == null) ? 0 : successfulList.size());
        return progress;
    }

    private int getSavedNumber(Location location) {
        return getNumberOfReplications(location);
    }

    private int getNumberOfReplications(Location location) {
        // TODO
        return 0;
//        return m_locationsManager.getReplications(location).size();
    }

    /*
     * Returns true only when SendProfiles are started but didn't finish execution
     * In other cases it returns false, no matter if there may be some file replications in other situations
     */
    @Override
    public boolean isSendProfilesInProgress(Location location) {
        int progress = getProgressNumber(location);
        int savedResult = getSavedNumber(location);
        return progress < savedResult;
    }

    @Override
    public int getProgressPercent(Location location) {
        int progress = getProgressNumber(location);
        int savedResult = getSavedNumber(location);
        return savedResult > 0 ? progress * 100 / savedResult : 0;
    }

    @Override
    public void resetReplications(String fqdn) {
        m_failedState.reset(fqdn);
    }

    /*
     * This method is used when application starts up and is already initialized
     * (first run task was executed in a previous system initialization)
     */
    @Override
    public void saveLatestReplicationsState(Location location) {
        m_failedState.saveState(location);
    }

    public void setSleepInterval(int sleepInterval) {
        m_sleepInterval = sleepInterval;
    }

    @Override
    public void logConfigChange(CONFIG_CHANGE_TYPE changeType, String configType, String configName) {
        MDC.put(EVENT_TYPE, CONFIG_CHANGE_EVENT);
        StringBuilder message = new StringBuilder();
        message.append(changeType);
        message.append(' ');
        message.append(configType);
        message.append(' ');
        message.append('\'');
        message.append(configName);
        message.append('\'');

        String escapedString = StringEscapeUtils.escapeJava(message.toString());

        log(escapedString);
    }

    @Override
    public void logProcessStateChange(PROCESS_STATE_CHANGE stateChange, String processName, Location location) {
        MDC.put(EVENT_TYPE, PROCESS_STATE_CHANGE_EVENT);
        StringBuilder message = new StringBuilder();
        message.append(stateChange);
        message.append(' ');
        message.append(processName);
        message.append(' ');
        message.append(" on ");
        message.append(location.getFqdn());

        String escapedString = StringEscapeUtils.escapeJava(message.toString());

        log(escapedString);
    }

    private void log(String message) {
        AUDIT_LOG.info(message);
    }

    private void logError(String message, Exception ex) {
        if (ex == null) {
            AUDIT_LOG.error(message);
        } else {
            AUDIT_LOG.error(message, ex);
        }
    }

    private class Worker implements Runnable {
        @Override
        public void run() {
            if (!m_tasks.isEmpty()) {
                AUDIT_LOG.info("Replicated: " + m_tasks.size() + " - Publish finished replication event ...");
                m_tasks.clear();
                m_applicationContext.publishEvent(new ReplicationsFinishedEvent(m_failedState));
            }
        }
    }
}
