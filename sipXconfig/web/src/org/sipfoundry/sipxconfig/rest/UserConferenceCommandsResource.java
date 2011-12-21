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

import org.apache.commons.lang.StringUtils;
import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.StringRepresentation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceContext;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.springframework.beans.factory.annotation.Required;

public class UserConferenceCommandsResource extends UserResource {
    private static final String INVITATION_SENT = "<command-response>Invitation sent</command-response>";
    private static final String INCORECT_INVITE_COMMAND = "Incorect invite command";

    private ConferenceBridgeContext m_conferenceBridgeContext;
    private ActiveConferenceContext m_activeConferenceContext;
    private String m_confName;
    private String[] m_arguments;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        m_confName = (String) getRequest().getAttributes().get("confName");
        String argumentList = (String) getRequest().getAttributes().get("command");
        m_arguments = StringUtils.split(argumentList, '&');
        getVariants().add(new Variant(MediaType.TEXT_ALL));
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        Conference conference = m_conferenceBridgeContext.findConferenceByName(m_confName);
        if (conference == null) {
            throw new ResourceException(Status.CLIENT_ERROR_NOT_FOUND, "Conference not found");
        }
        if (!conference.hasOwner() || !StringUtils.equals(conference.getOwner().getName(), getUser().getName())) {
            throw new ResourceException(Status.CLIENT_ERROR_FORBIDDEN, "User is not owner of this conference");
        }
        if (m_arguments == null || m_arguments.length == 0) {
            throw new ResourceException(Status.CLIENT_ERROR_BAD_REQUEST, "No conference command specified");
        }
        if (StringUtils.equals(m_arguments[0], "invite")) {
            if (m_arguments.length == 2) {
                m_activeConferenceContext.inviteParticipant(getUser(), conference, m_arguments[1]);
                return new StringRepresentation(INVITATION_SENT);
            } else {
                throw new ResourceException(Status.CLIENT_ERROR_BAD_REQUEST, INCORECT_INVITE_COMMAND);
            }
        } else if (StringUtils.equals(m_arguments[0], "inviteim")) {
            if (m_arguments.length == 2) {
                m_activeConferenceContext.inviteImParticipant(getUser(), conference, m_arguments[1]);
                return new StringRepresentation(INVITATION_SENT);
            } else {
                throw new ResourceException(Status.CLIENT_ERROR_BAD_REQUEST, INCORECT_INVITE_COMMAND);
            }
        }
        String response = m_activeConferenceContext.executeCommand(conference, m_arguments);
        return new StringRepresentation(response);
    }

    @Required
    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceBridgeContext) {
        m_conferenceBridgeContext = conferenceBridgeContext;
    }

    @Required
    public void setActiveConferenceContext(ActiveConferenceContext activeConferenceContext) {
        m_activeConferenceContext = activeConferenceContext;
    }

    public String[] getArguments() {
        return m_arguments;
    }
}
