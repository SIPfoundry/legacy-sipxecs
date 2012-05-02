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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;

public class InvalidChangeResolverTest {
    
    @Test
    public void resolve() {
        GlobalFeature g1 = new GlobalFeature("g1");
        GlobalFeature g2 = new GlobalFeature("g2");
        Set<GlobalFeature> enabled  = new HashSet<GlobalFeature>();
        enabled.add(g1);
        FeatureChangeRequest request = FeatureChangeRequest.enable(enabled, true);
        FeatureManager manager = createMock(FeatureManager.class);
        manager.getEnabledLocationFeatures();
        expectLastCall().andReturn(Collections.emptySet()).once();
        manager.getEnabledGlobalFeatures();
        expectLastCall().andReturn(Collections.emptySet()).once();
        replay(manager);
        FeatureChangeValidator validator = new FeatureChangeValidator(manager, request);
        validator.getInvalidChanges().add(InvalidChange.requires(g1, g2));
        InvalidChangeResolver resolver = new InvalidChangeResolver();
        Location primary = new Location("primary");
        assertEquals(1, validator.getInvalidChanges().size());
        boolean changed = resolver.resolve(validator, primary);
        assertTrue(changed);
        assertTrue(validator.getRequest().getEnable().contains(g2));
        assertEquals(0, validator.getInvalidChanges().size());        
        boolean changedAgain = resolver.resolve(validator, primary);
        assertFalse(changedAgain);
        verify(manager);
    }
}
