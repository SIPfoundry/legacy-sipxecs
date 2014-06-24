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
import javax.ws.rs.PUT;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;

import org.apache.cxf.jaxrs.model.wadl.Description;

@Description("Service Settings Management REST API")
public interface ServiceSettingsApi {

    @Path("settings")
    @GET
    public Response getServiceSettings(@Context HttpServletRequest request);

    @Path("settings/{path:.*}")
    @GET
    public Response getServiceSetting(@Description("Path to Service setting") @PathParam("path") String path,
            @Context HttpServletRequest request);

    @Path("settings/{path:.*}")
    @PUT
    @Consumes({
        MediaType.TEXT_PLAIN
    })
    public Response setServiceSetting(@Description("Path to Service setting") @PathParam("path") String path,
            String value);

    @Path("settings/{path:.*}")
    @DELETE
    public Response deleteServiceSetting(@Description("Path to Service setting") @PathParam("path") String path);

}
