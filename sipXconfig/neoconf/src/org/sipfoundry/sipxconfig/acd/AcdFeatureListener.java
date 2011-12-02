/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd;

import java.util.List;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.feature.SimpleFeatureListener;
import org.springframework.beans.factory.annotation.Required;

public class AcdFeatureListener extends SimpleFeatureListener {
    private AcdContext m_acdContext;

    @Override
    public void enableLocationFeature(FeatureEvent event, LocationFeature feature, Location location) {
        if (Acd.FEATURE.equals(feature)) {
            if (event == FeatureEvent.PRE_ENABLE) {
                m_acdContext.addNewServer(location);
            } else if (event == FeatureEvent.POST_DISABLE) {
                List<AcdServer> servers = m_acdContext.getServers();
                m_acdContext.removeServers(servers);
            }
        }
    }

    @Required
    public void setAcdContext(AcdContext acdContext) {
        m_acdContext = acdContext;
    }
}
