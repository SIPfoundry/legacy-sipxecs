/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.rest;

import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendantManager;
import org.springframework.beans.factory.annotation.Required;

import static org.restlet.data.MediaType.APPLICATION_JSON;
import static org.restlet.data.MediaType.TEXT_XML;
import static org.sipfoundry.sipxconfig.permission.PermissionName.RECORD_SYSTEM_PROMPTS;

public class SelectSpecialAttendantResource extends UserResource {
    private AutoAttendantManager m_autoAttendantManager;
    private String m_attendantId;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_XML));
        getVariants().add(new Variant(APPLICATION_JSON));
        setModifiable(getUser().hasPermission(RECORD_SYSTEM_PROMPTS));
        m_attendantId = (String) request.getAttributes().get("attendant");
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
        AutoAttendant aa = m_autoAttendantManager.getAutoAttendantBySystemName(m_attendantId);
        if (aa == null) {
            getResponse().setStatus(Status.CLIENT_ERROR_BAD_REQUEST);
            return;
        }

        m_autoAttendantManager.selectSpecial(aa);
        getResponse().setStatus(Status.SUCCESS_NO_CONTENT);
    }

    @Override
    public void removeRepresentations() throws ResourceException {
        AutoAttendant aa = m_autoAttendantManager.getAutoAttendantBySystemName(m_attendantId);
        if (aa == null) {
            getResponse().setStatus(Status.CLIENT_ERROR_BAD_REQUEST);
            return;
        }

        if (m_autoAttendantManager.getSpecialMode()) {
            getResponse().setStatus(Status.CLIENT_ERROR_CONFLICT);
            return;
        }

        m_autoAttendantManager.deselectSpecial(aa);
        getResponse().setStatus(Status.SUCCESS_NO_CONTENT);
    }

    @Required
    public void setAutoAttendantManager(AutoAttendantManager autoAttendantManager) {
        m_autoAttendantManager = autoAttendantManager;
    }
}
