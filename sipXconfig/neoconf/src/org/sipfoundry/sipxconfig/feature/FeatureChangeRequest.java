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

    /**
     * In these maps we keep only the features that are requested to be
     * enabled/disabled and are not currently enabled/disabled. These are not
     * available by default, you need to populate them when you need them (see
     * ConfigChange)
     */
    private Set<GlobalFeature> m_newlyEnable;
    private Set<GlobalFeature> m_newlyDisable;
    private Map<Location, Set<LocationFeature>> m_newlyEnabledByLocation;
    private Map<Location, Set<LocationFeature>> m_newlyDisabledByLocation;


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
        Collection<GlobalFeature> enabledBundleGlobalFeatures = CollectionUtils.intersection(enable,
                bundleGlobalFeatures);
        Collection<LocationFeature> bundleLocationFeatures = bundle.getLocationFeatures();
        Set<GlobalFeature> disable = new HashSet<GlobalFeature>(CollectionUtils.disjunction(bundleGlobalFeatures,
                enabledBundleGlobalFeatures));
        Map<Location, Set<LocationFeature>> disableByLocation = new HashMap<Location, Set<LocationFeature>>(
                enableByLocation.size());
        for (Entry<Location, Set<LocationFeature>> entry : enableByLocation.entrySet()) {
            Set<LocationFeature> offAtLocation = new HashSet<LocationFeature>(CollectionUtils.disjunction(
                    bundleLocationFeatures, entry.getValue()));
            disableByLocation.put(entry.getKey(), offAtLocation);
        }
        return new FeatureChangeRequest(new HashSet<GlobalFeature>(enabledBundleGlobalFeatures), disable,
                enableByLocation, disableByLocation);
    }

    public void enableFeature(GlobalFeature f, boolean enable) {
        if (enable) {
            getEnable().add(f);
            getDisable().remove(f);
        } else {
            getEnable().remove(f);
            getDisable().add(f);
        }
    }

    public void enableLocationFeature(LocationFeature f, Location location, boolean enable) {
        if (enable) {
            getEnableByLocation().get(location).add(f);
            safeRemove(getDisableByLocation().get(location), f);
        } else {
            safeRemove(getEnableByLocation().get(location), f);
            getDisableByLocation().get(location).add(f);
        }
    }

    public <T extends Feature> void safeRemove(Set<T> s, T f) {
        if (s != null) {
            s.remove(f);
        }
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
        "unchecked", "rawtypes"
    })
    Collection<Location> findLocationsByFeature(LocationFeature f, Map<Location, Set<LocationFeature>> map) {
        LocationByFeature findAndFilter = new LocationByFeature(f);
        Collection select = CollectionUtils.select(map.entrySet(), findAndFilter);
        Collection locations = CollectionUtils.collect(select, findAndFilter);
        return (Collection<Location>) locations;
    }

    public Map<Location, Set<LocationFeature>> getNewlyEnabledByLocation() {
        return m_newlyEnabledByLocation;
    }

    public void setNewlyEnabledByLocation(Map<Location, Set<LocationFeature>> newlyEnabledByLocation) {
        this.m_newlyEnabledByLocation = newlyEnabledByLocation;
    }

    public Map<Location, Set<LocationFeature>> getNewlyDisabledByLocation() {
        return m_newlyDisabledByLocation;
    }

    public void setNewlyDisabledByLocation(Map<Location, Set<LocationFeature>> newlyDisabledByLocation) {
        this.m_newlyDisabledByLocation = newlyDisabledByLocation;
    }

    public Set<GlobalFeature> getNewlyEnable() {
        return m_newlyEnable;
    }

    public void setNewlyEnable(Set<GlobalFeature> newlyEnable) {
        this.m_newlyEnable = newlyEnable;
    }

    public Set<GlobalFeature> getNewlyDisable() {
        return m_newlyDisable;
    }

    public void setNewlyDisable(Set<GlobalFeature> newlyDisable) {
        this.m_newlyDisable = newlyDisable;
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
