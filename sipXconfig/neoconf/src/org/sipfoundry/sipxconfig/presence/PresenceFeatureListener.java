/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.presence;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.feature.SimpleFeatureListener;
import org.springframework.beans.factory.annotation.Required;

public class PresenceFeatureListener extends SimpleFeatureListener {
    private PresenceServer m_presenceServer;

    public void enableLocationFeature(FeatureEvent event, LocationFeature feature, Location location) {
        if (feature.equals(PresenceServer.FEATURE)) {
            if (event == FeatureEvent.PRE_ENABLE) {
                m_presenceServer.initialize();
            }
        }
    }

    @Required
    public void setPresenceServer(PresenceServer presenceServer) {
        m_presenceServer = presenceServer;
    }
}
