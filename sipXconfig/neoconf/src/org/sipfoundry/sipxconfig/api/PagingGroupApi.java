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

import java.util.List;

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

import org.apache.cxf.jaxrs.ext.multipart.Attachment;
import org.apache.cxf.jaxrs.model.wadl.Description;
import org.sipfoundry.sipxconfig.api.model.PageGroupBean;

@Path("/pagegroups/")
@Produces({
    MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
})
@Description("Page Group Management REST API")
public interface PagingGroupApi {

    @GET
    public Response getPageGroups();

    @POST
    public Response newPageGroup(@Description("Page Group bean to save") PageGroupBean bean);

    @Path("{groupId}")
    @GET
    public Response getPageGroup(@Description("Page Group internal id") @PathParam("groupId") Integer groupId);

    @Path("{groupId}")
    @DELETE
    public Response deletePageGroup(@Description("Page Group internal id") @PathParam("groupId") Integer groupId);

    @Path("{groupId}")
    @PUT
    public Response updatePageGroup(@Description("Page Group internal id") @PathParam("groupId") Integer groupId,
            @Description("Page Group bean to save") PageGroupBean bean);

    @Path("settings")
    @GET
    public Response getSettings(@Context HttpServletRequest request);

    @Path("settings/{path:.*}")
    @GET
    public Response getSetting(@Description("Path to Orbit setting") @PathParam("path") String path,
            @Context HttpServletRequest request);

    @Path("settings/{path:.*}")
    @PUT
    @Consumes({
        MediaType.TEXT_PLAIN
    })
    public Response setSetting(@Description("Path to Orbit setting") @PathParam("path") String path, String value);

    @Path("settings/{path:.*}")
    @DELETE
    public Response deleteSetting(@Description("Path to Orbit setting") @PathParam("path") String path);

    @Path("prompts")
    @GET
    public Response getPrompts();

    @Path("prompts")
    @POST
    @Consumes(MediaType.MULTIPART_FORM_DATA)
    public Response uploadPrompts(List<Attachment> attachments, @Context HttpServletRequest request);

    @Path("prompts/{promptName}")
    @GET
    @Produces({
        "audio/x-wav", "audio/mpeg"
    })
    public Response downloadPrompt(@Description("Prompt name") @PathParam("promptName") String promptName);

    @Path("prompts/{promptName}")
    @DELETE
    public Response removePrompt(@Description("Prompt name") @PathParam("promptName") String promptName);

    @Path("prompts/{promptName}/stream")
    @GET
    @Produces({
        "audio/x-wav", "audio/mpeg"
    })
    public Response streamPrompt(@Description("Prompt name") @PathParam("promptName") String promptName);

}
