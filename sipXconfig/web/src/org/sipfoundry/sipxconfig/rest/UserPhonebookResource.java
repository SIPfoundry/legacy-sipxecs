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

import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;

public class UserPhonebookResource extends UserPhonebookSearchResource {

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        Collection<Phonebook> phonebooks = getPhonebooks();
        Collection<PhonebookEntry> entries = getPhonebookManager().getEntries(phonebooks);
        return new PhonebookEntryRepresentation(variant.getMediaType(), convertPhonebookEntries(entries));
    }
}
