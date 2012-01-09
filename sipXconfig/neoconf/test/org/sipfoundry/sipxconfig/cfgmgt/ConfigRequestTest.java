/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.util.Collections;

import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;

public class ConfigRequestTest {
    private static final Feature FEATURE = new GlobalFeature("ConfigRequestTest");
    private static final Location LOCATION = new Location();
    private ConfigRequest m_always;    
    private ConfigRequest m_location;
    private ConfigRequest m_features;
    
    @Before
    public void setUp() {
        m_always = ConfigRequest.always();
        m_location = ConfigRequest.only(Collections.singleton(LOCATION));
        m_features = ConfigRequest.only(FEATURE);
    }
    
    @Test
    public void features() {
        assertTrue(m_features.applies(FEATURE));        
        assertFalse(m_features.applies(new GlobalFeature("not")));        
    }    
    
    @Test
    public void mergeAlways() {
        assertTrue(ConfigRequest.merge(m_always, m_always).applies(FEATURE));
        assertTrue(ConfigRequest.merge(m_always, null).applies(FEATURE));
        assertTrue(ConfigRequest.merge(null, m_always).applies(FEATURE));
    }
    
    @Test
    public void mergeLocations() {
        Location two = new Location();
        assertTrue(ConfigRequest.merge(null, m_location).applies(FEATURE));
    }

    @Test
    public void mergeFeatures() {
        Location two = new Location();
        assertTrue(ConfigRequest.merge(null, m_features).applies(FEATURE));
        assertFalse(ConfigRequest.merge(null, m_features).applies(new GlobalFeature("not")));
    }
}
