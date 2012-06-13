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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class ConferenceBridgeConfigTest extends TestCase {
    public void testGenerate() throws Exception {                
        Location location1 = new Location();
        location1.setUniqueId();
        location1.setFqdn("test.example.com");

        Bridge bridge = new Bridge();
        bridge.setLocation(location1);
        bridge.setModelFilesContext(TestHelper.getModelFilesContext());
        bridge.getSettings();

        User user1 = new User();
        user1.setUserName("221");

        Domain domain = new Domain("example.com");
        DomainManager domainManager = createMock(DomainManager.class);
        domainManager.getDomain();
        expectLastCall().andReturn(domain);
        replay(domainManager);
        
        AddressManager addressManager = createMock(AddressManager.class);
        addressManager.getSingleAddress(Ivr.REST_API);
        expectLastCall().andReturn(new Address(Ivr.REST_API, "ivr.example.com", 1111)).anyTimes();
        addressManager.getSingleAddress(FreeswitchFeature.SIP_ADDRESS);
        expectLastCall().andReturn(new Address(FreeswitchFeature.SIP_ADDRESS, "fs.example.com", 2222)).anyTimes();        
        replay(addressManager);
        
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
        conf1.setAddressManager(addressManager);
        bridge.addConference(conf1);

        Conference conf2 = new Conference();
        conf2.setModelFilesContext(TestHelper.getModelFilesContext());
        conf2.initialize();
        conf2.getSettings();
        conf2.setName("Conf2");
        conf2.setExtension("234");
        conf2.setSettingValue(Conference.MAX_LEGS, "4");
        conf2.setUniqueId();
        conf2.setAddressManager(addressManager);
        bridge.addConference(conf2);
        
        ConferenceBridgeContext confContext = createMock(ConferenceBridgeContext.class);
        confContext.getBridgeByServer("test.example.com");
        expectLastCall().andReturn(bridge);
        replay(confContext);

        ConferenceBridgeConfig config = new ConferenceBridgeConfig();
        config.setDomainManager(domainManager);
        config.setConferenceBridgeContext(confContext);
        config.setAddressManager(addressManager);

        StringWriter actual = new StringWriter();
        XmlFile f = new XmlFile(actual);
        f.write(config.getDocument(location1));

        String expected = IOUtils.toString(getClass().getResourceAsStream("conference_bridge_config.test.xml"));
        assertEquals(expected.trim(), actual.toString().trim());
    }
}
