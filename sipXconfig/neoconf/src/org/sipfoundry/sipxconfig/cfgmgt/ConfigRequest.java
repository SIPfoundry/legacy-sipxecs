/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;



import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.springframework.util.CollectionUtils;

/**
 * Provides a general area where configuration needs to be rebuilt. Once generated into proper
 * location, all cfengine scripts on all hosts will be notified to pull information.
 */
public final class ConfigRequest {
    private Set<Feature> m_affectedFeatures;
    private boolean m_always;
    private Set<Location> m_locations;

    // NOTE: not sure this is nec. Idea was to allow singleton providers a place to store state
    // per request. Consider removing --Douglas
    private Set<Object> m_tokens = new HashSet<Object>();

    private ConfigRequest() {
    }

    /**
     * Only configuration in these areas
     */
    public static ConfigRequest only(Feature...affectedFeatures) {
        ConfigRequest request = new ConfigRequest();
        request.m_affectedFeatures = new HashSet<Feature>(affectedFeatures.length);
        for (Feature f : affectedFeatures) {
            request.m_affectedFeatures.add(f);
        }
        return request;
    }

    /**
     * Only configuration in these areas
    public static ConfigRequest only(Set<Feature> affectedFeatures) {
        ConfigRequest request = new ConfigRequest();
        request.m_affectedFeatures = affectedFeatures;
        return request;
    }
     */

    /**
     * All configuration should be generated
     */
    public static ConfigRequest always() {
        ConfigRequest request = new ConfigRequest();
        request.m_always = true;
        return request;
    }

    public static ConfigRequest only(Collection<Location> locations) {
        ConfigRequest request = new ConfigRequest();
        // currently if you specify a location, then it's all features.  Single feature at a single location
        // is not supported.
        request.m_always = true;
        request.m_locations = new HashSet<Location>(locations);
        return request;
    }

    /*
    public static ConfigRequest specificLocation(Location...location) {
        return ConfigRequest.specificLocations(Collections.singleton(location));
    }
    */

    public static ConfigRequest merge(ConfigRequest a, ConfigRequest b) {
        if (a == null) {
            return b;
        }
        if (b == null) {
            return a;
        }
        ConfigRequest m = new ConfigRequest();
        m.m_always = a.m_always || b.m_always;
        m.m_locations = mergeSet(a.m_locations, b.m_locations);
        m.m_affectedFeatures = mergeSet(a.m_affectedFeatures, b.m_affectedFeatures);
        return m;
    }

    private static <T> Set<T> mergeSet(Set<T> a, Set<T> b) {
        if (a == null) {
            return b;
        }
        if (b == null) {
            return a;
        }
        Set<T> m = new HashSet<T>(a);
        m.addAll(b);
        return m;
    }

    public boolean isFirstTime(Object token) {
        return m_tokens.contains(token);
    }

    public void firstTimeOver(Object token) {
        m_tokens.add(token);
    }

    /**
     * Test if you're ConfigProvider implementation should generate new configs
     */
    public boolean applies(Collection<Feature> features) {
        return m_always || CollectionUtils.containsAny(m_affectedFeatures, features);
    }

    /**
     * Test if you're ConfigProvider implementation should generate new configs
     */
    public boolean applies(Feature ... features) {
        return m_always || CollectionUtils.containsAny(m_affectedFeatures, Arrays.asList(features));
    }

    public Set<Location> locations(ConfigManager manager) {
        if (m_locations == null) {
            m_locations = new HashSet<Location>(manager.getLocationManager().getLocationsList());
        }

        return m_locations;
    }
}
