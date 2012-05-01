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


import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.apache.commons.collections.Transformer;
import org.sipfoundry.sipxconfig.commserver.Location;

public class FeatureChangeRequest {
    private Set<GlobalFeature> m_enable;
    private Set<GlobalFeature> m_disable;
    private Map<Location, Set<LocationFeature>> m_enableByLocation;
    private Map<Location, Set<LocationFeature>> m_disableByLocation;

    public FeatureChangeRequest(Set<GlobalFeature> enable, Set<GlobalFeature> disable,
            Map<Location, Set<LocationFeature>> enableByLocation,
            Map<Location, Set<LocationFeature>> disableByLocation) {
        if (enable != null) {
            m_enable = enable;
        } else {
            m_enable = new HashSet<GlobalFeature>(0);
        }
        if (disable != null) {
            m_disable = disable;
        } else {
            m_disable = new HashSet<GlobalFeature>(0);
        }
        if (enableByLocation != null) {
            m_enableByLocation = enableByLocation;
        } else {
            m_enableByLocation = new HashMap<Location, Set<LocationFeature>>(0);
        }
        if (disableByLocation != null) {
            m_disableByLocation = disableByLocation;
        } else {
            m_disableByLocation = new HashMap<Location, Set<LocationFeature>>(0);
        }
    }

    public static FeatureChangeRequest enable(Set<GlobalFeature> global, boolean enable) {
        if (enable) {
            return new FeatureChangeRequest(global, null, null, null);
        }
        return new FeatureChangeRequest(null, global, null, null);
    }

    public static FeatureChangeRequest enable(Map<Location, Set<LocationFeature>> byLocation, boolean enable) {
        if (enable) {
            return new FeatureChangeRequest(null, null, byLocation, null);
        }
        return new FeatureChangeRequest(null, null, null, byLocation);
    }

    public static FeatureChangeRequest byBundle(Bundle bundle, Set<GlobalFeature> enable,
            Map<Location, Set<LocationFeature>> enableByLocation) {
        Collection<GlobalFeature> bundleGlobalFeatures = bundle.getGlobalFeatures();
        Collection<LocationFeature> bundleLocationFeatures = bundle.getLocationFeatures();
        Set<GlobalFeature> disable = new HashSet<GlobalFeature>(CollectionUtils.disjunction(bundleGlobalFeatures,
                enable));
        Map<Location, Set<LocationFeature>> disableByLocation = new HashMap<Location, Set<LocationFeature>>(
                enableByLocation.size());
        for (Entry<Location, Set<LocationFeature>> entry : enableByLocation.entrySet()) {
            Set<LocationFeature> offAtLocation = new HashSet<LocationFeature>(CollectionUtils.disjunction(
                    bundleLocationFeatures, entry.getValue()));
            disableByLocation.put(entry.getKey(), offAtLocation);
        }
        return new FeatureChangeRequest(enable, disable, enableByLocation, disableByLocation);
    }

    public Set<GlobalFeature> getEnable() {
        return m_enable;
    }

    public Set<GlobalFeature> getDisable() {
        return m_disable;
    }

    public Map<Location, Set<LocationFeature>> getEnableByLocation() {
        return m_enableByLocation;
    }

    public Map<Location, Set<LocationFeature>> getDisableByLocation() {
        return m_disableByLocation;
    }

    public Collection<Location> getLocationsForEnabledFeature(LocationFeature f) {
        return findLocationsByFeature(f, m_enableByLocation);
    }

    public Collection<Location> getLocationsForDisabledFeature(LocationFeature f) {
        return findLocationsByFeature(f, m_disableByLocation);
    }

    public Set<Feature> getAllNewlyEnabledFeatures() {
        return all(m_enable, m_enableByLocation);
    }

    public Set<Feature> getAllNewlyDisabledFeatures() {
        return all(m_disable, m_disableByLocation);
    }

    public boolean hasChanged(Feature f) {
        if (getAllNewlyEnabledFeatures().contains(f)) {
            return true;
        }
        return getAllNewlyDisabledFeatures().contains(f);
    }

    Set<Feature> all(Set<GlobalFeature> global, Map<Location, Set<LocationFeature>> local) {
        Set<Feature> all = new HashSet<Feature>(global);
        for (Set<LocationFeature> atLocation : local.values()) {
            all.addAll(atLocation);
        }
        return all;
    }

    @SuppressWarnings({
        "unchecked"
    })
    Collection<Location> findLocationsByFeature(LocationFeature f, Map<Location, Set<LocationFeature>> map) {
        LocationByFeature alg = new LocationByFeature(f);
        return (Collection<Location>) CollectionUtils.collect(
                CollectionUtils.select(map.entrySet(), alg), alg);
    }

    static class LocationByFeature implements Transformer, Predicate {
        private LocationFeature m_feature;

        LocationByFeature(LocationFeature feature) {
            m_feature = feature;
        }

        @Override
        public Object transform(Object arg0) {
            @SuppressWarnings("unchecked")
            Entry<Location, Set<LocationFeature>> entry = (Entry<Location, Set<LocationFeature>>) arg0;
            return entry.getKey();
        }

        @Override
        public boolean evaluate(Object arg0) {
            @SuppressWarnings("unchecked")
            Entry<Location, Set<LocationFeature>> entry = (Entry<Location, Set<LocationFeature>>) arg0;
            return entry.getValue().contains(m_feature);
        }
    }
}
