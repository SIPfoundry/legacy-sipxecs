/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.openfire.provider;

import java.util.ArrayList;
import java.util.Collection;

import org.jivesoftware.openfire.component.ExternalComponentConfiguration;
import org.jivesoftware.openfire.component.ExternalComponentConfiguration.Permission;
import org.jivesoftware.openfire.provider.ExternalComponentProvider;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;

public class MongoExternalComponentProvider extends BaseMongoProvider implements ExternalComponentProvider {
    private static final String COLLECTION_NAME = "ofExtComponentConf";

    public MongoExternalComponentProvider() {
        setDefaultCollectionName(COLLECTION_NAME);
        DBCollection extCompCollection = getDefaultCollection();

        DBObject index = new BasicDBObject();
        index.put("subdomain", 1);

        extCompCollection.ensureIndex(index);
    }

    @Override
    public ExternalComponentConfiguration getConfiguration(String subdomain, boolean useWildcard) {
        ExternalComponentConfiguration conf = null;
        DBCollection extCompCollection = getDefaultCollection();

        DBObject query = new BasicDBObject();

        query.put("subdomain", subdomain);
        query.put("wildcard", false);

        DBObject confObj = extCompCollection.findOne(query);

        if (confObj != null) {
            String secret = (String) confObj.get("secret");
            String permission = (String) confObj.get("permission");
            conf = new ExternalComponentConfiguration(subdomain, false, Permission.valueOf(permission), secret);
        } else if (useWildcard) {
            query = new BasicDBObject();

            query.put("subdomain", "/" + subdomain + "/"); // mongodb regex
            query.put("wildcard", false);

            confObj = extCompCollection.findOne(query);
            if (confObj != null) {
                String secret = (String) confObj.get("secret");
                String permission = (String) confObj.get("permission");

                conf = new ExternalComponentConfiguration(subdomain, false, Permission.valueOf(permission), secret);
            }
        }

        return conf;
    }

    @Override
    public void addConfiguration(ExternalComponentConfiguration configuration) {
        DBCollection extCompCollection = getDefaultCollection();

        DBObject toInsert = new BasicDBObject();

        toInsert.put("subdomain", configuration.getSubdomain());
        toInsert.put("wildcard", configuration.isWildcard());
        toInsert.put("permission", configuration.getPermission().toString());
        toInsert.put("secret", configuration.getSecret());

        extCompCollection.insert(toInsert);
    }

    @Override
    public Collection<ExternalComponentConfiguration> getConfigurations(Permission permission) {
        Collection<ExternalComponentConfiguration> confs = new ArrayList<ExternalComponentConfiguration>();
        DBCollection extCompCollection = getDefaultCollection();

        DBObject query = new BasicDBObject();

        query.put("permission", permission != null ? permission.toString() : null);

        for (DBObject confObj : extCompCollection.find(query)) {
            String subdomain = (String) confObj.get("subdomain");
            String secret = (String) confObj.get("secret");
            Boolean wildcard = (Boolean) confObj.get("wildcard");

            confs.add(new ExternalComponentConfiguration(subdomain, wildcard, permission, secret));
        }

        return confs;
    }

    @Override
    public void deleteConfigurationFromDB(ExternalComponentConfiguration configuration) {
        DBCollection extCompCollection = getDefaultCollection();

        DBObject toDelete = new BasicDBObject();

        toDelete.put("subdomain", configuration.getSubdomain());
        toDelete.put("wildcard", configuration.isWildcard());

        extCompCollection.remove(toDelete);
    }
}
