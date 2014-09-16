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

import org.jivesoftware.openfire.provider.RemoteServerProvider;
import org.jivesoftware.openfire.server.RemoteServerConfiguration;
import org.jivesoftware.openfire.server.RemoteServerConfiguration.Permission;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;

public class MongoRemoteServerProvider extends BaseMongoProvider implements RemoteServerProvider {
    private static final String COLLECTION_NAME = "ofRemoteServerConf";

    public MongoRemoteServerProvider() {
        setDefaultCollectionName(COLLECTION_NAME);
        DBCollection rSrvCollection = getDefaultCollection();

        DBObject index = new BasicDBObject();
        index.put("xmppDomain", 1);

        rSrvCollection.ensureIndex(index);
    }

    @Override
    public void addConfiguration(RemoteServerConfiguration configuration) {
        DBCollection rSrvCollection = getDefaultCollection();

        DBObject toInsert = new BasicDBObject();
        toInsert.put("xmppDomain", configuration.getDomain());
        toInsert.put("remotePort", configuration.getRemotePort());
        toInsert.put("permission", configuration.getPermission().toString());

        rSrvCollection.insert(toInsert);
    }

    @Override
    public RemoteServerConfiguration getConfiguration(String domain) {
        DBCollection rSrvCollection = getDefaultCollection();

        DBObject query = new BasicDBObject();
        query.put("xmppDomain", domain);

        RemoteServerConfiguration conf;
        DBObject confObj = rSrvCollection.findOne(query);

        if (confObj != null) {
            Integer remote = (Integer) confObj.get("remotePort");
            String permission = (String) confObj.get("permission");

            conf = new RemoteServerConfiguration(domain);
            conf.setPermission(Permission.valueOf(permission));
            conf.setRemotePort(remote);
        } else {
            conf = null;
        }

        return conf;
    }

    @Override
    public Collection<RemoteServerConfiguration> getConfigurations(Permission permission) {
        Collection<RemoteServerConfiguration> confs = new ArrayList<RemoteServerConfiguration>();
        DBCollection rSrvCollection = getDefaultCollection();

        DBObject query = new BasicDBObject();
        query.put("permission", permission.toString());

        for (DBObject confObj : rSrvCollection.find(query)) {
            String domain = (String) confObj.get("domain");
            Integer remote = (Integer) confObj.get("remotePort");

            RemoteServerConfiguration conf = new RemoteServerConfiguration(domain);
            conf.setPermission(permission);
            conf.setRemotePort(remote);
            confs.add(conf);
        }

        return confs;
    }

    @Override
    public void deleteConfiguration(String domain) {
        DBCollection rSrvCollection = getDefaultCollection();

        DBObject toDelete = new BasicDBObject();
        toDelete.put("xmppDomain", domain);

        rSrvCollection.remove(toDelete);
    }
}
