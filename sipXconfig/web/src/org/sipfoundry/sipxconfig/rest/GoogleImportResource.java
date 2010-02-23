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

import java.io.Serializable;

import com.thoughtworks.xstream.XStream;
import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.phonebook.GoogleImporter.GoogleAuthUserException;
import org.sipfoundry.sipxconfig.phonebook.GoogleImporter.GoogleServiceUserException;
import org.sipfoundry.sipxconfig.phonebook.GoogleImporter.GoogleTransportUserException;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.springframework.beans.factory.annotation.Required;

import static org.restlet.data.MediaType.APPLICATION_JSON;
import static org.restlet.data.MediaType.APPLICATION_XML;

public class GoogleImportResource extends UserResource {
    public static final Status PHONEBOOK_GOOGLE_AUTH_ERROR = new Status(743);
    public static final Status PHONEBOOK_GOOGLE_SERVICE_ERROR = new Status(744);
    public static final Status PHONEBOOK_GOOGLE_TRANSPORT_ERROR = new Status(745);
    private PhonebookManager m_phonebookManager;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        setReadable(false);
        getVariants().add(new Variant(APPLICATION_JSON));
        getVariants().add(new Variant(APPLICATION_XML));
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
        GoogleImportRepresentation googleImport = new GoogleImportRepresentation(entity);
        Representable credentials = googleImport.getObject();
        Integer phonebookId = m_phonebookManager.getPrivatePhonebookCreateIfRequired(getUser()).getId();
        try {
            int entries = m_phonebookManager.addEntriesFromGoogleAccount(phonebookId, credentials.getAccount(),
                    credentials.getPassword());
            getResponse().setStatus(Status.SUCCESS_OK, String.valueOf(entries));
        } catch (GoogleAuthUserException ex) {
            getResponse().setStatus(PHONEBOOK_GOOGLE_AUTH_ERROR);
        } catch (GoogleServiceUserException ex) {
            getResponse().setStatus(PHONEBOOK_GOOGLE_SERVICE_ERROR);
        } catch (GoogleTransportUserException ex) {
            getResponse().setStatus(PHONEBOOK_GOOGLE_TRANSPORT_ERROR);
        }
    }

    @Required
    public void setPhonebookManager(PhonebookManager phonebookManager) {
        m_phonebookManager = phonebookManager;
    }

    static class Representable implements Serializable {
        private final String m_account;
        private final String m_password;

        public Representable(String account, String password) {
            m_account = account;
            m_password = password;
        }

        public String getAccount() {
            return m_account;
        }

        public String getPassword() {
            return m_password;
        }
    }

    static class GoogleImportRepresentation extends XStreamRepresentation<Representable> {

        public GoogleImportRepresentation(MediaType mediaType, Representable object) {
            super(mediaType, object);
        }

        public GoogleImportRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias("credentials", Representable.class);
        }
    }
}
