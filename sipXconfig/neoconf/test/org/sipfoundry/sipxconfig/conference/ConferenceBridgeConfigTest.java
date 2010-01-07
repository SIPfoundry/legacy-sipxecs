/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.conference;

import java.util.Arrays;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxIvrService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

public class ConferenceBridgeConfigTest extends TestCase {
    public void testGenerate() throws Exception {
        Location location1 = new Location();
        location1.setUniqueId();
        location1.setFqdn("test.example.com");

        SipxFreeswitchService sipxService = new SipxFreeswitchService();
        sipxService.setSettings(TestHelper.loadSettings("freeswitch/freeswitch.xml"));
        LocationSpecificService service = new LocationSpecificService(sipxService);
        service.setLocation(location1);

        Bridge bridge = new Bridge();
        bridge.setService(service);
        bridge.setModelFilesContext(TestHelper.getModelFilesContext());
        bridge.getSettings();

        User user1 = new User();
        user1.setUserName("221");

        Conference conf1 = new Conference();
        conf1.setModelFilesContext(TestHelper.getModelFilesContext());
        conf1.initialize();
        conf1.getSettings();
        conf1.setName("Conf1");
        conf1.setExtension("123");
        conf1.setOwner(user1);
        conf1.setSettingValue(Conference.MAX_LEGS, "0");
        conf1.setSettingValue(Conference.AUTO_RECORDING, "1");
        conf1.setUniqueId();
        bridge.addConference(conf1);

        Conference conf2 = new Conference();
        conf2.setModelFilesContext(TestHelper.getModelFilesContext());
        conf2.initialize();
        conf2.getSettings();
        conf2.setName("Conf2");
        conf2.setExtension("234");
        conf2.setSettingValue(Conference.MAX_LEGS, "4");
        conf2.setUniqueId();
        bridge.addConference(conf2);

        Domain domain = new Domain("example.com");
        DomainManager domainManager = createMock(DomainManager.class);
        domainManager.getDomain();
        expectLastCall().andReturn(domain);
        replay(domainManager);

        SipxIvrService ivrService = new SipxIvrService();
        ivrService.setModelFilesContext(TestHelper.getModelFilesContext());
        ivrService.setModelDir("sipxivr");
        ivrService.setModelName("sipxivr.xml");
        ivrService.setBeanName(SipxIvrService.BEAN_ID);
        ivrService.setSettingValue("ivr/httpsPort", "8085");

        LocationsManager locationsManager = createMock(LocationsManager.class);
        locationsManager.getLocationsForService(ivrService);
        expectLastCall().andReturn(Arrays.asList(location1));
        replay(locationsManager);

        ivrService.setLocationsManager(locationsManager);

        SipxServiceManager serviceManager = TestUtil.getMockSipxServiceManager(true, ivrService);

        ConferenceBridgeContext confContext = createMock(ConferenceBridgeContext.class);
        confContext.getBridgeByServer("test.example.com");
        expectLastCall().andReturn(bridge);
        replay(confContext);

        ConferenceBridgeConfig config = new ConferenceBridgeConfig();
        config.setDomainManager(domainManager);
        config.setConferenceBridgeContext(confContext);
        config.setSipxServiceManager(serviceManager);

        String generatedXml = AbstractConfigurationFile.getFileContent(config, location1);
        String referenceXml = IOUtils.toString(getClass().getResourceAsStream("conference_bridge_config.test.xml"));
        assertEquals(referenceXml, generatedXml);
    }
}
