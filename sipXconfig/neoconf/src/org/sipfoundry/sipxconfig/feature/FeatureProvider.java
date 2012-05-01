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

import java.util.Collection;

import org.sipfoundry.sipxconfig.commserver.Location;

/**
 * Implement this to register new features to the system. Manager will ask user which features
 * they want enabled on a system or in a specific location. To find out what the user has enabled,
 * consult FeatureManager.
 */
public interface FeatureProvider extends FeatureListener {

    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager);

    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l);

    public void getBundleFeatures(FeatureManager featureManager, Bundle b);
}
