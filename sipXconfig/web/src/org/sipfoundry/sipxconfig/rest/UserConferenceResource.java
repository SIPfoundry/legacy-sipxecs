/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.rest;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import com.thoughtworks.xstream.XStream;

import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.springframework.beans.factory.annotation.Required;

import static org.restlet.data.MediaType.APPLICATION_JSON;
import static org.restlet.data.MediaType.TEXT_XML;

public class UserConferenceResource extends UserResource {

    private ConferenceBridgeContext m_conferenceBridgeContext;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_XML));
        getVariants().add(new Variant(APPLICATION_JSON));
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        List<Conference> conferences = m_conferenceBridgeContext.findConferencesByOwner(getUser());
        return new ConferenceRepresentation(variant.getMediaType(), convertConferences(conferences));
    }

    protected ArrayList<Representable> convertConferences(List<Conference> conferences) {
        ArrayList<Representable> conferencesArray = new ArrayList<Representable>();
        for (Conference conference : conferences) {
            conferencesArray.add(new Representable(conference));
        }
        return conferencesArray;
    }

    @SuppressWarnings("serial")
    static class Representable implements Serializable {
        @SuppressWarnings("unused")
        private boolean m_enabled;
        @SuppressWarnings("unused")
        private String m_name;
        @SuppressWarnings("unused")
        private String m_description;
        @SuppressWarnings("unused")
        private String m_extension;

        public Representable(Conference conference) {
            m_enabled = conference.isEnabled();
            m_name = conference.getName();
            m_description = conference.getDescription();
            m_extension = conference.getExtension();
        }
    }

    static class ConferenceRepresentation extends XStreamRepresentation<Collection<Representable>> {
        private static final String ID = "m_id";
        private static final String ENABLED = "enabled";
        private static final String NAME = "name";
        private static final String DESCRIPTION = "description";
        private static final String EXTENSION = "extension";

        public ConferenceRepresentation(MediaType mediaType, Collection<Representable> object) {
            super(mediaType, object);
        }

        public ConferenceRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.omitField(BeanWithId.class, ID);
            xstream.alias("conferences", List.class);
            xstream.alias("conference", Representable.class);
            xstream.aliasField(ENABLED, Representable.class, ENABLED);
            xstream.aliasField(NAME, Representable.class, NAME);
            xstream.aliasField(DESCRIPTION, Representable.class, DESCRIPTION);
            xstream.aliasField(EXTENSION, Representable.class, EXTENSION);
            xstream.omitField(Representable.class, ID);
        }
    }

    @Required
    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceBridgeContext) {
        m_conferenceBridgeContext = conferenceBridgeContext;
    }
}
