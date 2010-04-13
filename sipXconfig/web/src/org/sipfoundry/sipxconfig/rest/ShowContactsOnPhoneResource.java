/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.rest;

import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Method;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.springframework.beans.factory.annotation.Required;

public class ShowContactsOnPhoneResource extends UserResource {
    private PhonebookManager m_phonebookManager;
    private boolean m_show;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);

        setModifiable(true);
        setReadable(false);

        m_show = Boolean.valueOf((String) getRequest().getAttributes().get("value"));

        // NOTE: Due to the bug in Restlet, it requires PUT and POST request must have
        // entity. The following hack is to workaround the bug.
        if (request.getMethod().equals(Method.PUT) && !request.isEntityAvailable()) {
            request.setEntity(" ", MediaType.APPLICATION_ATOM_XML);
        }
    }

    @Override
    public boolean allowPut() {
        return true;
    }

    @Override
    public boolean allowGet() {
        return true;
    }

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        User user = getUser();
        Phonebook phonebook = m_phonebookManager.getPrivatePhonebook(user);
        phonebook.setShowOnPhone(m_show);
        m_phonebookManager.savePhonebook(phonebook);
    }

    @Required
    public void setPhonebookManager(PhonebookManager phonebookManager) {
        m_phonebookManager = phonebookManager;
    }
}
