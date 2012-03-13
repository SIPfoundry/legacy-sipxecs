/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.feature;

import java.util.Collection;

import org.sipfoundry.sipxconfig.commserver.Location;

/**
 * Implement this to register new features to the system. Manager will ask user which features
 * they want enabled on a system or in a specific location. To find out what the user has enabled,
 * consult FeatureManager.
 */
public interface FeatureProvider {

    public Collection<GlobalFeature> getAvailableGlobalFeatures();

    public Collection<LocationFeature> getAvailableLocationFeatures(Location l);

    public void getBundleFeatures(Bundle b);
}
