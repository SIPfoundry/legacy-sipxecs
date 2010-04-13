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

import junit.framework.TestCase;

/**
 * Requires stats server to be running
 *
 * cd sipXstats/lib ruby server.rb
 *
 * Unclear how to integrate this
 */
public class AcdStatisticsImplTestStats extends TestCase {

    public void testCallStats() throws Exception {
        AcdStatsService service = new AcdStatisticsImpl(null).getAcdStatsService(null);
        CallStats[] callStats = service.getCallStats();
        assertNotNull(callStats);
    }

    public void testAgentStats() throws Exception {
        AcdStatsService service = new AcdStatisticsImpl(null).getAcdStatsService(null);
        AgentStats[] agentStats = service.getAgentStats();
        assertNotNull(agentStats);
    }

    public void testQueueStats() throws Exception {
        AcdStatsService service = new AcdStatisticsImpl(null).getAcdStatsService(null);
        QueueStats[] queueStats = service.getQueueStats();
        assertNotNull(queueStats);
    }

    public void testBirdArray() throws Exception {
        AcdStatsService service = new AcdStatisticsImpl(null).getAcdStatsService(null);
        Bird[] birds = service.getBirdArray();
        assertEquals("robin", birds[0].getSpecies());
        assertEquals("bluejay", birds[1].getSpecies());
    }
}
