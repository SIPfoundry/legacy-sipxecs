/*
 *
 *
 * Copyright (C) 2011 Karel Electronics Corp., All rights reserved.
 *
 **/
package org.sipfoundry.sipxconfig.phone.karel_ip11x;

import java.util.Collection;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;

public class KarelIP11xPhonebook extends ProfileContext {

    private static final String PHONEBOOK_TEMPLATE = "karel-ip11x/contactData1.xml.vm";

    private Collection<PhonebookEntry> m_phonebook;

    public KarelIP11xPhonebook(Collection<PhonebookEntry> phonebook) {
        super(null, PHONEBOOK_TEMPLATE);
        m_phonebook = phonebook;
    }

    public Collection<PhonebookEntry> getPhonebook() {
        return m_phonebook;
    }
}
