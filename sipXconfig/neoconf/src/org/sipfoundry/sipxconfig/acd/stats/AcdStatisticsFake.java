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
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Random;

import org.sipfoundry.sipxconfig.acd.AcdQueue;
import org.sipfoundry.sipxconfig.common.User;

/**
 * AcdStatisticsImpl - dummy implementation for now
 */
public class AcdStatisticsFake implements AcdStatistics {

    private static final String TEST_DOMAIN = "@sipfoundry.org";
    private static final Random RANDOM = new Random();

    public List getAgentsStats(Serializable acdServerId, String queueUri_) {
        List stats = new ArrayList();
        List states = AcdAgentStats.State.getEnumList();
        final int stateLen = states.size();
        for (int i = 0; i < 15; i++) {
            AcdAgentStats agentStats = new AcdAgentStats();
            agentStats.setAgentUri("a" + (100 + i));
            agentStats.setState((AcdAgentStats.State) states.get(i % stateLen));
            agentStats.setCurrentStateMillis(getTestInterval());
            stats.add(agentStats);
        }
        return stats;
    }

    public List getCallsStats(Serializable acdServerId, String queueUri_) {
        List stats = new ArrayList();
        AcdCallStats.State[] states = {
            AcdCallStats.State.IN_PROGRESS, AcdCallStats.State.WAITING
        };
        final int stateLen = states.length;
        for (int i = 0; i < 15; i++) {
            AcdCallStats callStats = new AcdCallStats();
            AcdCallStats.State state = states[i % stateLen];
            callStats.setState(state);
            if (state == AcdCallStats.State.IN_PROGRESS) {
                callStats.setAgentUri("agent" + (100 + i));
                callStats.setProcessingMillis(getTestInterval());
            }
            callStats.setCaller("caller" + (200 + i) + TEST_DOMAIN);
            callStats.setQueueUri("queue" + (200 + i));
            callStats.setTotalWaitMillis(getTestInterval());
            callStats.setQueueWaitMillis(getTestInterval());
            stats.add(callStats);
        }
        return stats;
    }

    public List getQueuesStats(Serializable acdServerId) {
        List stats = new ArrayList();
        for (int i = 0; i < 5; i++) {
            AcdQueueStats aqs = new AcdQueueStats();
            aqs.setQueueUri("q" + i);
            aqs.setIdleAgents(RANDOM.nextInt(15));
            aqs.setBusyAgents(RANDOM.nextInt(15));
            aqs.setAverageWaitMillis(getTestInterval());
            stats.add(aqs);
        }
        return stats;
    }

    // Test only
    private static int getTestInterval() {
        return RANDOM.nextInt(125000);
    }

    public void setUsers(Collection<User> users_) {
        // stub
    }

    public void setQueues(Collection<AcdQueue> queues_) {
        // stub
    }
}
