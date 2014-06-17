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

import javax.ws.rs.DELETE;
import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;

import org.apache.cxf.jaxrs.model.wadl.Description;

@Path("/registrations/")
@Produces({
    MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
})
@Description("SIP Registrations Management REST API")
public interface RegistrationApi {

    @GET
    public Response getRegistrations(@Description("First registration row") @QueryParam("start") Integer start,
            @Description("Number of registrations to be returned") @QueryParam("limit") Integer limit);

    @Path("user/{userid}")
    @GET
    public Response getRegistrationsByUser(
            @Description("User internal id or user name / alias") @PathParam("userid") String user,
            @Description("First registration row") @QueryParam("start") Integer start,
            @Description("Number of registrations to be returned") @QueryParam("limit") Integer limit);

    @Path("user/{userid}")
    @DELETE
    public Response dropRegistrationsByUser(
            @Description("User internal id or user name / alias") @PathParam("userid") String user);

    @Path("serialNo/{serialId}")
    @GET
    public Response getRegistrationsByMac(
            @Description("Phone MAC address") @PathParam("serialId") String serialId);

    @Path("serialNo/{serialId}")
    @DELETE
    public Response dropRegistrationsByMac(
            @Description("Phone MAC address") @PathParam("serialId") String serialId);

    @Path("ip/{ip}")
    @GET
    public Response getRegistrationsByIp(
            @Description("Phone IP address") @PathParam("ip") String ip);

    @Path("ip/{ip}")
    @DELETE
    public Response dropRegistrationsByIp(
            @Description("Phone IP address") @PathParam("ip") String ip);

    @Path("callid/{callid}")
    @GET
    public Response getRegistrationsByCallId(
            @Description("Registration call id") @PathParam("callid") String callid);

    @Path("callid/{callid}")
    @DELETE
    public Response dropRegistrationsByCallId(
            @Description("Registration call id") @PathParam("callid") String callid);

    @Path("server/{serverId}")
    @GET
    public Response getRegistrationsByServer(
            @Description("Server internal id or fqdn") @PathParam("serverId") String serverId,
            @Description("First registration row") @QueryParam("start") Integer start,
            @Description("Number of registrations to be returned") @QueryParam("limit") Integer limit);

    @Path("server/{serverId}")
    @DELETE
    public Response dropRegistrationsByServer(
            @Description("Server internal id or fqdn") @PathParam("serverId") String serverId);

}
