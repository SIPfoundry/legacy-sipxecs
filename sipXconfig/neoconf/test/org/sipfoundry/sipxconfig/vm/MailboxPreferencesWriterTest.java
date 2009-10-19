/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.vm;

import java.io.InputStream;
import java.io.StringWriter;

import org.sipfoundry.sipxconfig.vm.MailboxPreferences.ActiveGreeting;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;

public class MailboxPreferencesWriterTest extends TestCase {

    public void testWrite() throws Exception {
        MailboxPreferencesWriter writer = new MailboxPreferencesWriter();
        writer.setVelocityEngine(TestHelper.getVelocityEngine());

        MailboxPreferences mailboxPrefs = new MailboxPreferences();
        mailboxPrefs.setEmailAddress("dhubler@pingtel.com");
        mailboxPrefs.setActiveGreeting(ActiveGreeting.OUT_OF_OFFICE);

        StringWriter actual = new StringWriter();
        writer.writeObject(mailboxPrefs, actual);

        InputStream expectedIn = getClass().getResourceAsStream("mailboxprefs.test.xml");
        assertEquals(IOUtils.toString(expectedIn), actual.toString());
    }
}
