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

import javax.ws.rs.Consumes;
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
import org.sipfoundry.sipxconfig.api.model.ScheduleBean;
import org.sipfoundry.sipxconfig.api.model.WorkingHoursBean;

@Path("/schedules/")
@Produces({
    MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
})
@Description("Schedules REST API")

public interface ScheduleApi {

    @Path("general")
    @GET
    public Response getAllGeneralSchedules();

    @Path("group")
    @GET
    public Response getAllUserGroupSchedules();

    @Path("group/{groupId}")
    @GET
    public Response getUserGroupSchedules(
            @Description("User Group id")
            @PathParam("userGroupId") Integer userGroupId);

    @Path("user/{userId}/all")
    @GET
    public Response getAllUserSchedules(
            @Description("User id")
            @PathParam("userId") Integer userId);

    @Path("user/{userId}/personal")
    @GET
    public Response getPersonalUserSchedules(
            @Description("User id")
            @PathParam("userId") Integer userId);

    @POST
    @Consumes({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
    })
    public Response newSchedule(@Description("Schedule bean to save") ScheduleBean scheduleBean);

    @Path("{scheduleId}")
    @GET
    public Response getSchedule(@Description("Schedule Id") @PathParam("scheduleId") Integer scheduleId);

    @Path("{scheduleId}")
    @DELETE
    public Response deleteSchedule(@Description("Schedule Id") @PathParam("scheduleId") Integer scheduleId);

    @Path("{scheduleId}")
    @PUT
    @Consumes({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
    })
    public Response updateSchedule(
            @Description("Schedule id") @PathParam("scheduleId") Integer scheduleId,
            @Description("Schedule bean to save") ScheduleBean scheduleBean);

    @Path("{scheduleId}/period")
    @POST
    @Consumes({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
    })
    public Response addPeriod(@Description("Schedule id") @PathParam("scheduleId") Integer scheduleId,
        @Description("Period to add") WorkingHoursBean whBean);

    @Path("{scheduleId}/period/{index}")
    @DELETE
    @Consumes({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
    })
    public Response deletePeriod(@Description("Schedule id") @PathParam("scheduleId") Integer scheduleId,
        @Description("period index as it is saved in DB") @PathParam("index") Integer index);
}
