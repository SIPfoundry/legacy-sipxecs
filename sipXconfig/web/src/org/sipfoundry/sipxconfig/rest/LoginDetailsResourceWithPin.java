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

import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;

public class LoginDetailsResourceWithPin extends LoginDetailsResource {

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        LoginDetails details = (LoginDetails) super.represent(variant);
        Representable representable = details.getObject();

        RepresentableWithPin representableWithPin = new RepresentableWithPin(representable.getUserName(),
            representable.getImId(), representable.isLdapImAuth(), representable.getSipPassword(), getUser()
                .getPintoken());

        return new LoginDetails(details.getMediaType(), representableWithPin);
    }

    private static class RepresentableWithPin extends Representable {
        private static final long serialVersionUID = 1L;
        private final String m_pin;

        public RepresentableWithPin(String userName, String imId, boolean ldapAuth, String sipPassword, String pin) {
            super(userName, imId, ldapAuth, sipPassword);
            m_pin = pin;
        }

        @SuppressWarnings("unused")
        public String getPin() {
            return m_pin;
        }
    }
}
