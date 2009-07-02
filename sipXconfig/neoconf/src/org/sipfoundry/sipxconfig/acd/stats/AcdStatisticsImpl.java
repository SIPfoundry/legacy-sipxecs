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
import java.net.MalformedURLException;
import java.net.URL;
import java.rmi.RemoteException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import javax.xml.rpc.ServiceException;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.apache.commons.collections.Transformer;
import org.apache.commons.collections.functors.AndPredicate;
import org.apache.commons.collections.functors.TruePredicate;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.time.DateUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdQueue;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;

/**
 * Provides real-time statistics on ACD calls, agent and queues.
 */
public class AcdStatisticsImpl implements AcdStatistics {
    private static final Log LOG = LogFactory.getLog(AcdStatisticsImpl.class);

    private String m_queueUri;

    private final AcdContext m_acdContext;

    private Predicate m_agentFilter = TruePredicate.INSTANCE;

    private Predicate m_callFilter = TruePredicate.INSTANCE;

    private Predicate m_queueFilter = TruePredicate.INSTANCE;

    public AcdStatisticsImpl(AcdContext acdContext) {
        m_acdContext = acdContext;
    }

    public AcdStatsService getAcdStatsService(Serializable acdServerId) {
        try {
            URL agentUrl = getAgentUrl(acdServerId);
            AcdStatsService service = new StatsImplServiceLocator().getAcdStatsService(agentUrl);
            return service;
        } catch (ServiceException e) {
            throw new RuntimeException(e);
        } catch (MalformedURLException e) {
            throw new RuntimeException(e);
        }
    }

    protected URL getAgentUrl(Serializable acdServerId) throws MalformedURLException {
        AcdServer acdServer = m_acdContext.loadServer(acdServerId);
        URL agentUrl = new URL("http", acdServer.getLocation().getFqdn(), acdServer.getAgentPort(),
                StringUtils.EMPTY);
        LOG.debug("Agent URL: " + agentUrl);
        return agentUrl;
    }

    public List getAgentsStats(Serializable acdServerId, String queueUri) {
        try {
            AgentStats[] rawStats = getAcdStatsService(acdServerId).getAgentStats();
            Predicate filter = m_agentFilter;
            if (!StringUtils.isBlank(queueUri)) {
                filter = new AndPredicate(filter, new AgentFilterByQueueUri(queueUri));
            }
            List stats = getStats(rawStats, new AgentTransformer(), filter);
            return stats;
        } catch (RemoteException e) {
            LOG.warn("getAgentStats failed", e);
            return Collections.EMPTY_LIST;
        }
    }

    public List getCallsStats(Serializable acdServerId, String queueUri) {
        try {
            CallStats[] rawStats = getAcdStatsService(acdServerId).getCallStats();
            LOG.debug("Calls before filtering stats: " + rawStats.length);
            Predicate filter = m_callFilter;
            if (!StringUtils.isBlank(queueUri)) {
                String queueName = SipUri.extractUser(queueUri);
                Set queueNames = Collections.singleton(queueName);
                filter = new AndPredicate(filter, new QueueNameFilter(queueNames));
            }
            List stats = getStats(rawStats, new CallTransformer(), filter);
            LOG.debug("Calls after filtering stats: " + stats.size());
            return stats;
        } catch (RemoteException e) {
            LOG.warn("getCallStats failed", e);
            return Collections.EMPTY_LIST;
        }
    }

    public List getQueuesStats(Serializable acdServerId) {
        try {
            QueueStats[] rawStats = getAcdStatsService(acdServerId).getQueueStats();
            LOG.debug("Queues before filtering stats: " + rawStats.length);
            List stats = getStats(rawStats, new QueueTransformer(), m_queueFilter);
            LOG.debug("Queues after filtering stats: " + stats.size());
            return stats;
        } catch (RemoteException e) {
            LOG.warn("getQueueStats failed", e);
            return Collections.EMPTY_LIST;
        }
    }

    List getStats(Object[] rawStats, Transformer transformer, Predicate filter) {
        List stats = new ArrayList(Arrays.asList(rawStats));
        CollectionUtils.transform(stats, transformer);
        CollectionUtils.filter(stats, filter);
        return stats;
    }

    static class AgentNameFilter implements Predicate {

        private final Set<String> m_names;
        private final boolean m_allowBlankName;

        AgentNameFilter(Set<String> names, boolean allowBlankName) {
            m_names = names;
            m_allowBlankName = allowBlankName;
        }

        public boolean evaluate(Object object) {
            String name = ((AcdStatsItem.AgentName) object).getAgentName();
            if (StringUtils.isBlank(name)) {
                return m_allowBlankName;
            }
            boolean show = m_names.contains(name);
            return show;
        }
    }

