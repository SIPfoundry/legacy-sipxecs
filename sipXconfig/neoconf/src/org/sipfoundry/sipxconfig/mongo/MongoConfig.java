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
package org.sipfoundry.sipxconfig.mongo;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.CfengineModuleConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.region.Region;

import com.mongodb.util.JSON;

public class MongoConfig implements ConfigProvider {
    private static final String GLOBAL_REPLSET = "sipxecs";
    private static final String LOCAL_REPLSET = "sipxlocal";
    private MongoManager m_mongoManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(MongoManager.FEATURE_ID, LocationsManager.FEATURE, MongoManager.ARBITER_FEATURE,
                MongoManager.LOCAL_FEATURE)) {
            return;
        }
        FeatureManager fm = manager.getFeatureManager();
        Location[] all = manager.getLocationManager().getLocations();
        MongoSettings settings = m_mongoManager.getSettings();
        List<Location> dbs = fm.getLocationsForEnabledFeature(MongoManager.FEATURE_ID);
        String connStr = getConnectionString(dbs, GLOBAL_REPLSET, settings.getPort());
        String connUrl = getConnectionUrl(dbs, settings.getPort());
        List<Location> localServers = fm.getLocationsForEnabledFeature(MongoManager.LOCAL_FEATURE);
        List<Location> localArbiters = fm.getLocationsForEnabledFeature(MongoManager.LOCAL_ARBITER_FEATURE);
        Map<Integer, List<Location>> localServersByRegion = Region.locationsByRegion(localServers);
        Map<Integer, List<Location>> localArbitersByRegion = Region.locationsByRegion(localArbiters);
        for (Location location : all) {
            // CLIENT
            File dir = manager.getLocationDataDirectory(location);
            FileWriter client = new FileWriter(new File(dir, "mongo-client.ini"));
            try {
                writeClientConfig(client, connStr, connUrl);
            } finally {
                IOUtils.closeQuietly(client);
            }

            // SERVERS
            boolean mongod = fm.isFeatureEnabled(MongoManager.FEATURE_ID, location);
            boolean arbiter = fm.isFeatureEnabled(MongoManager.ARBITER_FEATURE, location);
            boolean local = fm.isFeatureEnabled(MongoManager.LOCAL_FEATURE, location);
            boolean localArbiter = fm.isFeatureEnabled(MongoManager.LOCAL_ARBITER_FEATURE, location);
            if (local) {
                File localFile = new File(dir, "mongo-local.ini");
                FileWriter localConfig = new FileWriter(localFile);
                try {
                    writeLocalClientConfig(localConfig, MongoSettings.LOCAL_PORT, location);
                } finally {
                    IOUtils.closeQuietly(localConfig);
                }               
            }
            
            if (local || localArbiter) {
                FileWriter localModel = null;
                try {
                    File f = new File(dir, "mongo-local.json");
                    localModel = new FileWriter(f);
                    List<Location> ldbs = localServersByRegion.get(location.getRegionId());
                    List<Location> larbs = localArbitersByRegion.get(location.getRegionId());
                    
                    // NOTE: If no region is specified, allow this local database to
                    // start w/o a region as a convenience to user to not required a 
                    // region for a single local database in one location.
                    if (ldbs == null && local) {
                    	ldbs = Collections.singletonList(location);
                    }
                    modelFile(localModel, ldbs, larbs, LOCAL_REPLSET,
                            MongoSettings.LOCAL_PORT, MongoSettings.LOCAL_ARBITER_PORT);
                } finally {
                    IOUtils.closeQuietly(localModel);
                }
            }

            FileWriter server = new FileWriter(new File(dir, "mongodb.cfdat"));
            try {
                writeServerConfig(server, mongod, arbiter, local, localArbiter);
            } finally {
                IOUtils.closeQuietly(server);
            }
        }

        List<Location> arbiters = fm.getLocationsForEnabledFeature(MongoManager.ARBITER_FEATURE);
        Writer w = null;
        try {
            File f = new File(manager.getGlobalDataDirectory(), "mongo.json");
            w = new FileWriter(f);
            modelFile(w, dbs, arbiters, GLOBAL_REPLSET, MongoSettings.SERVER_PORT, MongoSettings.ARBITER_PORT);
        } finally {
            IOUtils.closeQuietly(w);
        }
    }
    
    void modelFile(Writer sb, List<Location> servers, List<Location> arbiters, String replSet, int dbPort,
            int arbPort) throws IOException {
        Map<String, Object> model = new HashMap<String, Object>();
        if (servers.size() > 0) {
            model.put("servers", serverIdList(servers, dbPort));
        }
        if (arbiters != null && arbiters.size() > 0) {
            model.put("arbiters", serverIdList(arbiters, arbPort));
        }
        model.put("replSet", replSet);
        String json = JSON.serialize(model);
        sb.write(json);
    }

    List<String> serverIdList(Collection<Location> servers, int port) {
        List<String> ids = new ArrayList<String>(servers.size());
        for (Location l : servers) {
            ids.add(l.getFqdn() + ':' + port);
        }
        return ids;
    }

    void writeServerConfig(Writer w, boolean mongod, boolean arbiter, boolean local, boolean localArbiter) throws IOException {
        String bindToAll = "0.0.0.0";
        CfengineModuleConfiguration config = new CfengineModuleConfiguration(w);
        config.writeClass("mongod", mongod);
        config.write("mongoBindIp", bindToAll);
        config.write("mongoPort", MongoSettings.SERVER_PORT);
        config.writeClass("mongod_arbiter", arbiter);
        config.write("mongoArbiterBindIp", bindToAll);
        config.write("mongoArbiterPort", MongoSettings.ARBITER_PORT);
        config.writeClass("mongo_local", local);
        config.writeClass("mongo_arbiter_local", localArbiter);
    }

    void writeClientConfig(Writer w, String connStr, String connUrl) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(w);
        config.write("connectionUrl", connUrl);
        config.write("connectionString", connStr);
    }

    void writeLocalClientConfig(Writer w, int port, Location location) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(w);
        List<Location> ldb = Collections.singletonList(location);
        String lconnStr = getConnectionString(ldb, LOCAL_REPLSET, port);
        String lconnUrl = getConnectionUrl(ldb, port);
        writeClientConfig(w, lconnStr, lconnUrl);
        config.write("shardId", location.getId());
    }

    String getConnectionString(List<Location> servers, String replSet, int port) {
        StringBuilder r = new StringBuilder(replSet).append('/');
        for (int i = 0; i < servers.size(); i++) {
            Location server = servers.get(i);
            if (i > 0) {
                r.append(',');
            }
            r.append(server.getFqdn() + ':' + port);
        }
        return r.toString();
    }

    String getConnectionUrl(List<Location> servers, int port) {
        StringBuilder r = new StringBuilder("mongodb://");
        for (int i = 0; i < servers.size(); i++) {
            Location server = servers.get(i);
            if (i > 0) {
                r.append(',');
            }
            r.append(server.getFqdn() + ':' + port);
        }
        r.append("/?readPreference=nearest");
        return r.toString();
    }

    public void setMongoManager(MongoManager mongoManager) {
        m_mongoManager = mongoManager;
    }
}
