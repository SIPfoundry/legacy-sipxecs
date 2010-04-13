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
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import java.io.File;
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.acd.BeanWithSettingsTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxParkService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

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

    public void testAccessors()
    {
        Location location = new Location();
        SipxFreeswitchService sipxService = new SipxFreeswitchService();
        sipxService.setSettings(TestHelper.loadSettings("freeswitch/freeswitch.xml"));
        LocationSpecificService service = new LocationSpecificService(sipxService);
        service.setLocation(location);

        Bridge bridge = new Bridge();
        bridge.setService(service);

        bridge.setModelFilesContext(TestHelper.getModelFilesContext());
        assertTrue(bridge.getConferences().isEmpty());

        // Test the defaults
        assertEquals(null, bridge.getHost());
        assertEquals(15060, bridge.getFreeswitchService().getFreeswitchSipPort());
        assertEquals(null, bridge.getDescription());
        assertEquals(null, bridge.getName());
        assertEquals("http://null:8080/RPC2", bridge.getServiceUri());

        // Test setting explicit values
        location.setFqdn("bridge");
        location.setName("Example Bridge");

        assertEquals("bridge", bridge.getHost());
        assertEquals("Example Bridge", bridge.getDescription());
    }

    public void testGetAudioDirecotry() {
        Bridge bridge = new Bridge();
        LocalizationContext localContex = createMock(LocalizationContext.class);
        expect(localContex.getCurrentLanguage()).andReturn("fr");
        expect(localContex.getCurrentLanguageDir()).andReturn("stdprompts_fr");
        replay(localContex);
        String sysdirDoc = TestHelper.getSysDirProperties().getProperty("sysdir.doc");
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
        String sysdirDoc = TestHelper.getSysDirProperties().getProperty("sysdir.doc");
        bridge.setLocalizationContext(localContex);
        bridge.setPromptsDir(sysdirDoc);
        String path = sysdirDoc+System.getProperty("file.separator")+"stdprompts";
        new File(path+System.getProperty("file.separator")+"conf").mkdirs();
        bridge.setAudioDirectory(path);
        assertEquals(path, bridge.getAudioDirectory());
    }
}

