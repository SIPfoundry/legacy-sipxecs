/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.aastra;

import java.util.Collection;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;

public class AastraPhonebook extends ProfileContext {

    private Collection<PhonebookEntry> m_phonebook;

    public AastraPhonebook(Collection<PhonebookEntry> phonebook) {
        super(null, "aastra/aastra-phonebook.csv.vm");
        m_phonebook = phonebook;
    }

    public Collection<PhonebookEntry> getPhonebook() {
        return m_phonebook;
    }
}
