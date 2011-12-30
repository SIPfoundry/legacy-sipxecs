/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.rest;

import java.io.IOException;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;

import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.OutputRepresentation;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.OutputStreamProfileLocation;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public class ProfileResource extends Resource {
    private PhoneContext m_phoneContext;
    private String m_serialNumber;
    private String m_name;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        m_serialNumber = (String) request.getAttributes().get("serialNumber");
        try {
            m_name = URLDecoder.decode((String) request.getAttributes().get("name"), "UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException(e);
        }
        getVariants().add(new Variant(MediaType.ALL));
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        Integer phoneId = m_phoneContext.getPhoneIdBySerialNumber(m_serialNumber);
        if (phoneId == null) {
            // no phone found
            getResponse().setStatus(Status.CLIENT_ERROR_NOT_FOUND);
            return null;
        }
        Device device = m_phoneContext.loadPhone(phoneId);
        Profile[] profiles = device.getProfileTypes();
        for (int i = 0; i < profiles.length; i++) {
            if (profiles[i].getName().equals(m_name)) {
                return new ProfileRepresentation(device, profiles[i]);
            }
        }
        // no profile found
        getResponse().setStatus(Status.CLIENT_ERROR_NOT_FOUND);
        return null;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    static final class ProfileRepresentation extends OutputRepresentation {
        private final Device m_device;
        private final Profile m_profile;

        ProfileRepresentation(Device device, Profile profile) {
            super(new MediaType(profile.getMimeType()));
            m_device = device;
            m_profile = profile;
        }

        @Override
        public void write(OutputStream outputStream) throws IOException {
            OutputStreamProfileLocation location = new OutputStreamProfileLocation(outputStream);
            m_profile.generate(m_device, location);
        }
    }
}
