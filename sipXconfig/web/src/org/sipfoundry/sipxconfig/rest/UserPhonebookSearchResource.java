/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.rest;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import com.thoughtworks.xstream.XStream;
import org.restlet.Context;
import org.restlet.data.Form;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;

import static org.restlet.data.MediaType.APPLICATION_JSON;
import static org.restlet.data.MediaType.TEXT_XML;

public class UserPhonebookSearchResource extends UserResource {
    private PhonebookManager m_phonebookManager;
    private String m_searchTerm;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_XML));
        getVariants().add(new Variant(APPLICATION_JSON));
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        Form form = getRequest().getResourceRef().getQueryAsForm();
        m_searchTerm = form.getFirstValue("query");
        Collection<Phonebook> phonebooks = getPhonebooks();
        // TODO
        Collection<PhonebookEntry> entries = m_phonebookManager.search(phonebooks, m_searchTerm, getUser());

        return new PhonebookEntryRepresentation(variant.getMediaType(), convertPhonebookEntries(entries));
    }

    protected ArrayList<Representable> convertPhonebookEntries(Collection<PhonebookEntry> entries) {
        ArrayList<Representable> entriesArray = new ArrayList<Representable>();
        for (PhonebookEntry entry : entries) {
            entriesArray.add(new Representable(entry.getFirstName(), entry.getLastName(), entry.getNumber(), entry
                    .getAddressBookEntry(), entry.getId().toString()));
        }
        return entriesArray;
    }

    protected Collection<Phonebook> getPhonebooks() {
        User user = getUser();
        return m_phonebookManager.getPhonebooksByUser(user);
    }

    public void setPhonebookManager(PhonebookManager phonebookManager) {
        m_phonebookManager = phonebookManager;
    }

    public PhonebookManager getPhonebookManager() {
        return m_phonebookManager;
    }

    static class Representable implements Serializable {
        @SuppressWarnings("unused")
        private final String m_id;
        @SuppressWarnings("unused")
        private final String m_firstName;
        @SuppressWarnings("unused")
        private final String m_lastName;
        @SuppressWarnings("unused")
        private final String m_number;
        @SuppressWarnings("unused")
        private final AddressBookEntry m_addressBookEntry;

        public Representable(String firstName, String lastName, String number, AddressBookEntry addressBookEntry,
                String id) {
            m_firstName = firstName;
            m_lastName = lastName;
            m_number = number;
            m_addressBookEntry = addressBookEntry;
            m_id = id;
        }
    }

    static class PhonebookEntryRepresentation extends XStreamRepresentation<Collection<Representable>> {

        private static final String ID = "m_id";

        public PhonebookEntryRepresentation(MediaType mediaType, Collection<Representable> object) {
            super(mediaType, object);
        }

        public PhonebookEntryRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.omitField(BeanWithId.class, ID);
            xstream.alias("phonebook", List.class);
            xstream.alias("entry", Representable.class);
            xstream.aliasField("first-name", Representable.class, "firstName");
            xstream.aliasField("last-name", Representable.class, "lastName");
            xstream.aliasField("contact-information", Representable.class, "addressBookEntry");
            xstream.omitField(AddressBookEntry.class, "m_useBranchAddress");
            xstream.omitField(Representable.class, ID);
        }
    }
}
