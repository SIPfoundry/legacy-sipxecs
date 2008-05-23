/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.InputStream;
import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.SipxServer.ConflictingFeatureCodeException;
import org.sipfoundry.sipxconfig.admin.commserver.SipxServer.ConflictingFeatureCodeValidator;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxServerTest extends TestCase {

    private SipxServer m_server;

    protected void setUp() throws Exception {
        m_server = setUpSipxServer();
        InputStream configDefs = SipxServerTest.class.getResourceAsStream("config.defs");
        TestHelper
                .copyStreamToDirectory(configDefs, TestHelper.getTestDirectory(), "config.defs");
        InputStream sipxpresence = SipxServerTest.class
                .getResourceAsStream("sipxpresence-config.test.in");
        TestHelper.copyStreamToDirectory(sipxpresence, TestHelper.getTestDirectory(),
                "sipxpresence-config.in");
        InputStream registrar = SipxServerTest.class
                .getResourceAsStream("registrar-config.test.in");
        TestHelper.copyStreamToDirectory(registrar, TestHelper.getTestDirectory(),
                "registrar-config");
        // we read server location from sipxpresence-config
        sipxpresence = SipxServerTest.class.getResourceAsStream("sipxpresence-config.test.in");
        TestHelper.copyStreamToDirectory(sipxpresence, TestHelper.getTestDirectory(),
                "sipxpresence-config");
        registrar = SipxServerTest.class.getResourceAsStream("registrar-config.test.in");
        TestHelper.copyStreamToDirectory(registrar, TestHelper.getTestDirectory(),
                "registrar-config.in");
    }
    
    public void testGetSetting() {
        Setting settings = m_server.getSettings();
        assertNotNull(settings);
    }

    public void testConflictingFeatureCodes() {
        Setting settings = m_server.getSettings().copy();        
        ConflictingFeatureCodeValidator validator = new ConflictingFeatureCodeValidator();
        validator.validate(settings);
        
        settings = m_server.getSettings().copy();        
        settings.getSetting("call-pick-up/SIP_REDIRECT.180-PICKUP.GLOBAL_CALL_PICKUP_CODE").setValue("*7");
        try {
            validator.validate(settings);
            fail();
        } catch (ConflictingFeatureCodeException expected) {
            assertTrue(true);
        }
        
        settings = m_server.getSettings().copy();        
        settings.getSetting("enum/SIP_REDIRECT.200-ENUM.ADD_PREFIX").setValue("*7");
        try {
            validator.validate(settings);
            fail();
        } catch (ConflictingFeatureCodeException expected) {
            assertTrue(true);
        }
    }

    public void testGetAliasMappings() {
        IMocksControl coreContextCtrl = EasyMock.createControl();
        CoreContext coreContext = coreContextCtrl.createMock(CoreContext.class);
        coreContext.getDomainName();
        coreContextCtrl.andReturn("domain.com").atLeastOnce();
        coreContextCtrl.replay();

        m_server.setCoreContext(coreContext);
        assertNotNull(m_server.getPresenceServerUri());

        Collection aliasMappings = m_server.getAliasMappings();

        assertEquals(2, aliasMappings.size());
        for (Iterator i = aliasMappings.iterator(); i.hasNext();) {
            AliasMapping am = (AliasMapping) i.next();
            assertTrue(am.getIdentity().matches("\\*8\\d@domain.com"));
            assertTrue(am.getContact().matches("sip:\\*8\\d@presence.server.com:\\d+"));
        }

        coreContextCtrl.verify();
    }

    public void testGetPresenceServerUri() {
        assertEquals("sip:presence.server.com:5130", m_server.getPresenceServerUri());
    }

    public void testGetMusicOnHoldUri() {
        m_server.setMohUser("~~mh~");
        assertEquals("sip:~~mh~@example.org", m_server.getMusicOnHoldUri("example.org"));
    }

    public void testSetRegistrarDomainAliases() {
        String[] aliases = {
            "example.com", "example.org"
        };
        m_server.setRegistrarDomainAliases(Arrays.asList(aliases));
        assertEquals("${MY_FULL_HOSTNAME} ${MY_IP_ADDR} example.com example.org", m_server
                .getSettingValue("domain/SIP_REGISTRAR_DOMAIN_ALIASES"));
        m_server.setRegistrarDomainAliases(null);
        assertEquals("${MY_FULL_HOSTNAME} ${MY_IP_ADDR}", m_server
                .getSettingValue("domain/SIP_REGISTRAR_DOMAIN_ALIASES"));
    }

    public static SipxServer setUpSipxServer() {
        SipxServer server = new SipxServer();
        server.setConfigDirectory(TestHelper.getTestDirectory());

        server.setModelFilesContext(TestHelper.getModelFilesContext());
        server.setMohUser("~~mh~");
        return server;
    }

    public void testGetPagingLogLevel() {
        assertEquals("NOTICE", m_server.getPagingLogLevel());
    }
}
