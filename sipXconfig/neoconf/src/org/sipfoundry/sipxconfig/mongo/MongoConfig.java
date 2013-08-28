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
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
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
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.region.Region;
import org.sipfoundry.sipxconfig.region.RegionManager;

import com.mongodb.util.JSON;

public class MongoConfig implements ConfigProvider {
    private static final String GLOBAL_REPLSET = "sipxecs";
    private static final String LOCAL_REPLSET = "sipxlocal";
    private MongoManager m_mongoManager;
    private RegionManager m_regionManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(MongoManager.FEATURE_ID, LocationsManager.FEATURE, MongoManager.ARBITER_FEATURE,
                MongoManager.LOCAL_FEATURE, MongoManager.LOCAL_ARBITER_FEATURE)) {
            return;
        }
        FeatureManager fm = manager.getFeatureManager();
        Location[] all = manager.getLocationManager().getLocations();
        MongoSettings settings = m_mongoManager.getSettings();
        List<Location> dbs = fm.getLocationsForEnabledFeature(MongoManager.FEATURE_ID);
        String connStr = getConnectionString(dbs, GLOBAL_REPLSET, settings.getPort());
        String connUrl = getConnectionUrl(dbs, settings.getPort());
        
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
            FileWriter server = new FileWriter(new File(dir, "mongodb.cfdat"));
            try {
                writeServerConfig(server, mongod, arbiter, local, localArbiter);
            } finally {
                IOUtils.closeQuietly(server);
            }
        }
        
        // Global mongo model for UI
        List<Location> arbiters = fm.getLocationsForEnabledFeature(MongoManager.ARBITER_FEATURE);
        Writer w = null;
        try {
            File f = new File(manager.getGlobalDataDirectory(), "mongo.json");
            w = new FileWriter(f);
            modelFile(w, dbs, arbiters, GLOBAL_REPLSET, false, MongoSettings.SERVER_PORT, MongoSettings.ARBITER_PORT);
        } finally {
            IOUtils.closeQuietly(w);
        }
        
        // regional mongo server config and models      
        List<Location> localServers = fm.getLocationsForEnabledFeature(MongoManager.LOCAL_FEATURE);
        List<Location> localArbiters = fm.getLocationsForEnabledFeature(MongoManager.LOCAL_ARBITER_FEATURE);
        List<Region> regions = m_regionManager.getRegions();
        Map<Integer, String> localModelsByRegion = getLocalModelsByRegion(regions, localServers, localArbiters);
        Map<Integer, String> localInisByRegion = getLocalIniByRegion(regions, localServers);
        for (Location location : all) {
            File dir = manager.getLocationDataDirectory(location);
    		String model = null;
    		String ini = null;
    		Integer regionId = location.getRegionId();
        	if (regionId != null) {
        		model = localModelsByRegion.get(regionId);
        		ini = localInisByRegion.get(regionId);
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
    
    public static File getShardModelFile(ConfigManager configManager, Region r) {
    	File dir = configManager.getGlobalDataDirectory();
        String fname = format("mongo-local-%d.json", r.getId());
		return new File(dir, fname);    	
    }
    
	Map<Integer, String> getLocalIniByRegion(Collection<Region> regions,
			Collection<Location> localServers) throws IOException {
    	Map<Integer, String> inis = new HashMap<Integer, String>();
        Map<Integer, List<Location>> localServersByRegion = Region.locationsByRegion(localServers);
        for (Region region : regions) {    	
            StringWriter ini = new StringWriter();
	        List<Location> ldbs = localServersByRegion.get(region.getId());
	        if (ldbs != null) {
	        	writeLocalClientConfig(ini, ldbs, region.getId(), MongoSettings.LOCAL_PORT);
	        	inis.put(region.getId(), ini.toString());
	        }
        }
        return inis;        
    }
    
	Map<Integer, String> getLocalModelsByRegion(Collection<Region> regions,
			Collection<Location> localServers,
			Collection<Location> localArbiters) throws IOException {
    	Map<Integer, String> models = new HashMap<Integer, String>();
        Map<Integer, List<Location>> localServersByRegion = Region.locationsByRegion(localServers);
        Map<Integer, List<Location>> localArbitersByRegion = Region.locationsByRegion(localArbiters);
        for (Region region : regions) {    	
            StringWriter model = new StringWriter();
	        List<Location> ldbs = localServersByRegion.get(region.getId());
	        List<Location> larbs = localArbitersByRegion.get(region.getId());
	        if (ldbs != null || larbs != null) {
	        	modelFile(model, ldbs, larbs, LOCAL_REPLSET,
                    true, MongoSettings.LOCAL_PORT, MongoSettings.LOCAL_ARBITER_PORT);
	            models.put(region.getId(), model.toString());
	        }
        }
        return models;        
    }
    
	void modelFile(Writer sb, List<Location> servers, List<Location> arbiters,
			String replSet, boolean isLocal, int dbPort, int arbPort)
			throws IOException {
        Map<String, Object> model = new HashMap<String, Object>();
        model.put("servers", serverIdList(servers, dbPort));
        model.put("arbiters", serverIdList(arbiters, arbPort));
        model.put("replSet", replSet);
        model.put("local", isLocal);
        String json = JSON.serialize(model);
        sb.write(json);
    }

    List<String> serverIdList(Collection<Location> servers, int port) {
    	if (servers == null) {
    		return Collections.emptyList();
    	}
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
        config.writeClass("mongo_local_arbiter", localArbiter);
    }

    void writeClientConfig(Writer w, String connStr, String connUrl) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(w);
        config.write("connectionUrl", connUrl);
        config.write("connectionString", connStr);
    }

    void writeLocalClientConfig(Writer w, List<Location> servers, int shardId, int port) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(w);
        String lconnStr = getConnectionString(servers, LOCAL_REPLSET, port);
        String lconnUrl = getConnectionUrl(servers, port);
        writeClientConfig(w, lconnStr, lconnUrl);
        config.write("shardId", shardId);
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
    
    public void setRegionManager(RegionManager regionManager) {
    	m_regionManager = regionManager;
    }
}
