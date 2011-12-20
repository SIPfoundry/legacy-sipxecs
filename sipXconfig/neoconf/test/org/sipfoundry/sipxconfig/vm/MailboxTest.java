/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import java.io.File;

import junit.framework.TestCase;

public class MailboxTest extends TestCase {

    public void testFilePreferencesFile() {
        File thisDir = new File(getClass().getResource("mailboxprefs.test.xml").getFile()).getParentFile();
        LocalMailbox mbox = new LocalMailbox(thisDir, "200");
        assertTrue(mbox.getVoicemailPreferencesFile().isFile());
    }

}
