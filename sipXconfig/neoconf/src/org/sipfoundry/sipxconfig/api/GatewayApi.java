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
import org.sipfoundry.sipxconfig.api.model.GatewayBean;

@Path("/gateways/")
@Produces({
    MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
})
@Description("Gateway Management REST API")
public interface GatewayApi {

    @GET
    public Response getGateways();

    @Path("models")
    @GET
    public Response getGatewayModels();

    @Path("{gatewayId}/availablerules")
    @GET
    public Response getAvailableRules(@Description("Gateway id")
        @PathParam("gatewayId") String gatewayId);

    @Path("{gatewayId}")
    @GET
    public Response getGateway(@Description("Gateway id")
        @PathParam("gatewayId") String gatewayId);

    @POST
    @Consumes({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
    })
    public Response newGateway(@Description("Gateway bean to save") GatewayBean gatewayBean);

    @Path("{gatewayId}")
    @DELETE
    public Response deleteGateway(@Description("Gateway Id") @PathParam("gatewayId") String gatewayId);

    @Path("{gatewayId}")
    @PUT
    @Consumes({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
    })
    public Response updateGateway(
            @Description("Gateway id") @PathParam("gatewayId") String gatewayId,
            @Description("Gateway bean to save") GatewayBean gatewayBean);

    @Path("{gatewayId}/model/{modelName}/settings")
    @GET
    public Response getGatewayModelSettings(
            @Description("Gateway id/serial number")
            @PathParam("gatewayId") String gatewayId, @PathParam("modelName") String modelName,
            @Context HttpServletRequest request);

    @Path("{gatewayId}/model/{modelName}/settings/{path:.*}")
    @GET
    public Response getGatewaySetting(
            @Description("Gateway id or serial")
            @PathParam("gatewayId") String gatewayId,
            @PathParam("modelName") String modelName,
            @Description("Path to Gateway setting") @PathParam("path") String path,
            @Context HttpServletRequest request);

    @Path("{gatewayId}/model/{modelName}/settings/{path:.*}")
    @PUT
    @Consumes({
        MediaType.TEXT_PLAIN
    })
    public Response setGatewaySetting(
            @Description("Gateway internal id or name")
            @PathParam("gatewayId") String gatewayId,
            @PathParam("modelName") String modelName,
            @Description("Path to Gateway setting") @PathParam("path") String path, String value);

    @Path("{groupId}/model/{modelName}/settings/{path:.*}")
    @DELETE
    public Response deleteGatewaySetting(
            @Description("Gateway internal id or name")
            @PathParam("gatewayId") String gatewayId,
            @PathParam("modelName") String modelName,
            @Description("Path to Phone Group setting") @PathParam("path") String path);

    @Path("{gatewayId}/ports")
    @GET
    public Response getPorts(@Description("Gateway id")
        @PathParam("gatewayId") String gatewayId);

    @Path("{gatewayId}/ports/{portId}/settings")
    @GET
    public Response getGatewayPortSettings(
            @Description("Gateway id/serial number")
            @PathParam("gatewayId") String gatewayId, @PathParam("portId") Integer portId,
            @Context HttpServletRequest request);

    @Path("{gatewayId}/ports/{portId}/settings/{path:.*}")
    @GET
    public Response getGatewayPortSetting(
            @Description("Gateway id or serial")
            @PathParam("gatewayId") String gatewayId,
            @PathParam("portId") Integer portId,
            @Description("Path to Gateway Port setting") @PathParam("path") String path,
            @Context HttpServletRequest request);

    @Path("{gatewayId}/ports/{portId}/settings/{path:.*}")
    @PUT
    @Consumes({
        MediaType.TEXT_PLAIN
    })
    public Response setGatewayPortSetting(
            @Description("Gateway internal id or name")
            @PathParam("gatewayId") String gatewayId,
            @PathParam("portId") Integer portId,
            @Description("Path to Gateway Port setting") @PathParam("path") String path, String value);

    @Path("{gatewayId}/ports/{portId}/settings/{path:.*}")
    @DELETE
    public Response deleteGatewayPortSetting(
            @Description("Gateway internal id or name")
            @PathParam("gatewayId") String gatewayId,
            @PathParam("portId") Integer portId,
            @Description("Path to Phone Group setting") @PathParam("path") String path);

}
