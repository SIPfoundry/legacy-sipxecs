/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;

import java.io.File;

import org.sipfoundry.sipxconfig.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class BridgeTest extends BeanWithSettingsTestCase {

    public void testInsertConference() {
        Conference c = new Conference();
        c.setUniqueId();

        Bridge bridge = new Bridge();
        assertTrue(bridge.getConferences().isEmpty());
        bridge.addConference(c);

        assertEquals(1, bridge.getConferences().size());
        assertSame(c, bridge.getConferences().iterator().next());

        assertSame(bridge, c.getBridge());
    }

    public void testRemoveConference() {
        Conference c = new Conference();
        c.setUniqueId();

        Conference c1 = new Conference();
        c1.setUniqueId();

        Bridge bridge = new Bridge();
        assertTrue(bridge.getConferences().isEmpty());
        bridge.addConference(c);

        bridge.removeConference(c1);
        assertEquals(1, bridge.getConferences().size());

        bridge.removeConference(c);
        assertTrue(bridge.getConferences().isEmpty());
        assertNull(c.getBridge());
    }


    public void testGetAudioDirecotry() {
        Bridge bridge = new Bridge();
        LocalizationContext localContex = createMock(LocalizationContext.class);
        expect(localContex.getCurrentLanguage()).andReturn("fr");
        expect(localContex.getCurrentLanguageDir()).andReturn("stdprompts_fr");
        replay(localContex);
        String sysdirDoc = TestHelper.getTestProperties().getProperty("sysdir.doc");
        bridge.setLocalizationContext(localContex);
        bridge.setPromptsDir(sysdirDoc);
        String path = sysdirDoc+System.getProperty("file.separator")+"stdprompts_fr";
        new File(path+System.getProperty("file.separator")+"conf").mkdirs();
        bridge.setAudioDirectory(path);
        assertEquals(path, bridge.getAudioDirectory());
    }

    public void testGetDefaultAudioDirecotry() {
        Bridge bridge = new Bridge();
        LocalizationContext localContex = createMock(LocalizationContext.class);
        expect(localContex.getCurrentLanguage()).andReturn("ja");
        expect(localContex.getCurrentLanguageDir()).andReturn("stdprompts_ja");
        replay(localContex);
        String sysdirDoc = TestHelper.getTestProperties().getProperty("sysdir.doc");
        bridge.setLocalizationContext(localContex);
        bridge.setPromptsDir(sysdirDoc);
        String path = sysdirDoc+System.getProperty("file.separator")+"stdprompts";
        new File(path+System.getProperty("file.separator")+"conf").mkdirs();
        bridge.setAudioDirectory(path);
        assertEquals(path, bridge.getAudioDirectory());
    }
}

