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

import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;
import javax.ws.rs.ext.ExceptionMapper;

import org.springframework.dao.DataRetrievalFailureException;

public class SipxecsExceptionMapper implements ExceptionMapper<Throwable> {

    @Override
    public Response toResponse(Throwable exception) {
        if (exception instanceof DataRetrievalFailureException) {
            return Response.status(Status.NOT_FOUND).entity(exception.toString()).build();
        }
        return Response.status(Status.INTERNAL_SERVER_ERROR).entity(exception.toString()).build();
    }

}
