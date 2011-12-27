/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.feature;

import java.util.Collections;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class FeatureManagerTestIntegration extends IntegrationTestCase {
    FeatureManager m_featureManager;
    
    public void testEnableGlobalFeatures() {
        deleteFromTables(new String[] { "feature_global" });
        GlobalFeature f = new GlobalFeature("testEnableGlobalFeatures");
        assertFalse(m_featureManager.isFeatureEnabled(f));
        m_featureManager.enableGlobalFeatures(Collections.singleton(f));
        assertTrue(m_featureManager.isFeatureEnabled(f));
    }

    public void testEnableLocationFeatures() {
        deleteFromTables(new String[] { "feature_location" });
        Location l = new Location();
        LocationFeature f = new LocationFeature("testEnableLocationFeatures");
        assertFalse(m_featureManager.isFeatureEnabled(f, l));
        m_featureManager.enableLocationFeatures(Collections.singleton(f), l);
        assertTrue(m_featureManager.isFeatureEnabled(f, l));
    }

    public FeatureManager getFeatureManager() {
        return m_featureManager;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }
}
