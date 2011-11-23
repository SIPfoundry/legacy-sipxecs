/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.feature;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.BeanInitializationException;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.support.rowset.SqlRowSet;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class FeatureManagerImpl extends HibernateDaoSupport implements BeanFactoryAware, FeatureManager {
    private ListableBeanFactory m_beanFactory;
    private Collection<FeatureProvider> m_providers;
    private JdbcTemplate m_jdbcTemplate;

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    Collection<FeatureProvider> getFeatureProviders() {
        if (m_providers == null) {
            if (m_beanFactory == null) {
                throw new BeanInitializationException(getClass().getName() + " not initialized");
            }
            Map<String, FeatureProvider> beanMap = m_beanFactory.getBeansOfType(FeatureProvider.class, false, true);
            m_providers = new ArrayList<FeatureProvider>(beanMap.values());
        }

        return m_providers;
    }

    @Override
    public Set<GlobalFeature> getAvailableGlobalFeatures() {
        Set<GlobalFeature> features = new HashSet<GlobalFeature>();
        for (FeatureProvider p : getFeatureProviders()) {
            for (GlobalFeature f : p.getAvailableGlobalFeatures()) {
                features.add(f);
            }
        }
        return features;
    }

    public void setConfigJdbcTemplate(JdbcTemplate jdbcTemplate) {
        m_jdbcTemplate = jdbcTemplate;
    }

    @Override
    public boolean isLocationFeatureEnabled(LocationFeature feature, Location location) {
        SqlRowSet queryForRowSet = m_jdbcTemplate.queryForRowSet(
                "select 1 from feature_location where feature_id = ? and location_id = ?", feature.getId(),
                location.getId());
        return queryForRowSet.first();
    }

    @Override
    public boolean isGlobalFeatureEnabled(GlobalFeature feature) {
        SqlRowSet queryForRowSet = m_jdbcTemplate.queryForRowSet(
                "select 1 from feature_global where feature_id = ?", feature.getId());
        return queryForRowSet.first();
    }

    @Override
    public Set<LocationFeature> getAvailableLocationFeatures(Location location) {
        Set<LocationFeature> features = new HashSet<LocationFeature>();
        for (FeatureProvider p : getFeatureProviders()) {
            for (LocationFeature f : p.getAvailableLocationFeatures(location)) {
                features.add(f);
            }
        }
        return features;
    }

    @Override
    public void enableGlobalFeatures(Set<GlobalFeature> features) {
        String remove = "delete from feature_global";
        StringBuilder update = new StringBuilder();
        for (GlobalFeature f : features) {
            if (update.length() == 0) {
                update.append("insert into feature_global values");
            } else {
                update.append(',');
            }
            update.append("('").append(f.getId()).append("')");
        }
        m_jdbcTemplate.batchUpdate(new String[] {remove, update.toString()});
    }

    public Set<GlobalFeature> getEnabledGlobalFeatures() {
        List<String> queryForList = m_jdbcTemplate.queryForList("select feature_id from feature_global", String.class);
        Set<GlobalFeature> features = new HashSet<GlobalFeature>(queryForList.size());
        for (String id : queryForList) {
            features.add(new GlobalFeature(id));
        }
        return features;
    }

    public Set<LocationFeature> getEnabledLocationFeatures(Location location) {
        List<String> queryForList = m_jdbcTemplate.queryForList(
                "select feature_id from feature_local where location_id = ? ", String.class, location.getId());
        Set<LocationFeature> features = new HashSet<LocationFeature>(queryForList.size());
        for (String id : queryForList) {
            features.add(new LocationFeature(id));
        }
        return features;
    }

    @Override
    public void enableLocationFeatures(Set<LocationFeature> features, Location location) {
        String remove = "delete from feature_location where location_id = " + location.getId();
        StringBuilder update = new StringBuilder();
        for (LocationFeature f : features) {
            if (update.length() == 0) {
                update.append("insert into feature_location values");
            } else {
                update.append(',');
            }
            update.append('(');
            update.append('\'').append(f.getId()).append('\'');
            update.append(',');
            update.append(location.getId());
            update.append(')');
        }
        m_jdbcTemplate.batchUpdate(new String[] {remove, update.toString()});
    }
}
