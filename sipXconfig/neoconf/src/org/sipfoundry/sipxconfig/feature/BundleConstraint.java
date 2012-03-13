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

public interface BundleConstraint {

    public static final BundleConstraint SINGLE_LOCATION = new SimpleConstraint() {
        @Override
        public boolean isSingleLocation(FeatureManager manager, Feature feature) {
            return true;
        }
    };

    public static final BundleConstraint DEFAULT = new SimpleConstraint();

    public boolean isSingleLocation(FeatureManager manager, Feature feature);

    public boolean isLocationDependent(FeatureManager manager, Feature feature);

    public Collection<Location> getApplicableLocations(FeatureManager manager, Feature feature,
            Set<Feature> currentlyEnabledFeatures, Collection<Location> locations);
}
