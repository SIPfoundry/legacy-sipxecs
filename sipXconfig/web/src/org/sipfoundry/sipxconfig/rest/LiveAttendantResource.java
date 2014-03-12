/**
 *
 *
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
package org.sipfoundry.sipxconfig.rest;

import static org.restlet.data.MediaType.APPLICATION_JSON;
import static org.restlet.data.MediaType.TEXT_XML;

import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.dialplan.AutoAttendantManager;
import org.springframework.beans.factory.annotation.Required;

public class LiveAttendantResource extends UserResource {
    private AutoAttendantManager m_autoAttendantManager;
    private String m_code;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_XML));
        getVariants().add(new Variant(APPLICATION_JSON));
        m_code = (String) request.getAttributes().get("code");
    }

    @Override
    public boolean allowPost() {
        return true;
    }

    @Override
    public boolean allowPut() {
        return true;
    }

    @Override
    public boolean allowDelete() {
        return true;
    }

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        manageLiveAttendant(true);
    }

    @Override
    public void removeRepresentations() throws ResourceException {
        manageLiveAttendant(false);
    }

    private void manageLiveAttendant(boolean enable) {
        boolean changed = m_autoAttendantManager.manageLiveAttendant(m_code, enable);
        if (!changed) {
            getResponse().setStatus(Status.CLIENT_ERROR_NOT_FOUND);
            return;
        }
        getResponse().setStatus(Status.SUCCESS_OK);
    }

    @Required
    public void setAutoAttendantManager(AutoAttendantManager autoAttendantManager) {
        m_autoAttendantManager = autoAttendantManager;
    }
}
