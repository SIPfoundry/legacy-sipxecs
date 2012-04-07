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

import org.sipfoundry.sipxconfig.backup.BackupBean;
import org.sipfoundry.sipxconfig.backup.BackupPlan;

public class RestorePageTest extends TestCase {

    public void testValidateSelections() {
        File cfg = new File(BackupPlan.CONFIGURATION_ARCHIVE);
        File voicemail = new File(BackupPlan.VOICEMAIL_ARCHIVE);
        File cdr = new File(BackupPlan.CDR_ARCHIVE);
        File deviceConfig = new File(BackupPlan.DEVICE_CONFIG);

        BackupBean cfgBean = new BackupBean(cfg);
        BackupBean voicemailBean = new BackupBean(voicemail);
        BackupBean cdrBean = new BackupBean(cdr);
        BackupBean deviceConfigBean = new BackupBean(deviceConfig);

        assertTrue(RestorePage.validateSelections(Arrays.asList(cfgBean, deviceConfigBean, voicemailBean, cdrBean)));
        assertTrue(RestorePage.validateSelections(Arrays.asList(cfgBean, voicemailBean, cdrBean)));
        assertTrue(RestorePage.validateSelections(Arrays.asList(cfgBean, voicemailBean)));
        assertTrue(RestorePage.validateSelections(Arrays.asList(voicemailBean, cdrBean)));
        assertTrue(RestorePage.validateSelections(Arrays.asList(cfgBean, cdrBean)));
        assertTrue(RestorePage.validateSelections(Arrays.asList(cfgBean)));
        assertTrue(RestorePage.validateSelections(Arrays.asList(voicemailBean)));
        assertTrue(RestorePage.validateSelections(Arrays.asList(cdrBean)));
        assertTrue(RestorePage.validateSelections(Arrays.asList(deviceConfigBean)));
        assertTrue(RestorePage.validateSelections(Arrays.asList(cfgBean, deviceConfigBean)));

        assertFalse(RestorePage.validateSelections(Collections.<BackupBean>emptyList()));
        assertFalse(RestorePage.validateSelections(Arrays.asList(voicemailBean, voicemailBean)));
        assertFalse(RestorePage.validateSelections(Arrays.asList(cfgBean, voicemailBean, cdrBean, deviceConfigBean, new BackupBean())));
        assertFalse(RestorePage.validateSelections(Arrays.asList(cfgBean, cdrBean, cdrBean, new BackupBean())));
        assertFalse(RestorePage.validateSelections(Arrays.asList(cfgBean, deviceConfigBean, deviceConfigBean, new BackupBean())));
    }
}
