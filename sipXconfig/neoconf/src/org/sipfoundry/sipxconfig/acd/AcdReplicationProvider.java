/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.ReplicableProvider;

public class AcdReplicationProvider implements ReplicableProvider {
    private AcdContext m_acdContext;

    @Override
    public List<Replicable> getReplicables() {
        List<Replicable> replicables = new ArrayList<Replicable>();
        List<AcdLine> acdLines = m_acdContext.getLines();
        List<AcdServer> servers = m_acdContext.getServers();
        for (AcdLine line : acdLines) {
            replicables.add(line);
        }
        for (AcdServer server : servers) {
            replicables.add(server);
        }
        return replicables;
    }

    public AcdContext getAcdContext() {
        return m_acdContext;
    }

    public void setAcdContext(AcdContext acdContext) {
        m_acdContext = acdContext;
    }

}
