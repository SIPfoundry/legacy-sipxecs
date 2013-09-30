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
package org.sipfoundry.sipxconfig.mongo;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alarm.AlarmDefinition;
import org.sipfoundry.sipxconfig.alarm.AlarmProvider;
import org.sipfoundry.sipxconfig.alarm.AlarmServerManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventListenerAdvanced;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.InvalidChange;
import org.sipfoundry.sipxconfig.feature.InvalidChangeException;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallProvider;
import org.sipfoundry.sipxconfig.region.Region;
import org.sipfoundry.sipxconfig.region.RegionManager;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;
import org.springframework.beans.BeansException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.jdbc.core.JdbcTemplate;

public class MongoManagerImpl implements AddressProvider, FeatureProvider, MongoManager, ProcessProvider,
        SetupListener, FirewallProvider, AlarmProvider, BeanFactoryAware, FeatureListener, DaoEventListenerAdvanced {
    private static final Log LOG = LogFactory.getLog(MongoManagerImpl.class);
    private BeanWithSettingsDao<MongoSettings> m_settingsDao;
    private ConfigManager m_configManager;
    private FeatureManager m_featureManager;
    private Map<Integer, MongoReplSetManager> m_shardManagers;
    private MongoReplSetManager m_globalManager;
    private RegionManager m_regionManager;
    private ListableBeanFactory m_beans;
    private JdbcTemplate m_configJdbcTemplate;

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public ConfigManager getConfigManager() {
        return m_configManager;
    }

    @Override
    public MongoSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(MongoSettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return Arrays.asList(new DefaultFirewallRule(ADDRESS_ID), new DefaultFirewallRule(ARBITOR_ADDRESS_ID),
                new DefaultFirewallRule(LOCAL_ADDRESS_ID), new DefaultFirewallRule(LOCAL_ARBITOR_ADDRESS_ID));
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        if (!type.equalsAnyOf(ADDRESS_ID, ARBITOR_ADDRESS_ID, LOCAL_ADDRESS_ID, LOCAL_ARBITOR_ADDRESS_ID)) {
            return null;
        }

        LocationFeature feature = addressToFeature(type);
        Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(feature);
        Collection<Address> addresses = Location.toAddresses(type, locations);
        LOG.debug("Found " + locations.size() + " locations: " + addresses);
        return addresses;
    }

    LocationFeature addressToFeature(AddressType type) {
        if (type == ARBITOR_ADDRESS_ID) {
            return ARBITER_FEATURE;
        }
        if (type == LOCAL_ADDRESS_ID) {
            return LOCAL_FEATURE;
        }
        if (type == LOCAL_ARBITOR_ADDRESS_ID) {
            return LOCAL_ARBITER_FEATURE;
        }
        return FEATURE_ID;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        if (l.isPrimary()) {
            // we show arbiter as an option even though there are many situations where
            // it would not make sense, but there are situations where you have to show it
            // in case admins needs to disable it. e.g admin deletes 2 of 3 servers and
            // then needs to disable arbiter on primary.
            return Collections.singleton(ARBITER_FEATURE);
        }
        return Arrays.asList(FEATURE_ID, ARBITER_FEATURE);
    }

    public void setSettingsDao(BeanWithSettingsDao<MongoSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        Collection<ProcessDefinition> procs = new ArrayList<ProcessDefinition>(2);
        if (manager.getFeatureManager().isFeatureEnabled(FEATURE_ID, location) || location.isPrimary()) {
            procs.add(ProcessDefinition.sysvByRegex("mongod", ".*/mongod.*-f.*/mongodb{0,1}.conf", true));
        }

        addProcess(manager, location, procs, ARBITER_FEATURE, "mongod-arbiter",
                ".*/mongod.*-f.*/mongod-arbiter.conf", "restart_mongo_arbiter");
        addProcess(manager, location, procs, LOCAL_FEATURE, "mongo-local", ".*/mongod.*-f.*/mongo-local.conf",
                "restart_mongo_local");
        addProcess(manager, location, procs, LOCAL_ARBITER_FEATURE, "mongo-local-arbiter",
                ".*/mongod.*-f.*/mongo-local-arbiter.conf", "restart_mongo_local_arbiter");

        return procs;
    }

    void addProcess(SnmpManager manager, Location location, Collection<ProcessDefinition> procs, LocationFeature f,
            String process, String regex, String restart) {
        if (manager.getFeatureManager().isFeatureEnabled(f, location)) {
            ProcessDefinition def = ProcessDefinition.sipxByRegex(process, regex);
            def.setRestartClass(restart);
            procs.add(def);
        }
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        // we do not list features in features list because enabling/disabling dbs
        // and arbiters required a dedicated ui.
    }

    @Override
    public boolean setup(SetupManager manager) {
        if (!manager.isTrue(FEATURE_ID.getId())) {
            Location primary = manager.getConfigManager().getLocationManager().getPrimaryLocation();
            manager.getFeatureManager().enableLocationFeature(FEATURE_ID, primary, true);
            manager.setTrue(FEATURE_ID.getId());
        }
        return true;
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        FeatureChangeRequest request = validator.getRequest();
        if (!request.hasChanged(FEATURE_ID)) {
            return;
        }

        Collection<Location> mongos = validator.getLocationsForEnabledFeature(FEATURE_ID);
        if (mongos.size() == 0) {
            InvalidChangeException err = new InvalidChangeException("&error.noMongos");
            InvalidChange needArbiter = new InvalidChange(FEATURE_ID, err);
            validator.getInvalidChanges().add(needArbiter);
        } else {
            boolean includesPrimary = false;
            for (Location l : mongos) {
                includesPrimary = l.isPrimary();
                if (includesPrimary) {
                    break;
                }
            }
            if (!includesPrimary) {
                InvalidChangeException err = new InvalidChangeException("&error.mongoOnPrimaryRequired");
                InvalidChange removeArbiter = new InvalidChange(FEATURE_ID, err);
                validator.getInvalidChanges().add(removeArbiter);
            }
        }

        // Cannot use local database when there is no local database
        Collection<Location> noMongos = validator.getRequest().getLocationsForDisabledFeature(FEATURE_ID);
        for (Location l : noMongos) {
            validator.getRequest().enableLocationFeature(LOCAL_FEATURE, l, false);
        }
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
    }

    @Override
    public Collection<AlarmDefinition> getAvailableAlarms(AlarmServerManager manager) {
        if (!manager.getFeatureManager().isFeatureEnabled(MongoManager.FEATURE_ID)
                || !manager.getFeatureManager().isFeatureEnabled(MongoManager.FEATURE_ID)
                || !manager.getFeatureManager().isFeatureEnabled(MongoManager.LOCAL_FEATURE)
                || !manager.getFeatureManager().isFeatureEnabled(MongoManager.LOCAL_ARBITER_FEATURE)) {
            return null;
        }
        Collection<AlarmDefinition> defs = Arrays.asList(new AlarmDefinition[] {
            MONGO_FATAL_REPLICATION_STOP, MONGO_FAILED_ELECTION, MONGO_MEMBER_DOWN, MONGO_NODE_STATE_CHANGED,
            MONGO_CANNOT_SEE_MAJORITY
        });
        return defs;
    }

    @Override
    public boolean isMisconfigured() {
        // TODO Auto-generated method stub
        return false;
    }

    @Override
    public MongoReplSetManager getShardManager(Region region) {
        // we cache manager because rest API needs to know if things are in progress
        // between requests.
        MongoReplSetManager manager = getShardManagers().get(region.getId());
        if (manager != null) {
            return manager;
        }

        manager = (MongoReplSetManager) m_beans.getBean("mongoReplSetManager");
        ((MongoReplSetManagerImpl) manager).setRegion(region);
        getShardManagers().put(region.getId(), manager);
        return manager;
    }

    Map<Integer, MongoReplSetManager> getShardManagers() {
        if (m_shardManagers != null) {
            return m_shardManagers;
        }
        m_shardManagers = new HashMap<Integer, MongoReplSetManager>();
        return m_shardManagers;
    }

    @Override
    public String addFirstLocalDatabase(String server) {
        throw new IllegalArgumentException();
    }

    @Override
    public String removeLastLocalDatabase(String server) {
        throw new IllegalArgumentException();
    }

    @Override
    public String removeLastLocalArbiter(String hostPort) {
        throw new IllegalArgumentException();
    }

    @Override
    public String getLastConfigError() {
        return m_globalManager.getLastConfigError();
    }

    @Override
    public boolean isInProgress() {
        return m_globalManager.isInProgress();
    }

    @Override
    public MongoMeta getMeta() {
        return m_globalManager.getMeta();
    }

    @Override
    public String addDatabase(String primary, String server) {
        return m_globalManager.addDatabase(primary, server);
    }

    @Override
    public String addArbiter(String primary, String server) {
        return m_globalManager.addArbiter(primary, server);
    }

    @Override
    public String removeDatabase(String primary, String server) {
        return m_globalManager.removeDatabase(primary, server);
    }

    @Override
    public String removeArbiter(String primary, String server) {
        return m_globalManager.removeArbiter(primary, server);
    }

    @Override
    public String takeAction(String primary, String server, String action) {
        return m_globalManager.takeAction(primary, server, action);
    }

    public void setGlobalManager(MongoReplSetManager globalManager) {
        m_globalManager = globalManager;
    }

    @Override
    public void setBeanFactory(BeanFactory factory) throws BeansException {
        m_beans = (ListableBeanFactory) factory;
    }

    public FeatureManager getFeatureManager() {
        return m_featureManager;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    @Override
    public String addLocalDatabase(String primary, String hostPort) {
        throw new IllegalArgumentException();
    }

    @Override
    public String addLocalArbiter(String primary, String hostPort) {
        throw new IllegalArgumentException();
    }

    @Override
    public String removeLocalDatabase(String primary, String hostPort) {
        throw new IllegalArgumentException();
    }

    @Override
    public String removeLocalArbiter(String primary, String hostPort) {
        throw new IllegalArgumentException();
    }

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof Region) {
            Region r = (Region) entity;
            checkRegionForDatabase(r);
        }
    }

    void checkRegionForDatabase(Region r) {
        String sql = "select count(*) from feature_local f, location l where "
                + "l.region_id = ? and l.location_id = f.location_id and " + "f.feature_id in (?,?)";
        int nLocalDbs = m_configJdbcTemplate.queryForInt(sql, r.getId(), LOCAL_FEATURE.getId(),
                LOCAL_ARBITER_FEATURE.getId());
        if (nLocalDbs > 0) {
            throw new UserException("&error.localDbsWithRegion");
        }
    }

    @Override
    public void onSave(Object entity) {
    }

    public void setConfigJdbcTemplate(JdbcTemplate configJdbc) {
        m_configJdbcTemplate = configJdbc;
    }

    public void setRegionManager(RegionManager regionManager) {
        m_regionManager = regionManager;
    }

    @Override
    public void onBeforeSave(Object entity) {
        if (entity instanceof Location) {
            // Don't allow changing server region if there are regional databases there.
            // they should be removed first
            Location l = (Location) entity;
            Location o = l.getOriginalCopy();
            if (o != null) {
                if (o.getRegionId() != null) {
                    if (!o.getRegionId().equals(l.getRegionId())) {
                        Region region = m_regionManager.getRegion(o.getRegionId());
                        checkRegionForDatabase(region);
                    }
                }
            }
        }
    }

    @Override
    public void onAfterDelete(Object entity) {
    }
}
