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
import org.sipfoundry.sipxconfig.api.model.PhoneBean.LineBean;

@Path("/phones/{phoneId}/lines")
@Produces({
    MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
})
@Description("Phone Line Management REST API")
public interface PhoneLineApi {

    @GET
    public Response getPhoneLines(
            @Description("Phone internal id or MAC address") @PathParam("phoneId") String phoneId);

    @DELETE
    public Response deletePhoneLines(
            @Description("Phone internal id or MAC address") @PathParam("phoneId") String phoneId);

    @POST
    @Consumes({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
    })
    public Response newLine(@Description("Phone internal id or MAC address") @PathParam("phoneId") String phoneId,
            @Description("Line bean to save") LineBean line);

    @Path("{lineId}")
    @GET
    public Response getPhoneLine(
            @Description("Phone internal id or MAC address") @PathParam("phoneId") String phoneId,
            @Description("Line internal id") @PathParam("lineId") Integer lineId);

    @Path("{lineId}")
    @DELETE
    public Response deletePhoneLine(
            @Description("Phone internal id or MAC address") @PathParam("phoneId") String phoneId,
            @Description("Line internal id") @PathParam("lineId") Integer lineId);

    @Path("{lineId}/settings")
    @GET
    public Response getPhoneLineSettings(
            @Description("Phone internal id or MAC address") @PathParam("phoneId") String phoneId,
            @Description("Line internal id") @PathParam("lineId") Integer lineId,
            @Context HttpServletRequest request);

    @Path("{lineId}/settings/{path:.*}")
    @GET
    public Response getPhoneLineSetting(
            @Description("Phone internal id or MAC address") @PathParam("phoneId") String phoneId,
            @Description("Line internal id") @PathParam("lineId") Integer lineId,
            @Description("Setting Path") @PathParam("path") String path,
            @Context HttpServletRequest request);

    @Path("{lineId}/settings/{path:.*}")
    @PUT
    @Consumes({
        MediaType.TEXT_PLAIN
    })
    public Response setPhoneLineSetting(
            @Description("Phone internal id or MAC address") @PathParam("phoneId") String phoneId,
            @Description("Line internal id") @PathParam("lineId") Integer lineId,
            @Description("Setting Path") @PathParam("path") String path, String value);

    @Path("{lineId}/settings/{path:.*}")
    @DELETE
    public Response deletePhoneLineSetting(
            @Description("Phone internal id or MAC address") @PathParam("phoneId") String phoneId,
            @Description("Line internal id") @PathParam("lineId") Integer lineId,
            @Description("Setting Path") @PathParam("path") String path);

}
