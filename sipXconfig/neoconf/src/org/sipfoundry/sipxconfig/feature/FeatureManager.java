/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.feature;

import java.util.Set;

import org.sipfoundry.sipxconfig.admin.commserver.Location;

/**
 * Simple flags denoting that a feature has been enable for disabled globally or to a specific location.
 * The implications of a feature on/off is up to the spring beans to decide.
 */
public interface FeatureManager {

    public Set<GlobalFeature> getEnabledGlobalFeatures();

    public Set<LocationFeature> getEnabledLocationFeatures(Location location);

    public void enableLocationFeatures(Set<LocationFeature> features, Location location);

    public void enableGlobalFeatures(Set<GlobalFeature> features);

    public boolean isLocationFeatureEnabled(LocationFeature feature, Location location);

    public boolean isGlobalFeatureEnabled(GlobalFeature feature);

    public Set<GlobalFeature> getAvailableGlobalFeatures();

    public Set<LocationFeature> getAvailableLocationFeatures(Location location);
}
