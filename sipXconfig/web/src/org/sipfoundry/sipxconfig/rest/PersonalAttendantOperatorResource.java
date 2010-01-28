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

import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.springframework.beans.factory.annotation.Required;

public class PersonalAttendantOperatorResource extends UserResource {
    private MailboxManager m_mailboxManager;
    private String m_operator;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);

        setModifiable(true);
        setReadable(false);

        m_operator = (String) getRequest().getAttributes().get("operator");
    }

    @Override
    public boolean allowPut() {
        return true;
    }

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        User user = getUser();
        m_mailboxManager.updatePersonalAttendantForUser(user, m_operator);
        user.getSettings().getSetting("personal-attendant/operator").setValue(m_operator);
        getCoreContext().saveUser(user);
    }

    @Required
    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }
}
