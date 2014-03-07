/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

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

import com.thoughtworks.xstream.XStream;

public class UserConferenceResource extends UserResource {
    private ConferenceBridgeContext m_conferenceBridgeContext;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_XML));
        getVariants().add(new Variant(APPLICATION_JSON));
    }

    // GET
    @Override
    public Representation represent(Variant variant) throws ResourceException {
        Representation r = null;
        String name = getNameFromRequest();

        if (name == null) {
            List<Conference> conferences = m_conferenceBridgeContext.findConferencesByOwner(getUser());
            r = new ConferencesRepresentation(variant.getMediaType(), convertConferences(conferences));
        } else {
            Conference conf = m_conferenceBridgeContext.findConferenceByName(name);
            r = new ConferenceRepresentation(variant.getMediaType(), new RepresentableFull(conf));
        }

        return r;
    }

    // PUT
    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        String name = getNameFromRequest();

        if (name != null) {
            RepresentableFull repr = new ConferenceRepresentation(entity).getObject();
            Conference conf = m_conferenceBridgeContext.findConferenceByName(name);

            if (repr.m_enabled != null) {
                conf.setEnabled(repr.m_enabled);
            }
            if (repr.m_name != null) {
                conf.setName(repr.m_name);
            }
            if (repr.m_description != null) {
                conf.setDescription(repr.m_description);
            }
            if (repr.m_autorecorded != null) {
                conf.setAutorecorded(repr.m_autorecorded);
            }
            if (repr.m_accessCode != null) {
                conf.setParticipantAccessCode(repr.m_accessCode);
            }
            if (repr.m_moderatorCode != null) {
                conf.setModeratorAccessCode(repr.m_moderatorCode);
            }
            if (repr.m_quickstart != null) {
                conf.setQuickstart(repr.m_quickstart);
            }
            if (repr.m_video != null) {
                conf.setVideoConference(repr.m_video);
            }
            if (repr.m_sendActiveVideoOnly != null) {
                conf.setVideoToggleFloor(repr.m_sendActiveVideoOnly);
            }
            if (repr.m_maxMembers != null) {
                conf.setConfMaxMembers(repr.m_maxMembers);
            }
            if (repr.m_moh != null) {
                conf.setMohSource(repr.m_moh);
            }
            if (repr.m_publicRoom != null) {
                conf.setPublicRoom(repr.m_publicRoom);
            }
            if (repr.m_moderatedRoom != null) {
                conf.setModeratedRoom(repr.m_moderatedRoom);
            }

            m_conferenceBridgeContext.saveConference(conf);
        }
    }

    private static List<Representable> convertConferences(List<Conference> conferences) {
        List<Representable> conferencesArray = new ArrayList<Representable>();
        for (Conference conference : conferences) {
            conferencesArray.add(new Representable(conference));
        }
        return conferencesArray;
    }

    private String getNameFromRequest() {
        return (String) getRequest().getAttributes().get("name");
    }

    @Required
    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceBridgeContext) {
        m_conferenceBridgeContext = conferenceBridgeContext;
    }

    @SuppressWarnings("serial")
    private static class RepresentableFull implements Serializable {
        private final Boolean m_enabled;
        private final String m_name;
        private final String m_description;
        private final Boolean m_autorecorded;
        private final String m_accessCode;
        private final String m_moderatorCode;
        private final Boolean m_quickstart;
        private final Boolean m_video;
        private final Boolean m_sendActiveVideoOnly;
        private final Integer m_maxMembers;
        private final String m_moh;
        private final Boolean m_moderatedRoom;
        private final Boolean m_publicRoom;

        public RepresentableFull(Conference conference) {
            m_enabled = conference.isEnabled();
            m_name = conference.getName();
            m_description = conference.getDescription();
            m_autorecorded = conference.isAutorecorded();
            m_accessCode = conference.getParticipantAccessCode();
            m_moderatorCode = conference.getModeratorAccessCode();
            m_quickstart = conference.isQuickstart();
            m_video = conference.isVideoConference();
            m_sendActiveVideoOnly = conference.isVideoToggleFloor();
            m_maxMembers = conference.getConfMaxMembers();
            m_moh = conference.getMohSource();
            m_moderatedRoom = conference.isModeratedRoom();
            m_publicRoom = conference.isPublicRoom();
        }
    }

    private static class ConferenceRepresentation extends XStreamRepresentation<RepresentableFull> {

        public ConferenceRepresentation(MediaType mediaType, RepresentableFull representable) {
            super(mediaType, representable);
        }

        public ConferenceRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias("setting", RepresentableFull.class);
        }
    }

    @SuppressWarnings("serial")
    private static class Representable implements Serializable {
        @SuppressWarnings("unused")
        private final boolean m_enabled;
        @SuppressWarnings("unused")
        private final String m_name;
        @SuppressWarnings("unused")
        private final String m_description;
        @SuppressWarnings("unused")
        private final String m_extension;
        @SuppressWarnings("unused")
        private final String m_accessCode;

        public Representable(Conference conference) {
            m_enabled = conference.isEnabled();
            m_name = conference.getName();
            m_description = conference.getDescription();
            m_extension = conference.getExtension();
            m_accessCode = conference.getParticipantAccessCode();
        }
    }

    private static class ConferencesRepresentation extends XStreamRepresentation<Collection<Representable>> {
        private static final String ID = "m_id";
        private static final String ENABLED = "enabled";
        private static final String NAME = "name";
        private static final String DESCRIPTION = "description";
        private static final String EXTENSION = "extension";

        public ConferencesRepresentation(MediaType mediaType, Collection<Representable> object) {
            super(mediaType, object);
        }

        public ConferencesRepresentation(Representation representation) {
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
            xstream.aliasField("accessCode", Representable.class, "participantAccessCode");
            xstream.omitField(Representable.class, ID);
        }
    }
}
