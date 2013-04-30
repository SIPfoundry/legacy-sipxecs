/**
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


import static org.easymock.EasyMock.anyObject;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class FeatureChangeValidatorTest {

    @Before
    public void setUp() {
        TestHelper.initDefaultDomain();
    }
    
    @Test
    public void requiresGlobalFeature() {
        GlobalFeature a = new GlobalFeature("a"); 
        GlobalFeature b = new GlobalFeature("b");
        
        FeatureManager manager = getDummyManager();        
        
        Set<GlobalFeature> enable = new HashSet<GlobalFeature>();
        enable.add(a);
        FeatureChangeRequest request = FeatureChangeRequest.enable(enable, true);
        FeatureChangeValidator validator = new FeatureChangeValidator(manager, request);
        
        validator.requiresGlobalFeature(a, b);
        
        assertFalse(validator.isValid());        
        assertEquals(1, validator.getInvalidChanges().size());
        InvalidChange item = validator.getInvalidChanges().get(0);
        assertEquals(b, item.getFeature());
    }
    
    FeatureManager getDummyManager() {
        FeatureManager manager = createMock(FeatureManager.class);
        manager.getEnabledLocationFeatures();
        expectLastCall().andReturn(Collections.emptySet()).anyTimes();
        manager.getEnabledGlobalFeatures();
        expectLastCall().andReturn(Collections.emptySet()).anyTimes();
        manager.isFeatureEnabled((GlobalFeature) anyObject());
        expectLastCall().andReturn(false).anyTimes();
        manager.getLocationsForEnabledFeature((LocationFeature) anyObject());
        expectLastCall().andReturn(Collections.emptyList()).anyTimes();
        replay(manager);
        return manager;
    }
    
    @Test
    public void requiredOnSameHostWithResolver() {
        LocationFeature a = new LocationFeature("a"); 
        LocationFeature b = new LocationFeature("b");
        FeatureManager manager = getDummyManager();
        Location l1 = new Location("l1");
        l1.setUniqueId(1);
        Location l2 = new Location("l2");
        l2.setUniqueId(2);
        Set<LocationFeature> enable1 = new HashSet<LocationFeature>();
        enable1.add(a);
        Set<LocationFeature> enable2 = new HashSet<LocationFeature>();
        Map<Location, Set<LocationFeature>> enable = new HashMap<Location, Set<LocationFeature>>();
        enable.put(l1, enable1);
        enable.put(l2, enable2);
        FeatureChangeRequest request = FeatureChangeRequest.enable(enable, true);
        FeatureChangeValidator validator = new FeatureChangeValidator(manager, request);
        
        validator.requiredOnSameHost(a, b);
        assertFalse(validator.isValid());        
        assertEquals(1, validator.getInvalidChanges().size());
        InvalidChange item = validator.getInvalidChanges().get(0);
        assertEquals(b, item.getFeature());
        
        InvalidChangeResolver resolver = new InvalidChangeResolver();
        assertTrue(resolver.resolve(validator, l1));
        assertFalse(resolver.resolve(validator, l1));
    }
}
