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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.util.Arrays;
import java.util.List;
import java.util.Map;

import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;

public class RegionTest {

	@Test
	public void locationsByRegion() {
		Location[] locations = new Location[] {
				new Location("one"),
				new Location("two"),
				new Location("three"),
				new Location("four"),
				new Location("five")
		};
		locations[0].setRegionId(1);
		locations[1].setRegionId(1);
		locations[2].setRegionId(2);
		Map<Integer, List<Location>> actual = Region.locationsByRegion(Arrays.asList(locations));
		assertEquals(2, actual.size());
		assertEquals(2, actual.get(1).size());
		assertTrue(actual.get(1).contains(locations[0]));
		assertTrue(actual.get(1).contains(locations[1]));
		assertEquals(1, actual.get(2).size());		
		assertTrue(actual.get(2).contains(locations[2]));
	}
}
