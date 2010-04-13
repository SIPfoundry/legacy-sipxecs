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

import java.util.ArrayList;
import java.util.List;

import com.thoughtworks.xstream.XStream;
import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendantManager;
import org.springframework.beans.factory.annotation.Required;

import static org.restlet.data.MediaType.APPLICATION_JSON;
import static org.restlet.data.MediaType.TEXT_XML;

public class AutoAttendantsResource extends UserResource {

    private AutoAttendantManager m_autoAttendantManager;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_XML));
        getVariants().add(new Variant(APPLICATION_JSON));
    }

    @Override
    public boolean allowGet() {
        return true;
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        List<AutoAttendant> autoAttendants = m_autoAttendantManager.getAutoAttendants();
        AutoAttendant selectedAttendant = m_autoAttendantManager.getSelectedSpecialAttendant();
        boolean specialMode = m_autoAttendantManager.getSpecialMode();

        List<AutoAttendantRestInfo> data = new ArrayList<AutoAttendantRestInfo>(autoAttendants.size());
        for (AutoAttendant autoAttendant : autoAttendants) {
            boolean selected = specialMode && autoAttendant.equals(selectedAttendant);
            AutoAttendantRestInfo info = new AutoAttendantRestInfo(autoAttendant, selected);
            data.add(info);
        }

        return new AutoAttendantsRepresentation(variant.getMediaType(), data);
    }

    @Required
    public void setAutoAttendantManager(AutoAttendantManager autoAttendantManager) {
        m_autoAttendantManager = autoAttendantManager;
    }

    static class AutoAttendantsRepresentation extends XStreamRepresentation<List<AutoAttendantRestInfo>> {

        public AutoAttendantsRepresentation(MediaType mediaType, List<AutoAttendantRestInfo> object) {
            super(mediaType, object);
        }

        public AutoAttendantsRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias("autoAttendants", List.class);
            xstream.alias("autoAttendant", AutoAttendantRestInfo.class);
        }
    }

    static class AutoAttendantRestInfo {
        private final String m_name;
        private final String m_systemId;
        private final boolean m_specialSelected;

        public AutoAttendantRestInfo(AutoAttendant aa, boolean specialSelected) {
            m_name = aa.getName();
            m_systemId = aa.getSystemId();
            m_specialSelected = specialSelected;
        }

        public String getName() {
            return m_name;
        }

        public String getSystemId() {
            return m_systemId;
        }

        public boolean getSpecialSelected() {
            return m_specialSelected;
        }
    }
}
