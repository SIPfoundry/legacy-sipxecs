/**
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
package org.sipfoundry.sipxconfig.api.impl;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;

import org.sipfoundry.sipxconfig.api.ServerApi;
import org.sipfoundry.sipxconfig.api.model.ServerBean;
import org.sipfoundry.sipxconfig.api.model.ServerBean.BundleBean;
import org.sipfoundry.sipxconfig.api.model.ServerBean.BundleList;
import org.sipfoundry.sipxconfig.api.model.ServerBean.FeatureBean;
import org.sipfoundry.sipxconfig.api.model.ServerBean.FeatureList;
import org.sipfoundry.sipxconfig.api.model.ServerBean.JobList;
import org.sipfoundry.sipxconfig.api.model.ServerBean.ServiceBean;
import org.sipfoundry.sipxconfig.api.model.ServerBean.ServiceList;
import org.sipfoundry.sipxconfig.api.model.ServerList;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.job.Job;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public class ServerApiImpl implements ServerApi {
    public static final String LOCATION_TYPE = "location";
    private LocationsManager m_locationsManager;
    private ConfigManager m_configManager;
    private SnmpManager m_snmpManager;
    private JobContext m_jobContext;
    private FeatureManager m_featureManager;

    @Override
    public Response getServers() {
        List<Location> locations = Arrays.asList(m_locationsManager.getLocations());
        Collection<Location> registeredLocations = m_configManager.getRegisteredLocations(locations);
        return Response.ok().entity(ServerList.convertLocationList(locations, registeredLocations)).build();
    }

    @Override
    public Response newServer(ServerBean server) {
        Location location = new Location();
        m_locationsManager.saveLocation(ServerBean.convertToLocation(location, server));
        return Response.ok().build();
    }

    @Override
    public Response getServer(String serverId) {
        Location location = getLocationByIdOrFqdn(serverId);
        if (location != null) {
            List<Location> locations = Arrays.asList(m_locationsManager.getLocations());
            Collection<Location> registeredLocations = m_configManager.getRegisteredLocations(locations);
            return Response.ok()
                    .entity(ServerBean.convertLocation(location, registeredLocations.contains(location))).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response deleteServer(String serverId) {
        Location location = getLocationByIdOrFqdn(serverId);
        if (location != null) {
            m_locationsManager.deleteLocation(location);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response updateServer(String serverId, ServerBean server) {
        Location location = getLocationByIdOrFqdn(serverId);
        if (location != null) {
            m_locationsManager.saveLocation(ServerBean.convertToLocation(location, server));
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getServices(String serverId) {
        Location location = getLocationByIdOrFqdn(serverId);
        if (location != null) {
            List<ServiceStatus> statuses = m_snmpManager.getServicesStatuses(location);
            return Response.ok().entity(ServiceList.convertServiceList(statuses)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getService(String serverId, String serviceId) {
        Location location = getLocationByIdOrFqdn(serverId);
        if (location != null) {
            List<ServiceStatus> statuses = m_snmpManager.getServicesStatuses(location);
            for (ServiceStatus status : statuses) {
                if (status.getServiceBeanId().equals(serviceId)) {
                    return Response.ok().entity(ServiceBean.convertService(status)).build();
                }
            }
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response restartService(String serverId, String serviceId) {
        Location location = getLocationByIdOrFqdn(serverId);
        if (location != null) {
            List<ProcessDefinition> defs = m_snmpManager.getProcessDefinitions(location,
                    Collections.singletonList(serviceId));
            if (defs != null) {
                m_snmpManager.restartProcesses(location, defs);
                return Response.ok().build();
            }
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getBundles() {
        return Response.ok().entity(BundleList.convertBundleList(m_featureManager.getBundles())).build();
    }

    @Override
    public Response getBundle(String bundleId) {
        List<Bundle> bundles = m_featureManager.getBundles();
        for (Bundle bundle : bundles) {
            if (bundle.getId().equals(bundleId)) {
                return Response.ok().entity(BundleBean.convertBundle(bundle)).build();
            }
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getFeatures() {
        List<Bundle> bundles = m_featureManager.getBundles();
        List<FeatureBean> features = new LinkedList<FeatureBean>();
        for (Bundle bundle : bundles) {
            for (Feature feature : bundle.getFeatures()) {
                if (feature instanceof GlobalFeature) {
                    features.add(FeatureBean.convertFeature(feature,
                            m_featureManager.isFeatureEnabled((GlobalFeature) feature), bundle.getId(), "global"));
                } else {
                    features.add(FeatureBean.convertFeature(feature,
                            m_featureManager.isFeatureEnabled((LocationFeature) feature), bundle.getId(),
                            LOCATION_TYPE));
                }
            }
        }
        return Response.ok().entity(FeatureList.buildFeatureList(features)).build();
    }

    @Override
    public Response enableGlobalFeature(String featureId) {
        m_featureManager.enableGlobalFeature(new GlobalFeature(featureId), true);
        return Response.ok().build();
    }

    @Override
    public Response disableGlobalFeature(String featureId) {
        m_featureManager.enableGlobalFeature(new GlobalFeature(featureId), false);
        return Response.ok().build();
    }

    @Override
    public Response getFeatures(String serverId) {
        Location location = getLocationByIdOrFqdn(serverId);
        List<Bundle> bundles = m_featureManager.getBundles();
        List<FeatureBean> features = new LinkedList<FeatureBean>();
        if (location != null) {
            for (Bundle bundle : bundles) {
                for (Feature feature : bundle.getFeatures()) {
                    if (feature instanceof LocationFeature) {
                        features.add(FeatureBean.convertFeature(feature,
                                m_featureManager.isFeatureEnabled((LocationFeature) feature, location),
                                bundle.getId(), LOCATION_TYPE));
                    }
                }
            }
            return Response.ok().entity(FeatureList.buildFeatureList(features)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response enableFeature(String serverId, String featureId) {
        Location location = getLocationByIdOrFqdn(serverId);
        if (location != null) {
            m_featureManager.enableLocationFeature(new LocationFeature(featureId), location, true);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response disableFeature(String serverId, String featureId) {
        Location location = getLocationByIdOrFqdn(serverId);
        if (location != null) {
            m_featureManager.enableLocationFeature(new LocationFeature(featureId), location, false);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getJobs(HttpServletRequest request) {
        List<Job> jobs = m_jobContext.getJobs();
        return Response.ok().entity(JobList.convertJobList(jobs, request.getLocale())).build();
    }

    @Override
    public Response getFailedJobs(HttpServletRequest request) {
        List<Job> jobs = m_jobContext.getFailedJobs();
        return Response.ok().entity(JobList.convertJobList(jobs, request.getLocale())).build();
    }

    @Override
    public Response getCompletedJobs(HttpServletRequest request) {
        List<Job> jobs = m_jobContext.getNotFailedJobs();
        return Response.ok().entity(JobList.convertJobList(jobs, request.getLocale())).build();
    }

    @Override
    public Response clearJobs() {
        m_jobContext.clear();
        return Response.ok().build();
    }

    @Override
    public Response clearFailedJobs() {
        m_jobContext.clearFailed();
        return Response.ok().build();
    }

    @Override
    public Response clearCompletedJobs() {
        m_jobContext.removeCompleted();
        return Response.ok().build();
    }

    @Override
    public Response getJobs(String serverId, HttpServletRequest request) {
        Location location = getLocationByIdOrFqdn(serverId);
        return ResponseUtils.buildJobListResponse(m_jobContext.getJobs(), location, request.getLocale());
    }

    @Override
    public Response getFailedJobs(String serverId, HttpServletRequest request) {
        Location location = getLocationByIdOrFqdn(serverId);
        return ResponseUtils.buildJobListResponse(m_jobContext.getFailedJobs(), location, request.getLocale());
    }

    @Override
    public Response getCompletedJobs(String serverId, HttpServletRequest request) {
        Location location = getLocationByIdOrFqdn(serverId);
        return ResponseUtils.buildJobListResponse(m_jobContext.getNotFailedJobs(), location, request.getLocale());
    }

    private Location getLocationByIdOrFqdn(String id) {
        Location location = null;
        try {
            int locationId = Integer.parseInt(id);
            location = m_locationsManager.getLocation(locationId);
        } catch (NumberFormatException e) {
            // no id then it must be MAC
            location = m_locationsManager.getLocationByFqdn(id);
        }
        return location;
    }

    public void setLocationsManager(LocationsManager manager) {
        m_locationsManager = manager;
    }

    public void setConfigManager(ConfigManager manager) {
        m_configManager = manager;
    }

    public void setSnmpManager(SnmpManager manager) {
        m_snmpManager = manager;
    }

    public void setJobContext(JobContext context) {
        m_jobContext = context;
    }

    public void setFeatureManager(FeatureManager manager) {
        m_featureManager = manager;
    }

}
