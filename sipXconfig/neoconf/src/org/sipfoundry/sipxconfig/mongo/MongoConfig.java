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

import static java.lang.String.format;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringWriter;
import java.io.Writer;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.CfengineModuleConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.region.Region;
import org.sipfoundry.sipxconfig.region.RegionManager;

import com.mongodb.util.JSON;

public class MongoConfig implements ConfigProvider {
    private static final String GLOBAL_REPLSET = "sipxecs";
    private static final String LOCAL_REPLSET = "sipxlocal";

    //clusterId and shardId are mongo readPreference tags that forces the mongo read to go locally (clusterId tag key)
    //if local read not available (mongo down or corrupted)
    //we will redirect read on other node in region (shardId tag key)
    //if no region is configured, shardId is always 0,
    //meaning that we will redirect the read to 'nearest' node in cluster
    //order matters on how the read tags are added in mongo query: first the clusterId, then shardId
    private static final String CLUSTER_ID = "clusterId";
    private static final String SHARD_ID = "shardId";
    private MongoManager m_mongoManager;
    private RegionManager m_regionManager;
    private ConfigManager m_configManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(MongoManager.FEATURE_ID, LocationsManager.FEATURE, DomainManager.FEATURE,
            MongoManager.ARBITER_FEATURE, MongoManager.LOCAL_FEATURE, MongoManager.LOCAL_ARBITER_FEATURE)) {
            return;
        }
        FeatureManager fm = manager.getFeatureManager();
        Location[] all = manager.getLocationManager().getLocations();
        MongoSettings settings = m_mongoManager.getSettings();
        List<Location> dbs = fm.getLocationsForEnabledFeature(MongoManager.FEATURE_ID);
        // regional mongo server config and models
        List<Location> localServers = fm.getLocationsForEnabledFeature(MongoManager.LOCAL_FEATURE);
        List<Location> localArbiters = fm.getLocationsForEnabledFeature(MongoManager.LOCAL_ARBITER_FEATURE);
        List<Region> regions = m_regionManager.getRegions();
        Map<Integer, String> localModelsByRegion = getLocalModelsByRegion(regions, localServers, localArbiters);
        Map<Integer, String> localInisByRegionAndLocation = getLocalIniByRegionAndLocation(regions, localServers,
            settings);

        // Read tags instruct mongo client drivers connected to GLOBAL database to prefer
        // a server that's located in their region first. This requires special readPreference tags
        // to be set in replicaset meta data which should there is mongo management tools
        // have done their job.  If not, all queries will fail. XX-10668
        // XX-11378 -always use readTags; also when no regions configured

        for (Location location : all) {
            // CLIENT
            File dir = manager.getLocationDataDirectory(location);
            FileWriter client = new FileWriter(new File(dir, "mongo-client.ini"));

            Integer regionId = location.getRegionId();
            int shardId = (regionId != null ? regionId : 0);
            int clusterId = location.getId();
            String connStr = getConnectionString(dbs, GLOBAL_REPLSET, settings.getPort());
            String connUrl = getConnectionUrl(dbs, clusterId, shardId, settings.getPort());
            try {
                writeClientConfig(client, connStr, connUrl, clusterId, shardId, settings);
            } finally {
                IOUtils.closeQuietly(client);
            }

            // SERVERS
            boolean mongod = fm.isFeatureEnabled(MongoManager.FEATURE_ID, location);
            boolean arbiter = fm.isFeatureEnabled(MongoManager.ARBITER_FEATURE, location);
            boolean local = fm.isFeatureEnabled(MongoManager.LOCAL_FEATURE, location);
            boolean localArbiter = fm.isFeatureEnabled(MongoManager.LOCAL_ARBITER_FEATURE, location);
            FileWriter server = new FileWriter(new File(dir, "mongodb.cfdat"));
            try {
                writeServerConfig(server, mongod, arbiter, local, localArbiter);
            } finally {
                IOUtils.closeQuietly(server);
            }
        }

        // Global mongo model for UI
        List<Location> arbiters = fm.getLocationsForEnabledFeature(MongoManager.ARBITER_FEATURE);
        writeGlobalModel(manager, dbs, arbiters);

        for (Location location : all) {
            File dir = manager.getLocationDataDirectory(location);
            String model = null;
            String ini = null;
            Integer regionId = location.getRegionId();
            Integer locationId = location.getId();
            if (regionId != null) {
                model = localModelsByRegion.get(regionId);
                ini = localInisByRegionAndLocation.get(locationId);
            }

            File modelFile = new File(dir, "mongo-local.json");
            if (model != null) {
                FileUtils.writeStringToFile(modelFile, model);
            } else {
                if (modelFile.exists()) {
                    modelFile.delete();
                }
            }

            File iniFile = new File(dir, "mongo-local.ini");
            if (ini != null) {
                FileUtils.writeStringToFile(iniFile, ini);
            } else {
                if (iniFile.exists()) {
                    iniFile.delete();
                }
            }
        }

        // Local models used for UI to display each cluster's status
        for (Region region : regions) {
            File modelFile = getShardModelFile(manager, region);
            String model = localModelsByRegion.get(region.getId());
            if (model != null) {
                FileUtils.writeStringToFile(modelFile, model);
            } else {
                if (modelFile.exists()) {
                    modelFile.delete();
                }
            }
        }
    }

    public void writeGlobalModel() throws IOException {
        FeatureManager fm = m_configManager.getFeatureManager();
        List<Location> dbs = fm.getLocationsForEnabledFeature(MongoManager.FEATURE_ID);
        List<Location> arbiters = fm.getLocationsForEnabledFeature(MongoManager.ARBITER_FEATURE);
        writeGlobalModel(m_configManager, dbs, arbiters);
    }

    void writeGlobalModel(ConfigManager manager, List<Location> dbs, List<Location> arbiters) throws IOException {
        Writer w = null;
        try {
            File f = new File(manager.getGlobalDataDirectory(), "mongo.json");
            w = new FileWriter(f);
            modelFile(w, dbs, arbiters, GLOBAL_REPLSET, false, MongoSettings.SERVER_PORT, MongoSettings.ARBITER_PORT);
        } finally {
            IOUtils.closeQuietly(w);
        }
    }

    public static File getShardModelFile(ConfigManager configManager, Region r) {
        File dir = configManager.getGlobalDataDirectory();
        String fname = format("mongo-local-%d.json", r.getId());
        return new File(dir, fname);
    }

    Map<Integer, String> getLocalIniByRegionAndLocation(Collection<Region> regions, Collection<Location> localServers,
        MongoSettings settings) throws IOException {

        Map<Integer, String> inis = new HashMap<Integer, String>();
        Map<Integer, List<Location>> localServersByRegion = Region.locationsByRegion(localServers);
        for (Region region : regions) {
            List<Location> ldbs = localServersByRegion.get(region.getId());
            if (ldbs != null) {
                for (Location ldb : ldbs) {
                    StringWriter ini = new StringWriter();
                    writeLocalClientConfig(ini, ldbs, ldb.getId(), region.getId(), MongoSettings.LOCAL_PORT, settings);
                    inis.put(ldb.getId(), ini.toString());
                }
            }
        }
        return inis;
    }

    Map<Integer, String> getLocalModelsByRegion(Collection<Region> regions, Collection<Location> localServers,
            Collection<Location> localArbiters) throws IOException {
        Map<Integer, String> models = new HashMap<Integer, String>();
        Map<Integer, List<Location>> localServersByRegion = Region.locationsByRegion(localServers);
        Map<Integer, List<Location>> localArbitersByRegion = Region.locationsByRegion(localArbiters);
        for (Region region : regions) {
            StringWriter model = new StringWriter();
            List<Location> ldbs = localServersByRegion.get(region.getId());
            List<Location> larbs = localArbitersByRegion.get(region.getId());
            if (ldbs != null || larbs != null) {
                modelFile(model, ldbs, larbs, LOCAL_REPLSET, true, MongoSettings.LOCAL_PORT,
                        MongoSettings.LOCAL_ARBITER_PORT);
                models.put(region.getId(), model.toString());
            }
        }
        return models;
    }

    void modelFile(Writer sb, List<Location> servers, List<Location> arbiters, String replSet, boolean isLocal,
            int dbPort, int arbPort) throws IOException {
        Map<String, Object> model = new HashMap<String, Object>();
        //XX-11378: include "read tags" always - also for regional databases
        model.put("servers", serverIdMap(servers, true, dbPort));
        model.put("arbiters", serverIdMap(arbiters, false, arbPort));
        model.put("replSet", replSet);
        model.put("local", isLocal);
        String json = JSON.serialize(model);
        sb.write(json);
    }

    Map<String, Object> serverIdMap(Collection<Location> servers, boolean includeTags, int port) {
        if (servers == null) {
            return Collections.emptyMap();
        }

        Map<String, Object> meta = new HashMap<String, Object>();
        for (Location l : servers) {
            String id = l.getFqdn() + ':' + port;
            Map<String, Object> server = new HashMap<String, Object>();
            if (includeTags) {
                Map<String, String> tags = new LinkedHashMap<String, String>();
                String shardId = l.getRegionId() == null ? "0" : l.getRegionId().toString();
                tags.put(CLUSTER_ID, String.valueOf(l.getId()));
                tags.put(SHARD_ID, shardId);
                server.put("tags", tags);
            }
            meta.put(id, server);
        }

        return meta;
    }

    void writeServerConfig(Writer w, boolean mongod, boolean arbiter, boolean local, boolean localArbiter)
        throws IOException {
        String bindToAll = "0.0.0.0";
        CfengineModuleConfiguration config = new CfengineModuleConfiguration(w);
        config.writeClass("mongod", mongod);
        config.write("mongoBindIp", bindToAll);
        config.write("mongoPort", MongoSettings.SERVER_PORT);
        config.writeClass("mongod_arbiter", arbiter);
        config.write("mongoArbiterBindIp", bindToAll);
        config.write("mongoArbiterPort", MongoSettings.ARBITER_PORT);
        config.writeClass("mongo_local", local);
        config.writeClass("mongo_local_arbiter", localArbiter);
    }

    /**
     * Write mong configuration
     * @param w - writer, the Output stream where to write config data
     * @param connStr - the connection string
     * @param connUrl - the connection url
     * @param clusterId - the location id value
     * @param shardId - the shard id value (same as region id)
     * @param settings - mongo settings, if null is passed, no mongo settings are written
     * @throws IOException
     */
    void writeClientConfig(Writer w, String connStr, String connUrl, int clusterId, int shardId,
            MongoSettings settings) throws IOException {

        KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(w);
        config.write("connectionUrl", connUrl);
        config.write("connectionString", connStr);
        config.write(CLUSTER_ID, clusterId);
        config.write(SHARD_ID, shardId);
        config.write("useReadTags", true);
        if (settings != null) {
            config.writeSettings(settings.getSettings());
        }
    }

    void writeLocalClientConfig(Writer w, List<Location> servers, int clusterId, int shardId,
        int port, MongoSettings settings) throws IOException {

        String lconnStr = getConnectionString(servers, LOCAL_REPLSET, port);
        String lconnUrl = getConnectionUrl(servers, clusterId, shardId, port);
        writeClientConfig(w, lconnStr, lconnUrl, clusterId, shardId, settings);
    }

    // C++ driver/projects use connection string format
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

    // java driver/projects use URL format
    // XX-11378: always use "read tags"
    String getConnectionUrl(List<Location> servers, int clusterId, int shardId, int port) {
        StringBuilder r = new StringBuilder("mongodb://");
        for (int i = 0; i < servers.size(); i++) {
            Location server = servers.get(i);
            if (i > 0) {
                r.append(',');
            }
            r.append(server.getFqdn() + ':' + port);
        }
        r.append("/?readPreference=nearest");
        // Format from
        //   http://api.mongodb.org/java/current/com/mongodb/MongoURI.html
        // doc is unclear if ';' usage here is deprecated in favor of '&' for this param
        r.append("&readPreferenceTags=clusterId:").append(clusterId);
        r.append(";readPreferenceTags=shardId:").append(shardId);
        r.append(";readPreferenceTags="); // all else, use any server
        return r.toString();
    }

    public void setMongoManager(MongoManager mongoManager) {
        m_mongoManager = mongoManager;
    }

    public void setRegionManager(RegionManager regionManager) {
        m_regionManager = regionManager;
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }
}
