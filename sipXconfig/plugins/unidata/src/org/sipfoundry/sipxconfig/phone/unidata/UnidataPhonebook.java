/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.unidata;

import java.util.Collection;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;

public class UnidataPhonebook extends ProfileContext {

    private final Collection<PhonebookEntry> m_phonebook;

    public UnidataPhonebook(Collection<PhonebookEntry> phonebook) {
        super(null, "unidata/unidata-phonebook.vm");
        m_phonebook = phonebook;
    }

    public Collection<PhonebookEntry> getPhonebook() {
        return m_phonebook;
    }
}
