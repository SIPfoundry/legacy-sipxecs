/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.io.File;
import java.util.Arrays;
import java.util.Collections;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.BackupBean;
import org.sipfoundry.sipxconfig.admin.BackupPlan;

public class RestorePageTest extends TestCase {

    public void testValidateSelections() {
        File cfg = new File(BackupPlan.CONFIGURATION_ARCHIVE);
        File voicemail = new File(BackupPlan.VOICEMAIL_ARCHIVE);

        BackupBean cfgBean = new BackupBean(cfg);
        BackupBean voicemailBean = new BackupBean(voicemail);

        assertTrue(RestorePage.validateSelections(Arrays.asList(cfgBean, voicemailBean)));
        assertTrue(RestorePage.validateSelections(Arrays.asList(cfgBean)));
        assertTrue(RestorePage.validateSelections(Arrays.asList(voicemailBean)));

        assertFalse(RestorePage.validateSelections(Collections.<BackupBean>emptyList()));
        assertFalse(RestorePage.validateSelections(Arrays.asList(voicemailBean, voicemailBean)));
        assertFalse(RestorePage.validateSelections(Arrays.asList(cfgBean, voicemailBean, new BackupBean())));
    }
}
