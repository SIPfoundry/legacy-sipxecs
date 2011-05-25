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

import java.io.IOException;
import java.util.Collections;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;

public class UnidataPhonebookTest extends TestCase {
    private ProfileGenerator m_pg;
    private MemoryProfileLocation m_location;

    @Override
    protected void setUp() {
        m_location = new MemoryProfileLocation();
        VelocityProfileGenerator pg = new VelocityProfileGenerator();
        pg.setVelocityEngine(TestHelper.getVelocityEngine());
        m_pg = pg;
    }

    public void testPhonebook() throws IOException {
        PhonebookEntry phonebookEntry = new PhonebookEntry();
        phonebookEntry.setFirstName("Joe");
        phonebookEntry.setNumber("1234");

        UnidataPhonebook book = new UnidataPhonebook(Collections.singleton(phonebookEntry));

        m_pg.generate(m_location, book, null, "phonebook");
        List<String> list = IOUtils.readLines(m_location.getReader());
        assertEquals(2, list.size());
        assertEquals("\"Joe \",,1234,", list.get(1));
    }

}
