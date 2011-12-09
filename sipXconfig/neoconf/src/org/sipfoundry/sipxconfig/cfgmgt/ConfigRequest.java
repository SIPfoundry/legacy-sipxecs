/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

import org.sipfoundry.sipxconfig.feature.Feature;
import org.springframework.util.CollectionUtils;

import edu.emory.mathcs.backport.java.util.Arrays;

/**
 * Provides a general area where configuration needs to be rebuilt. Once generated into proper
 * location, all cfengine scripts on all hosts will be notified to pull information.
 */
public final class ConfigRequest {
    private Set<Feature> m_affectedFeatures;
    private Set<Object> m_tokens = new HashSet<Object>();
    private boolean m_always;

    private ConfigRequest() {
    }

    /**
     * Only configuration in these areas
     */
    public static ConfigRequest only(Set<Feature> affectedFeatures) {
        ConfigRequest request = new ConfigRequest();
        request.m_affectedFeatures = affectedFeatures;
        return request;
    }

    /**
     * All configuration should be generated
     */
    public static ConfigRequest always() {
        ConfigRequest request = new ConfigRequest();
        request.m_always = true;
        return request;
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

    /**
     * Test if you're ConfigProvider implementation should generate new configs
     */
    public boolean applies(Feature feature) {
        return m_always || m_affectedFeatures.contains(feature);
    }
}
