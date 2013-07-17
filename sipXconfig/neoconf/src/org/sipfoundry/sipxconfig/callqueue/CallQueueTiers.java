/*
 * Copyright (C) 2013 SibTelCom, JSC., certain elements licensed under a Contributor Agreement.
 * Author: Konstantin S. Vishnivetsky
 * E-mail: info@siplabs.ru
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
*/

package org.sipfoundry.sipxconfig.callqueue;

import java.util.Set;
import java.util.HashSet;

public class CallQueueTiers {
    private Set<CallQueueTier> m_tiers = new HashSet<CallQueueTier>();

    public void setTiers(Set<CallQueueTier> tiers) {
        m_tiers = tiers;
    }

    public Set<CallQueueTier> getTiers() {
        return m_tiers;
    }

    public void addTier(Integer callQueueId, Integer callQueueAgentId) {
        CallQueueTier newTier = new CallQueueTier();
        newTier.setCallQueueId(callQueueId);
        newTier.setCallQueueAgentId(callQueueAgentId);
        m_tiers.add(newTier);
    }

    public void removeFromQueue(Integer callqueueid) {
        Set<CallQueueTier> tiers = getTiers();
        for (CallQueueTier callqueuetier : tiers) {
            if (callqueuetier.getCallQueueId().equals(callqueueid)) {
                tiers.remove(callqueuetier);
                break;
            }
        }
    }

    public void copyTiersTo(Set<CallQueueTier> dst) {
        if (null != dst) {
            for (CallQueueTier callqueuetier : m_tiers) {
                CallQueueTier newTier = new CallQueueTier();
                callqueuetier.copyTo(newTier);
                dst.add(newTier);
            }
        }
    }
}
