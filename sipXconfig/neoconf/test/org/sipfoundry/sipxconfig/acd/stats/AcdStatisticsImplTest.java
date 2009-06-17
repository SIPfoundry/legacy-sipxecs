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
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import junit.framework.TestCase;

import org.apache.commons.collections.Predicate;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.acd.stats.AcdStatisticsImpl.AgentNameFilter;
import org.sipfoundry.sipxconfig.acd.stats.AcdStatisticsImpl.AgentTransformer;
import org.sipfoundry.sipxconfig.acd.stats.AcdStatisticsImpl.CallTransformer;
import org.sipfoundry.sipxconfig.acd.stats.AcdStatisticsImpl.QueueTransformer;
import org.sipfoundry.sipxconfig.common.User;

public class AcdStatisticsImplTest extends TestCase {

    public void testQueueFilter() throws Exception {
        AcdQueueStats qs = new AcdQueueStats();
        qs.setQueueUri("sip:abc@example.org");

        Set<String> name = Collections.singleton("abc");
        Predicate filter = new AcdStatisticsImpl.QueueNameFilter(name);
        assertTrue(filter.evaluate(qs));

        qs.setQueueUri("sip:something_else@example.org");
        assertFalse(filter.evaluate(qs));

        try {
            assertFalse(filter.evaluate(new AcdAgentStats()));
            fail();
        } catch (ClassCastException e) {
            assertTrue(true);
        }
    }

    public void testQueueTransformer() throws Exception {
        QueueTransformer transformer = new QueueTransformer();

        QueueStats qs = new QueueStats();

        qs.setAvg_wait_time(10);
        qs.setMax_wait_time(20);
        qs.setWaiting_calls(5);

        qs.setAbandoned_calls(1);
        qs.setAvg_abandoned_time(3);
        qs.setMax_abandoned_time(5);

        qs.setProcessed_calls(2);
        qs.setAvg_processing_time(33);
        qs.setMax_processing_time(50);

        qs.setBusy_agents(2);
        qs.setIdle_agents(15);
        qs.setQueue_uri("sip:abc@example.org");

        Object result = transformer.transform(qs);
        assertTrue(result instanceof AcdQueueStats);
        AcdQueueStats acdQs = (AcdQueueStats) result;

        assertEquals(10 * 1000, acdQs.getAverageWaitMillis());
        assertEquals(20 * 1000, acdQs.getMaxWaitMillis());
        assertEquals(5, acdQs.getWaitingCalls());

        assertEquals(33 * 1000, acdQs.getAverageProcessingMillis());
        assertEquals(50 * 1000, acdQs.getMaxProcessingMillis());
        assertEquals(2, acdQs.getProcessedCalls());

        assertEquals(3 * 1000, acdQs.getAverageAbandonedMillis());
        assertEquals(5 * 1000, acdQs.getMaxAbandonedMillis());
        assertEquals(1, acdQs.getAbandonedCalls());

        assertEquals(2, acdQs.getBusyAgents());
        assertEquals(15, acdQs.getIdleAgents());
        assertEquals("sip:abc@example.org", acdQs.getQueueUri());
        assertEquals("abc", acdQs.getQueueName());
    }

    public void testAgentTransformer() throws Exception {
        AgentTransformer transformer = new AgentTransformer();

        AgentStats as = new AgentStats();
        as.setCurrent_state_time(10);
        as.setAgent_uri("sip:abc@example.org");
        as.setState("busy");
        as.setQueues(new String[] {
            "sip:queue@example.org"
        });

        Object result = transformer.transform(as);
        assertTrue(result instanceof AcdAgentStats);
        AcdAgentStats acdAs = (AcdAgentStats) result;

        assertEquals(10 * 1000, acdAs.getCurrentStateMillis());
        assertEquals("sip:abc@example.org", acdAs.getAgentUri());
        assertEquals("abc", acdAs.getAgentName());
        assertEquals(AcdAgentStats.State.BUSY, acdAs.getState());
        assertTrue(acdAs.isInQueue("sip:queue@example.org"));
    }

