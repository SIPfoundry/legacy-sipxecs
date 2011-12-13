/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.feature;

import org.sipfoundry.sipxconfig.commserver.Location;

public interface FeatureListener {

    public static enum FeatureEvent {
        PRE_ENABLE, POST_ENABLE, PRE_DISABLE, POST_DISABLE
    };

    public void enableLocationFeature(FeatureManager manager, FeatureEvent event, LocationFeature feature,
            Location location);

    public void enableGlobalFeature(FeatureManager manager, FeatureEvent event, GlobalFeature feature);
}
