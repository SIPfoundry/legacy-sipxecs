/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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

import org.restlet.data.MediaType;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.security.core.context.SecurityContextHolder;

import com.thoughtworks.xstream.XStream;

import edu.emory.mathcs.backport.java.util.Collections;

public class LoginDetailsResourceWithPin extends LoginDetailsResource {

    private ConfigManager m_configManager;

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        //This is the password that user uses to authenticate to this REST service
        String password = SecurityContextHolder.getContext().getAuthentication().getCredentials().toString();

        LoginDetails details = (LoginDetails) super.represent(variant);
        Representable representable = details.getObject();
        FeatureManager featureManager = m_configManager.getFeatureManager();
        List<Location> locations = featureManager.getLocationsForEnabledFeature(ImManager.FEATURE);
        List<LocationRepresentable> imLocations = new ArrayList<LocationRepresentable>();
        for (Location location : locations) {
            imLocations.add(new LocationRepresentable(location.getFqdn()));
        }
        Collections.shuffle(imLocations);
        RepresentableWithPin representableWithPin = new RepresentableWithPin(representable.getUserName(),
            representable.getImId(), representable.isLdapImAuth(), representable.getSipPassword(),
            password, imLocations);

        return new LoginDetailsWithPin(details.getMediaType(), representableWithPin);
    }

    private static class RepresentableWithPin extends Representable {
        private static final long serialVersionUID = 1L;
        private final String m_pin;
        private final List<LocationRepresentable> m_imLocations = new ArrayList<LocationRepresentable>();

        public RepresentableWithPin(String userName, String imId, boolean ldapAuth, String sipPassword,
            String pin, List<LocationRepresentable> imLocations) {
            super(userName, imId, ldapAuth, sipPassword);
            m_pin = pin;
            m_imLocations.addAll(imLocations);
        }

        @SuppressWarnings("unused")
        public String getPin() {
            return m_pin;
        }

        @SuppressWarnings("unused")
        public List<LocationRepresentable> getImLocations() {
            return m_imLocations;
        }
    }

    private static class LocationRepresentable implements Serializable {
        private static final long serialVersionUID = 1L;
        private String m_fqdn;

        public LocationRepresentable(String fqdn) {
            m_fqdn = fqdn;
        }

        @SuppressWarnings("unused")
        public String getFqdn() {
            return m_fqdn;
        }
    }

    @Required
    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    protected static class LoginDetailsWithPin extends LoginDetails {
        public LoginDetailsWithPin(MediaType mediaType, Representable object) {
            super(mediaType, object);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            super.configureXStream(xstream);
            xstream.aliasField("im-locations", RepresentableWithPin.class, "imLocations");
            xstream.alias("im-location", LocationRepresentable.class);
        }

        @Override
        protected void configureImplicitCollections(XStream xstream) {
            xstream.addImplicitCollection(RepresentableWithPin.class, "m_imLocations", LocationRepresentable.class);
        }
    }
}
