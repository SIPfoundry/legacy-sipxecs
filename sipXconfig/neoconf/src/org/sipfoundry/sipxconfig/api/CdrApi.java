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
import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;

import org.apache.cxf.jaxrs.model.wadl.Description;

@Path("/cdrs/")
@Description("CDR Management REST API")
public interface CdrApi extends ServiceSettingsApi {

    @Path("active")
    @GET
    @Produces({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
    })
    public Response getActiveCdrs(@Context HttpServletRequest request);

    @Path("history")
    @GET
    @Produces({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
    })
    public Response getCdrHistory(
            @Description("From Date, format yyyyMMddHHmm. If not specified defaults to yesterday")
            @QueryParam("fromDate") String fromDate,
            @Description("To Date, format yyyyMMddHHmm. If not specified defaults to now")
            @QueryParam("toDate") String toDate,
            @Description("Calls From") @QueryParam("from") String from,
            @Description("Calls to") @QueryParam("to") String to,
            @Description("Limit row") @QueryParam("limit") Integer limit,
            @Description("Offset Cdr row") @QueryParam("offset") Integer offset,
            @Description("Order by criteria. Could have values of caller, callee, startTime, duration, termination")
            @QueryParam("orderBy") String orderBy,
            @Context HttpServletRequest request);

    @Path("user/{userId}/active")
    @GET
    @Produces({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
    })
    public Response getUserActiveCdrs(
            @Description("User internal id or user name / alias") @PathParam("userId") String user,
            @Context HttpServletRequest request);

    @Path("user/{userId}/history")
    @GET
    @Produces({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
    })
    public Response getUserCdrHistory(
            @Description("User internal id or user name / alias") @PathParam("userId") String userId,
            @Description("From Date, format yyyyMMddHHmm. If nto specified defaults to yesterday")
            @QueryParam("fromDate") String fromDate,
            @Description("To Date, format yyyyMMddHHmm. If not specified defaults to now")
            @QueryParam("toDate") String toDate,
            @Description("Calls From") @QueryParam("from") String from,
            @Description("Calls to") @QueryParam("to") String to,
            @Description("Limit row") @QueryParam("limit") Integer limit,
            @Description("Offset Cdr row") @QueryParam("offset") Integer offset,
            @Description("Order by criteria. Could have values of caller, callee, startTime, duration, termination")
            @QueryParam("orderBy") String orderBy,
            @Context HttpServletRequest request);

    @Path("reports")
    @GET
    @Produces({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
    })
    public Response getCdrReports(@Context HttpServletRequest request);

    @Path("reports/cdr-table")
    @GET
    @Produces({
        "application/pdf"
    })
    public Response getCdrTableReports(
            @Description("From Date, format yyyyMMddHHmm. If nto specified defaults to yesterday")
            @QueryParam("fromDate") String fromDate,
            @Description("To Date, format yyyyMMddHHmm. If not specified defaults to now")
            @QueryParam("toDate") String toDate,
            @Description("Calls From") @QueryParam("from") String from,
            @Description("Calls to") @QueryParam("to") String to,
            @Description("Limit row") @QueryParam("limit") Integer limit,
            @Description("Offset Cdr row") @QueryParam("offset") Integer offset,
            @Description("Order by criteria. Could have values of caller, callee, startTime, duration, termination")
            @QueryParam("orderBy") String orderBy,
            @Context HttpServletRequest request);

    @Path("user/{userId}/reports/cdr-table")
    @GET
    @Produces({
        "application/pdf"
    })
    public Response getUserCdrTableReports(
            @Description("User internal id or user name / alias") @PathParam("userId") String userId,
            @Description("From Date, format yyyyMMddHHmm. If nto specified defaults to yesterday")
            @QueryParam("fromDate") String fromDate,
            @Description("To Date, format yyyyMMddHHmm. If not specified defaults to now")
            @QueryParam("toDate") String toDate,
            @Description("Calls From") @QueryParam("from") String from,
            @Description("Calls to") @QueryParam("to") String to,
            @Description("Limit row") @QueryParam("limit") Integer limit,
            @Description("Offset Cdr row") @QueryParam("offset") Integer offset,
            @Description("Order by criteria. Could have values of caller, callee, startTime, duration, termination")
            @QueryParam("orderBy") String orderBy,
            @Context HttpServletRequest request);

}