    public void testCallTransformer() throws Exception {
        CallTransformer transformer = new CallTransformer();

        CallStats cs = new CallStats();
        cs.setProcessing_time(10);
        cs.setQueue_wait_time(15);
        cs.setTotal_wait_time(20);
        cs.setFrom("1234@example.org");
        cs.setAgent_uri("agent@example.org");
        cs.setQueue_uri("queue@example.org");
        cs.setState("in_progress");

        Object result = transformer.transform(cs);
        assertTrue(result instanceof AcdCallStats);
        AcdCallStats acdCs = (AcdCallStats) result;

        assertEquals(10 * 1000, acdCs.getProcessingMillis());
        assertEquals(15 * 1000, acdCs.getQueueWaitMillis());
        assertEquals(20 * 1000, acdCs.getTotalWaitMillis());
        assertEquals("agent", acdCs.getAgentName());
        assertEquals("agent@example.org", acdCs.getAgentUri());
        assertEquals("queue", acdCs.getQueueName());
        assertEquals("queue@example.org", acdCs.getQueueUri());
        assertEquals("1234", acdCs.getCallerName());
        assertEquals("1234@example.org", acdCs.getCaller());
        assertEquals(AcdCallStats.State.IN_PROGRESS, acdCs.getState());
    }

    public void testGetCallStats() throws Exception {

        CallStats[] stats = new CallStats[3];
        for (int i = 0; i < stats.length; i++) {
            CallStats cs = new CallStats();
            cs.setProcessing_time(10);
            cs.setQueue_wait_time(15);
            cs.setTotal_wait_time(20);
            cs.setFrom("1234@example.org");
            cs.setAgent_uri("agent@example.org");
            cs.setQueue_uri("sip:queue" + i + "@example.org");
            cs.setState("in_progress");
            stats[i] = cs;
        }

        IMocksControl acdStatsServiceCtrl = EasyMock.createControl();
        AcdStatsService acdStatsService = acdStatsServiceCtrl.createMock(AcdStatsService.class);
        acdStatsService.getCallStats();
        acdStatsServiceCtrl.andReturn(stats).times(2);
        acdStatsServiceCtrl.replay();

        AdcStatsContextMock statsContext = new AdcStatsContextMock(acdStatsService);

        // filtered query
        List callsStats = statsContext.getCallsStats(null, "sip:queue1@example.org");
        assertEquals(1, callsStats.size());
        AcdCallStats acdCs = (AcdCallStats) callsStats.get(0);
        assertEquals("sip:queue1@example.org", acdCs.getQueueUri());

        // unfiltered query
        statsContext = new AdcStatsContextMock(acdStatsService);
        callsStats = statsContext.getCallsStats(null, null);
        assertEquals(3, callsStats.size());

        acdStatsServiceCtrl.verify();
    }

    public void testGetAgentStats() throws Exception {

        AgentStats[] stats = new AgentStats[3];
        for (int i = 0; i < stats.length; i++) {
            AgentStats as = new AgentStats();
            as.setAgent_uri("agent" + i + "@example.org");
            as.setCurrent_state_time(10);
            String[] queues = {
                "sip:all@example.org", "sip:queue" + i + "@example.org"
            };
            as.setQueues(queues);
            as.setState("idle");
            stats[i] = as;
        }

        IMocksControl acdStatsServiceCtrl = EasyMock.createControl();
        AcdStatsService acdStatsService = acdStatsServiceCtrl.createMock(AcdStatsService.class);
        acdStatsService.getAgentStats();
        acdStatsServiceCtrl.andReturn(stats).times(3);
        acdStatsServiceCtrl.replay();

        AdcStatsContextMock statsContext = new AdcStatsContextMock(acdStatsService);

        // filtered query
        List agentStats = statsContext.getAgentsStats(null, "sip:queue1@example.org");
        assertEquals(1, agentStats.size());
        AcdAgentStats acdAs = (AcdAgentStats) agentStats.get(0);
        assertEquals("agent1@example.org", acdAs.getAgentUri());

        // unfiltered query
        statsContext = new AdcStatsContextMock(acdStatsService);
        agentStats = statsContext.getAgentsStats(null, null);
        assertEquals(3, agentStats.size());

        // filtered
        statsContext = new AdcStatsContextMock(acdStatsService);
        agentStats = statsContext.getAgentsStats(null, "sip:all@example.org");
        assertEquals(3, agentStats.size());

        acdStatsServiceCtrl.verify();
    }

    private static class AdcStatsContextMock extends AcdStatisticsImpl {

