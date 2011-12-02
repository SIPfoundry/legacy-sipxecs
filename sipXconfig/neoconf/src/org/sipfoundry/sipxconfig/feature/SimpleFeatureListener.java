/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.feature;

import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class SimpleFeatureListener implements FeatureListener {

    @Override
    public void enableLocationFeature(FeatureEvent event, LocationFeature feature, Location location) {
    }

    @Override
    public void enableGlobalFeature(FeatureEvent event, GlobalFeature feature) {
    }
}
