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

import org.sipfoundry.sipxconfig.TestHelper;

public class MailboxTest extends TestCase {

    public void testFilePreferencesFile() {
        LocalMailbox mbox = new LocalMailbox(new File(TestHelper.getSourceDirectory(this.getClass())), "200");
        assertTrue(mbox.getVoicemailPreferencesFile().isFile());
    }

}
