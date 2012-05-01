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
package org.sipfoundry.sipxconfig.feature;

import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.commserver.Location;

/**
 * Simple flags denoting that a feature has been enable for disabled globally or to a specific location.
 * The implications of a feature on/off is up to the spring beans to decide.
 */
public interface FeatureManager {

    public Set<GlobalFeature> getEnabledGlobalFeatures();

    public Set<LocationFeature> getEnabledLocationFeatures();

    public Set<LocationFeature> getEnabledLocationFeatures(Location location);

    public void enableLocationFeature(LocationFeature feature, Location location, boolean enable);

    public void enableLocationFeatures(Set<LocationFeature> features, Location location, boolean enable);

    public void enableGlobalFeature(GlobalFeature feature, boolean enable);

    public void enableGlobalFeatures(Set<GlobalFeature> features, boolean enable);

    void validateFeatureChange(FeatureChangeValidator validator);

    /**
     * Will validate change before applying so no need to call validateFeatureChange manually
     */
    public void applyFeatureChange(FeatureChangeValidator validator);

    public boolean isFeatureEnabled(LocationFeature feature, Location location);

    /**
     * At one or more locations
     */
    public boolean isFeatureEnabled(LocationFeature feature);

    public boolean isFeatureEnabled(GlobalFeature feature);

    public Set<GlobalFeature> getAvailableGlobalFeatures();

    public Set<LocationFeature> getAvailableLocationFeatures(Location location);

    public List<Location> getLocationsForEnabledFeature(LocationFeature feature);

    public Bundle getBundle(String id);

    public List<Bundle> getBundles();

//    public void enableBundleOnPrimary(Bundle b);
}
