/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.feature;

import java.util.Collection;
import java.util.Set;

import org.sipfoundry.sipxconfig.commserver.Location;

public class SimpleConstraint implements BundleConstraint {

    @Override
    public boolean isSingleLocation(FeatureManager manager, Feature feature) {
        return false;
    }

    @Override
    public boolean isLocationDependent(FeatureManager manager, Feature feature) {
        return feature instanceof GlobalFeature;
    }

    @Override
    public Collection<Location> getApplicableLocations(FeatureManager manager, Feature feature,
            Set<Feature> currentlyEnabledFeatures, Collection<Location> locations) {
        return locations;
    }
}
