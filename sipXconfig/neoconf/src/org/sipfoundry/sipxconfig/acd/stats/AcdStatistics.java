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
import java.util.List;

import org.sipfoundry.sipxconfig.acd.AcdQueue;
import org.sipfoundry.sipxconfig.common.User;

public interface AcdStatistics {
    /**
     * @param queueUri null is all queues
     */
    List getAgentsStats(Serializable acdServerId, String queueUri);

    /**
     * @param queueUri null is all queues
     */
    List getCallsStats(Serializable acdServerId, String queueUri);

    List getQueuesStats(Serializable acdServerId);

    void setUsers(Collection<User> users);

    void setQueues(Collection<AcdQueue> queues);
}
