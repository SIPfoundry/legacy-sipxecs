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
import org.sipfoundry.sipxconfig.api.model.DialingRuleBean;

@Path("/rules/")
@Produces({
    MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
})
@Description("Dial Plan REST API")
public interface DialPlanApi {

    @Path("raw")
    @GET
    public Response getRawRules();

    @GET
    public Response getRules();

    @Path("{ruleId}")
    @GET
    public Response getRule(@Description("Rule Id") @PathParam("ruleId") Integer ruleId);

    @POST
    @Consumes({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
    })
    public Response newRule(@Description("DialPlan bean to save") DialingRuleBean ruleBean);

    @Path("{ruleId}")
    @DELETE
    public Response deleteRule(@Description("Rule Id") @PathParam("ruleId") Integer ruleId);

    @Path("{ruleId}")
    @PUT
    @Consumes({
        MediaType.APPLICATION_JSON, MediaType.TEXT_XML, MediaType.APPLICATION_XML
    })
    public Response updateRule(
            @Description("Rule id") @PathParam("ruleId") Integer ruleId,
            @Description("Schedule bean to save") DialingRuleBean ruleBean);
}
