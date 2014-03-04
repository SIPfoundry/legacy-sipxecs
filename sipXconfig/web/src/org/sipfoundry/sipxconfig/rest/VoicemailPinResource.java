/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.rest;

import static org.sipfoundry.sipxconfig.permission.PermissionName.TUI_CHANGE_PIN;

import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.sipfoundry.sipxconfig.common.AbstractUser;
import org.sipfoundry.sipxconfig.common.User;

public class VoicemailPinResource extends UserResource {
    private String m_newPin;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);

        setModifiable(getUser().hasPermission(TUI_CHANGE_PIN));
        setReadable(false);

        m_newPin = (String) getRequest().getAttributes().get("pin");
    }

    @Override
    public boolean allowPut() {
        return true;
    }

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        if (!(m_newPin == null) && m_newPin.length() >= AbstractUser.VOICEMAIL_PIN_LEN) {
            User user = getUser();
            user.setVoicemailPin(m_newPin);
            user.setForcePinChange(false);
            getCoreContext().saveUser(user);
        } else {
            throw new ResourceException(Status.CLIENT_ERROR_BAD_REQUEST, String.format(
                "Voicemail PIN must be at least %d characters long", AbstractUser.VOICEMAIL_PIN_LEN));
        }
    }
}
