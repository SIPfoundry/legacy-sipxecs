/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.region;

import java.util.List;

import org.sipfoundry.sipxconfig.feature.GlobalFeature;

public interface RegionManager {
    public static final GlobalFeature FEATURE_ID = new GlobalFeature("region");

    public List<Region> getRegions();

    public Region getRegion(int id);

    public void saveRegion(Region region);

    public void deleteRegion(Region region);

    List<Integer> getServersByRegion(int regionId);
}
