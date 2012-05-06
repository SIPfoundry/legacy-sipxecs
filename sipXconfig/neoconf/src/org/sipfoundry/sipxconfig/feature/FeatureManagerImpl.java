/**
 *
 *
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

import static java.lang.String.format;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.BeanInitializationException;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.support.rowset.SqlRowSet;

/**
 * NOTE: This implementation pays no attention to efficiency. Should work in caching or optimize
 * queries accordingly if found to be inefficient during testing.
 */
public class FeatureManagerImpl extends SipxHibernateDaoSupport implements BeanFactoryAware, FeatureManager,
        DaoEventListener, BundleProvider {
    private ListableBeanFactory m_beanFactory;
    private Collection<FeatureProvider> m_providers;
    private Collection<BundleProvider> m_bundleProviders;
    private Collection<FeatureListener> m_listeners;
    private JdbcTemplate m_jdbcTemplate;
    private LocationsManager m_locationsManager;
    private List<Bundle> m_bundles;
    private boolean m_showExperimentalBundles = true;

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    Collection<FeatureProvider> getFeatureProviders() {
        if (m_providers == null) {
            Map<String, FeatureProvider> beanMap = safeGetListableBeanFactory().getBeansOfType(
                    FeatureProvider.class, false, false);
            m_providers = new ArrayList<FeatureProvider>(beanMap.values());
        }

        return m_providers;
    }

    Collection<BundleProvider> getBundleProviders() {
        if (m_bundleProviders == null) {
            Map<String, BundleProvider> beanMap = safeGetListableBeanFactory().getBeansOfType(BundleProvider.class,
                    false, false);
            m_bundleProviders = new ArrayList<BundleProvider>(beanMap.values());
        }

        return m_bundleProviders;
    }

    private ListableBeanFactory safeGetListableBeanFactory() {
        if (m_beanFactory == null) {
            throw new BeanInitializationException(getClass().getName() + " not initialized");
        }
        return m_beanFactory;
    }

    Collection<FeatureListener> getFeatureListeners() {
        if (m_listeners == null) {
            Map<String, FeatureListener> beanMap = safeGetListableBeanFactory().getBeansOfType(
                    FeatureListener.class, false, false);
            m_listeners = new ArrayList<FeatureListener>(beanMap.values());
        }

        return m_listeners;
    }

    @Override
    public Set<GlobalFeature> getAvailableGlobalFeatures() {
        Set<GlobalFeature> features = new HashSet<GlobalFeature>();
        for (FeatureProvider p : getFeatureProviders()) {
            Collection<GlobalFeature> gfeatures = p.getAvailableGlobalFeatures(this);
            if (gfeatures != null) {
                features.addAll(gfeatures);
            }
        }
        return features;
    }

    public void setConfigJdbcTemplate(JdbcTemplate jdbcTemplate) {
        m_jdbcTemplate = jdbcTemplate;
    }

    @Override
    public boolean isFeatureEnabled(LocationFeature feature, Location location) {
        SqlRowSet queryForRowSet = m_jdbcTemplate.queryForRowSet(
                "select 1 from feature_local where feature_id = ? and location_id = ?", feature.getId(),
                location.getId());
        return queryForRowSet.first();
    }

    @Override
    public boolean isFeatureEnabled(GlobalFeature feature) {
        SqlRowSet queryForRowSet = m_jdbcTemplate.queryForRowSet(
                "select 1 from feature_global where feature_id = ?", feature.getId());
        return queryForRowSet.first();
    }

    @Override
    public boolean isFeatureEnabled(LocationFeature feature) {
        SqlRowSet queryForRowSet = m_jdbcTemplate.queryForRowSet("select 1 from feature_local where feature_id = ?",
                feature.getId());
        return queryForRowSet.first();
    }

    @Override
    public Set<LocationFeature> getAvailableLocationFeatures(Location location) {
        Set<LocationFeature> features = new HashSet<LocationFeature>();
        for (FeatureProvider p : getFeatureProviders()) {
            Collection<LocationFeature> lfeatures = p.getAvailableLocationFeatures(this, location);
            if (lfeatures != null) {
                features.addAll(lfeatures);
            }
        }
        return features;
    }

    @Override
    public void enableGlobalFeatures(Set<GlobalFeature> features, boolean enable) {
        FeatureChangeRequest request = FeatureChangeRequest.enable(features, enable);
        applyFeatureChange(new FeatureChangeValidator(this, request));
    }

    @Override
    public void applyFeatureChange(FeatureChangeValidator validator) {
        validateFeatureChange(validator);
        if (!validator.isValid()) {
            throw validator.getInvalidChanges().get(0).getMessage();
        } else {
            FeatureChangeRequest request = validator.getRequest();
            saveFeatureChange(request);
            sendPostcommitEvent(request);
        }
    }

    @Override
    public void validateFeatureChange(FeatureChangeValidator validator) {
        boolean resubmit;
        do {
            resubmit = false;
            for (FeatureListener listener : getFeatureListeners()) {
                listener.featureChangePrecommit(this, validator);
            }
            if (!validator.isValid()) {
                Location primary = m_locationsManager.getPrimaryLocation();
                resubmit = new InvalidChangeResolver().resolve(validator, primary);
            }
        } while (resubmit);
    }

    void sendPostcommitEvent(FeatureChangeRequest request) {
        for (FeatureListener listener : getFeatureListeners()) {
            listener.featureChangePostcommit(this, request);
        }
    }

    public void saveFeatureChange(FeatureChangeRequest request) {
        // we delete what we're about to enable to avoid duplicate key constraints in case
        // we're enabling features that are already enabled.
        List<String> sql = new ArrayList<String>();
        deleteSql(sql, request.getDisable());
        deleteSql(sql, request.getEnable());
        for (GlobalFeature f : request.getEnable()) {
            sql.add(format("insert into feature_global values ('%s')", f));
        }
        deleteSql(sql, request.getDisableByLocation());
        deleteSql(sql, request.getEnableByLocation());
        for (Entry<Location, Set<LocationFeature>> entry : request.getEnableByLocation().entrySet()) {
            Location location = entry.getKey();
            for (LocationFeature f : entry.getValue()) {
                sql.add(format("insert into feature_local (feature_id, location_id) values ('%s', %d)",
                    f, location.getId()));
            }
        }
        if (!sql.isEmpty()) {
            m_jdbcTemplate.batchUpdate(sql.toArray(new String[0]));
        }
    }

    void deleteSql(List<String> sql, Set<GlobalFeature> set) {
        if (!set.isEmpty()) {
            sql.add(format("delete from feature_global where feature_id in (%s)", comma(set)));
        }
    }

    void deleteSql(List<String> sql, Map<Location, Set<LocationFeature>> map) {
        if (!map.isEmpty()) {
            for (Entry<Location, Set<LocationFeature>> entry : map.entrySet()) {
                Location location = entry.getKey();
                if (!entry.getValue().isEmpty()) {
                    sql.add(format("delete from feature_local where feature_id in (%s) and location_id = %d",
                            comma(entry.getValue()), location.getId()));
                }
            }
        }
    }

    <T> String comma(Collection<T> l) {
        StringBuilder sb = new StringBuilder();
        sb.append('\'').append(StringUtils.join(l, "\',\'")).append('\'');
        return sb.toString();
    }

    static class DeltaSet<T extends Feature> implements DeployConfigOnEdit {
        private Set<T> m_newlyAdded;
        private Set<T> m_newlyRemoved;
        private Set<Feature> m_allChanges;

        DeltaSet(Set<T> newSet, Set<T> oldSet) {
            m_newlyAdded = new HashSet<T>(newSet);
            m_newlyAdded.removeAll(oldSet);

            m_newlyRemoved = new HashSet<T>(oldSet);
            m_newlyRemoved.removeAll(newSet);

            m_allChanges = new HashSet<Feature>(m_newlyAdded);
            m_allChanges.addAll(m_newlyRemoved);
        }

        @Override
        public Collection<Feature> getAffectedFeaturesOnChange() {
            return m_allChanges;
        }
    }

    @Override
    public Set<GlobalFeature> getEnabledGlobalFeatures() {
        List<String> queryForList = m_jdbcTemplate.queryForList("select feature_id from feature_global",
                String.class);
        Set<GlobalFeature> features = new HashSet<GlobalFeature>(queryForList.size());
        for (String id : queryForList) {
            features.add(new GlobalFeature(id));
        }
        return features;
    }

    public Set<LocationFeature> getEnabledLocationFeatures() {
        List<String> queryForList = m_jdbcTemplate
                .queryForList("select feature_id from feature_local", String.class);
        return locationFeatures(queryForList);
    }

    @Override
    public Set<LocationFeature> getEnabledLocationFeatures(Location location) {
        List<String> queryForList = m_jdbcTemplate.queryForList(
                "select feature_id from feature_local where location_id = ? ", String.class, location.getId());
        return locationFeatures(queryForList);
    }

    Set<LocationFeature> locationFeatures(List<String> ids) {
        Set<LocationFeature> features = new HashSet<LocationFeature>(ids.size());
        for (String id : ids) {
            features.add(new LocationFeature(id));
        }
        return features;
    }

    @Override
    public void enableLocationFeatures(Set<LocationFeature> features, Location location, boolean enable) {
        Map<Location, Set<LocationFeature>> byLocation = new HashMap<Location, Set<LocationFeature>>(1);
        byLocation.put(location, features);
        FeatureChangeRequest request = FeatureChangeRequest.enable(byLocation, enable);
        applyFeatureChange(new FeatureChangeValidator(this, request));
    }

    public void enableLocationFeature(LocationFeature feature, Location location, boolean enable) {
        Set<LocationFeature> features = getEnabledLocationFeatures(location);
        if (update(features, feature, enable)) {
            enableLocationFeatures(features, location, enable);
        }
    }

    @Override
    public List<Location> getLocationsForEnabledFeature(LocationFeature feature) {
        List<Location> locations = (List<Location>) getHibernateTemplate().findByNamedQueryAndNamedParam(
                "locationsForEnabledFeature", "featureId", feature.getId());
        return locations;
    }

    @Override
    public void enableGlobalFeature(GlobalFeature feature, boolean enable) {
        Set<GlobalFeature> features = getEnabledGlobalFeatures();
        if (update(features, feature, enable)) {
            enableGlobalFeatures(features, enable);
        }
    }

    private <T extends Feature> boolean update(Collection<T> features, T feature, boolean enable) {
        if (features.contains(feature)) {
            if (!enable) {
                features.remove(feature);
            } else {
                return false;
            }
        } else {
            if (enable) {
                features.add(feature);
            } else {
                return false;
            }
        }
        return true;
    }

    public void setJdbcTemplate(JdbcTemplate jdbcTemplate) {
        m_jdbcTemplate = jdbcTemplate;
    }

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof Location) {
            // When deleting a location, treat this as if someone disabled all features at this
            // location.
            Location location = (Location) entity;
            Set<LocationFeature> on = getEnabledLocationFeatures(location);
            enableLocationFeatures(on, location, false);
        }
    }

    @Override
    public void onSave(Object entity) {
    }

    @Override
    public List<Bundle> getBundles() {
        if (m_bundles == null) {
            m_bundles = new ArrayList<Bundle>();
            for (BundleProvider bp : getBundleProviders()) {
                Collection<Bundle> sublist = bp.getBundles(this);
                if (sublist != null) {
                    for (Bundle b : sublist) {
                        for (FeatureProvider fp : getFeatureProviders()) {
                            fp.getBundleFeatures(this, b);
                        }
                        m_bundles.add(b);
                    }
                }
            }
        }

        return m_bundles;
    }

    @Override
    public Collection<Bundle> getBundles(FeatureManager manager) {
        List<Bundle> bundles = new ArrayList<Bundle>(Arrays.asList(Bundle.CORE, Bundle.CORE_TELEPHONY,
                Bundle.CALL_CENTER, Bundle.IM, Bundle.PROVISION));
        if (m_showExperimentalBundles) {
            bundles.add(Bundle.EXPERIMENTAL);
        }
        return bundles;
    }

    void separateGlobalFromLocal(Collection<Feature> features, Set<GlobalFeature> global, Set<LocationFeature> local) {
        for (Feature f : features) {
            if (f instanceof GlobalFeature) {
                global.add((GlobalFeature) f);
            } else {
                local.add((LocationFeature) f);
            }
        }
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Override
    public Bundle getBundle(String id) {
        for (Bundle b : getBundles()) {
            if (b.getId().equals(id)) {
                return b;
            }
        }
        return null;
    }

    public void setShowExperimentalBundles(boolean showExperimentalBundles) {
        m_showExperimentalBundles = showExperimentalBundles;
    }
}
