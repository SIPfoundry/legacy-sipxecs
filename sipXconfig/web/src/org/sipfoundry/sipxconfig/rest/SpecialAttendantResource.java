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

import com.thoughtworks.xstream.XStream;
import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendantManager;

import static org.restlet.data.MediaType.APPLICATION_JSON;
import static org.restlet.data.MediaType.TEXT_XML;
import static org.sipfoundry.sipxconfig.permission.PermissionName.RECORD_SYSTEM_PROMPTS;

public class SpecialAttendantResource extends UserResource {

    private AutoAttendantManager m_autoAttendantManager;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_XML));
        getVariants().add(new Variant(APPLICATION_JSON));
        setModifiable(getUser().hasPermission(RECORD_SYSTEM_PROMPTS));
    }

    @Override
    public boolean allowGet() {
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
    public Representation represent(Variant variant) throws ResourceException {
        AutoAttendantSpecialModeRestInfo specialModeRest = new AutoAttendantSpecialModeRestInfo();
        specialModeRest.setSpecialMode(m_autoAttendantManager.getSpecialMode());
        return new SpecialAttendantRepresentation(variant.getMediaType(), specialModeRest);
    }

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        m_autoAttendantManager.setSpecialMode(true);
        getResponse().setStatus(Status.SUCCESS_NO_CONTENT);
    }

    @Override
    public void removeRepresentations() throws ResourceException {
        m_autoAttendantManager.setSpecialMode(false);
        getResponse().setStatus(Status.SUCCESS_NO_CONTENT);
    }

    public void setAutoAttendantManager(AutoAttendantManager autoAttendantManager) {
        m_autoAttendantManager = autoAttendantManager;
    }

    static class AutoAttendantSpecialModeRestInfo {
        private boolean m_specialMode;

        public void setSpecialMode(boolean specialMode) {
            m_specialMode = specialMode;
        }

        public boolean getSpecialMode() {
            return m_specialMode;
        }
    }

    static class SpecialAttendantRepresentation extends XStreamRepresentation<AutoAttendantSpecialModeRestInfo> {

        public SpecialAttendantRepresentation(MediaType mediaType, AutoAttendantSpecialModeRestInfo object) {
            super(mediaType, object);
        }

        public SpecialAttendantRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias("specialAttendant", AutoAttendantSpecialModeRestInfo.class);
        }
    }
}
