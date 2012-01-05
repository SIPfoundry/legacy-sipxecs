/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.feature;

import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class FeatureManagerTestIntegration extends IntegrationTestCase {
    private FeatureManager m_featureManager;
    
    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        sql("commserver/SeedLocations.sql");
        sql("feature/seed-feature.sql");        
    }
    
    public void testEnableGlobalFeatures() {
        GlobalFeature f = new GlobalFeature("testEnableGlobalFeatures");
        assertFalse(m_featureManager.isFeatureEnabled(f));
        m_featureManager.enableGlobalFeatures(Collections.singleton(f));
        assertTrue(m_featureManager.isFeatureEnabled(f));
    }

    public void testEnableLocationFeatures() {
        Location l = new Location();
        LocationFeature f = new LocationFeature("testEnableLocationFeatures");
        assertFalse(m_featureManager.isFeatureEnabled(f, l));
        m_featureManager.enableLocationFeatures(Collections.singleton(f), l);
        assertTrue(m_featureManager.isFeatureEnabled(f, l));
    }
    
    public void testGetLocationsForEnabledFeature() throws Exception {
        List<Location> noLocations = m_featureManager.getLocationsForEnabledFeature(new LocationFeature(
                "testGetLocationsForEnabledFeature"));
        assertTrue(noLocations.isEmpty());
        
        LocationFeature bongo = new LocationFeature("bongo");
        List<Location> oneLocation = m_featureManager.getLocationsForEnabledFeature(bongo);
        assertFalse(oneLocation.isEmpty());
        assertEquals((Integer) 101, oneLocation.get(0).getId());
    }

    public FeatureManager getFeatureManager() {
        return m_featureManager;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }
}
