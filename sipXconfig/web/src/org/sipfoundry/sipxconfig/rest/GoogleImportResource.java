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
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.springframework.beans.factory.annotation.Required;

import static org.sipfoundry.sipxconfig.phonebook.GoogleImporter.GmailAuthUserException;
import static org.sipfoundry.sipxconfig.phonebook.GoogleImporter.GmailServiceUserException;
import static org.sipfoundry.sipxconfig.phonebook.GoogleImporter.GmailTransportUserException;

public class GoogleImportResource extends UserResource {
    public static final Status PHONEBOOK_GMAIL_AUTH_ERROR = new Status(743);
    public static final Status PHONEBOOK_GMAIL_SERVICE_ERROR = new Status(744);
    public static final Status PHONEBOOK_GMAIL_TRANSPORT_ERROR = new Status(745);
    private PhonebookManager m_phonebookManager;
    private String m_account;
    private String m_password;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        setReadable(false);
        m_account = (String) getRequest().getAttributes().get("account");
        m_password = (String) getRequest().getAttributes().get("password");
    }

    @Override
    public boolean allowPost() {
        return true;
    }

    @Override
    public boolean allowGet() {
        return false;
    }

    @Override
    public void post(Representation entity) {
        Integer phonebookId = m_phonebookManager.getPrivatePhonebookCreateIfRequired(getUser()).getId();

        try {
            int entries = m_phonebookManager.addEntriesFromGmailAccount(phonebookId, m_account, m_password);
            getResponse().setStatus(Status.SUCCESS_OK, String.valueOf(entries));
        } catch (GmailAuthUserException ex) {
            getResponse().setStatus(PHONEBOOK_GMAIL_AUTH_ERROR);
        } catch (GmailServiceUserException ex) {
            getResponse().setStatus(PHONEBOOK_GMAIL_SERVICE_ERROR);
        } catch (GmailTransportUserException ex) {
            getResponse().setStatus(PHONEBOOK_GMAIL_TRANSPORT_ERROR);
        }
    }

    @Required
    public void setPhonebookManager(PhonebookManager phonebookManager) {
        m_phonebookManager = phonebookManager;
    }
}
