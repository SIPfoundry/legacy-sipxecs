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
import java.util.List;
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
import org.springframework.jdbc.support.rowset.SqlRowSet;

public class MongoManagerImpl implements AddressProvider, FeatureProvider, MongoManager, ProcessProvider,
        SetupListener, FirewallProvider, AlarmProvider, BeanFactoryAware, FeatureListener {
    private static final Log LOG = LogFactory.getLog(MongoManagerImpl.class);
    private BeanWithSettingsDao<MongoSettings> m_settingsDao;
    private ConfigManager m_configManager;
    private FeatureManager m_featureManager;
    private JdbcTemplate m_db;
    private Map<Integer, MongoReplSetManager> m_shardManagers;
    private MongoReplSetManager m_globalManager;
    private ListableBeanFactory m_beans;

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
        addProcess(manager, location, procs, LOCAL_FEATURE, "mongo-local",
                ".*/mongod.*-f.*/mongo-local.conf", "restart_mongo_local");
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
    public Collection<MongoShard> getShards() {
        List<MongoShard> shards = new ArrayList<MongoShard>();
        SqlRowSet rs = m_db.queryForRowSet("select shard_id, name from shard");
        while (rs.next()) {
            MongoShard shard = new MongoShard();
            shard.setUniqueId(rs.getInt(1));
            shard.setName(rs.getString(2));
        }

        return shards;
    }

    public MongoShard getShard(Integer shardId) {
        return null;
    }

    public void saveShard(MongoShard shard) {
        Integer id = shard.getId();
        if (id == null || id <= 0) {
            id = m_db.queryForInt("select nextval('shard_seq')");
            m_db.update("insert into shard (shard_id, name) values (?, ?)", id, shard.getName());
        } else {
            m_db.update("update shard set name = ? where id = ?", shard.getName(), id);
        }
    }

    public void deleteShard(MongoShard shard) {
        m_db.update("delete from from shard where id = ?", shard.getId());
        getShardManagers().remove(shard.getId());
    }

    public void setConfigJdbcTemplate(JdbcTemplate jdbcTemplate) {
        m_db = jdbcTemplate;
    }

    @Override
    public MongoReplSetManager getShardManager(MongoShard shard) {
        // we cache manager because rest API needs to know if things are in progress
        // between requests.
        MongoReplSetManager manager = getShardManagers().get(shard.getId());
        if (manager != null) {
            return manager;
        }

        manager = (MongoReplSetManager) m_beans.getBean("mongoReplSetManager");
        ((MongoReplSetManagerImpl) manager).setShard(shard);
        getShardManagers().put(shard.getId(), manager);
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
    public String newLocalDatabase(String server) {
        throw new IllegalArgumentException();
    }

    @Override
    public String removeLocalDatabase(String server) {
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
}
