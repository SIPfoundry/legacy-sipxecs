/**
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

import java.net.URLDecoder;

import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.sipfoundry.sipxconfig.common.AbstractUser;
import org.sipfoundry.sipxconfig.common.User;

public class PasswordResource extends UserResource {
    private String m_newPin;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);

        setReadable(false);

        m_newPin = URLDecoder.decode((String) getRequest().getAttributes().get("password"));
    }

    @Override
    public boolean allowGet() {
        return false;
    }

    @Override
    public boolean allowPut() {
        return true;
    }

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        if (!(m_newPin == null) && m_newPin.length() >= AbstractUser.PASSWORD_LEN) {
            User user = getUser();
            user.setPin(m_newPin);
            getCoreContext().saveUser(user);
        } else {
            throw new ResourceException(Status.CLIENT_ERROR_BAD_REQUEST, String.format(
                    "Password must be at least %d characters long", AbstractUser.PASSWORD_LEN));
        }
    }

    public String getNewPin() {
        return m_newPin;
    }
}
