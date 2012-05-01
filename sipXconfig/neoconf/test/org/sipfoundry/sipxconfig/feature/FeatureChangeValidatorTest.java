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

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.junit.Test;

public class FeatureChangeValidatorTest {
    
    @Test
    public void requiresGlobalFeature() {
        GlobalFeature a = new GlobalFeature("a"); 
        GlobalFeature b = new GlobalFeature("b"); 

        FeatureManager manager = createMock(FeatureManager.class);
        manager.getEnabledLocationFeatures();
        expectLastCall().andReturn(Collections.emptySet()).once();
        manager.getEnabledGlobalFeatures();
        expectLastCall().andReturn(Collections.emptySet()).once();
        manager.isFeatureEnabled(b);
        expectLastCall().andReturn(false).once();
        replay(manager);
        
        Set<GlobalFeature> global = new HashSet<GlobalFeature>();
        global.add(a);
        FeatureChangeRequest request = FeatureChangeRequest.enable(global, true);
        FeatureChangeValidator validator = new FeatureChangeValidator(manager, request);
        
        validator.requiresGlobalFeature(a, b);
        
        assertFalse(validator.isValid());        
        assertEquals(1, validator.getInvalidChanges().size());
        InvalidChange item = validator.getInvalidChanges().get(0);
        assertEquals(b, item.getFeature());

        verify(manager);
    }
}
