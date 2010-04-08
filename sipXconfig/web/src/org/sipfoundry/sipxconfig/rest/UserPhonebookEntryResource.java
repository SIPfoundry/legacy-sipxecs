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
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.thoughtworks.xstream.XStream;

import org.apache.commons.lang.StringUtils;
import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.springframework.beans.factory.annotation.Required;

import static org.restlet.data.MediaType.APPLICATION_JSON;
import static org.restlet.data.MediaType.TEXT_XML;

public class UserPhonebookEntryResource extends UserResource {

    public static final Status PHONEBOOK_DUPLICATE_ENTRY_ERROR = new Status(747);

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
        Phonebook privatePhonebook = m_phonebookManager.getPrivatePhonebook(getUser());
        if (privatePhonebook != null) {
            Collection<PhonebookEntry> entries = privatePhonebook.getEntries();
            ArrayList<String> ids = new ArrayList<String>();
            for (PhonebookEntry entry : entries) {
                ids.add(String.valueOf(entry.getId()));
            }
            if (ids.contains(m_entryId)) {
                PhonebookEntry entry = m_phonebookManager.getPhonebookEntry(Integer.parseInt(m_entryId));
                PhonebookEntry reprEntry = (PhonebookEntry) entry.duplicate();
                return new PhonebookEntryRepresentation(variant.getMediaType(), reprEntry);
            }
        }
        return null;
    }

    @Override
    public void acceptRepresentation(Representation entity) throws ResourceException {
        PhonebookEntryRepresentation representation = new PhonebookEntryRepresentation(entity);
        PhonebookEntry newEntry = representation.getObject();
        if (!validatePhonebookEntry(newEntry)) {
            return;
        }

        if (m_phonebookManager.getDuplicatePhonebookEntry(newEntry, getUser()) != null) {
            setDuplicateEntryStatus();
            return;
        }

        User user = getUser();
        Phonebook privatePhonebook = m_phonebookManager.getPrivatePhonebookCreateIfRequired(user);
        newEntry.setPhonebook(privatePhonebook);
        (privatePhonebook.getEntries()).add(newEntry);
        m_phonebookManager.savePhonebook(privatePhonebook);
    }

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        PhonebookEntryRepresentation representation = new PhonebookEntryRepresentation(entity);
        PhonebookEntry newEntry = representation.getObject();
        if (!validatePhonebookEntry(newEntry)) {
            return;
        }

        PhonebookEntry duplicateEntry = m_phonebookManager.getDuplicatePhonebookEntry(newEntry, getUser());
        if (duplicateEntry != null && !duplicateEntry.getId().equals(Integer.parseInt(m_entryId))) {
            setDuplicateEntryStatus();
            return;
        }

        Phonebook privatePhonebook = m_phonebookManager.getPrivatePhonebook(getUser());
        if (privatePhonebook != null) {
            newEntry.setPhonebook(privatePhonebook);
            PhonebookEntry entry = m_phonebookManager.getPhonebookEntry(Integer.parseInt(m_entryId));
            entry.update(newEntry);
            m_phonebookManager.updatePhonebookEntry(entry);
        }
    }

    private void setDuplicateEntryStatus() {
        getResponse().setStatus(PHONEBOOK_DUPLICATE_ENTRY_ERROR, "Duplicate Entry");
    }

    @Override
    public void removeRepresentations() throws ResourceException {
        Phonebook privatePhonebook = m_phonebookManager.getPrivatePhonebook(getUser());
        if (privatePhonebook != null) {
            Collection<PhonebookEntry> entries = privatePhonebook.getEntries();
            ArrayList<String> ids = new ArrayList<String>();
            for (PhonebookEntry entry : entries) {
                ids.add(String.valueOf(entry.getId()));
            }
            if (ids.contains(m_entryId)) {
                PhonebookEntry entry = m_phonebookManager.getPhonebookEntry(Integer.parseInt(m_entryId));
                m_phonebookManager.deletePhonebookEntry(entry);
            }
        }
    }

    private boolean validatePhonebookEntry(PhonebookEntry newEntry) {
        // validate given values - first name, last name, number and e-mail (if provided)
        if (StringUtils.isEmpty(newEntry.getFirstName()) || StringUtils.isEmpty(newEntry.getLastName())
                || StringUtils.isEmpty(newEntry.getNumber())) {
            getResponse().setStatus(Status.CLIENT_ERROR_BAD_REQUEST,
                    "First Name / Last Name / Number cannot be Null");
            return false;
        }

        if (!StringUtils.isEmpty(newEntry.getAddressBookEntry().getEmailAddress())) {
            Pattern emailPattern = Pattern.compile("^([a-zA-Z0-9_.\\-+])+@(([a-zA-Z0-9\\-])+\\.)+[a-zA-Z0-9]{2,4}$");
            Matcher matcher = emailPattern.matcher(newEntry.getAddressBookEntry().getEmailAddress());
            if (!matcher.matches()) {
                getResponse().setStatus(Status.CLIENT_ERROR_BAD_REQUEST, "Invalid Email Address");
                return false;
            }
        }
        return true;
    }

    @Required
    public void setPhonebookManager(PhonebookManager phonebookManager) {
        m_phonebookManager = phonebookManager;
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
            xstream.omitField(AddressBookEntry.class, "m_branchOfficeAddress");
        }
    }

}
