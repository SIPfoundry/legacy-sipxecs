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

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;

public class MongoConfig implements ConfigProvider {
    private static final String DEFAULT_PORT = ":27017";
    private MongoFeature m_mongo;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (request.applies(MongoFeature.FEATURE_ID)) {
            Location[] all = manager.getLocationManager().getLocations();
            MongoSettings settings = m_mongo.getSettings();
            int port = settings.getPort();
            String conStr = getConnectionString(all, port);
            String conUrl = getConnectionUrl(all, port);
            String equals = "=";
            for (Location location : all) {
                // every location gets a mongo client config
                File dir = manager.getLocationDataDirectory(location);
                File file = new File(dir, "mongo-client.ini.cfdat");
                FileWriter wtr = new FileWriter(file);
                KeyValueConfiguration config = new KeyValueConfiguration(wtr, equals);
                config.write("connectionUrl", conUrl);
                config.write("connectionString", conStr);
                IOUtils.closeQuietly(wtr);

                // only mongod servers get mondo config
                if (manager.getFeatureManager().isFeatureEnabled(MongoFeature.FEATURE_ID, location)) {
                    File filed = new File(dir, "mongod.conf.cfdat");
                    FileWriter wtrd = new FileWriter(filed);
                    KeyValueConfiguration configd = new KeyValueConfiguration(wtrd, equals);
                    config.write(settings.getSettings().getSetting("mongod"));
                    // TODO this should be 127.0.0.1 only
                    config.write("bind_ip", "0.0.0.0");
                    IOUtils.closeQuietly(wtrd);
                }
            }
        }
    }

    String getConnectionString(Location[] locations, int port) {
        if (locations.length == 1) {
            return "localhost:" + port;
        }
        StringBuilder r = new StringBuilder(locations[0].getAddress());
        for (int i = 1; i < locations.length; i++) {
            r.append(',').append(locations[i].getAddress()).append(':').append(port);
        }
        return r.toString();
    }

    String getConnectionUrl(Location[] locations, int port) {
        if (locations.length == 1) {
            return "mongodb://localhost:" + port + "/?slaveOk=true";
        }
        StringBuilder r = new StringBuilder("mongodb://").append(locations[0].getAddress());
        r.append(':').append(port);
        for (int i = 1; i < locations.length; i++) {
            r.append(',').append(locations[i].getAddress()).append(':').append(port);
        }
        r.append("/?slaveOk=true");
        return r.toString();
    }
}
