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
import org.sipfoundry.sipxconfig.phonebook.PhonebookFileEntry;

public class UserPhonebookResource extends UserPhonebookSearchResource {
    @Override
    public Representation represent(Variant variant) throws ResourceException {
        Collection<Phonebook> phonebooks = getPhonebooks();
        Collection<PhonebookEntry> entries = getPhonebookManager().getEntries(phonebooks, getUser());
        return new PhonebookEntryRepresentation(variant.getMediaType(), convertPhonebookEntries(entries));
    }

    @Override
    public void acceptRepresentation(Representation entity) throws ResourceException {
        PrivatePhonebookRepresentation representation = new PrivatePhonebookRepresentation(entity);
        Collection<PhonebookFileEntry> newEntries = representation.getObject();
        if (newEntries.isEmpty()) {
            return;
        }

        User user = getUser();
        Phonebook privatePhonebook = getPhonebookManager().getPrivatePhonebook(user);
        if (privatePhonebook == null) {
            Phonebook newPhonebook = new Phonebook();
            newPhonebook.setName("privatePhonebook_" + user.getUserName());
            newPhonebook.setUser(user);
            newPhonebook.setEntries(newEntries);
            for (PhonebookFileEntry entry : newEntries) {
                entry.setPhonebook(newPhonebook);
            }
            getPhonebookManager().savePhonebook(newPhonebook);
        } else {
            Collection<PhonebookFileEntry> entries = privatePhonebook.getEntries();
            entries.addAll(newEntries);
            for (PhonebookFileEntry entry : entries) {
                entry.setPhonebook(privatePhonebook);
            }
            getPhonebookManager().savePhonebook(privatePhonebook);
        }
    }

    static class PrivatePhonebookRepresentation extends XStreamRepresentation<Collection<PhonebookFileEntry>> {
        public PrivatePhonebookRepresentation(MediaType mediaType, Collection<PhonebookFileEntry> object) {
            super(mediaType, object);
        }

        public PrivatePhonebookRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias("phonebook", List.class);
            xstream.alias("entry", PhonebookFileEntry.class);
            xstream.aliasField("first-name", PhonebookFileEntry.class, "firstName");
            xstream.aliasField("last-name", PhonebookFileEntry.class, "lastName");
            xstream.aliasField("contact-information", PhonebookFileEntry.class, "addressBookEntry");
            xstream.omitField(Phonebook.class, "m_Phonebook");
        }
    }
}
