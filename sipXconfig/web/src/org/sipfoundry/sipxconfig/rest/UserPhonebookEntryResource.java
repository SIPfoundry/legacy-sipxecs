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

import java.util.ArrayList;
import java.util.Collection;

import com.thoughtworks.xstream.XStream;

import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;

import static org.restlet.data.MediaType.APPLICATION_JSON;
import static org.restlet.data.MediaType.TEXT_XML;

public class UserPhonebookEntryResource extends UserResource {
    private PhonebookManager m_phonebookManager;
    private String m_entryId;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_XML));
        getVariants().add(new Variant(APPLICATION_JSON));
        m_entryId = (String) getRequest().getAttributes().get("entryId");
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        Phonebook privatePhonebook = getPhonebookManager().getPrivatePhonebook(getUser());
        if (privatePhonebook != null) {
            Collection<PhonebookEntry> entries = privatePhonebook.getEntries();
            ArrayList<String> ids = new ArrayList<String>();
            for (PhonebookEntry entry : entries) {
                ids.add(String.valueOf(entry.getId()));
            }
            if (ids.contains(m_entryId)) {
                PhonebookEntry entry = getPhonebookManager().getPhonebookEntry(Integer.parseInt(m_entryId));
                PhonebookEntry reprEntry = (PhonebookEntry) entry.duplicate();
                return new PhonebookEntryRepresentation(variant.getMediaType(), reprEntry);
            }
        }
        return null;
    }

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        PhonebookEntryRepresentation representation = new PhonebookEntryRepresentation(entity);
        PhonebookEntry newEntry = representation.getObject();

        Phonebook privatePhonebook = getPhonebookManager().getPrivatePhonebook(getUser());
        if (privatePhonebook != null) {
            newEntry.setPhonebook(privatePhonebook);
            Collection<PhonebookEntry> entries = privatePhonebook.getEntries();
            ArrayList<String> ids = new ArrayList<String>();
            for (PhonebookEntry entry : entries) {
                ids.add(String.valueOf(entry.getId()));
            }
            if (!ids.contains(m_entryId)) {
                entries.add(newEntry);
                getPhonebookManager().savePhonebook(privatePhonebook);
            } else {
                PhonebookEntry entry = getPhonebookManager().getPhonebookEntry(Integer.parseInt(m_entryId));
                entry.update(newEntry);
                getPhonebookManager().savePhonebookEntry(entry);
            }
        }
    }

    @Override
    public void removeRepresentations() throws ResourceException {
        Phonebook privatePhonebook = getPhonebookManager().getPrivatePhonebook(getUser());
        if (privatePhonebook != null) {
            Collection<PhonebookEntry> entries = privatePhonebook.getEntries();
            ArrayList<String> ids = new ArrayList<String>();
            for (PhonebookEntry entry : entries) {
                ids.add(String.valueOf(entry.getId()));
            }
            if (ids.contains(m_entryId)) {
                PhonebookEntry entry = getPhonebookManager().getPhonebookEntry(Integer.parseInt(m_entryId));
                getPhonebookManager().deletePhonebookEntry(entry);
            }
        }
    }

    public void setPhonebookManager(PhonebookManager phonebookManager) {
        m_phonebookManager = phonebookManager;
    }

    public PhonebookManager getPhonebookManager() {
        return m_phonebookManager;
    }

    static class PhonebookEntryRepresentation extends XStreamRepresentation<PhonebookEntry> {
        public PhonebookEntryRepresentation(MediaType mediaType, PhonebookEntry object) {
            super(mediaType, object);
        }

        public PhonebookEntryRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.omitField(BeanWithId.class, "m_id");
            xstream.alias("entry", PhonebookEntry.class);
            xstream.aliasField("first-name", PhonebookEntry.class, "firstName");
            xstream.aliasField("last-name", PhonebookEntry.class, "lastName");
            xstream.aliasField("contact-information", PhonebookEntry.class, "addressBookEntry");
            xstream.omitField(PhonebookEntry.class, "m_phonebook");
            xstream.omitField(AddressBookEntry.class, "m_useBranchAddress");
        }
    }
}
