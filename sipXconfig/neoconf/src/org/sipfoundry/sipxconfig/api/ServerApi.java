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
package org.sipfoundry.sipxconfig.api;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.Consumes;
import javax.ws.rs.DELETE;
import javax.ws.rs.GET;
import javax.ws.rs.POST;
import javax.ws.rs.PUT;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;

import org.apache.cxf.jaxrs.model.wadl.Description;
import org.sipfoundry.sipxconfig.api.model.ServerBean;

@Path("/servers/")
@Produces({
    MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
})
@Description("Servers Management REST API")
public interface ServerApi {

    @GET
    public Response getServers();

    @Path("profiles")
    @PUT
    public Response sendServerProfiles();

    @POST
    @Consumes({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
    })
    public Response newServer(@Description("Server bean to save") ServerBean server);

    @Path("{serverId}")
    @GET
    public Response getServer(@Description("Server internal id or FQDN") @PathParam("serverId") String serverId);

    @Path("{serverId}")
    @DELETE
    public Response deleteServer(@Description("Server internal id or FQDN") @PathParam("serverId") String serverId);

    @Path("{serverId}")
    @PUT
    @Consumes({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
    })
    public Response updateServer(@Description("Server internal id or FQDN") @PathParam("serverId") String serverId,
            @Description("Server bean to save") ServerBean server);

    @Path("{serverId}/profiles")
    @PUT
    public Response sendServerProfile(
            @Description("Server internal id or FQDN") @PathParam("serverId") String serverId);

    @Path("{serverId}/service/")
    @GET
    public Response getServices(@Description("Server internal id or FQDN") @PathParam("serverId") String serverId);

    @Path("{serverId}/service/{serviceId}")
    @GET
    public Response getService(@Description("Server internal id or FQDN") @PathParam("serverId") String serverId,
            @Description("Service id") @PathParam("serviceId") String serviceId);

    @Path("{serverId}/service/{serviceId}/restart")
    @GET
    public Response restartService(
            @Description("Server internal id or FQDN") @PathParam("serverId") String serverId,
            @Description("Service id") @PathParam("serviceId") String serviceId);

    @Path("bundles")
    @GET
    public Response getBundles();

    @Path("bundles/{bundleId}")
    @GET
    public Response getBundle(@Description("Bundle internal id") @PathParam("bundleId") String bundleId);

    @Path("features")
    @GET
    public Response getFeatures();

    @Path("features/{featureId}")
    @DELETE
    public Response disableGlobalFeature(
            @Description("Global Feature id to disable") @PathParam("featureId") String featureId);

    @Path("features/{featureId}")
    @PUT
    @Consumes({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML, MediaType.TEXT_PLAIN
    })
    public Response enableGlobalFeature(
            @Description("Global Feature id to enable") @PathParam("featureId") String featureId);

    @Path("{serverId}/features/")
    @GET
    public Response getFeatures(@Description("Server internal id or FQDN") @PathParam("serverId") String serverId);

    @Path("{serverId}/features/{featureId}")
    @DELETE
    public Response disableFeature(
            @Description("Server internal id or FQDN") @PathParam("serverId") String serverId,
            @Description("Feature id") @PathParam("featureId") String featureId);

    @Path("{serverId}/features/{featureId}")
    @PUT
    @Consumes({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML, MediaType.TEXT_PLAIN
    })
    public Response enableFeature(@Description("Server internal id or FQDN") @PathParam("serverId") String serverId,
            @Description("Feature id") @PathParam("featureId") String featureId);

    @Path("jobs")
    @GET
    public Response getJobs(@Context HttpServletRequest request);

    @Path("jobs")
    @DELETE
    public Response clearJobs();

    @Path("jobs/failed")
    @GET
    public Response getFailedJobs(@Context HttpServletRequest request);

    @Path("jobs/failed")
    @DELETE
    public Response clearFailedJobs();

    @Path("jobs/completed")
    @GET
    public Response getCompletedJobs(@Context HttpServletRequest request);

    @Path("jobs/completed")
    @DELETE
    public Response clearCompletedJobs();

    @Path("{serverId}/jobs/")
    @GET
    public Response getJobs(@Description("Server internal id or FQDN") @PathParam("serverId") String serverId,
            @Context HttpServletRequest request);

    @Path("{serverId}/jobs/failed")
    @GET
    public Response getFailedJobs(@Description("Server internal id or FQDN") @PathParam("serverId") String serverId,
            @Context HttpServletRequest request);

    @Path("{serverId}/jobs/completed")
    @GET
    public Response getCompletedJobs(
            @Description("Server internal id or FQDN") @PathParam("serverId") String serverId,
            @Context HttpServletRequest request);

}
