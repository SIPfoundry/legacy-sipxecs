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
package org.sipfoundry.sipxconfig.mongo;


import static org.restlet.data.MediaType.APPLICATION_JSON;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.StringRepresentation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;

import com.mongodb.util.JSON;

class MongoApi extends Resource {
    private static final Log LOG = LogFactory.getLog(MongoApi.class);

    private static final String HOST = "host";
    private static final String PRIORITY = "priority";
    private MongoManager m_mongoManager;
    private LocationsManager m_locationsManager;
    private MongoReplSetManager m_rsManager;
    private MongoShard m_shard;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(APPLICATION_JSON));
        String shardIdParam = (String) getRequest().getAttributes().get("shardId");
        if (StringUtils.isNotBlank(shardIdParam)) {
            Integer shardId = Integer.parseInt(shardIdParam);
            m_shard = m_mongoManager.getShard(shardId);
            m_rsManager = m_mongoManager.getShardManager(m_shard);
        } else {
            m_rsManager = m_mongoManager;
        }
    }

    @Override
    public boolean allowGet() {
        return true;
    }

    @Override
    public boolean allowPost() {
        return true;
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        getResponse().setStatus(Status.SUCCESS_OK);
        Collection<Location> locations = m_mongoManager.getConfigManager().getRegisteredLocations(
                m_locationsManager.getLocationsList());
        Map<String, Object> meta = metaMap(m_rsManager, m_rsManager.getMeta(), locations);
        String json = JSON.serialize(meta);
        return new StringRepresentation(json);
    }

    public void acceptRepresentation(Representation entity) throws ResourceException {
        String json;
        try {
            json = IOUtils.toString(entity.getStream());
            @SuppressWarnings("unchecked")
            Map<String, Object> form = (Map<String, Object>) JSON.parse(json);
            String action = (String) form.get("action");
            String hostPort = (String) form.get("server");
            MongoMeta meta = m_rsManager.getMeta();
            takeAction(m_rsManager, meta, action, hostPort);
        } catch (IOException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        }
    };

    MongoNode requirePrimary(MongoMeta meta) {
        MongoNode primary = meta.getPrimary();
        if (primary == null) {
            throw new UserException("No primary found");
        }
        return primary;
    }

    void takeAction(MongoReplSetManager rsManager, MongoMeta meta, String action, String hostPort) {
        String delete = "DELETE";
        if (action.startsWith("NEW_") || action.equals(delete)) {
            MongoNode node = meta.getNode(hostPort);
            MongoNode primary = requirePrimary(meta);
            if (action.equals("NEW_DB")) {
                rsManager.addDatabase(primary.getHostPort(), hostPort);
            } else if (action.equals("NEW_ARBITER")) {
                rsManager.addArbiter(primary.getHostPort(), hostPort);
            } else if (action.equals(delete)) {
                if (node.isArbiter()) {
                    rsManager.removeArbiter(primary.getHostPort(), hostPort);
                } else {
                    rsManager.removeDatabase(primary.getHostPort(), hostPort);
                }
            }
        } else {
            String primaryHostport = null;
            MongoNode primary = meta.getPrimary();
            if (primary != null) {
                primaryHostport = primary.getHostPort();
            }

            rsManager.takeAction(primaryHostport, hostPort, action);
        }
    }

    Map<String, Object> metaMap(MongoReplSetManager rsManager, MongoMeta meta, Collection<Location> locations) {
        Map<String, Object> map = new HashMap<String, Object>();

        Map<String, Location> locationMap = new HashMap<String, Location>();
        for (Location l : locations) {
            locationMap.put(l.getFqdn(), l);
        }
        List<String> candidateArbiters = new ArrayList<String>();
        List<String> candidateDatabases = new ArrayList<String>();
        Set<String> nodes = new HashSet<String>(meta.getServers());
        for (Map.Entry<String, Location> entry : locationMap.entrySet()) {
            String arbHostPort = entry.getKey() + ':' + MongoSettings.ARBITER_PORT;
            if (!nodes.contains(arbHostPort)) {
                candidateArbiters.add(arbHostPort);
            }
            String dbHostPort = entry.getKey() + ':' + MongoSettings.SERVER_PORT;
            if (!nodes.contains(dbHostPort)) {
                candidateDatabases.add(dbHostPort);
            }
        }
        Collections.sort(candidateArbiters);
        Collections.sort(candidateDatabases);
        map.put("inProgress", rsManager.isInProgress());
        map.put("lastConfigError", rsManager.getLastConfigError());
        map.put("arbiterCandidates", candidateArbiters);
        map.put("dbCandidates", candidateDatabases);

        Map<String, Object> dbs = new HashMap<String, Object>();
        Map<String, Object> arbiters = new HashMap<String, Object>();
        MongoNode primary = meta.getPrimary();
        Map<String, Object> primaryMeta = primary != null ? meta.getMetaData(primary.getHostPort()) : null;
        for (MongoNode node : meta.getNodes()) {
            Location l = locationMap.get(node.getFqdn());
            if (l == null) {
                LOG.warn("Could not find location for mongo node " + node.getFqdn());
                continue;
            }
            Map<String, Object> nmap = new HashMap<String, Object>();
            nmap.put("status", node.getStatus());
            nmap.put(HOST, l.getHostname());
            nmap.put("region", l.getRegionId());
            boolean local = m_mongoManager.getFeatureManager().isFeatureEnabled(MongoManager.LOCAL_FEATURE, l);
            nmap.put("local", local);
            nmap.put("required", meta.getRequiredActions(node.getHostPort()));
            nmap.put("available", meta.getAvailableActions(node.getHostPort()));
            Map<String, Object> config = getMemberConfig(primaryMeta, node.getHostPort());
            if (config != null) {
                Integer votes = (Integer) config.get("votes");
                nmap.put("voting", votes == null || votes >= 1);
                Double priority = (Double) config.get(PRIORITY);
                if (priority != null) {
                    nmap.put(PRIORITY, priority);
                }
            }
            if (node.isArbiter()) {
                arbiters.put(node.getHostPort(), nmap);
            } else {
                dbs.put(node.getHostPort(), nmap);
            }
        }
        map.put("databases", dbs);
        map.put("arbiters", arbiters);
        return map;
    }

    @SuppressWarnings("unchecked")
    public Map<String, Object> getMemberConfig(Map<String, Object> primaryMeta, String hostPort) {
        if (primaryMeta != null) {
            Map<String, Object> config = (Map<String, Object>) primaryMeta.get("config");
            if (config != null) {
                List<Map<String, Object>> members = (List<Map<String, Object>>) config.get("members");
                if (members != null) {
                    for (Map<String, Object> member : members) {
                        if (hostPort.equals(member.get(HOST))) {
                            return member;
                        }
                    }
                }
            }
        }
        return null;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setMongoManager(MongoManager mongoManager) {
        m_mongoManager = mongoManager;
    }
}
