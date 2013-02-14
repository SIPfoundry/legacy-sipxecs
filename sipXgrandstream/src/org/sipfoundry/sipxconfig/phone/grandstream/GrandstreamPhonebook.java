/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.phone.grandstream;

import java.util.Collection;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;

public class GrandstreamPhonebook extends ProfileContext {

    private final Collection<PhonebookEntry> m_phonebook;

    public GrandstreamPhonebook(Collection<PhonebookEntry> phonebook) {
        super(null, "grandstream/gs_phonebook.xml.vm");
        m_phonebook = phonebook;
    }

    public Collection<PhonebookEntry> getPhonebook() {
        return m_phonebook;
    }
}
