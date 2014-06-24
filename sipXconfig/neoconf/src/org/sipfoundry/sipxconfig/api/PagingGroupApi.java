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
import javax.ws.rs.POST;
import javax.ws.rs.PUT;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;

import org.apache.cxf.jaxrs.model.wadl.Description;
import org.sipfoundry.sipxconfig.api.model.PageGroupBean;

@Path("/pagegroups/")
@Produces({
    MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
})
@Description("Page Group Management REST API")
public interface PagingGroupApi extends PromptsApi {

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

}