    static class QueueNameFilter implements Predicate {
        private final Set<String> m_names;

        QueueNameFilter(Set<String> names) {
            m_names = names;
            if (LOG.isDebugEnabled()) {
                LOG.debug("Filtered queues: " + StringUtils.join(names, ", "));
            }
        }

        public boolean evaluate(Object object) {
            String name = ((AcdStatsItem.QueueName) object).getQueueName();
            boolean show = m_names.contains(name);
            return show;
        }
    }

    static class AgentTransformer implements Transformer {

        public Object transform(Object input) {
            AgentStats as = (AgentStats) input;
            AcdAgentStats stats = new AcdAgentStats();
            stats.setAgentUri(as.getAgent_uri());
            stats.setCurrentStateMillis(as.getCurrent_state_time() * DateUtils.MILLIS_PER_SECOND);
            stats.setState(AcdAgentStats.State.getEnum(as.getState()));
            stats.setQueues(as.getQueues());
            return stats;
        }
    }

    static final class QueueTransformer implements Transformer {

        public Object transform(Object input) {
            QueueStats qs = (QueueStats) input;
            AcdQueueStats stats = new AcdQueueStats();

            stats.setAverageWaitMillis(qs.getAvg_wait_time() * DateUtils.MILLIS_PER_SECOND);
            stats.setMaxWaitMillis(qs.getMax_wait_time() * DateUtils.MILLIS_PER_SECOND);
            stats.setWaitingCalls(qs.getWaiting_calls());

            stats.setAverageAbandonedMillis(qs.getAvg_abandoned_time() * DateUtils.MILLIS_PER_SECOND);
            stats.setMaxAbandonedMillis(qs.getMax_abandoned_time() * DateUtils.MILLIS_PER_SECOND);
            stats.setAbandonedCalls(qs.getAbandoned_calls());

            stats.setAverageProcessingMillis(qs.getAvg_processing_time() * DateUtils.MILLIS_PER_SECOND);
            stats.setMaxProcessingMillis(qs.getMax_processing_time() * DateUtils.MILLIS_PER_SECOND);
            stats.setProcessedCalls(qs.getProcessed_calls());

            stats.setBusyAgents(qs.getBusy_agents());
            stats.setIdleAgents(qs.getIdle_agents());
            stats.setQueueUri(qs.getQueue_uri());
            return stats;
        }
    }

    static class CallTransformer implements Transformer {

        public Object transform(Object input) {
            CallStats cs = (CallStats) input;
            AcdCallStats stats = new AcdCallStats();
            stats.setAgentUri(cs.getAgent_uri());
            stats.setCaller(cs.getFrom());
            stats.setCallId(StringUtils.EMPTY); // no call-id available
            stats.setProcessingMillis(cs.getProcessing_time() * DateUtils.MILLIS_PER_SECOND);
            stats.setQueueWaitMillis(cs.getQueue_wait_time() * DateUtils.MILLIS_PER_SECOND);
            stats.setQueueUri(cs.getQueue_uri());
            stats.setState(AcdCallStats.State.getEnum(cs.getState()));
            stats.setTotalWaitMillis(cs.getTotal_wait_time() * DateUtils.MILLIS_PER_SECOND);
            return stats;
        }
    }

    static class AgentFilterByQueueUri implements Predicate {
        private final String m_queueUri;

        public AgentFilterByQueueUri(String queueUri) {
            m_queueUri = queueUri;
        }

        public boolean evaluate(Object object) {
            AcdAgentStats acdAgentStats = (AcdAgentStats) object;
            boolean show = acdAgentStats.isInQueue(m_queueUri);
            return show;
        }
    }

    public void setUsers(Collection<User> users) {
        setUsers(toName(users));
    }

    public void setQueues(Collection<AcdQueue> queues) {
        setQueues(toName(queues));
    }

    void setUsers(Set<String> users) {
        Predicate filter = new AgentNameFilter(users, true);
        m_callFilter = new AndPredicate(m_callFilter, filter);
        m_agentFilter = new AndPredicate(m_agentFilter, filter);
        // queues not rellevant
    }

    public String getQueueUri() {
        return m_queueUri;
    }

    void setQueues(Set<String> queues) {
        Predicate filter = new QueueNameFilter(queues);
        m_queueFilter = new AndPredicate(m_queueFilter, filter);
        m_callFilter = new AndPredicate(m_callFilter, filter);
        // agents not rellevant
    }

    static Set<String> toName(Collection namedObjects) {
        Set<String> names = new HashSet<String>(namedObjects.size());
        CollectionUtils.collect(namedObjects, new NamedObject.ToName(), names);
        return names;
    }
}
