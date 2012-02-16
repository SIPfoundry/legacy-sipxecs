/**
 *
 *
 * Copyright (c) 2010 / 2012 eZuce, Inc. All rights reserved.
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

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import com.thoughtworks.xstream.XStream;

import org.apache.commons.lang.StringUtils;

import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.ActiveConference;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceContext;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceMember;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.springframework.beans.factory.annotation.Required;

import static org.restlet.data.MediaType.APPLICATION_JSON;
import static org.restlet.data.MediaType.TEXT_XML;

public class UserConferenceDetailsResource extends UserResource {
    private ActiveConferenceContext m_activeConferenceContext;
    private ConferenceBridgeContext m_conferenceBridgeContext;
    private String m_confName;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        m_confName = (String) getRequest().getAttributes().get("confName");
        getVariants().add(new Variant(TEXT_XML));
        getVariants().add(new Variant(APPLICATION_JSON));
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        Conference conference = m_conferenceBridgeContext.findConferenceByName(m_confName);
        ActiveConference activeConference = null;
        if (conference == null) {
            throw new ResourceException(Status.CLIENT_ERROR_NOT_FOUND, "Conference not found");
        } else if (!conference.hasOwner()
                || !StringUtils.equals(conference.getOwner().getName(), getUser().getName())) {
            throw new ResourceException(Status.CLIENT_ERROR_FORBIDDEN, "User is not owner of this conference");
        } else {
            activeConference = m_activeConferenceContext.getActiveConference(conference);
            if (activeConference == null) {
                throw new ResourceException(Status.CLIENT_ERROR_NOT_ACCEPTABLE, "Conference found, but not active");
            }
        }
        return new ConferenceDetails(variant.getMediaType(), convertConference(activeConference));
    }

    protected Representable convertConference(ActiveConference conference) {
        String extension = conference.getExtension();
        String description = conference.getDescription();
        boolean locked = conference.isLocked();
        List<ActiveConferenceMember> members = m_activeConferenceContext.
            getConferenceMembers(conference.getConference());
        ArrayList<Member> reprMembers = new ArrayList<Member>();
        User user = null;
        Member reprMember = null;
        for (ActiveConferenceMember member : members) {
            String number = member.getNumber();
            user = getCoreContext().loadUserByUserNameOrAlias(number);
            reprMember = new Member(member.getId(), number);
            reprMember.setImId(user != null ? user.getImId() : null);
            reprMember.setUuid(member.getUuid());
            reprMember.setCanHear(member.getCanHear());
            reprMember.setCanSpeak(member.getCanSpeak());
            reprMember.setVolumeIn(member.getVolumeIn());
            reprMember.setVolumeOut(member.getVolumeOut());
            reprMember.setEnergyLevel(member.getEnergyLevel());
            reprMembers.add(reprMember);
        }

        Representable representable = new Representable(extension, description, locked, reprMembers);
        return representable;
    }

    @Required
    public void setActiveConferenceContext(ActiveConferenceContext activeConferenceContext) {
        m_activeConferenceContext = activeConferenceContext;
    }

    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceBridgeContext) {
        m_conferenceBridgeContext = conferenceBridgeContext;
    }

    @SuppressWarnings("serial")
    static class Representable implements Serializable {
        @SuppressWarnings("unused")
        private String m_extension;
        @SuppressWarnings("unused")
        private String m_description;
        @SuppressWarnings("unused")
        private boolean m_locked;
        @SuppressWarnings("unused")
        private ArrayList<Member> m_members;
        public Representable(String extension, String description, boolean locked, ArrayList<Member> members) {
            m_extension = extension;
            m_description = description;
            m_locked = locked;
            m_members = members;
        }
    }

    static class Member {
        @SuppressWarnings("unused")
        private int m_id;
        @SuppressWarnings("unused")
        private String m_name;
        @SuppressWarnings("unused")
        private String m_imId;
        @SuppressWarnings("unused")
        private String m_uuid;
        @SuppressWarnings("unused")
        private int m_volumeIn;
        @SuppressWarnings("unused")
        private int m_volumeOut;
        @SuppressWarnings("unused")
        private int m_energyLevel;
        @SuppressWarnings("unused")
        private boolean m_canHear;
        @SuppressWarnings("unused")
        private boolean m_canSpeak;

        public Member(int id, String name) {
            m_id = id;
            m_name = name;
        }
        public void setImId(String imId) {
            m_imId = imId;
        }
        public void setUuid(String uuid) {
            m_uuid = uuid;
        }
        public void setVolumeIn(int volumeIn) {
            m_volumeIn = volumeIn;
        }
        public void setVolumeOut(int volumeOut) {
            m_volumeOut = volumeOut;
        }
        public void setEnergyLevel(int energyLevel) {
            m_energyLevel = energyLevel;
        }
        public void setCanHear(boolean canHear) {
            m_canHear = canHear;
        }
        public void setCanSpeak(boolean canSpeak) {
            m_canSpeak = canSpeak;
        }
    }

    static class ConferenceDetails extends XStreamRepresentation<Representable> {

        public ConferenceDetails(MediaType mediaType, Representable object) {
            super(mediaType, object);
        }

        public ConferenceDetails(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias("conference", Representable.class);
            xstream.alias("members", List.class);
            xstream.alias("member", Member.class);
        }
    }
}
