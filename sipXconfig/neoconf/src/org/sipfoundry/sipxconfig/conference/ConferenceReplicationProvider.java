/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
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
