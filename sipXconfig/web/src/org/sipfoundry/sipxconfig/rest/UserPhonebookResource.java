/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.rest;

import java.util.Collection;
import java.util.List;

import com.thoughtworks.xstream.XStream;

import org.restlet.data.MediaType;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;

public class UserPhonebookResource extends UserPhonebookSearchResource {
    @Override
    public Representation represent(Variant variant) throws ResourceException {
        Collection<Phonebook> phonebooks = getPhonebookManager().getAllPhonebooksByUser(getUser());
        Collection<PhonebookEntry> entries = getPhonebookManager().getEntries(phonebooks, getUser());
        return new PhonebookEntryRepresentation(variant.getMediaType(), convertPhonebookEntries(entries));
    }

    @Override
    public void acceptRepresentation(Representation entity) throws ResourceException {
        PrivatePhonebookRepresentation representation = new PrivatePhonebookRepresentation(entity);
        Collection<PhonebookEntry> newEntries = representation.getObject();
        if (newEntries.isEmpty()) {
            return;
        }

        User user = getUser();
        Phonebook privatePhonebook = getPhonebookManager().getPrivatePhonebookCreateIfRequired(user);
        Collection<PhonebookEntry> entries = privatePhonebook.getEntries();
        entries.addAll(newEntries);
        for (PhonebookEntry entry : entries) {
            entry.setPhonebook(privatePhonebook);
        }
        getPhonebookManager().savePhonebook(privatePhonebook);
    }

    static class PrivatePhonebookRepresentation extends XStreamRepresentation<Collection<PhonebookEntry>> {
        public PrivatePhonebookRepresentation(MediaType mediaType, Collection<PhonebookEntry> object) {
            super(mediaType, object);
        }

        public PrivatePhonebookRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias("phonebook", List.class);
            xstream.alias("entry", PhonebookEntry.class);
            xstream.aliasField("first-name", PhonebookEntry.class, "firstName");
            xstream.aliasField("last-name", PhonebookEntry.class, "lastName");
            xstream.aliasField("contact-information", PhonebookEntry.class, "addressBookEntry");
            xstream.omitField(Phonebook.class, "m_Phonebook");
        }
    }
}
