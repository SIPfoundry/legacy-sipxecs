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
import org.sipfoundry.sipxconfig.region.Region;
import org.sipfoundry.sipxconfig.region.RegionManager;

import com.mongodb.util.JSON;

public class MongoLocalApi extends Resource {
    private MongoManager m_mongoManager;
    private RegionManager m_regionManager;
    private LocationsManager m_locationsManager;
    private MongoApi m_globalApi;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(APPLICATION_JSON));
    }

    @Override
    public boolean allowGet() {
        return true;
    }

    @Override
    public boolean allowPost() {
        return true;
    }

    // POST
    public void acceptRepresentation(Representation entity) throws ResourceException {
        String json;
        try {
            json = IOUtils.toString(entity.getStream());
            @SuppressWarnings("unchecked")
            Map<String, Object> form = (Map<String, Object>) JSON.parse(json);
            String action = (String) form.get("action");
            String hostPort = (String) form.get("server");
            String fqdn = MongoNode.fqdn(hostPort);
            Location location = m_locationsManager.getLocationByFqdn(fqdn);
            if (location.getRegionId() != null) {
                Region region = m_regionManager.getRegion(location.getRegionId());
                MongoReplSetManager rsMgr = m_mongoManager.getShardManager(region);
                MongoMeta meta = rsMgr.getMeta();
                if (action.startsWith("NEW_")) {
                    boolean first = (meta.getNodes().size() == 0);
                    if (action.equals("NEW_LOCAL")) {
                        if (first) {
                            rsMgr.addFirstLocalDatabase(hostPort);
                        } else {
                            MongoNode primary = meta.getPrimary();
                            rsMgr.addLocalDatabase(primary.getHostPort(), hostPort);
                        }
                    } else {
                        if (first) {
                            throw new IllegalStateException("Add servers before you add arbiters");
                        }
                        MongoNode primary = meta.getPrimary();
                        rsMgr.addLocalArbiter(primary.getHostPort(), hostPort);
                    }
                } else if (action.equals("DELETE_LOCAL")) {
                    rsMgr.removeLastLocalDatabase(hostPort);
                } else if (action.equals("DELETE_LOCAL_ARBITER")) {
                    rsMgr.removeLastLocalArbiter(hostPort);
                } else {
                    m_globalApi.takeAction(rsMgr, meta, action, hostPort);
                }
            }
        } catch (IOException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        } catch (UserException ex) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, ex.getMessage());
        }
    };

    // GET
    @Override
    public Representation represent(Variant variant) throws ResourceException {
        Map<String, Object> meta = new HashMap<String, Object>();
        getResponse().setStatus(Status.SUCCESS_OK);

        Collection<Location> locations = m_mongoManager.getConfigManager().getRegisteredLocations(
                m_locationsManager.getLocationsList());
        Collection<Location> locals = m_mongoManager.getFeatureManager().getLocationsForEnabledFeature(
                MongoManager.LOCAL_FEATURE);
        Collection<Location> localArbs = m_mongoManager.getFeatureManager().getLocationsForEnabledFeature(
                MongoManager.LOCAL_ARBITER_FEATURE);

        List<String> candidateDbs = new ArrayList<String>();
        List<String> candidateArbs = new ArrayList<String>();
        for (Location location : locations) {
            // Make strict requirement to define a region for any location otherwise
            // logic gets messy w/o region object
            if (location.getRegionId() != null) {
                if (!locals.contains(location)) {
                    candidateDbs.add(location.getFqdn() + ':' + MongoSettings.LOCAL_PORT);
                }
                if (!localArbs.contains(location)) {
                    candidateArbs.add(location.getFqdn() + ':' + MongoSettings.LOCAL_ARBITER_PORT);
                }
            }
        }
        Collections.sort(candidateDbs);
        meta.put("dbCandidates", candidateDbs);
        meta.put("arbiterCandidates", candidateArbs);

        List<Map<String, Object>> shards = new ArrayList<Map<String, Object>>();
        Set<Location> localsAndArbs = new HashSet<Location>();
        localsAndArbs.addAll(locals);
        localsAndArbs.addAll(localArbs);
        boolean inProgress = false;
        Map<Integer, List<Location>> regionLocations = Region.locationsByRegion(localsAndArbs);
        for (Region region : m_regionManager.getRegions()) {
            List<Location> miniCluster = regionLocations.get(region.getId());
            if (miniCluster != null && miniCluster.size() > 0) {
                MongoReplSetManager rsMgr = m_mongoManager.getShardManager(region);
                Map<String, Object> localMeta = m_globalApi.metaMap(rsMgr, rsMgr.getMeta(), miniCluster);
                inProgress = inProgress | rsMgr.isInProgress();
                shards.add(localMeta);
            }
        }
        meta.put("shards", shards);
        meta.put("inProgress", inProgress);

        String json = JSON.serialize(meta);
        return new StringRepresentation(json);
    }

    public void setMongoManager(MongoManager mongoManager) {
        m_mongoManager = mongoManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setGlobalApi(MongoApi globalApi) {
        m_globalApi = globalApi;
    }

    public void setRegionManager(RegionManager regionManager) {
        m_regionManager = regionManager;
    }
}