        private final AcdStatsService m_service;

        AdcStatsContextMock(AcdStatsService service) {
            super(null);
            m_service = service;
        }

        @Override
        public AcdStatsService getAcdStatsService(Serializable acdServerId) {
            return m_service;
        }
    }

    public void testAgentNameFilterAgentStats() {
        Set<String> names = new HashSet();
        names.add("agent0");
        Predicate p = new AgentNameFilter(names, true);

        AcdAgentStats stat = new AcdAgentStats();
        assertTrue(p.evaluate(stat));

        stat.setAgentUri("agent0@example.org");
        assertTrue(p.evaluate(stat));

        stat.setAgentUri("agent1@example.org");
        assertFalse(p.evaluate(stat));

        p = new AgentNameFilter(names, false);
        assertFalse(p.evaluate(stat));
    }

    public void testAgentNameFilterCallStats() {
        Set<String> names = new HashSet();
        names.add("agent0");
        Predicate p = new AgentNameFilter(names, true);

        AcdCallStats stat = new AcdCallStats();
        stat.setAgentUri("agent0@example.org");
        assertTrue(p.evaluate(stat));
        stat.setAgentUri("agent1@example.org");
        assertFalse(p.evaluate(stat));
    }

    public void testFilteredUsersAgentStats() throws Exception {
        // FIXME: Same seed as other test
        AgentStats[] stats = new AgentStats[3];
        for (int i = 0; i < stats.length; i++) {
            stats[i] = new AgentStats();
            stats[i].setAgent_uri("agent" + i + "@example.org");
            stats[i].setCurrent_state_time(10);
            String[] queues = {
                "sip:all@example.org", "sip:queue" + i + "@example.org"
            };
            stats[i].setQueues(queues);
            stats[i].setState("idle");
        }

        IMocksControl acdStatsServiceCtrl = EasyMock.createControl();
        AcdStatsService acdStatsService = acdStatsServiceCtrl.createMock(AcdStatsService.class);
        acdStatsService.getAgentStats();
        acdStatsServiceCtrl.andReturn(stats);
        acdStatsServiceCtrl.replay();

        AdcStatsContextMock statsContext = new AdcStatsContextMock(acdStatsService);
        Set<String> names = new HashSet();
        names.add("agent0");
        statsContext.setUsers(names);
        List agentStats = statsContext.getAgentsStats(null, null);
        assertEquals(1, agentStats.size());
        AcdAgentStats stat = (AcdAgentStats) agentStats.get(0);
        assertEquals("agent0", stat.getAgentName());
    }

    public void testFilteredCallStats() throws Exception {
        // FIXME: Same seed as other test
        CallStats[] stats = new CallStats[4];
        for (int i = 0; i < stats.length; i++) {
            stats[i] = new CallStats();
            stats[i].setAgent_uri("agent" + i + "@example.org");
            stats[i].setQueue_uri("q" + i + "@example.org");
        }
        // first call not picked-up should show-up
        stats[0].setAgent_uri(null);

        IMocksControl acdStatsServiceCtrl = EasyMock.createControl();
        AcdStatsService acdStatsService = acdStatsServiceCtrl.createMock(AcdStatsService.class);
        acdStatsService.getCallStats();
        acdStatsServiceCtrl.andReturn(stats);
        acdStatsServiceCtrl.replay();

        AdcStatsContextMock statsContext = new AdcStatsContextMock(acdStatsService);

        statsContext.setUsers(Collections.singleton("agent1"));

        Set<String> queues = new HashSet<String>();
        queues.add("q0");
        queues.add("q1");
        statsContext.setQueues(queues);

        List callStats = statsContext.getCallsStats(null, null);
        assertEquals(2, callStats.size());
        AcdCallStats stat0 = (AcdCallStats) callStats.get(0);
        assertNull(stat0.getAgentName());
        // should allow agents
        AcdCallStats stat3 = (AcdCallStats) callStats.get(1);
        assertEquals("agent1", stat3.getAgentName());
    }

    public void testToName() {
        User user = new User();
        user.setUserName("greebe");
        Collection objs = Collections.singletonList(user);
        Collection names = AcdStatisticsImpl.toName(objs);
        assertEquals(1, names.size());
        assertEquals("greebe", names.iterator().next());
    }
}
