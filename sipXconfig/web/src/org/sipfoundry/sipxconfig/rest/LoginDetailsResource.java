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

import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.beans.factory.annotation.Required;

import com.thoughtworks.xstream.XStream;

public class LoginDetailsResource extends UserResource {

    private LdapManager m_ldapManager;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_XML));
        getVariants().add(new Variant(APPLICATION_JSON));
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        User user = getUser();
        boolean ldapAuth = m_ldapManager.getSystemSettings().isEnableOpenfireConfiguration();

        return new LoginDetails(variant.getMediaType(), new Representable(user.getUserName(), user.getImId(),
            ldapAuth, user.getSipPassword()));
    }

    @Required
    public void setLdapManager(LdapManager ldapManager) {
        m_ldapManager = ldapManager;
    }

    protected static class Representable implements Serializable {
        private static final long serialVersionUID = 1L;

        private final String m_userName;
        private final String m_imId;
        private final boolean m_ldapImAuth;
        private final String m_sipPassword;

        public Representable(String userName, String imId, boolean ldapAuth, String sipPassword) {
            m_userName = userName;
            m_imId = imId;
            m_ldapImAuth = ldapAuth;
            m_sipPassword = sipPassword;
        }

        public String getUserName() {
            return m_userName;
        }

        public String getImId() {
            return m_imId;
        }

        public boolean isLdapImAuth() {
            return m_ldapImAuth;
        }

        public String getSipPassword() {
            return m_sipPassword;
        }
    }

    protected static class LoginDetails extends XStreamRepresentation<Representable> {
        private Class< ? > m_representableClass;

        public LoginDetails(MediaType mediaType, Representable object) {
            super(mediaType, object);
            m_representableClass = object.getClass();
        }

        public LoginDetails(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias("login-details", m_representableClass);
        }
    }

}
