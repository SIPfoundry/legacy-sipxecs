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

import static org.restlet.data.MediaType.TEXT_PLAIN;
import static org.restlet.data.MediaType.TEXT_VCARD;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Collection;
import java.util.List;

import org.apache.log4j.Logger;
import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.StringRepresentation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager.PhonebookFormat;

import com.thoughtworks.xstream.XStream;

public class UserPhonebookResource extends UserPhonebookSearchResource {
    private static final Logger LOG = Logger.getLogger(UserPhonebookResource.class);

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_PLAIN));
        getVariants().add(new Variant(TEXT_VCARD));
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        Collection<Phonebook> phonebooks = getPhonebookManager().getAllPhonebooksByUser(getUser());
        Collection<PhonebookEntry> entries = getPhonebookManager().getEntries(phonebooks, getUser());
        Representation reprObj;
        if (TEXT_PLAIN.equals(variant.getMediaType())) {
            reprObj = new StringRepresentation(convert(entries, PhonebookFormat.CSV));
        } else if (TEXT_VCARD.equals(variant.getMediaType())) {
            reprObj = new StringRepresentation(convert(entries, PhonebookFormat.VCARD));
        } else {
            reprObj = new PhonebookEntryRepresentation(variant.getMediaType(), convertPhonebookEntries(entries));
        }

        return reprObj;
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

    @Override
    public void removeRepresentations() throws ResourceException {
        User user = getUser();
        getPhonebookManager().removePrivatePhonebook(user);
    }

    private String convert(Collection<PhonebookEntry> entries, PhonebookFormat fmt) {
        String converted = "";
        ByteArrayOutputStream stream = new ByteArrayOutputStream();
        try {
            m_phonebookManager.exportPhonebook(entries, stream, fmt);
            converted = stream.toString("UTF-8");
        } catch (IOException e) {
            LOG.error(String.format("Failed to convert phonebook to %s format: %s", fmt, e.getMessage()));
        }

        return converted;
    }

    private static class PrivatePhonebookRepresentation extends XStreamRepresentation<Collection<PhonebookEntry>> {
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
