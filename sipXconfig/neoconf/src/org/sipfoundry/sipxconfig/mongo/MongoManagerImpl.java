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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alarm.AlarmDefinition;
import org.sipfoundry.sipxconfig.alarm.AlarmProvider;
import org.sipfoundry.sipxconfig.alarm.AlarmServerManager;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
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

public class MongoManagerImpl implements AddressProvider, FeatureProvider, MongoManager, ProcessProvider,
        SetupListener, FirewallProvider, AlarmProvider {
    private static final Log LOG = LogFactory.getLog(MongoManagerImpl.class);
    private BeanWithSettingsDao<MongoSettings> m_settingsDao;

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
        return Arrays.asList(new DefaultFirewallRule(ADDRESS_ID), new DefaultFirewallRule(ARBITOR_ADDRESS_ID));
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        if (!type.equalsAnyOf(ADDRESS_ID, ARBITOR_ADDRESS_ID)) {
            return null;
        }

        LocationFeature feature = (type == ADDRESS_ID ? FEATURE_ID : ARBITER_FEATURE);
        Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(feature);
        Collection<Address> addresses = Location.toAddresses(type, locations);
        LOG.debug("Found " + locations.size() + " locations: " + addresses);
        return addresses;
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
        if (manager.getFeatureManager().isFeatureEnabled(ARBITER_FEATURE, location)) {
            ProcessDefinition def = ProcessDefinition.sipxByRegex("mongod-arbiter",
                    ".*/mongod.*-f.*/mongod-arbiter.conf");
            def.setRestartClass("restart_mongo_arbiter");
            procs.add(def);
        }
        return procs;
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE) {
            b.addFeature(FEATURE_ID);
            b.addFeature(ARBITER_FEATURE);
        }
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
        if (!request.hasChanged(FEATURE_ID) || !request.hasChanged(ARBITER_FEATURE)) {
            return;
        }

        Collection<Location> mongos = validator.getLocationsForEnabledFeature(FEATURE_ID);
        Collection<Location> arbiters = validator.getLocationsForEnabledFeature(ARBITER_FEATURE);
        if ((mongos.size() % 2) == 0) {
            if (arbiters.size() != 1) {
                InvalidChangeException err = new InvalidChangeException("&error.missingMongoArbiter");
                InvalidChange needArbiter = new InvalidChange(ARBITER_FEATURE, err);
                validator.getInvalidChanges().add(needArbiter);
            }
        } else if (mongos.size() > 1) {
            if (arbiters.size() != 0) {
                InvalidChangeException err = new InvalidChangeException("&error.extraMongoArbiter");
                InvalidChange removeArbiter = new InvalidChange(ARBITER_FEATURE, err);
                validator.getInvalidChanges().add(removeArbiter);
            }
        }
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
    }

    @Override
    public Collection<AlarmDefinition> getAvailableAlarms(AlarmServerManager manager) {
        if (!manager.getFeatureManager().isFeatureEnabled(MongoManager.FEATURE_ID)
                || !manager.getFeatureManager().isFeatureEnabled(MongoManager.FEATURE_ID)) {
            return null;
        }
        Collection<AlarmDefinition> defs = Arrays.asList(new AlarmDefinition[] {
            MONGO_FATAL_REPLICATION_STOP, MONGO_FAILED_ELECTION, MONGO_MEMBER_DOWN,
            MONGO_NODE_STATE_CHANGED, MONGO_CANNOT_SEE_MAJORITY
        });
        return defs;
    }
}
