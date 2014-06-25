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
import org.sipfoundry.sipxconfig.api.model.GroupBean;

@Path("/phoneGroups/")
@Produces({
    MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
})
@Description("Phone Groups Management REST API")
public interface PhoneGroupApi {
    @GET
    public Response getPhoneGroups();

    @POST
    @Consumes({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
    })
    public Response newPhoneGroup(@Description("Phone Group bean to save") GroupBean phoneGroup);

    @Path("{phoneGroupId}")
    @GET
    public Response getPhoneGroup(@Description("Phone group id or name")
        @PathParam("phoneGroupId") String phoneGroupId);

    @Path("{groupId}")
    @DELETE
    public Response deletePhoneGroup(@Description("Group internal id or name") @PathParam("groupId") String groupId);

    @Path("{groupId}")
    @PUT
    @Consumes({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
    })
    public Response updatePhoneGroup(
            @Description("Phone group internal id or name") @PathParam("groupId") String groupId,
            @Description("Phone group bean to save") GroupBean groupBean);

    @Path("{groupId}/up")
    @PUT
    public Response movePhoneGroupUp(
            @Description("Phone group internal id or name") @PathParam("groupId") String groupId);

    @Path("{groupId}/down")
    @PUT
    public Response movePhoneGroupDown(
            @Description("Phone group internal id or name") @PathParam("groupId") String groupId);

    @Path("{groupId}/models")
    @GET
    public Response getPhoneGroupModels(
            @Description("Phone models given group")
            @PathParam("groupId") String groupId);

    @Path("{groupId}/model/{modelName}/settings")
    @GET
    public Response getPhoneGroupModelSettings(
            @Description("Phone group internal id or MAC address")
            @PathParam("groupId") String groupId, @PathParam("modelName") String modelName,
            @Context HttpServletRequest request);

    @Path("{groupId}/model/{modelName}/settings/{path:.*}")
    @GET
    public Response getPhoneGroupSetting(
            @Description("Phone internal id or name")
            @PathParam("groupId") String groupId,
            @PathParam("modelName") String modelName,
            @Description("Path to Phone setting") @PathParam("path") String path, @Context HttpServletRequest request);

    @Path("{groupId}/model/{modelName}/settings/{path:.*}")
    @PUT
    @Consumes({
        MediaType.TEXT_PLAIN
    })
    public Response setPhoneGroupSetting(
            @Description("Phone group internal id or name")
            @PathParam("groupId") String groupId,
            @PathParam("modelName") String modelName,
            @Description("Path to Phone group setting") @PathParam("path") String path, String value);

    @Path("{groupId}/model/{modelName}/settings/{path:.*}")
    @DELETE
    public Response deletePhoneGroupSetting(
            @Description("Phone group internal id or name")
            @PathParam("groupId") String groupId,
            @PathParam("modelName") String modelName,
            @Description("Path to Phone Group setting") @PathParam("path") String path);
}
