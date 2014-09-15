/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.parkorbit;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.ExtensionInUseException;
import org.sipfoundry.sipxconfig.common.NameInUseException;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SipxCollectionUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.imdb.ReplicationManager;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.registrar.RegistrarSettings;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.jdbc.core.JdbcTemplate;

public class ParkOrbitContextImpl extends SipxHibernateDaoSupport implements ParkOrbitContext, BeanFactoryAware,
        FeatureProvider, DaoEventListener {
    private static final String VALUE = "value";
    private static final String QUERY_PARK_ORBIT_IDS_WITH_ALIAS = "parkOrbitIdsWithAlias";
    private static final String PARK_ORBIT_BY_NAME = "parkOrbitByName";
    private AliasManager m_aliasManager;
    private BeanFactory m_beanFactory;
    private SettingDao m_settingDao;
    private FeatureManager m_featureManager;
    private JdbcTemplate m_jdbcTemplate;
    private ReplicationManager m_replicationManager;

    public void storeParkOrbit(ParkOrbit parkOrbit) {
        // Check for duplicate names and extensions before saving the park orbit
        String name = parkOrbit.getName();
        String extension = parkOrbit.getExtension();
        final String parkOrbitTypeName = "&label.callPark";
        if (!m_aliasManager.canObjectUseAlias(parkOrbit, name)) {
            throw new NameInUseException(parkOrbitTypeName, name);
        }
        if (!m_aliasManager.canObjectUseAlias(parkOrbit, extension)) {
            throw new ExtensionInUseException(parkOrbitTypeName, extension);
        }

        if (parkOrbit.isNew()) {
            getHibernateTemplate().save(parkOrbit);
        } else {
            getHibernateTemplate().merge(parkOrbit);
        }
        getDaoEventPublisher().publishSave(parkOrbit);
    }

    public void removeParkOrbits(Collection ids) {
        if (ids.isEmpty()) {
            return;
        }
        removeAll(ParkOrbit.class, ids);
    }

    public ParkOrbit loadParkOrbit(Integer id) {
        return (ParkOrbit) getHibernateTemplate().load(ParkOrbit.class, id);
    }

    public Collection getParkOrbits() {
        return getHibernateTemplate().loadAll(ParkOrbit.class);
    }

    public Collection getParkOrbits(Integer locationId) {
        return getHibernateTemplate().findByNamedQueryAndNamedParam("parkOrbitsByServer", "locationId", locationId);
    }

    public String getDefaultMusicOnHold() {
        return getBackgroundMusic().getMusic();
    }

    public void setDefaultMusicOnHold(String music) {
        BackgroundMusic backgroundMusic = getBackgroundMusic();
        backgroundMusic.setMusic(music);
        if (backgroundMusic.isNew()) {
            getHibernateTemplate().save(backgroundMusic);
        } else {
            getHibernateTemplate().merge(backgroundMusic);
        }
    }

    private BackgroundMusic getBackgroundMusic() {
        List musicList = getHibernateTemplate().loadAll(BackgroundMusic.class);
        if (!musicList.isEmpty()) {
            return (BackgroundMusic) musicList.get(0);
        }
        return new BackgroundMusic();
    }

    @Required
    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }

    public boolean isAliasInUse(String alias) {
        // Look for the ID of a park orbit with the specified alias as its name or extension.
        // If there is one, then the alias is in use.
        List objs = getHibernateTemplate().findByNamedQueryAndNamedParam(QUERY_PARK_ORBIT_IDS_WITH_ALIAS, VALUE,
                alias);
        return SipxCollectionUtils.safeSize(objs) > 0;
    }

    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        Collection ids = getHibernateTemplate().findByNamedQueryAndNamedParam(QUERY_PARK_ORBIT_IDS_WITH_ALIAS,
                VALUE, alias);
        Collection bids = BeanId.createBeanIdCollection(ids, ParkOrbit.class);
        return bids;
    }

    /**
     * Remove all park orbits - mostly used for testing
     */
    public void clear() {
        removeAll(ParkOrbit.class);
    }

    public ParkOrbit newParkOrbit() {
        ParkOrbit orbit = (ParkOrbit) m_beanFactory.getBean(ParkOrbit.class.getName(), ParkOrbit.class);

        // All auto attendants share same group: default
        Set groups = orbit.getGroups();
        if (groups == null || groups.isEmpty()) {
            orbit.addGroup(getDefaultParkOrbitGroup());
        }

        return orbit;
    }

    public Group getDefaultParkOrbitGroup() {
        return m_settingDao.getGroupCreateIfNotFound(PARK_ORBIT_GROUP_ID, "default");
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    public static boolean isParkOrbitGroup(Group group) {
        return PARK_ORBIT_GROUP_ID.equals(group.getResource());
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return Collections.singleton(FEATURE);
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE_TELEPHONY) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        validator.requiredOnSameHost(FEATURE, FreeswitchFeature.FEATURE);
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        // remove park orbits on feature disabled
        if (request.getAllNewlyDisabledFeatures().contains(ParkOrbitContext.FEATURE)) {
            Collection<Location> locations = request.getLocationsForDisabledFeature(ParkOrbitContext.FEATURE);
            Collection<Integer> ids = new ArrayList<Integer>();
            for (Location location : locations) {
                Collection<ParkOrbit> parkOrbits = getParkOrbits(location.getId());
                for (ParkOrbit orbit : parkOrbits) {
                    ids.add(orbit.getId());
                }
            }
            if (ids.size() > 0) {
                removeParkOrbits(ids);
            }
        }
    }

    @Override
    public ParkOrbit loadParkOrbitByName(String name) {
        List<ParkOrbit> conferences = getHibernateTemplate().findByNamedQueryAndNamedParam(PARK_ORBIT_BY_NAME,
                VALUE, name);
        return (ParkOrbit) DataAccessUtils.singleResult(conferences);
    }

    @Override
    public List<Replicable> getReplicables() {
        if (m_featureManager.isFeatureEnabled(ParkOrbitContext.FEATURE)) {
            List<Replicable> replicables = new ArrayList<Replicable>();
            replicables.addAll(getParkOrbits());
            return replicables;
        }
        return Collections.EMPTY_LIST;
    }

    @Required
    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    @Required
    public void setJdbcTemplate(JdbcTemplate jdbcTemplate) {
        m_jdbcTemplate = jdbcTemplate;
    }

    @Required
    public void setReplicationManager(ReplicationManager replicationManager) {
        m_replicationManager = replicationManager;
    }

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof Location) {
            int count = m_jdbcTemplate.queryForInt("select count(*) from park_orbit where location_id = ?",
                    ((Location) entity).getId());
            if (count >= 1) {
                throw new UserException("&err.location.orbitAssigned");
            }
        }
    }

    @Override
    public void onSave(Object entity) {
        if (entity instanceof RegistrarSettings) {
            Collection<ParkOrbit> orbits = getParkOrbits();
            for (ParkOrbit orbit : orbits) {
                m_replicationManager.replicateEntity(orbit);
            }
        }
    }
}
