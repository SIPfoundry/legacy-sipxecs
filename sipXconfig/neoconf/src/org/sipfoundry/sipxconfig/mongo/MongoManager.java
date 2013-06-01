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

import java.util.Collection;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alarm.AlarmDefinition;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface MongoManager extends MongoReplSetManager {
    public static final String MONGO = "mongo";
    public static final String ARBITOR = "mongoArbiter";
    public static final String LOCAL_MONGO = "mongoLocal";
    public static final String LOCAL_ARBITOR = "mongoLocalArbiter";
    public static final AddressType ADDRESS_ID = new AddressType(MONGO, MongoSettings.SERVER_PORT);
    public static final AddressType ARBITOR_ADDRESS_ID = new AddressType(ARBITOR, MongoSettings.ARBITER_PORT);
    public static final AddressType LOCAL_ADDRESS_ID = new AddressType(LOCAL_MONGO, MongoSettings.LOCAL_PORT);
    public static final AddressType LOCAL_ARBITOR_ADDRESS_ID = new AddressType(LOCAL_ARBITOR,
            MongoSettings.LOCAL_ARBITER_PORT);
    public static final LocationFeature FEATURE_ID = new LocationFeature(MONGO);
    public static final LocationFeature ARBITER_FEATURE = new LocationFeature(ARBITOR);
    public static final LocationFeature LOCAL_FEATURE = new LocationFeature(LOCAL_MONGO);
    public static final LocationFeature LOCAL_ARBITER_FEATURE = new LocationFeature(LOCAL_ARBITOR);
    public static final AlarmDefinition MONGO_FATAL_REPLICATION_STOP = new AlarmDefinition(
            "MONGO_FATAL_REPLICATION_STOP");
    public static final AlarmDefinition MONGO_FAILED_ELECTION = new AlarmDefinition("MONGO_FAILED_ELECTION", 1);
    public static final AlarmDefinition MONGO_MEMBER_DOWN = new AlarmDefinition("MONGO_MEMBER_DOWN", 2);
    public static final AlarmDefinition MONGO_NODE_STATE_CHANGED = new AlarmDefinition("MONGO_NODE_STATE_CHANGED");
    public static final AlarmDefinition MONGO_CANNOT_SEE_MAJORITY = new AlarmDefinition("MONGO_CANNOT_SEE_MAJORITY");

    public MongoSettings getSettings();

    public boolean isMisconfigured();

    public void saveSettings(MongoSettings settings);

    public Collection<MongoShard> getShards();

    public void saveShard(MongoShard shard);

    public void deleteShard(MongoShard shard);

    public MongoShard getShard(Integer shardId);

    public MongoReplSetManager getShardManager(MongoShard shard);

    public ConfigManager getConfigManager();

    public FeatureManager getFeatureManager();
}
