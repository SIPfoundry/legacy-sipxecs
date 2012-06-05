/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.rest;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import org.apache.commons.io.IOUtils;

import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.OutputRepresentation;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.commons.userdb.profile.UserProfileService;

public class UserAvatarResource extends Resource {
    private String m_userName;
    private UserProfileService m_avatarService;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(MediaType.TEXT_ALL));
        m_userName = (String) getRequest().getAttributes().get("user");
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        return new AvatarRepresentation(MediaType.IMAGE_PNG, m_avatarService.getAvatar(m_userName));
    }

    static class AvatarRepresentation extends OutputRepresentation {

        private InputStream m_is;
        public AvatarRepresentation(MediaType mediaType, InputStream is) {
            super(mediaType);
            m_is = is;
        }

        @Override
        public void write(OutputStream outputStream) throws IOException {
            if (m_is != null) {
                IOUtils.copy(m_is, outputStream);
            }
            IOUtils.closeQuietly(m_is);
        }
    }

    public void setUserAvatarService(UserProfileService service) {
        m_avatarService = service;
    }

}
