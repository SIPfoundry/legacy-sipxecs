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
import javax.ws.rs.Produces;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;

import org.apache.cxf.jaxrs.model.wadl.Description;

@Path("/ftp/")
@Produces({
    MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
})
@Description("FTP Management REST API")
public interface FtpApi {

    @Path("settings")
    @GET
    public Response getFtpSettings(@Context HttpServletRequest request);

    @Path("settings/{path:.*}")
    @GET
    public Response getFtpSetting(@Description("Path to Phone setting") @PathParam("path") String path,
            @Context HttpServletRequest request);

    @Path("settings/{path:.*}")
    @PUT
    @Consumes({
        MediaType.TEXT_PLAIN
    })
    public Response setFtpSetting(@Description("Path to Phone setting") @PathParam("path") String path, String value);

    @Path("settings/{path:.*}")
    @DELETE
    public Response deleteFtpSetting(@Description("Path to Phone setting") @PathParam("path") String path);

}
