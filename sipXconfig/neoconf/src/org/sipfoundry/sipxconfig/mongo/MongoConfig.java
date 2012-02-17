/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mongo;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.CfengineModuleConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.feature.FeatureManager;

public class MongoConfig implements ConfigProvider {
    private MongoManager m_mongoManager;
    private MongoReplicaSetManager m_mongoReplicaSetManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(MongoManager.FEATURE_ID, LocationsManager.FEATURE)) {
            return;
        }
        FeatureManager fm = manager.getFeatureManager();
        Location[] all = manager.getLocationManager().getLocations();
        //TODO  - get firewall/encryption details from system
        boolean firewall = true;
        boolean encrypt = false;
        List<Location> secondary = fm.getLocationsForEnabledFeature(MongoManager.FEATURE_ID);
        Location primary = manager.getLocationManager().getPrimaryLocation();
        MongoSettings settings = m_mongoManager.getSettings();
        int port = settings.getPort();
        String connStr = getConnectionString(primary, secondary, port, firewall, encrypt);
        String connUrl = getConnectionUrl(primary, secondary, port, firewall, encrypt);
        for (Location location : all) {

            // CLIENT
            File dir = manager.getLocationDataDirectory(location);
            FileWriter client = new FileWriter(new File(dir, "mongo-client.ini"));
            try {
                writeClientConfig(client, connStr, connUrl);
            } finally {
                IOUtils.closeQuietly(client);
            }

            // SERVER
            boolean enabled = fm.isFeatureEnabled(MongoManager.FEATURE_ID, location) || location.isPrimary();
            FileWriter server = new FileWriter(new File(dir, "mongodb.cfdat"));
            try {
                writeServerConfig(server, enabled, firewall, encrypt);
            } finally {
                IOUtils.closeQuietly(server);
            }
        }

        // NOTE:  live updating of mongo settings.
        m_mongoReplicaSetManager.checkMembers();
    }

    void writeServerConfig(Writer w, boolean enabled, boolean firewall, boolean encrypt) throws IOException {
        CfengineModuleConfiguration config = new CfengineModuleConfiguration(w);
        config.writeClass("mongod", enabled);
        // TODO: consider stunnel/encrypt
        config.write("mongoBindIp", firewall ? "0.0.0.0" : "127.0.0.1");
        config.write("mongoPort", "27017");
    }

    void writeClientConfig(Writer w, String connStr, String connUrl) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(w);
        config.write("connectionUrl", connUrl);
        config.write("connectionString", connStr);
    }

    String getConnectionString(Location primary, List<Location> secondary, int port, boolean firewall,
            boolean encrypt) {
        if (!firewall) {
            return "sipxecs/127.0.0.1:" + port;
        }
        StringBuilder r = new StringBuilder("sipxecs/").append(primary.getFqdn()).append(':').append(port);
        if (secondary != null) {
            for (Location location : secondary) {
                r.append(',').append(location.getFqdn()).append(':').append(port);
            }
        }
        return r.toString();
    }

    String getConnectionUrl(Location primary, List<Location> secondary, int port, boolean firewall, boolean encrypt) {
        if (!firewall) {
            return "mongodb://127.0.0.1:" + port + "/?slaveOk=true";
        }
        StringBuilder r = new StringBuilder("mongodb://").append(primary.getFqdn());
        r.append(':').append(port);
        if (secondary != null) {
            for (Location location : secondary) {
                r.append(',').append(location.getFqdn()).append(':').append(port);
            }
        }
        r.append("/?slaveOk=true");
        return r.toString();
    }

    public void setMongoManager(MongoManager mongoManager) {
        m_mongoManager = mongoManager;
    }

    public void setMongoReplicaSetManager(MongoReplicaSetManager mongoReplicaSetManager) {
        m_mongoReplicaSetManager = mongoReplicaSetManager;
    }
}
