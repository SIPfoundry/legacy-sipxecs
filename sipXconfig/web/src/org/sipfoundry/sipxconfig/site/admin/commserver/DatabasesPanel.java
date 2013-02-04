/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.util.ArrayList;
import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.mongo.MongoManager;
import org.sipfoundry.sipxconfig.mongo.MongoReplicaSetManager;
import org.sipfoundry.sipxconfig.mongo.MongoServer;
import org.sipfoundry.sipxconfig.mongo.MongoSettings;

public abstract class DatabasesPanel extends BaseComponent implements PageBeginRenderListener {

    @InjectObject("spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    @InjectObject("spring:mongoReplicaSetManager")
    public abstract MongoReplicaSetManager getReplicaSetManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract SelectMap getMongoSelections();

    @Asset("/images/server.png")
    public abstract IAsset getServerIcon();

    @Asset("/images/whistle.png")
    public abstract IAsset getArbiterIcon();

    @Asset("/images/cross.png")
    public abstract IAsset getErrorIcon();

    @Asset("/images/running.png")
    public abstract IAsset getRunningIcon();

    @Asset("/images/error.png")
    public abstract IAsset getUnconfiguredIcon();

    public abstract void setMongos(Collection<MongoServer> mongos);

    public abstract Collection<MongoServer> getMongos();

    public abstract MongoServer getMongo();

    public abstract int getTotalVoters();

    public abstract void setTotalVoters(int voters);

    public Collection getAllSelected() {
        return getSelections().getAllSelected();
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        Map<String, MongoServer> servers = new LinkedHashMap<String, MongoServer>();
        addMongoFeatures(getFeatureManager().getLocationsForEnabledFeature(MongoManager.FEATURE_ID),
                MongoSettings.SERVER_PORT, servers);
        addMongoFeatures(getFeatureManager().getLocationsForEnabledFeature(MongoManager.ARBITER_FEATURE),
                MongoSettings.ARBITER_PORT, servers);
        List<MongoServer> serversInReplicaSet = new ArrayList<MongoServer>();
        try {
            serversInReplicaSet = getReplicaSetManager().getMongoServers(false, true);
        } catch (Exception ex) {
            getValidator().record(new UserException(ex.getMessage()), getMessages());
        }
        int totalNoOfVoters = 0;
        for (MongoServer server : serversInReplicaSet) {
            servers.put(server.getName(), server);
            if (server.isVotingMember()) {
                totalNoOfVoters++;
            }
        }
        setMongos(servers.values());
        setTotalVoters(totalNoOfVoters);
    }

    private void addMongoFeatures(List<Location> mongos, int port, Map<String, MongoServer> servers) {
        for (Location mongo : mongos) {
            MongoServer server = new MongoServer();
            String name = mongo.getFqdn() + ":" + port;
            server.setName(name);
            servers.put(name, server);
        }
    }

    public void addInReplicaSet(String name) {
        try {
            getReplicaSetManager().addInReplicaSet(name);
        } catch (Exception ex) {
            getValidator().record(new UserException(ex.getMessage()), getMessages());
        }
    }

    public void removeFromReplicaSet(String name) {
        try {
            getReplicaSetManager().removeFromReplicaSet(name);
        } catch (Exception ex) {
            getValidator().record(new UserException(ex.getMessage()), getMessages());
        }
    }

    public void removeVoter(Integer id) {
        try {
            getReplicaSetManager().removeVoter(id);
        } catch (Exception ex) {
            getValidator().record(new UserException(ex.getMessage()), getMessages());
        }
    }

    public void addVoter(Integer id) {
        try {
            getReplicaSetManager().addVoter(id);
        } catch (Exception ex) {
            getValidator().record(new UserException(ex.getMessage()), getMessages());
        }
    }

    public void stepDown() {
        try {
            getReplicaSetManager().stepDown();
        } catch (Exception ex) {
            getValidator().record(new UserException(ex.getMessage()), getMessages());
        }
    }

    public void forceReconfig() {
        try {
            getReplicaSetManager().forceReconfig();
        } catch (Exception ex) {
            getValidator().record(new UserException(ex.getMessage()), getMessages());
        }
    }

    public IPrimaryKeyConverter getConverter() {
        return new IPrimaryKeyConverter() {

            @Override
            public Object getValue(Object arg0) {
                if (arg0 instanceof MongoServer) {
                    return arg0;
                }
                return null;
            }

            @Override
            public Object getPrimaryKey(Object arg0) {
                if (arg0 instanceof MongoServer) {
                    MongoServer mongo = (MongoServer) arg0;
                    return mongo.getReplicaSetId();
                }
                return null;
            }
        };
    }

}
