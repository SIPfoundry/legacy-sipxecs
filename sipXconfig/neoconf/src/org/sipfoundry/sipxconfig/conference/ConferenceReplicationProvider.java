/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.ReplicableProvider;

public class ConferenceReplicationProvider implements ReplicableProvider {
    private ConferenceBridgeContext m_bridgeContext;

    @Override
    public List<Replicable> getReplicables() {
        List<Replicable> replicables = new ArrayList<Replicable>();
        for (Conference conf : m_bridgeContext.getAllConferences()) {
            replicables.add(conf);
        }
        return replicables;
    }

    public void setBridgeContext(ConferenceBridgeContext bridgeContext) {
        m_bridgeContext = bridgeContext;
    }
}
